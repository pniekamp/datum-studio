//
// Console Log Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "ui_consoleplugin.h"

//-------------------------- ConsolePlugin ----------------------------------
//---------------------------------------------------------------------------

class ConsolePlugin : public Studio::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "ConsolePlugin" FILE "consoleplugin.json")
  Q_INTERFACES(Studio::Plugin)

  public:
    ConsolePlugin();
    virtual ~ConsolePlugin();

    bool initialise(QStringList const &arguments, QString *errormsg);

    void shutdown();

  public slots:

    void log_message(QtMsgType type, QString const &message);

  protected:

    void on_statusview_changed(QString const &view);

  private:

    QAction *m_statusview;

    QWidget *m_container;

    Ui::ConsolePlugin ui;
};
