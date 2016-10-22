//
// Hdr Importer Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- HdrImporter ------------------------------------
//---------------------------------------------------------------------------

class HdrImporter : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "HdrImporter" FILE "hdrimporter.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    HdrImporter();
    virtual ~HdrImporter();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    bool try_import(QString const &src, QString const &dst, QJsonObject metadata);
};

