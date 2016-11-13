//
// Folder View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "folderview.h"
#include "contentapi.h"
#include <QDir>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QLineEdit>

#include <QtDebug>

using namespace std;

const int PathRole = Qt::UserRole + 1;

namespace
{
  void refresh_folder_contents(QTreeWidgetItem *root, QDir const &path)
  {
    for(auto &entry : path.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
      QTreeWidgetItem *item = nullptr;

      for(int i = 0; i < root->childCount(); ++i)
      {
        if (root->child(i)->text(0) == entry.fileName())
          item = root->child(i);
      }

      if (!item)
      {
        item = new QTreeWidgetItem(root, { entry.fileName() });
      }

      item->setData(0, PathRole, entry.filePath());

      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

      refresh_folder_contents(item, entry.filePath());
    }

    for(int i = root->childCount() - 1; i >= 0; --i)
    {
      if (!QDir(root->child(i)->data(0, PathRole).toString()).exists())
        delete root->child(i);
    }

    root->sortChildren(0, Qt::AscendingOrder);
  }

}

//|---------------------- FolderView ----------------------------------------
//|--------------------------------------------------------------------------
//| Content Folder View
//|

///////////////////////// FolderView::Constructor ///////////////////////////
FolderView::FolderView(QWidget *parent)
  : QTreeWidget(parent)
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, &QTreeWidget::itemSelectionChanged, this, &FolderView::itemSelectionChanged);
}


///////////////////////// FolderView::set_base //////////////////////////////
void FolderView::set_base(QString const &basepath)
{
  m_basepath = basepath;

  clear();

  auto root = new QTreeWidgetItem(this, { "Content" });

  root->setData(0, PathRole, m_basepath);

  root->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled);

  setCurrentItem(root);

  expandItem(root);

  refresh();
}


///////////////////////// FolderView::selected_path /////////////////////////
QString FolderView::selected_path() const
{
  return currentItem()->data(0, PathRole).toString();
}


///////////////////////// FolderView::select_path ///////////////////////////
void FolderView::select_path(QString const &path)
{
  for(auto &item : findItems(QFileInfo(path).fileName(), Qt::MatchExactly | Qt::MatchRecursive))
  {
    if (item->data(0, PathRole).toString() == path)
    {
      setCurrentItem(item);
    }
  }
}


///////////////////////// FolderView::itemSelectionChanged //////////////////
void FolderView::itemSelectionChanged()
{
  emit selection_changed(selected_path());
}


///////////////////////// FolderView::trigger_rename ////////////////////////
void FolderView::trigger_rename(QTreeWidgetItem *item)
{
  setFocus();

  setCurrentItem(item);

  editItem(item);
}


///////////////////////// FolderView::renamed ///////////////////////////////
void FolderView::commitData(QWidget *editor)
{
  QString src = currentItem()->data(0, PathRole).toString();
  QString dst = QFileInfo(src).dir().absolutePath() + "/" + qobject_cast<QLineEdit*>(editor)->text();

  if (src != dst)
  {
    closePersistentEditor(currentItem());

    emit item_renamed(src, dst);

    emit selection_changed(selected_path());
  }
}


///////////////////////// FolderView::mimeTypes /////////////////////////////
QStringList FolderView::mimeTypes() const
{
  return { "text/uri-list" };
}


///////////////////////// FolderView::mimeData //////////////////////////////
QMimeData *FolderView::mimeData(QList<QTreeWidgetItem *> const items) const
{
  QMimeData *mimedata = new QMimeData();

  QList<QUrl> urls;

  for(auto &item : items)
  {
    urls.append(QUrl::fromLocalFile(item->data(0, PathRole).toString()));
  }

  mimedata->setUrls(urls);

  return mimedata;
}


///////////////////////// FolderView::supportedDropActions //////////////////
Qt::DropActions FolderView::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}


///////////////////////// FolderView::dropEvent //////////////////////////
void FolderView::dropEvent(QDropEvent *event)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  QTreeWidgetItem *item = itemAt(event->pos());

  QString dst = item ? item->data(0, PathRole).toString() : m_basepath;

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

  event->setDropAction(Qt::IgnoreAction);
  event->accept();

  QTreeWidget::dropEvent(event);
}


///////////////////////// FolderView::update ////////////////////////////////
void FolderView::update(QString const &path)
{
  QFileInfo pathinfo(path);

  if (!pathinfo.isFile())
  {
    refresh();
  }
}


///////////////////////// FolderView::refresh ///////////////////////////////
void FolderView::refresh()
{
  if (topLevelItemCount() == 0)
    return;

  refresh_folder_contents(topLevelItem(0), m_basepath);
}
