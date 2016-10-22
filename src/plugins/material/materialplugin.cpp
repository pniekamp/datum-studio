//
// Material Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "materialplugin.h"
#include "material.h"
#include "materialeditor.h"
#include "contentapi.h"
#include "buildapi.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- MaterialPlugin ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MaterialPlugin::Constructor ///////////////////////
MaterialPlugin::MaterialPlugin()
{
}


///////////////////////// MaterialPlugin::Destructor ////////////////////////
MaterialPlugin::~MaterialPlugin()
{
  shutdown();
}


///////////////////////// MaterialPlugin::initialise ////////////////////////
bool MaterialPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Material", this);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("Material", this);

  actionmanager->register_action("Material.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("Material", this);

  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->register_builder("Material", this);

  return true;
}


///////////////////////// MaterialPlugin::shutdown //////////////////////////
void MaterialPlugin::shutdown()
{
}


///////////////////////// MaterialPlugin::create_view ///////////////////////
QWidget *MaterialPlugin::create_view(QString const &type)
{
  return new MaterialEditor;
}


///////////////////////// MaterialPlugin::create ////////////////////////////
bool MaterialPlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  MaterialDocument::create(path.toStdString(), Color3(1.0f, 1.0f, 1.0f), 1.0f, 1.0f);

  return true;
}


///////////////////////// MaterialPlugin::hash //////////////////////////////
bool MaterialPlugin::hash(Studio::Document *document, size_t *key)
{
  MaterialDocument::hash_part(document, key);

  return true;
}


///////////////////////// MaterialPlugin::build /////////////////////////////
bool MaterialPlugin::build(Studio::Document *document, QString const &path)
{
  MaterialDocument::build(document, path.toStdString());

  return true;
}
