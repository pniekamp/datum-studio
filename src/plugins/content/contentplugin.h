//
// Content Browser Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "ui_contentplugin.h"

//-------------------------- ContentPlugin ----------------------------------
//---------------------------------------------------------------------------

class ContentPlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "ContentPlugin" FILE "contentplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    ContentPlugin();
    virtual ~ContentPlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  protected:

    void on_project_changed(QString const &projectfile);

    void on_statusview_changed(QString const &view);

    void on_item_triggered(QString const &path);

    void on_item_renamed(QString const &src, QString const &dst);

    void on_contextmenu_requested(QPoint pos);

  protected slots:

    void on_Create_triggered(QAction *action);
    void on_Import_triggered();
    void on_Reimport_triggered();
    void on_Rename_triggered();
    void on_Delete_triggered();

  private:

    QAction *m_statusview;

    QWidget *m_container;

    QMenu *m_createmenu;

    Ui::ContentPlugin ui;
};
