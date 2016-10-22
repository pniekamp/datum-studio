//
// AssImp Importer Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- AssImporter ------------------------------------
//---------------------------------------------------------------------------

class AssImporter : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "AssImporter" FILE "assimporter.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    AssImporter();
    virtual ~AssImporter();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    bool try_import(QString const &src, QString const &dst, QJsonObject metadata);
};

