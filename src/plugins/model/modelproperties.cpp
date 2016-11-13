//
// Model Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "modelproperties.h"
#include "contentapi.h"
#include "buildapi.h"
#include "assetfile.h"

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- ModelProperties -----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ModelProperties::Constructor //////////////////////
ModelProperties::ModelProperties(QWidget *parent)
  : QDockWidget(parent)
{
  ui.setupUi(this);

  connect(ui.MeshList, &MeshWidget::selection_changed, ui.SlotsList, &SlotsWidget::set_mesh);
  connect(ui.MeshList, &MeshWidget::selection_changed, ui.Material, &MaterialWidget::set_mesh);
  connect(ui.SlotsList, &SlotsWidget::selection_changed, ui.Material, &MaterialWidget::set_slot);
}


///////////////////////// ModelProperties::edit /////////////////////////////
void ModelProperties::edit(Studio::Document *document)
{
  m_document = document;

  ui.MeshList->edit(document);
  ui.SlotsList->edit(document);
  ui.Material->edit(document);

  connect(&m_document, &ModelDocument::document_changed, this, &ModelProperties::refresh);
  connect(&m_document, &ModelDocument::dependant_changed, this, &ModelProperties::refresh);

  refresh();
}


///////////////////////// ModelProperties::refresh /////////////////////////
void ModelProperties::refresh()
{

}
