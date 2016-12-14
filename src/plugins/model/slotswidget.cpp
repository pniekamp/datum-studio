//
// Slots Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "slotswidget.h"
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- SlotsWidget ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SlotsWidget::Constructor //////////////////////////
SlotsWidget::SlotsWidget(QWidget *parent)
  : QListWidget(parent)
{
  m_mesh = -1;

  connect(this, &QListWidget::itemSelectionChanged, this, &SlotsWidget::itemSelectionChanged);
}


///////////////////////// SlotsWidget::edit /////////////////////////////////
void SlotsWidget::edit(ModelDocument *document)
{
  m_document = document;

  connect(m_document, &ModelDocument::document_changed, this, &SlotsWidget::refresh);
  connect(m_document, &ModelDocument::dependant_changed, this, &SlotsWidget::refresh);

  refresh();
}


///////////////////////// SlotsWidget::set_mesh /////////////////////////////
void SlotsWidget::set_mesh(int index)
{
  if (m_mesh != index)
  {
    m_mesh = index;

    refresh();
  }
}


///////////////////////// SlotsWidget::refresh //////////////////////////////
void SlotsWidget::refresh()
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


///////////////////////// SlotsWidget::itemSelectionChanged /////////////////
void SlotsWidget::itemSelectionChanged()
{
  emit selection_changed(currentRow());
}


///////////////////////// SlotsWidget::dragEnterEvent ///////////////////////
QStringList SlotsWidget::mimeTypes() const
{
  return { "text/uri-list" };
}


///////////////////////// SlotsWidget::supportedDropActions /////////////////
Qt::DropActions SlotsWidget::supportedDropActions() const
{
  return Qt::MoveAction | Qt::CopyAction;
}


////////////////////// SlotsWidget::dragEnterEvent //////////////////////////
void SlotsWidget::dragEnterEvent(QDragEnterEvent *event)
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


///////////////////////// SlotsWidget::dragMoveEvent ////////////////////////
void SlotsWidget::dragMoveEvent(QDragMoveEvent *event)
{
  if (!indexAt(event->pos()).isValid())
  {
    event->ignore();
    return;
  }

  QListWidget::dragMoveEvent(event);
}


///////////////////////// SlotsWidget::dropEvent ////////////////////////////
void SlotsWidget::dropEvent(QDropEvent *event)
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

