//
// Text Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "textplugin.h"
#include "text.h"
#include "contentapi.h"
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
  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("Text", this);

  actionmanager->register_action("Text.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("Text", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("Text", this);

  return true;
}


///////////////////////// TextPlugin::shutdown //////////////////////////////
void TextPlugin::shutdown()
{
}


///////////////////////// TextPlugin::create ////////////////////////////////
bool TextPlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  TextDocument::create(path.toStdString());

  return true;
}


///////////////////////// TextPlugin::pack //////////////////////////////////
bool TextPlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  TextDocument::pack(asset, fout);

  return true;
}
