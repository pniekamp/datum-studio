//
// Image Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "packapi.h"

//-------------------------- ImagePlugin ------------------------------------
//---------------------------------------------------------------------------

class ImagePlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "ImagePlugin" FILE "imageplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    ImagePlugin();
    virtual ~ImagePlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    QWidget *create_view(QString const &type);

    bool pack(Studio::PackerState &asset, std::ofstream &fout);
};

