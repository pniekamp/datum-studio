//
// Mesh Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "meshplugin.h"
#include "mesh.h"
#include "meshviewer.h"
#include "buildapi.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- MeshPlugin ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MeshPlugin::Constructor ///////////////////////////
MeshPlugin::MeshPlugin()
{
}


///////////////////////// MeshPlugin::Destructor ////////////////////////////
MeshPlugin::~MeshPlugin()
{
  shutdown();
}


///////////////////////// MeshPlugin::initialise ////////////////////////////
bool MeshPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Mesh", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("Mesh", this);

  return true;
}


///////////////////////// MeshPlugin::shutdown //////////////////////////////
void MeshPlugin::shutdown()
{
}


///////////////////////// MeshPlugin::create_view ///////////////////////////
QWidget *MeshPlugin::create_view(QString const &type)
{
  return new MeshViewer;
}


///////////////////////// MeshPlugin::hash //////////////////////////////////
bool MeshPlugin::hash(Studio::Document *document, size_t *key)
{
  MeshDocument::hash(document, key);

  return true;
}


///////////////////////// MeshPlugin::pack //////////////////////////////////
bool MeshPlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  MeshDocument::pack(asset, fout);

  return true;
}
