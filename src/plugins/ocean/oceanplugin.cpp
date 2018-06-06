//
// Ocean Plugin
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "oceanplugin.h"
#include "oceanmaterial.h"
#include "materialeditor.h"
#include "contentapi.h"
#include "buildapi.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- OceanPlugin ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// OceanPlugin::Constructor //////////////////////////
OceanPlugin::OceanPlugin()
{
}


///////////////////////// OceanPlugin::Destructor ///////////////////////////
OceanPlugin::~OceanPlugin()
{
  shutdown();
}


///////////////////////// OceanPlugin::initialise ///////////////////////////
bool OceanPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Material\\Ocean", this);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("Material\\Ocean", this);

  actionmanager->register_action("Material\\Ocean.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("Material\\Ocean", this);

  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->register_builder("Material\\Ocean", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("Material\\Ocean", this);
  packmanager->register_packer("Material\\Ocean.AlbedoMap", this);
  packmanager->register_packer("Material\\Ocean.SpecularMap", this);
  packmanager->register_packer("Material\\Ocean.NormalMap", this);

  return true;
}


///////////////////////// OceanPlugin::shutdown /////////////////////////////
void OceanPlugin::shutdown()
{
}


///////////////////////// OceanPlugin::create_view //////////////////////////
QWidget *OceanPlugin::create_view(QString const &type)
{
  return new MaterialEditor;
}


///////////////////////// OceanPlugin::create ///////////////////////////////
bool OceanPlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  OceanMaterialDocument::create(path.toStdString(), Color3(1.0f, 1.0f, 1.0f), 0.32f);

  return true;
}


///////////////////////// OceanPlugin::hash /////////////////////////////////
bool OceanPlugin::hash(Studio::Document *document, size_t *key)
{
  OceanMaterialDocument::build_hash(document, key);

  return true;
}


///////////////////////// OceanPlugin::build ////////////////////////////////
bool OceanPlugin::build(Studio::Document *document, QString const &path)
{
  OceanMaterialDocument::build(document, path.toStdString());

  return true;
}


///////////////////////// OceanPlugin::pack /////////////////////////////////
bool OceanPlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  OceanMaterialDocument::pack(asset, fout);

  return true;
}
