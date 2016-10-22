//
// Text Importer Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- TextImporter -----------------------------------
//---------------------------------------------------------------------------

class TextImporter : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "TextImporter" FILE "textimporter.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    TextImporter();
    virtual ~TextImporter();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    bool try_import(QString const &src, QString const &dst, QJsonObject metadata);
};

