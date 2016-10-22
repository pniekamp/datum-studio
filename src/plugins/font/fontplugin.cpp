//
// Font Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "fontplugin.h"
#include "font.h"
#include "fonteditor.h"
#include "contentapi.h"
#include "buildapi.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- FontPlugin ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// FontPlugin::Constructor ///////////////////////////
FontPlugin::FontPlugin()
{
}


///////////////////////// FontPlugin::Destructor ////////////////////////////
FontPlugin::~FontPlugin()
{
  shutdown();
}


///////////////////////// FontPlugin::initialise ////////////////////////////
bool FontPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Font", this);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("Font", this);

  actionmanager->register_action("Font.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("Font", this);

  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->register_builder("Font", this);

  return true;
}


///////////////////////// FontPlugin::shutdown //////////////////////////////
void FontPlugin::shutdown()
{
}


///////////////////////// FontPlugin::create_view ///////////////////////////
QWidget *FontPlugin::create_view(QString const &type)
{
  return new FontEditor;
}


///////////////////////// FontPlugin::create ////////////////////////////////
bool FontPlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  FontDocument::create(path.toStdString(), "Arial", 14, 50);

  return true;
}


///////////////////////// FontPlugin::create ////////////////////////////////
bool FontPlugin::hash(Studio::Document *document, size_t *key)
{
  FontDocument::hash(document, key);

  return true;
}


///////////////////////// FontPlugin::build /////////////////////////////////
bool FontPlugin::build(Studio::Document *document, QString const &path)
{
  FontDocument::build(document, path.toStdString());

  return true;
}
