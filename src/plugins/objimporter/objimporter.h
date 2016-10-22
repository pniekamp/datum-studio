//
// Obj Importer Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- ObjImporter ------------------------------------
//---------------------------------------------------------------------------

class ObjImporter : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "ObjImporter" FILE "objimporter.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    ObjImporter();
    virtual ~ObjImporter();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    bool try_import(QString const &src, QString const &dst, QJsonObject metadata);
};

