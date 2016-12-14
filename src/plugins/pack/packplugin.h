//
// Content Browser Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "ui_packplugin.h"
#include "pack.h"

//-------------------------- PackPlugin -------------------------------------
//---------------------------------------------------------------------------

class PackPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "PackPlugin" FILE "packplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    PackPlugin();
    virtual ~PackPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    void build();

    void build_export();

  protected:

    void on_project_changed(QString const &projectfile);

    void on_project_saving(QString const &projectfile);

    void on_project_closing(bool *cancel);

    void on_project_closed();

    void on_metamode_changed(QString const &mode);

    void on_item_triggered(PackModel::Node *node);

    void on_item_renamed(PackModel::Node *node, QString const &str);

    void on_contextmenu_requested(QPoint pos);

  protected slots:

    void on_CreateFolder_triggered();
    void on_Rename_triggered();
    void on_Remove_triggered();

    void on_PackProperties_triggered();

  private:

    PackManager *m_manager;

    PackModel *m_pack;

    QAction *m_build;

    QAction *m_metamode;

    QMainWindow *m_container;

    Ui::PackPlugin ui;
};

