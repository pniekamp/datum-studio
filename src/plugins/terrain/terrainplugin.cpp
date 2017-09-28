//
// Terrain Plugin
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "terrainplugin.h"
#include "terrainmaterial.h"
#include "materialeditor.h"
#include "contentapi.h"
#include "buildapi.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- TerrainPlugin -------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// TerrainPlugin::Constructor ////////////////////////
TerrainPlugin::TerrainPlugin()
{
}


///////////////////////// TerrainPlugin::Destructor /////////////////////////
TerrainPlugin::~TerrainPlugin()
{
  shutdown();
}


///////////////////////// TerrainPlugin::initialise /////////////////////////
bool TerrainPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Material\\Terrain", this);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("Material\\Terrain", this);

  actionmanager->register_action("Material\\Terrain.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("Material\\Terrain", this);

  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->register_builder("Material\\Terrain", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("Material\\Terrain", this);
  packmanager->register_packer("Material\\Terrain.AlbedoMap", this);
  packmanager->register_packer("Material\\Terrain.SpecularMap", this);
  packmanager->register_packer("Material\\Terrain.NormalMap", this);

  return true;
}


///////////////////////// TerrainPlugin::shutdown ///////////////////////////
void TerrainPlugin::shutdown()
{
}


///////////////////////// TerrainPlugin::create_view ////////////////////////
QWidget *TerrainPlugin::create_view(QString const &type)
{
  return new MaterialEditor;
}


///////////////////////// TerrainPlugin::create /////////////////////////////
bool TerrainPlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  TerrainMaterialDocument::create(path.toStdString(), Color3(1.0f, 1.0f, 1.0f));

  return true;
}


///////////////////////// TerrainPlugin::hash ///////////////////////////////
bool TerrainPlugin::hash(Studio::Document *document, size_t *key)
{
  TerrainMaterialDocument::build_hash(document, key);

  return true;
}


///////////////////////// TerrainPlugin::build //////////////////////////////
bool TerrainPlugin::build(Studio::Document *document, QString const &path)
{
  TerrainMaterialDocument::build(document, path.toStdString());

  return true;
}


///////////////////////// TerrainPlugin::pack ///////////////////////////////
bool TerrainPlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  TerrainMaterialDocument::pack(asset, fout);

  return true;
}
