//
// Document Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- DocumentPlugin ---------------------------------
//---------------------------------------------------------------------------

class DocumentPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "DocumentPlugin" FILE "documentplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    DocumentPlugin();
    virtual ~DocumentPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();
};

