//
// BinEditor Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "bineditorplugin.h"
#include "bineditor.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- BinEditorPlugin -----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// BinEditorPlugin::Constructor //////////////////////
BinEditorPlugin::BinEditorPlugin()
{
}


///////////////////////// BinEditorPlugin::Destructor ///////////////////////
BinEditorPlugin::~BinEditorPlugin()
{
  shutdown();
}


///////////////////////// BinEditorPlugin::initialise ///////////////////////
bool BinEditorPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Binary", this);

  return true;
}


///////////////////////// BinEditorPlugin::shutdown /////////////////////////
void BinEditorPlugin::shutdown()
{
}


///////////////////////// BinEditorPlugin::create_view //////////////////////
QWidget *BinEditorPlugin::create_view(QString const &type)
{
  return new BinEditor;
}
