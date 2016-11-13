//
// Text Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "textplugin.h"
#include "text.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- TextPlugin ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// TextPlugin::Constructor ///////////////////////////
TextPlugin::TextPlugin()
{
}


///////////////////////// TextPlugin::Destructor ////////////////////////////
TextPlugin::~TextPlugin()
{
  shutdown();
}


///////////////////////// TextPlugin::initialise ///////////////////////////
bool TextPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("Text", this);

  return true;
}


///////////////////////// TextPlugin::shutdown //////////////////////////////
void TextPlugin::shutdown()
{
}


///////////////////////// TextPlugin::pack /////////////////////////////////
bool TextPlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  TextDocument::pack(asset, fout);

  return true;
}
