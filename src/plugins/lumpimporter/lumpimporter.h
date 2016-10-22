//
// Lump Importer Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- LumpImporter -----------------------------------
//---------------------------------------------------------------------------

class LumpImporter : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "LumpImporter" FILE "lumpimporter.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    LumpImporter();
    virtual ~LumpImporter();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    bool try_import(QString const &src, QString const &dst, QJsonObject metadata);
};

