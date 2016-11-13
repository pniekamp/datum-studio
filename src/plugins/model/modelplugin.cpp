//
// Model Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "modelplugin.h"
#include "model.h"
#include "modeleditor.h"
#include "contentapi.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- ModelPlugin ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ModelPlugin::Constructor //////////////////////////
ModelPlugin::ModelPlugin()
{
}


///////////////////////// ModelPlugin::Destructor ///////////////////////////
ModelPlugin::~ModelPlugin()
{
  shutdown();
}


///////////////////////// ModelPlugin::initialise ///////////////////////////
bool ModelPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Model", this);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("Model", this);

  actionmanager->register_action("Model.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("Model", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("Model", this);

  return true;
}


///////////////////////// ModelPlugin::shutdown /////////////////////////////
void ModelPlugin::shutdown()
{
}


///////////////////////// ModelPlugin::create_view //////////////////////////
QWidget *ModelPlugin::create_view(QString const &type)
{
  return new ModelEditor;
}


///////////////////////// ModelPlugin::create ///////////////////////////////
bool ModelPlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  ModelDocument::create(path.toStdString());

  return true;
}


///////////////////////// ModelPlugin::pack /////////////////////////////////
bool ModelPlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  ModelDocument::pack(asset, fout);

  return true;
}
