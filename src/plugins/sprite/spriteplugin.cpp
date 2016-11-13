//
// Sprite Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "spriteplugin.h"
#include "spritesheet.h"
#include "spriteeditor.h"
#include "contentapi.h"
#include "buildapi.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- SpritePlugin --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SpritePlugin::Constructor /////////////////////////
SpritePlugin::SpritePlugin()
{
}


///////////////////////// SpritePlugin::Destructor //////////////////////////
SpritePlugin::~SpritePlugin()
{
  shutdown();
}


///////////////////////// SpritePlugin::initialise //////////////////////////
bool SpritePlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("SpriteSheet", this);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("SpriteSheet", this);

  actionmanager->register_action("SpriteSheet.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("SpriteSheet", this);

  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->register_builder("SpriteSheet", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("SpriteSheet", this);

  return true;
}


///////////////////////// SpritePlugin::shutdown ////////////////////////////
void SpritePlugin::shutdown()
{
}


///////////////////////// SpritePlugin::create_view /////////////////////////
QWidget *SpritePlugin::create_view(QString const &type)
{
  return new SpriteEditor;
}


///////////////////////// SpritePlugin::create //////////////////////////////
bool SpritePlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  SpriteSheetDocument::create(path.toStdString());

  return true;
}


///////////////////////// SpritePlugin::hash ////////////////////////////////
bool SpritePlugin::hash(Studio::Document *document, size_t *key)
{
  SpriteSheetDocument::build_hash(document, key);

  return true;
}


///////////////////////// SpritePlugin::build ///////////////////////////////
bool SpritePlugin::build(Studio::Document *document, QString const &path)
{
  SpriteSheetDocument::build(document, path.toStdString());

  return true;
}


///////////////////////// SpritePlugin::pack /////////////////////////////////
bool SpritePlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  SpriteSheetDocument::pack(asset, fout);

  return true;
}
