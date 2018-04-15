//
// DatumUi Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "packapi.h"

//-------------------------- DatumUiPlugin ----------------------------------
//---------------------------------------------------------------------------

class DatumUiPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "DatumUiPlugin" FILE "datumuiplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    DatumUiPlugin();
    virtual ~DatumUiPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    QWidget *create_view(QString const &type);

    bool create(QString const &type, QString const &path, QJsonObject metadata);

    bool hash(Studio::Document *document, size_t *key);

    bool build(Studio::Document *document, QString const &path);

    bool pack(Studio::PackerState &asset, std::ofstream &fout);
};
