//
// Material Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "documentapi.h"

//-------------------------- MaterialPlugin ---------------------------------
//---------------------------------------------------------------------------

class MaterialPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "MaterialPlugin" FILE "materialplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    MaterialPlugin();
    virtual ~MaterialPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    QWidget *create_view(QString const &type);

    bool create(QString const &type, QString const &path, QJsonObject metadata);

    bool hash(Studio::Document *document, size_t *key);

    bool build(Studio::Document *document, QString const &path);
};

