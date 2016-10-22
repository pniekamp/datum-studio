//
// Meta Bar
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include <QToolBar>
#include <QActionGroup>

//-------------------------- MetaBar ----------------------------------------
//---------------------------------------------------------------------------

class MetaBar : public QToolBar
{
  Q_OBJECT

  public:
    MetaBar(QWidget *parent);

    QString metamode() const;

    void set_metamode(QString const &text);

  signals:

    void metamode_changed(QString const &text);

  protected:

    void actionEvent(QActionEvent *event) override;

  private:

    QActionGroup *m_metamodes;
};


//-------------------------- MetaBox ----------------------------------------
//---------------------------------------------------------------------------

class MetaBox : public QToolBar
{
  Q_OBJECT

  public:
    MetaBox(QWidget *parent);
};


//-------------------------- ModeManager ------------------------------------
//---------------------------------------------------------------------------

class ModeManager : public Studio::ModeManager
{
  Q_OBJECT

  public:
    ModeManager(MetaBar *metabar, MetaBox *metabox, QStackedWidget *container);

    void add_metamode(int index, QAction *action);

    QString metamode() const;

    void set_metamode(QString const &metamode);

    QStackedWidget *container();

  private:

    MetaBar *m_metabar;
    MetaBox *m_metabox;
    QStackedWidget *m_container;
};
