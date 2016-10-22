//
// Build Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- BuildPlugin ------------------------------------
//---------------------------------------------------------------------------

class BuildPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "BuildPlugin" FILE "buildplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    BuildPlugin();
    virtual ~BuildPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();
};

