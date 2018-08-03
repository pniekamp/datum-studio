//
// Mesh List Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "meshlistwidget.h"
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QLineEdit>
#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- MeshListWidget ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MeshListWidget::Constructor ///////////////////////
MeshListWidget::MeshListWidget(QWidget *parent)
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

  connect(m_renameaction, &QAction::triggered, this, &MeshListWidget::on_Rename_triggered);
  connect(m_deleteaction, &QAction::triggered, this, &MeshListWidget::on_Delete_triggered);
  connect(this, &QListWidget::itemSelectionChanged, this, &MeshListWidget::itemSelectionChanged);
  connect(this, &QListWidget::customContextMenuRequested, this, &MeshListWidget::on_contextmenu_requested);
}


///////////////////////// MeshListWidget::edit //////////////////////////////
void MeshListWidget::edit(ModelDocument *document)
{
  m_document = document;

  connect(m_document, &ModelDocument::document_changed, this, &MeshListWidget::refresh);
  connect(m_document, &ModelDocument::dependant_changed, this, &MeshListWidget::refresh);

  refresh();
}


///////////////////////// MeshListWidget::set_selection /////////////////////
void MeshListWidget::set_selection(int index)
{
  if (currentRow() != index)
  {
    setCurrentRow(index);
  }
}


///////////////////////// MeshListWidget::refresh ///////////////////////////
void MeshListWidget::refresh()
{
  auto currentrow = currentRow();

  clear();

  for(int i = 0; i < m_document->meshes(); ++i)
  {
    auto &mesh = m_document->mesh(i);

    QListWidgetItem *item = new QListWidgetItem(mesh.name, this);

    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);

    item->setIcon(mesh.document ? mesh.document->icon() : QIcon());
  }

  setCurrentRow(currentrow);
}


///////////////////////// MeshListWidget::itemSelectionChanged //////////////
void MeshListWidget::itemSelectionChanged()
{ 
  emit selection_changed(currentRow());
}


///////////////////////// MeshListWidget::renamed ///////////////////////////
void MeshListWidget::commitData(QWidget *editor)
{
  if (currentItem())
  {
    QString text = qobject_cast<QLineEdit*>(editor)->text();

    if (text != currentItem()->text())
    {
      closePersistentEditor(currentItem());

      m_document->set_mesh_name(currentRow(), text);
    }
  }
}


///////////////////////// MeshListWidget::dragEnterEvent ////////////////////
QStringList MeshListWidget::mimeTypes() const
{
  return { "datumstudio/modelmeshmodelitem", "text/uri-list" };
}


///////////////////////// MeshListWidget::mimeData //////////////////////////
QMimeData *MeshListWidget::mimeData(QList<QListWidgetItem *> const items) const
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


///////////////////////// MeshListWidget::supportedDropActions //////////////
Qt::DropActions MeshListWidget::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}


////////////////////// MeshListWidget::dragEnterEvent ///////////////////////
void MeshListWidget::dragEnterEvent(QDragEnterEvent *event)
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


///////////////////////// MeshListWidget::dropEvent /////////////////////////
void MeshListWidget::dropEvent(QDropEvent *event)
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

  if (event->mimeData()->hasFormat("datumstudio/modelmeshmodelitem"))
  {
    QByteArray encoded = event->mimeData()->data("datumstudio/modelmeshmodelitem");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
      int index;

      stream >> index;

      m_document->move_mesh(index, position);

      position = (position <= index) ? position + 1 : position;
    }
  }

  if (event->mimeData()->hasUrls())
  {
    for(auto &url : event->mimeData()->urls())
    {
      QString src = url.toLocalFile();

      m_document->add_mesh(position, src);

      position += 1;
    }
  }

  event->setDropAction(Qt::IgnoreAction);
  event->accept();

  QListWidget::dropEvent(event);
}


///////////////////////// MeshListWidget::contextmenu ///////////////////////
void MeshListWidget::on_contextmenu_requested(QPoint pos)
{
  if (currentRow() != -1)
  {
    QMenu menu;

    menu.addAction(m_renameaction);
    menu.addAction(m_deleteaction);

    menu.exec(QCursor::pos());
  }
}


///////////////////////// MeshListWidget::Rename ////////////////////////////
void MeshListWidget::on_Rename_triggered()
{
  setFocus();

  editItem(currentItem());
}


///////////////////////// MeshListWidget::Delete ////////////////////////////
void MeshListWidget::on_Delete_triggered()
{
  if (currentRow() != -1)
  {
    if (QMessageBox::question(this, "Remove Mesh", "Remove Selected Mesh\n\nSure ?", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
    {
      m_document->erase_mesh(currentRow());

      emit selection_changed(currentRow());
    }
  }
}
