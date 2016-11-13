//
// Mesh Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "meshwidget.h"
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QLineEdit>
#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- MeshWidget ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MeshWidget::Constructor ///////////////////////////
MeshWidget::MeshWidget(QWidget *parent)
  : QListWidget(parent)
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  m_renameaction = new QAction("Rename", this);
  m_renameaction->setShortcut(QKeySequence(Qt::Key_F2));
  m_renameaction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  m_deleteaction = new QAction("Delete", this);
  m_deleteaction->setShortcut(QKeySequence(Qt::Key_Delete));
  m_deleteaction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  addAction(m_renameaction);
  addAction(m_deleteaction);

  connect(m_renameaction, &QAction::triggered, this, &MeshWidget::on_Rename_triggered);
  connect(m_deleteaction, &QAction::triggered, this, &MeshWidget::on_Delete_triggered);
  connect(this, &QListWidget::itemSelectionChanged, this, &MeshWidget::itemSelectionChanged);
  connect(this, &QListWidget::customContextMenuRequested, this, &MeshWidget::on_contextmenu_requested);
}


///////////////////////// MeshWidget::edit //////////////////////////////////
void MeshWidget::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &ModelDocument::document_changed, this, &MeshWidget::refresh);
  connect(&m_document, &ModelDocument::dependant_changed, this, &MeshWidget::refresh);

  refresh();
}


///////////////////////// MeshWidget::refresh ///////////////////////////////
void MeshWidget::refresh()
{
  auto currentrow = currentRow();

  clear();

  for(int i = 0; i < m_document.meshes(); ++i)
  {
    QListWidgetItem *item = new QListWidgetItem(m_document.mesh(i).name, this);

    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);

    item->setIcon(m_document.mesh(i).document ? m_document.mesh(i).document->icon() : QIcon());
  }

  setCurrentRow(currentrow);
}


///////////////////////// MeshWidget::itemSelectionChanged ////////////////////////////////
void MeshWidget::itemSelectionChanged()
{
  emit selection_changed(currentRow());
}


///////////////////////// MeshWidget::renamed ///////////////////////////////
void MeshWidget::commitData(QWidget *editor)
{
  QString text = qobject_cast<QLineEdit*>(editor)->text();

  if (text != currentItem()->text())
  {
    closePersistentEditor(currentItem());

    m_document.set_mesh_name(currentRow(), text);
  }
}


///////////////////////// MeshWidget::dragEnterEvent ////////////////////////
QStringList MeshWidget::mimeTypes() const
{
  return { "datumstudio/modelmeshmodelitem", "text/uri-list" };
}


///////////////////////// MeshWidget::mimeData //////////////////////////////
QMimeData *MeshWidget::mimeData(QList<QListWidgetItem *> const items) const
{
  QMimeData *mimedata = new QMimeData();

  QByteArray encoded;
  QDataStream stream(&encoded, QIODevice::WriteOnly);

  for(auto &item : items)
  {
    stream << indexFromItem(item).row();
  }

  mimedata->setData("datumstudio/modelmeshmodelitem", encoded);

  return mimedata;
}


///////////////////////// MeshWidget::supportedDropActions //////////////////
Qt::DropActions MeshWidget::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}


////////////////////// MeshWidget::dragEnterEvent ///////////////////////////
void MeshWidget::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("datumstudio/modelmeshmodelitem") && event->source() == this)
  {
    event->accept();
  }

  if (event->mimeData()->urls().size() == 1 && event->source())
  {
    auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

    if (auto document = documentmanager->open(event->mimeData()->urls().at(0).toLocalFile()))
    {
      if (document->type() == "Mesh")
      {
        event->accept();
      }

      if (document->type() == "Model")
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


///////////////////////// MeshWidget::dropEvent /////////////////////////////
void MeshWidget::dropEvent(QDropEvent *event)
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

  if (event->mimeData()->hasFormat("datumstudio/modelmeshmodelitem"))
  {
    QByteArray encoded = event->mimeData()->data("datumstudio/modelmeshmodelitem");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
      int index;

      stream >> index;

      m_document.move_mesh(index, position);
    }
  }

  if (event->mimeData()->hasUrls())
  {
    for(auto &url : event->mimeData()->urls())
    {
      QString src = url.toLocalFile();

      m_document.add_mesh(position, src);
    }
  }

  event->setDropAction(Qt::IgnoreAction);
  event->accept();

  QListWidget::dropEvent(event);
}


///////////////////////// MeshWidget::contextmenu ///////////////////////////
void MeshWidget::on_contextmenu_requested(QPoint pos)
{
  if (currentRow() != -1)
  {
    QMenu menu;

    menu.addAction(m_renameaction);
    menu.addAction(m_deleteaction);

    menu.exec(QCursor::pos());
  }
}


///////////////////////// MeshWidget::Rename ////////////////////////////////
void MeshWidget::on_Rename_triggered()
{
  setFocus();

  editItem(currentItem());
}


///////////////////////// MeshWidget::Delete ////////////////////////////////
void MeshWidget::on_Delete_triggered()
{
  if (currentRow() != -1)
  {
    if (QMessageBox::question(this, "Remove Mesh", "Remove Selected Mesh\n\nSure ?", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
    {
      m_document.erase_mesh(currentRow());
    }
  }
}
