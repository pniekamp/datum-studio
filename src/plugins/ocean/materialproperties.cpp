//
// Material Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "materialproperties.h"

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- MaterialProperties --------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MaterialProperties::Constructor ///////////////////
MaterialProperties::MaterialProperties(QWidget *parent)
  : QDockWidget(parent)
{
  ui.setupUi(this);
}


///////////////////////// MaterialProperties::edit //////////////////////////
void MaterialProperties::edit(Studio::Document *document)
{
  m_document = document;

  ui.Material->edit(document);

  connect(&m_document, &OceanMaterialDocument::document_changed, this, &MaterialProperties::refresh);
  connect(&m_document, &OceanMaterialDocument::dependant_changed, this, &MaterialProperties::refresh);

  refresh();
}


///////////////////////// MaterialProperties::refresh ///////////////////////
void MaterialProperties::refresh()
{
}
