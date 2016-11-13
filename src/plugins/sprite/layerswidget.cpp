//
// Layers Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "layerswidget.h"
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QDebug>

using namespace std;

//|---------------------- LayersWidget --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// LayersWidget::Constructor /////////////////////////
LayersWidget::LayersWidget(QWidget *parent)
  : QListWidget(parent)
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  m_deleteaction = new QAction("Delete", this);
  m_deleteaction->setShortcut(QKeySequence(Qt::Key_Delete));
  m_deleteaction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  addAction(m_deleteaction);

  connect(m_deleteaction, &QAction::triggered, this, &LayersWidget::on_Delete_triggered);
  connect(this, &QListWidget::customContextMenuRequested, this, &LayersWidget::on_contextmenu_requested);
}


///////////////////////// LayersWidget::edit ////////////////////////////////
void LayersWidget::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &SpriteSheetDocument::document_changed, this, &LayersWidget::refresh);
  connect(&m_document, &SpriteSheetDocument::dependant_changed, this, &LayersWidget::refresh);

  refresh();
}


///////////////////////// LayersWidget::refresh /////////////////////////////
void LayersWidget::refresh()
{
  auto currentrow = currentRow();

  clear();

  for(int i = 0; i < m_document.layers(); ++i)
  {
    QListWidgetItem *item = new QListWidgetItem(QString("Layer %1").arg(i), this);

    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

    item->setIcon(m_document.layer(i) ? m_document.layer(i)->icon() : QIcon(":/spriteplugin/blank.png"));
  }

  setCurrentRow(currentrow);
}


///////////////////////// LayersWidget::dragEnterEvent //////////////////////
QStringList LayersWidget::mimeTypes() const
{
  return { "datumstudio/spritelayermodelitem", "text/uri-list" };
}


///////////////////////// LayersWidget::mimeData ////////////////////////////
QMimeData *LayersWidget::mimeData(QList<QListWidgetItem *> const items) const
{
  QMimeData *mimedata = new QMimeData();

  QByteArray encoded;
  QDataStream stream(&encoded, QIODevice::WriteOnly);

  for(auto &item : items)
  {
    stream << indexFromItem(item).row();
  }

  mimedata->setData("datumstudio/spritelayermodelitem", encoded);

  return mimedata;
}


///////////////////////// LayersWidget::supportedDropActions ////////////////
Qt::DropActions LayersWidget::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}


////////////////////// LayersWidget::dragEnterEvent /////////////////////////
void LayersWidget::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("datumstudio/spritelayermodelitem") && event->source() == this)
  {
    event->accept();
  }

  if (event->mimeData()->urls().size() == 1 && event->source())
  {
    auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

    if (auto document = documentmanager->open(event->mimeData()->urls().at(0).toLocalFile()))
    {
      if (document->type() == "Image")
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


///////////////////////// LayersWidget::dropEvent ///////////////////////////
void LayersWidget::dropEvent(QDropEvent *event)
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
      position = count();
      break;
  }

  if (event->mimeData()->hasFormat("datumstudio/spritelayermodelitem"))
  {
    QByteArray encoded = event->mimeData()->data("datumstudio/spritelayermodelitem");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
      int index;

      stream >> index;

      m_document.move_layer(index, position);
    }
  }

  if (event->mimeData()->hasUrls())
  {
    for(auto &url : event->mimeData()->urls())
    {
      QString src = url.toLocalFile();

      m_document.add_layer(position, src);
    }
  }

  event->setDropAction(Qt::IgnoreAction);
  event->accept();

  QListWidget::dropEvent(event);
}


///////////////////////// LayersWidget::contextmenu /////////////////////////
void LayersWidget::on_contextmenu_requested(QPoint pos)
{
  if (currentRow() != -1)
  {
    QMenu menu;

    menu.addAction(m_deleteaction);

    menu.exec(QCursor::pos());
  }
}


///////////////////////// LayersWidget::Delete //////////////////////////////
void LayersWidget::on_Delete_triggered()
{
  if (currentRow() != -1)
  {
    if (QMessageBox::question(this, "Remove Layer", "Remove Selected Layer\n\nSure ?", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
    {
      m_document.erase_layer(currentRow());
    }
  }
}
