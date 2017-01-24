//
// Particle Plugin
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "particleplugin.h"
#include "particlesystem.h"
#include "particleeditor.h"
#include "contentapi.h"
#include "buildapi.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- ParticlePlugin ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ParticlePlugin::Constructor ///////////////////////
ParticlePlugin::ParticlePlugin()
{
}


///////////////////////// ParticlePlugin::Destructor ////////////////////////
ParticlePlugin::~ParticlePlugin()
{
  shutdown();
}


///////////////////////// ParticlePlugin::initialise ////////////////////////
bool ParticlePlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("ParticleSystem", this);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("ParticleSystem", this);

  actionmanager->register_action("ParticleSystem.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("ParticleSystem", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("ParticleSystem", this);

  return true;
}


///////////////////////// ParticlePlugin::shutdown //////////////////////////
void ParticlePlugin::shutdown()
{
}


///////////////////////// ParticlePlugin::create_view ///////////////////////
QWidget *ParticlePlugin::create_view(QString const &type)
{
  return new ParticleEditor;
}


///////////////////////// ParticlePlugin::create ////////////////////////////
bool ParticlePlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  ParticleSystemDocument::create(path.toStdString());

  return true;
}


///////////////////////// ParticlePlugin::pack //////////////////////////////
bool ParticlePlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  ParticleSystemDocument::pack(asset, fout);

  return true;
}
