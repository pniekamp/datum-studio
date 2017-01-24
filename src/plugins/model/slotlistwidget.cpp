//
// Slots List Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "slotlistwidget.h"
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- SlotListWidget ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SlotListWidget::Constructor ///////////////////////
SlotListWidget::SlotListWidget(QWidget *parent)
  : QListWidget(parent)
{
  m_mesh = -1;

  connect(this, &QListWidget::itemSelectionChanged, this, &SlotListWidget::itemSelectionChanged);
}


///////////////////////// SlotListWidget::edit //////////////////////////////
void SlotListWidget::edit(ModelDocument *document)
{
  m_document = document;

  connect(m_document, &ModelDocument::document_changed, this, &SlotListWidget::refresh);
  connect(m_document, &ModelDocument::dependant_changed, this, &SlotListWidget::refresh);

  refresh();
}


///////////////////////// SlotListWidget::set_mesh //////////////////////////
void SlotListWidget::set_mesh(int index)
{
  if (m_mesh != index)
  {
    m_mesh = index;

    refresh();
  }
}


///////////////////////// SlotListWidget::refresh ///////////////////////////
void SlotListWidget::refresh()
{
  auto currentrow = currentRow();

  clear();

  if (0 <= m_mesh && m_mesh < m_document->meshes())
  {
    auto &mesh = m_document->mesh(m_mesh);

    for(auto &material : mesh.materials)
    {
      QListWidgetItem *item = new QListWidgetItem(material.name, this);

      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled);

      item->setIcon(material.document ? material.document->icon() : QIcon(":/modelplugin/blank.png"));
    }
  }

  setCurrentRow(currentrow);
}


///////////////////////// SlotListWidget::itemSelectionChanged //////////////
void SlotListWidget::itemSelectionChanged()
{
  emit selection_changed(currentRow());
}


///////////////////////// SlotListWidget::dragEnterEvent ////////////////////
QStringList SlotListWidget::mimeTypes() const
{
  return { "text/uri-list" };
}


///////////////////////// SlotListWidget::supportedDropActions //////////////
Qt::DropActions SlotListWidget::supportedDropActions() const
{
  return Qt::MoveAction | Qt::CopyAction;
}


////////////////////// SlotListWidget::dragEnterEvent ///////////////////////
void SlotListWidget::dragEnterEvent(QDragEnterEvent *event)
{
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


///////////////////////// SlotListWidget::dragMoveEvent /////////////////////
void SlotListWidget::dragMoveEvent(QDragMoveEvent *event)
{
  if (!indexAt(event->pos()).isValid())
  {
    event->ignore();
    return;
  }

  QListWidget::dragMoveEvent(event);
}


///////////////////////// SlotListWidget::dropEvent /////////////////////////
void SlotListWidget::dropEvent(QDropEvent *event)
{
  int slot = indexAt(event->pos()).row();

  for(auto &url : event->mimeData()->urls())
  {
    QString src = url.toLocalFile();

    m_document->set_mesh_material(m_mesh, slot, src);

    setCurrentRow(slot);
  }

  event->setDropAction(Qt::IgnoreAction);
  event->accept();

  QListWidget::dropEvent(event);
}

