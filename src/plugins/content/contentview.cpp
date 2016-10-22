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

  connect(this, &QListWidget::itemChanged, this, &ContentView::on_item_changed);
  connect(this, &QListWidget::itemDoubleClicked, this, &ContentView::on_item_triggered);
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

  foreach(QListWidgetItem *item, selectedItems())
  {
    paths << item->data(PathRole).toString();
  }

  return paths;
}


///////////////////////// ContentView::trigger_rename ///////////////////////
void ContentView::trigger_rename(QString const &path)
{
  setFocus();

  for(int i = 0; i < count(); ++i)
  {
    if (item(i)->data(PathRole).toString() == path)
    {
      editItem(item(i));
    }
  }
}


///////////////////////// ContentView::item_changed /////////////////////////
void ContentView::on_item_changed(QListWidgetItem *item)
{
  if (item->text() == "")
  {
    item->setText(QFileInfo(item->data(PathRole).toString()).completeBaseName());
  }

  if (item->data(Qt::DisplayRole).isValid())
  {
    QString src = item->data(PathRole).toString();
    QString dst = m_path + "/" + item->text();

    if (QFileInfo(src).suffix() != "")
      dst = dst + "." + QFileInfo(src).suffix();

    if (src != dst)
    {
      emit item_renamed(src, dst);
    }
  }
}


///////////////////////// ContentView::item_triggered ///////////////////////
void ContentView::on_item_triggered(QListWidgetItem *item)
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

  foreach(QListWidgetItem *item, items)
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

  foreach(QUrl url, event->mimeData()->urls())
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
    auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

    bool found = false;

    foreach(QListWidgetItem *item, findItems(pathinfo.completeBaseName(), Qt::MatchExactly | Qt::MatchRecursive))
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
