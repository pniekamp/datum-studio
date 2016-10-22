//
// Skybox Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "skyboxplugin.h"
#include "skybox.h"
#include "skyboxeditor.h"
#include "contentapi.h"
#include "buildapi.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- SkyboxPlugin --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SkyboxPlugin::Constructor /////////////////////////
SkyboxPlugin::SkyboxPlugin()
{
}


///////////////////////// SkyboxPlugin::Destructor //////////////////////////
SkyboxPlugin::~SkyboxPlugin()
{
  shutdown();
}


///////////////////////// SkyboxPlugin::initialise //////////////////////////
bool SkyboxPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("SkyBox", this);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("SkyBox", this);

  actionmanager->register_action("Skybox.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("SkyBox", this);

  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->register_builder("SkyBox", this);

  return true;
}


///////////////////////// SkyboxPlugin::shutdown ////////////////////////////
void SkyboxPlugin::shutdown()
{
}


///////////////////////// SkyboxPlugin::create_view /////////////////////////
QWidget *SkyboxPlugin::create_view(QString const &type)
{
  return new SkyboxEditor;
}


///////////////////////// SkyboxPlugin::create //////////////////////////////
bool SkyboxPlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  SkyboxDocument::create(path.toStdString());

  return true;
}


///////////////////////// SkyboxPlugin::hash ////////////////////////////////
bool SkyboxPlugin::hash(Studio::Document *document, size_t *key)
{
  SkyboxDocument::hash(document, key);

  return true;
}


///////////////////////// SkyboxPlugin::build ///////////////////////////////
bool SkyboxPlugin::build(Studio::Document *document, QString const &path)
{
  SkyboxDocument::build(document, path.toStdString());

  return true;
}
