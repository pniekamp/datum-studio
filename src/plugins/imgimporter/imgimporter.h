//
// Image Importer Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- ImageImporter ----------------------------------
//---------------------------------------------------------------------------

class ImageImporter : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "ImgImporter" FILE "imgimporter.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    ImageImporter();
    virtual ~ImageImporter();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    bool try_import(QString const &src, QString const &dst, QJsonObject metadata);
};

