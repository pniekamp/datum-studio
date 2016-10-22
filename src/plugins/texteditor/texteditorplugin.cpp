//
// TextEditor Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "texteditorplugin.h"
#include "texteditor.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- TextEditorPlugin ----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// TextEditorPlugin::Constructor /////////////////////
TextEditorPlugin::TextEditorPlugin()
{
}


///////////////////////// TextEditorPlugin::Destructor //////////////////////
TextEditorPlugin::~TextEditorPlugin()
{
  shutdown();
}


///////////////////////// TextEditorPlugin::initialise //////////////////////
bool TextEditorPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Text", this);

  return true;
}


///////////////////////// TextEditorPlugin::shutdown ////////////////////////
void TextEditorPlugin::shutdown()
{
}


///////////////////////// TextEditorPlugin::create_view /////////////////////
QWidget *TextEditorPlugin::create_view(QString const &type)
{
  return new TextEditor;
}
