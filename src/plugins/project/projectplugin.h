//
// Project Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

//-------------------------- ProjectPlugin ----------------------------------
//---------------------------------------------------------------------------

class ProjectPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "ProjectPlugin" FILE "projectplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    ProjectPlugin();
    virtual ~ProjectPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  protected:

    void on_mainwindow_closing(bool *cancel);

  protected slots:

    void on_NewProject_triggered();
    void on_OpenProject_triggered();
    void on_SaveProject_triggered();
};

