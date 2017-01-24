//
// Sprite Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "spriteproperties.h"
#include "contentapi.h"
#include "buildapi.h"
#include "assetfile.h"

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- SpriteProperties ----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SpriteProperties::Constructor /////////////////////
SpriteProperties::SpriteProperties(QWidget *parent)
  : QDockWidget(parent)
{
  ui.setupUi(this);
}


///////////////////////// SpriteProperties::edit ////////////////////////////
void SpriteProperties::edit(Studio::Document *document)
{
  m_document = document;

  ui.LayerList->edit(document);

  connect(&m_document, &SpriteSheetDocument::document_changed, this, &SpriteProperties::refresh);
  connect(&m_document, &SpriteSheetDocument::dependant_changed, this, &SpriteProperties::refresh);

  refresh();
}


///////////////////////// SpriteProperties::refresh /////////////////////////
void SpriteProperties::refresh()
{
  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->request_build(m_document, this, &SpriteProperties::on_sprite_build_complete);

  ui.Layers->setText(QString::number(m_document.layers()));
}


///////////////////////// SpriteProperties::sprite_build_complete ///////////
void SpriteProperties::on_sprite_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  PackImageHeader imag;

  if (read_asset_header(fin, 1, &imag))
  {
    ui.Width->setText(QString::number(imag.width));
    ui.Height->setText(QString::number(imag.height));
  }
}

