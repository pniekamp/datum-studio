//
// Model Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "packapi.h"

//-------------------------- ModelPlugin ------------------------------------
//---------------------------------------------------------------------------

class ModelPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "ModelPlugin" FILE "modelplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    ModelPlugin();
    virtual ~ModelPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    QWidget *create_view(QString const &type);

    bool create(QString const &type, QString const &path, QJsonObject metadata);

    bool pack(Studio::PackerState &asset, std::ofstream &fout);
};

