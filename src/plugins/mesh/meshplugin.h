//
// Mesh Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "packapi.h"

//-------------------------- MeshPlugin -------------------------------------
//---------------------------------------------------------------------------

class MeshPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "MeshPlugin" FILE "meshplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    MeshPlugin();
    virtual ~MeshPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    QWidget *create_view(QString const &type);

    bool hash(Studio::Document *document, size_t *key);

    bool pack(Studio::PackerState &asset, std::ofstream &fout);
};

