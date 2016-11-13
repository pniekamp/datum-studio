//
// Pack Importer Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- PackImporter -----------------------------------
//---------------------------------------------------------------------------

class PackImporter : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "PackImporter" FILE "packimporter.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    PackImporter();
    virtual ~PackImporter();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    bool try_import(QString const &src, QString const &dst, QJsonObject metadata);
};

