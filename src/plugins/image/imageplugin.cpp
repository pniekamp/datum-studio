//
// Image Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "imageplugin.h"
#include "imgviewer.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- ImagePlugin ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ImagePlugin::Constructor //////////////////////////
ImagePlugin::ImagePlugin()
{
}


///////////////////////// ImagePlugin::Destructor ///////////////////////////
ImagePlugin::~ImagePlugin()
{
  shutdown();
}


///////////////////////// ImagePlugin::initialise ///////////////////////////
bool ImagePlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Image", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("Image", this);

  return true;
}


///////////////////////// ImagePlugin::shutdown /////////////////////////////
void ImagePlugin::shutdown()
{
}


///////////////////////// ImagePlugin::create_view //////////////////////////
QWidget *ImagePlugin::create_view(QString const &type)
{
  return new ImageViewer;
}


///////////////////////// ImagePlugin::pack /////////////////////////////////
bool ImagePlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  ImageDocument::pack(asset, fout);

  return true;
}
