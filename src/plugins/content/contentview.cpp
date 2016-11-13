//
// Content View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "contentview.h"
#include "contentapi.h"
#include "documentapi.h"
#include <QDir>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QLineEdit>
#include <QTimer>

#include <QtDebug>

using namespace std;

const int PathRole = Qt::UserRole + 1;

//|---------------------- ContentView ---------------------------------------
//|--------------------------------------------------------------------------
//| Content View
//|

///////////////////////// ContentView::Constructor //////////////////////////
ContentView::ContentView(QWidget *parent)
  : QListWidget(parent)
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, &QListWidget::itemDoubleClicked, this, &ContentView::itemDoubleClicked);
}


///////////////////////// ContentView::set_path /////////////////////////////
void ContentView::set_path(QString const &path)
{
  m_path = QFileInfo(path).absoluteFilePath();

  refresh();
}


///////////////////////// ContentView::selected_paths ///////////////////////
QStringList ContentView::selected_paths() const
{
  QStringList paths;

  for(auto &item : selectedItems())
  {
    paths << item->data(PathRole).toString();
  }

  return paths;
}


///////////////////////// ContentView::select_path //////////////////////////
void ContentView::select_path(QString const &path)
{
  for(auto &item : findItems(QFileInfo(path).completeBaseName(), Qt::MatchExactly | Qt::MatchRecursive))
  {
    if (item->data(PathRole).toString() == path)
    {
      setCurrentItem(item);
    }
  }
}


///////////////////////// ContentView::trigger_rename ///////////////////////
void ContentView::trigger_rename(QListWidgetItem *item)
{
  setFocus();

  setCurrentItem(item);

  editItem(item);
}


///////////////////////// ContentView::renamed //////////////////////////////
void ContentView::commitData(QWidget *editor)
{
  QString src = currentItem()->data(PathRole).toString();
  QString dst = m_path + "/" + qobject_cast<QLineEdit*>(editor)->text();

  if (QFileInfo(src).suffix() != "")
    dst = dst + "." + QFileInfo(src).suffix();

  if (src != dst)
  {
    closePersistentEditor(currentItem());

    emit item_renamed(src, dst);
  }
}


///////////////////////// ContentView::triggered ////////////////////////////
void ContentView::itemDoubleClicked(QListWidgetItem *item)
{
  emit item_triggered(item->data(PathRole).toString());
}


///////////////////////// ContentView::dragEnterEvent ///////////////////////
QStringList ContentView::mimeTypes() const
{
  return { "text/uri-list" };
}


///////////////////////// ContentView::mimeData /////////////////////////////
QMimeData *ContentView::mimeData(QList<QListWidgetItem *> const items) const
{
  QMimeData *mimedata = new QMimeData();

  QList<QUrl> urls;

  for(auto &item : items)
  {
    urls.append(QUrl::fromLocalFile(item->data(PathRole).toString()));
  }

  mimedata->setUrls(urls);

  return mimedata;
}


///////////////////////// ContentView::supportedDropActions /////////////////
Qt::DropActions ContentView::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}


///////////////////////// ContentView::dropEvent ////////////////////////////
void ContentView::dropEvent(QDropEvent *event)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  QListWidgetItem *item = itemAt(event->pos());

  QString dst = item ? item->data(PathRole).toString() : m_path;

  for(auto &url : event->mimeData()->urls())
  {
    QString src = url.toLocalFile();

    if (event->source() && event->dropAction() == Qt::MoveAction)
    {
      contentmanager->rename_content(src, QDir(dst).filePath(QFileInfo(src).fileName()));
    }

    if (event->source() == nullptr || event->dropAction() == Qt::CopyAction)
    {
      if (contentmanager->import(src, dst))
      {
        if (event->dropAction() == Qt::MoveAction)
        {
          contentmanager->delete_content(src);
        }
      }
    }
  }

  refresh();

  event->setDropAction(Qt::IgnoreAction);
  event->accept();

  QListWidget::dropEvent(event);
}


///////////////////////// ContentView::update ///////////////////////////////
void ContentView::update(QString const &path)
{
  QFileInfo pathinfo(path);

  if (pathinfo.absoluteDir() == m_path)
  {
    bool found = false;

    auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

    for(auto &item : findItems(pathinfo.completeBaseName(), Qt::MatchExactly | Qt::MatchRecursive))
    {
      if (item->data(PathRole).toString() == path)
      {
        if (pathinfo.isFile())
        {
          if (auto document = documentmanager->open(path))
          {
            item->setIcon(document->icon());

            documentmanager->close(document);
          }
        }

        if (pathinfo.exists())
        {
          found = true;
        }
      }
    }

    if (!found)
    {
      refresh();
    }
  }
}


///////////////////////// ContentView::refresh //////////////////////////////
void ContentView::refresh()
{
  if (m_path.isEmpty())
    return;

  clear();

  blockSignals(true);

  for(auto &entry : QDir(m_path).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name))
  {
    QListWidgetItem *item = new QListWidgetItem(entry.completeBaseName(), this);

    item->setData(Qt::ToolTipRole, entry.completeBaseName());

    item->setData(PathRole, entry.filePath());

    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);

    item->setSizeHint(QSize(96, 68));

    if (entry.isDir())
    {
      item->setIcon(QIcon(":/contentplugin/folder.png"));
      item->setFlags(item->flags() | Qt::ItemIsDropEnabled);
    }
    else
    {
      item->setIcon(QIcon(":/contentplugin/icon.png"));

      QTimer::singleShot(50, this, [=]() { update(entry.filePath()); });
    }
  }

  blockSignals(false);
}
