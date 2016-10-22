//
// BinEditorPlugin Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- BinEditorPlugin --------------------------------
//---------------------------------------------------------------------------

class BinEditorPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "BinEditorPlugin" FILE "bineditorplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    BinEditorPlugin();
    virtual ~BinEditorPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    QWidget *create_view(QString const &type);
};

