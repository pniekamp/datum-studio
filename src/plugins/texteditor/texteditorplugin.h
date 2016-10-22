//
// TextEditorPlugin Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- TextEditorPlugin -------------------------------
//---------------------------------------------------------------------------

class TextEditorPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "TextEditorPlugin" FILE "texteditorplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    TextEditorPlugin();
    virtual ~TextEditorPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    QWidget *create_view(QString const &type);
};

