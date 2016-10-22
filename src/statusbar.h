//
// Status Bar
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "commandbar.h"
#include <QActionGroup>

//-------------------------- StatusBar --------------------------------------
//---------------------------------------------------------------------------

class StatusBar : public QToolBar
{
  Q_OBJECT

  public:
    StatusBar(QWidget *parent);

    QString statusview() const;

    void set_statusview(QString const &text);

  signals:

    void statusview_changed(QString const &text);

  protected:

    void actionEvent(QActionEvent *event) override;

  private:

    QAction *m_selected;
};


//-------------------------- StatusBox --------------------------------------
//---------------------------------------------------------------------------

class StatusBox : public CommandBar
{
  Q_OBJECT

  public:
    StatusBox(QWidget *parent);
};


//-------------------------- StatusReport -----------------------------------
//---------------------------------------------------------------------------

class StatusReport : public CommandBar
{
  Q_OBJECT

  public:
    StatusReport(QWidget *parent);
};


//-------------------------- StatusManager ----------------------------------
//---------------------------------------------------------------------------

class StatusManager : public Studio::StatusManager
{
  Q_OBJECT

  public:
    StatusManager(StatusBar *statusbar, StatusBox *statusbox, StatusReport *statusreport, QStackedWidget *container);

    void add_statusview(QAction *action);

    QString statusview() const;

    void set_statusview(QString const &statusview);

    QStackedWidget *container();

  protected:

    void on_statusview_changed(QString const &text);

  private:

    StatusBar *m_statusbar;
    StatusBox *m_statusbox;
    StatusReport *m_statusreport;
    QStackedWidget *m_container;
};
