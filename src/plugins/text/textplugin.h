//
// TextPlugin Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "packapi.h"

//-------------------------- TextPlugin -------------------------------------
//---------------------------------------------------------------------------

class TextPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "TextPlugin" FILE "textplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    TextPlugin();
    virtual ~TextPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    bool pack(Studio::PackerState &asset, std::ofstream &fout);
};

