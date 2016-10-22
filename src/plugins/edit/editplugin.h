//
// Edit Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "editormanager.h"
#include "ui_editplugin.h"

//-------------------------- EditPlugin -------------------------------------
//---------------------------------------------------------------------------

class EditPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "EditPlugin" FILE "editplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    EditPlugin();
    virtual ~EditPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    void on_project_changed(QString const &projectfile);

    void on_project_saving(QString const &projectfile);

    void on_project_closing(bool *cancel);

    void on_metamode_changed(QString const &mode);

  protected:

    void on_expand_editor();
    void on_collapse_editor();

  private:

    QAction *m_metamode;

    QWidget *m_container;

    EditorManager *m_manager;

    Ui::EditPlugin ui;
};

