//
// DevIL Importer Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- DevILImporter ----------------------------------
//---------------------------------------------------------------------------

class DevILImporter : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "DevILImporter" FILE "devilimporter.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    DevILImporter();
    virtual ~DevILImporter();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    bool try_import(QString const &src, QString const &dst, QJsonObject metadata);
};

