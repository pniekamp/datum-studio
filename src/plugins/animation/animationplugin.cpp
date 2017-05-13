//
// Animation Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "animationplugin.h"
#include "animation.h"
#include "animationviewer.h"
#include "buildapi.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- AnimationPlugin -----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// AnimationPlugin::Constructor ///////////////////////
AnimationPlugin::AnimationPlugin()
{
}


///////////////////////// AnimationPlugin::Destructor ///////////////////////
AnimationPlugin::~AnimationPlugin()
{
  shutdown();
}


///////////////////////// AnimationPlugin::initialise ///////////////////////
bool AnimationPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Animation", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("Animation", this);

  return true;
}


///////////////////////// AnimationPlugin::shutdown /////////////////////////
void AnimationPlugin::shutdown()
{
}


///////////////////////// AnimationPlugin::create_view //////////////////////
QWidget *AnimationPlugin::create_view(QString const &type)
{
  return new AnimationViewer;
}


///////////////////////// AnimationPlugin::hash /////////////////////////////
bool AnimationPlugin::hash(Studio::Document *document, size_t *key)
{
  AnimationDocument::hash(document, key);

  return true;
}


///////////////////////// AnimationPlugin::pack /////////////////////////////
bool AnimationPlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  AnimationDocument::pack(asset, fout);

  return true;
}
