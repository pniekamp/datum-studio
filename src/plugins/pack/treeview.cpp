//
// Tree View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "treeview.h"
#include <QDragMoveEvent>
#include <QMimeData>
#include <QFileInfo>
#include <cassert>

#include <QtDebug>

using namespace std;

//|---------------------- TreeView ------------------------------------------
//|--------------------------------------------------------------------------
//| Tree View
//|

///////////////////////// TreeView::Constructor /////////////////////////////
TreeView::TreeView(QWidget *parent)
  : QTreeWidget(parent)
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, &QTreeWidget::itemSelectionChanged, this, &TreeView::on_selection_changed);
  connect(model(), &QAbstractItemModel::rowsRemoved, this, &TreeView::on_selection_changed);

  connect(this, &QTreeWidget::itemDoubleClicked, this, &TreeView::on_item_triggered);
}


///////////////////////// TreeView::set_model ///////////////////////////////
void TreeView::set_model(PackModel *model)
{
  m_model = model;

  connect(m_model, &PackModel::reset, this, &TreeView::on_model_reset);
  connect(m_model, &PackModel::added, this, &TreeView::on_model_added);
  connect(m_model, &PackModel::changed, this, &TreeView::on_model_changed);
  connect(m_model, &PackModel::removed, this, &TreeView::on_model_removed);

  on_model_reset();
}


///////////////////////// TreeView::on_model_reset //////////////////////////
void TreeView::on_model_reset()
{
  clear();
}


///////////////////////// TreeView::on_model_added //////////////////////////
void TreeView::on_model_added(size_t index, PackModel::Asset *asset)
{
  auto item = new QTreeWidgetItem;

  insertTopLevelItem(index, item);

  on_model_changed(index, asset);
}


///////////////////////// TreeView::on_model_changed /////////////////////////
void TreeView::on_model_changed(size_t index, PackModel::Asset *asset)
{
  blockSignals(true);

  auto item = topLevelItem(index);

  item->setText(0, QFileInfo(asset->path()).completeBaseName());

  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

  if (asset->document())
    item->setIcon(0, QIcon(":/packplugin/asset.png"));
  else
    item->setIcon(0, QIcon(":/packplugin/error.png"));

  blockSignals(false);
}


///////////////////////// TreeView::on_model_removed ////////////////////////
void TreeView::on_model_removed(size_t index, PackModel::Asset *asset)
{
  delete topLevelItem(index);
}


///////////////////////// TreeView::on_selection_changed ////////////////////
void TreeView::on_selection_changed()
{
  emit selection_changed((selectedItems().size() != 0) ? (m_model->begin() + currentIndex().row())->get() : nullptr);
}


///////////////////////// TreeView::on_item_triggered ///////////////////////
void TreeView::on_item_triggered(QTreeWidgetItem *item, int column)
{
  emit item_triggered((m_model->begin() + currentIndex().row())->get());
}


///////////////////////// TreeView::mimeTypes ///////////////////////////////
QStringList TreeView::mimeTypes() const
{
  return { "datumstudio/packmodelitem", "text/uri-list" };
}


///////////////////////// TreeView::mimeData ////////////////////////////////
QMimeData *TreeView::mimeData(QList<QTreeWidgetItem *> const items) const
{
  QMimeData *mimedata = new QMimeData();

  QByteArray encoded;
  QDataStream stream(&encoded, QIODevice::WriteOnly);

  foreach(QTreeWidgetItem *item, items)
  {
    stream << indexOfTopLevelItem(item);
  }

  mimedata->setData("datumstudio/packmodelitem", encoded);

  return mimedata;
}


///////////////////////// TreeView::supportedDropActions ////////////////////
Qt::DropActions TreeView::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}


///////////////////////// TreeView::dragEnterEvent //////////////////////////
void TreeView::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->source() == nullptr)
    return;

  QTreeWidget::dragEnterEvent(event);
}


///////////////////////// TreeView::dropEvent ///////////////////////////////
void TreeView::dropEvent(QDropEvent *event)
{
  size_t position = 0;

  switch(dropIndicatorPosition())
  {
    case QAbstractItemView::OnItem:
      break;

    case QAbstractItemView::AboveItem:
      position = indexAt(event->pos()).row();
      break;

    case QAbstractItemView::BelowItem:
      position = indexAt(event->pos()).row() + 1;
      break;

    case QAbstractItemView::OnViewport:
      position = topLevelItemCount();
      break;
  }

  if (event->mimeData()->hasFormat("datumstudio/packmodelitem"))
  {
    QByteArray encoded = event->mimeData()->data("datumstudio/packmodelitem");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
      int index;

      stream >> index;

      m_model->move_asset(index, position);
    }
  }

  if (event->mimeData()->hasUrls())
  {
    foreach(QUrl url, event->mimeData()->urls())
    {
      QString src = url.toLocalFile();

      m_model->add_asset(position, src);
    }
  }

  event->setDropAction(Qt::IgnoreAction);
  event->accept();

  QTreeWidget::dropEvent(event);
}
