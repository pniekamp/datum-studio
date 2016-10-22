//
// Home Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "ui_homeplugin.h"

//-------------------------- HomePlugin -------------------------------------
//---------------------------------------------------------------------------

class HomePlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "HomePlugin" FILE "homeplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    HomePlugin();
    virtual ~HomePlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    void on_metamode_changed(QString const &mode);

    void on_NewProject_triggered();
    void on_OpenProject_triggered(QString const &link);

  private:

    QAction *m_metamode;

    QWidget *m_container;

    Ui::HomePlugin ui;
};

