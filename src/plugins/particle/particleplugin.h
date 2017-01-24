//
// Particle Plugin
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "api.h"
#include "packapi.h"

//-------------------------- ParticlePlugin ---------------------------------
//---------------------------------------------------------------------------

class ParticlePlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "ParticlePlugin" FILE "particleplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    ParticlePlugin();
    virtual ~ParticlePlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    QWidget *create_view(QString const &type);

    bool create(QString const &type, QString const &path, QJsonObject metadata);

    bool pack(Studio::PackerState &asset, std::ofstream &fout);
};

