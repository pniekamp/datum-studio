//
// Material Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "materialproperties.h"
#include "buildapi.h"
#include "assetfile.h"

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

  ui.LayerList->edit(document);

  connect(&m_document, &TerrainMaterialDocument::document_changed, this, &MaterialProperties::refresh);
  connect(&m_document, &TerrainMaterialDocument::dependant_changed, this, &MaterialProperties::refresh);

  refresh();
}


///////////////////////// MaterialProperties::refresh ///////////////////////
void MaterialProperties::refresh()
{
  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->request_build(m_document, this, &MaterialProperties::on_material_build_complete);

  ui.Layers->setText(QString::number(m_document.layers()));
}


///////////////////////// MaterialProperties::material_build_complete ///////
void MaterialProperties::on_material_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  PackImageHeader imag;

  if (read_asset_header(fin, 1, &imag))
  {
    ui.Width->setText(QString::number(imag.width));
    ui.Height->setText(QString::number(imag.height));
  }
}
