//
// Layer List Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "layerlistwidget.h"
#include <QDragMoveEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

using namespace std;

//|---------------------- LayerListWidget -----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// LayerListWidget::Constructor //////////////////////
LayerListWidget::LayerListWidget(QWidget *parent)
  : QListWidget(parent)
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  m_deleteaction = new QAction("Delete", this);
  m_deleteaction->setShortcut(QKeySequence(Qt::Key_Delete));
  m_deleteaction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  addAction(m_deleteaction);

  connect(m_deleteaction, &QAction::triggered, this, &LayerListWidget::on_Delete_triggered);
  connect(this, &QListWidget::customContextMenuRequested, this, &LayerListWidget::on_contextmenu_requested);
}


///////////////////////// LayerListWidget::edit /////////////////////////////
void LayerListWidget::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &TerrainMaterialDocument::document_changed, this, &LayerListWidget::refresh);
  connect(&m_document, &TerrainMaterialDocument::dependant_changed, this, &LayerListWidget::refresh);

  refresh();
}


///////////////////////// LayerListWidget::refresh //////////////////////////
void LayerListWidget::refresh()
{
  auto currentrow = currentRow();

  clear();

  for(int i = 0; i < m_document.layers(); ++i)
  {
    auto name = QString("Layer %1").arg(i);

    if (m_document.layer(i))
    {
      name = QFileInfo(Studio::Core::instance()->find_object<Studio::DocumentManager>()->path(m_document.layer(i))).completeBaseName();
    }

    QListWidgetItem *item = new QListWidgetItem(name, this);

    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

    item->setIcon(m_document.layer(i) ? m_document.layer(i)->icon() : QIcon(":/terrainplugin/blank.png"));
  }

  setCurrentRow(currentrow);
}


///////////////////////// LayerListWidget::dragEnterEvent ///////////////////
QStringList LayerListWidget::mimeTypes() const
{
  return { "datumstudio/terrainlayermodelitem", "text/uri-list" };
}


///////////////////////// LayerListWidget::mimeData /////////////////////////
QMimeData *LayerListWidget::mimeData(QList<QListWidgetItem *> const items) const
{
  QMimeData *mimedata = new QMimeData();

  QByteArray encoded;
  QDataStream stream(&encoded, QIODevice::WriteOnly);

  for(auto &item : items)
  {
    stream << indexFromItem(item).row();
  }

  mimedata->setData("datumstudio/terrainlayermodelitem", encoded);

  return mimedata;
}


///////////////////////// LayerListWidget::supportedDropActions /////////////
Qt::DropActions LayerListWidget::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}


////////////////////// LayerListWidget::dragEnterEvent //////////////////////
void LayerListWidget::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("datumstudio/terrainlayermodelitem") && event->source() == this)
  {
    event->accept();
  }

  if (event->mimeData()->urls().size() == 1 && event->source())
  {
    auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

    if (auto document = documentmanager->open(event->mimeData()->urls().at(0).toLocalFile()))
    {
      if (document->type() == "Material")
      {
        event->accept();
      }

      documentmanager->close(document);
    }
  }

  if (!event->isAccepted())
    return;

  QListWidget::dragEnterEvent(event);
}


///////////////////////// LayerListWidget::dropEvent ////////////////////////
void LayerListWidget::dropEvent(QDropEvent *event)
{
  int position = 0;

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
      position = count();
      break;
  }

  if (event->mimeData()->hasFormat("datumstudio/terrainlayermodelitem"))
  {
    QByteArray encoded = event->mimeData()->data("datumstudio/terrainlayermodelitem");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
      int index;

      stream >> index;

      m_document.move_layer(index, position);

      position = (position <= index) ? position + 1 : position;
    }
  }

  if (event->mimeData()->hasUrls())
  {
    for(auto &url : event->mimeData()->urls())
    {
      QString src = url.toLocalFile();

      m_document.add_layer(position, src);

      position += 1;
    }
  }

  event->setDropAction(Qt::IgnoreAction);
  event->accept();

  QListWidget::dropEvent(event);
}


///////////////////////// LayerListWidget::contextmenu //////////////////////
void LayerListWidget::on_contextmenu_requested(QPoint pos)
{
  if (currentRow() != -1)
  {
    QMenu menu;

    menu.addAction(m_deleteaction);

    menu.exec(QCursor::pos());
  }
}


///////////////////////// LayerListWidget::Delete ///////////////////////////
void LayerListWidget::on_Delete_triggered()
{
  if (currentRow() != -1)
  {
    if (QMessageBox::question(this, "Remove Layer", "Remove Selected Layer\n\nSure ?", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
    {
      m_document.erase_layer(currentRow());
    }
  }
}
