//
// Datum Studio Core
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include <memory>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QMainWindow>

//-------------------------- ActionContainer --------------------------------
//---------------------------------------------------------------------------

class ActionContainerMenu : public Studio::ActionContainer
{
  Q_OBJECT

  public:
    ActionContainerMenu(QString const &id, QMenu *menu);

    void add_back(QAction *action);
    void add_front(QAction *action);

    void add_back(ActionContainer *menu);

  public:

    QMenu *menu() const { return m_menu; }

  private:

    QString m_id;
    QMenu *m_menu;
};


//-------------------------- ActionContainer --------------------------------
//---------------------------------------------------------------------------

class ActionContainerMenubar : public Studio::ActionContainer
{
  Q_OBJECT

  public:
    ActionContainerMenubar(QString const &id, QMenuBar *toolbar);

    void add_back(QAction *action);
    void add_front(QAction *action);

    void add_back(ActionContainer *menu);

  private:

    QString m_id;
    QMenuBar *m_menubar;
};


//-------------------------- ActionContainer --------------------------------
//---------------------------------------------------------------------------

class ActionContainerToolbar : public Studio::ActionContainer
{
  Q_OBJECT

  public:
    ActionContainerToolbar(QString const &id, QToolBar *toolbar);

    void add_back(QAction *action);
    void add_front(QAction *action);

    void add_back(ActionContainer *menu);

  private:

    QString m_id;
    QToolBar *m_toolbar;
};


//-------------------------- ActionManager ----------------------------------
//---------------------------------------------------------------------------

class ActionManager : public Studio::ActionManager
{
  Q_OBJECT

  public:
    ActionManager();

    QAction *action(QString const &id);

    QAction *register_action(QString const &id, QAction *action);

    Studio::ActionContainer *container(QString const &id);

    Studio::ActionContainer *register_container(QString const &id, QMenu *menu);
    Studio::ActionContainer *register_container(QString const &id, QMenuBar *menubar);
    Studio::ActionContainer *register_container(QString const &id, QToolBar *toolbar);

  private:

    std::map<QString, QAction*> m_actions;
    std::map<QString, Studio::ActionContainer*> m_containers;
};


//-------------------------- MainWindow -------------------------------------
//---------------------------------------------------------------------------

class MainWindow : public Studio::MainWindow
{
  Q_OBJECT

  public:
    MainWindow(QMainWindow *mainwindow);

    QWidget *handle();

    bool close();

  private:

    QMainWindow *m_mainwindow;
};


//-------------------------- ViewFactory ------------------------------------
//---------------------------------------------------------------------------

class ViewFactory : public Studio::ViewFactory
{
  Q_OBJECT

  public:
    ViewFactory();

    QWidget *create_view(QString const &type);

    void register_factory(QString const &type, QObject *factory);

  private:

    QMap<QString, QObject*> m_factories;
};


//-------------------------- Core -------------------------------------------
//---------------------------------------------------------------------------

class Core : public Studio::Core
{
  Q_OBJECT

  public:

    static Core *instance();

  public:

    QList<QObject*> objects() const;

    void add_object(QObject *object);

  private:
    Core();

    QList<QObject*> m_objects;
};
