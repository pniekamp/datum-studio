//
// Datum Studio Core
//

//
// Copyright (C) 2016 Peter Niekamp
//


#include "core.h"
#include <QGuiApplication>
#include <cassert>

#include <QtDebug>

using namespace std;

//|---------------------- ActionContainer  ----------------------------------
//|--------------------------------------------------------------------------
//| ActionContainer
//|

///////////////////////// ActionContainer::Constructor //////////////////////
ActionContainerMenu::ActionContainerMenu(QString const &id, QMenu *menu)
  : m_id(id),
    m_menu(menu)
{
}


///////////////////////// ActionContainer::add_back /////////////////////////
void ActionContainerMenu::add_back(QAction *action)
{
  m_menu->addAction(action);
}


///////////////////////// ActionContainer::add_front ////////////////////////
void ActionContainerMenu::add_front(QAction *action)
{
  assert(m_menu->actions().count() != 0);

  m_menu->insertAction(m_menu->actions().front(), action);
}


///////////////////////// ActionContainer::add_back /////////////////////////
void ActionContainerMenu::add_back(ActionContainer *menu)
{
  ActionContainerMenu *container = qobject_cast<ActionContainerMenu*>(menu);

  if (container)
  {
    m_menu->addMenu(container->menu());
  }
}



//|---------------------- ActionContainer  ----------------------------------
//|--------------------------------------------------------------------------
//| ActionContainer
//|

///////////////////////// ActionContainer::Constructor //////////////////////
ActionContainerMenubar::ActionContainerMenubar(QString const &id, QMenuBar *menubar)
  : m_id(id),
    m_menubar(menubar)
{
}


///////////////////////// ActionContainer::add_back /////////////////////////
void ActionContainerMenubar::add_back(QAction *action)
{
  m_menubar->addAction(action);
}


///////////////////////// ActionContainer::add_front ////////////////////////
void ActionContainerMenubar::add_front(QAction *action)
{
  assert(m_menubar->actions().count() != 0);

  m_menubar->insertAction(m_menubar->actions().front(), action);
}


///////////////////////// ActionContainer::add_back /////////////////////////
void ActionContainerMenubar::add_back(ActionContainer *menu)
{
  ActionContainerMenu *container = qobject_cast<ActionContainerMenu*>(menu);

  if (container)
  {
    m_menubar->insertMenu(m_menubar->actions().back(), container->menu());
  }
}



//|---------------------- ActionContainer  ----------------------------------
//|--------------------------------------------------------------------------
//| ActionContainer
//|

///////////////////////// ActionContainer::Constructor //////////////////////
ActionContainerToolbar::ActionContainerToolbar(QString const &id, QToolBar *toolbar)
  : m_id(id),
    m_toolbar(toolbar)
{
}



///////////////////////// ActionContainer::add_back /////////////////////////
void ActionContainerToolbar::add_back(QAction *action)
{
  m_toolbar->addAction(action);
}


///////////////////////// ActionContainer::add_front ////////////////////////
void ActionContainerToolbar::add_front(QAction *action)
{
  assert(m_toolbar->actions().count() != 0);

  m_toolbar->insertAction(m_toolbar->actions().front(), action);
}


///////////////////////// ActionContainer::add_back /////////////////////////
void ActionContainerToolbar::add_back(ActionContainer *menu)
{
}




//|---------------------- ActionManager -------------------------------------
//|--------------------------------------------------------------------------
//| ActionManager
//|

///////////////////////// ActionManager::Constructor ////////////////////////
ActionManager::ActionManager()
{
}


///////////////////////// ActionManager::action /////////////////////////////
QAction *ActionManager::action(QString const &id)
{
  auto j = m_actions.find(id);

  if (j != m_actions.end())
    return j->second;

  return nullptr;
}


///////////////////////// ActionManager::register_action ////////////////////
QAction *ActionManager::register_action(QString const &id, QAction *action)
{
  m_actions[id] = action;

  return action;
}


///////////////////////// ActionManager::container //////////////////////////
Studio::ActionContainer *ActionManager::container(QString const &id)
{
  auto j = m_containers.find(id);

  if (j != m_containers.end())
    return j->second;

  return nullptr;
}


///////////////////////// ActionManager::register_container /////////////////
Studio::ActionContainer *ActionManager::register_container(QString const &id, QMenu *menu)
{
  auto container = new ActionContainerMenu(id, menu);

  m_containers[id] = container;

  return container;
}


///////////////////////// ActionManager::register_container /////////////////
Studio::ActionContainer *ActionManager::register_container(QString const &id, QMenuBar *menubar)
{
  auto container = new ActionContainerMenubar(id, menubar);

  m_containers[id] = container;

  return container;
}


///////////////////////// ActionManager::register_container /////////////////
Studio::ActionContainer *ActionManager::register_container(QString const &id, QToolBar *toolbar)
{
  auto container = new ActionContainerToolbar(id, toolbar);

  m_containers[id] = container;

  return container;
}



//|---------------------- MainWindow ----------------------------------------
//|--------------------------------------------------------------------------
//| Main Window
//|

///////////////////////// MainWindow::Constructor ///////////////////////////
MainWindow::MainWindow(QMainWindow *mainwindow)
  : m_mainwindow(mainwindow)
{
  connect(qApp, &QGuiApplication::applicationStateChanged, this, [=](Qt::ApplicationState state) { if (state == Qt::ApplicationActive) emit activated(); });
}


///////////////////////// MainWindow::handle ////////////////////////////////
QWidget *MainWindow::handle()
{
  return m_mainwindow;
}


///////////////////////// MainWindow::close /////////////////////////////////
bool MainWindow::close()
{
  bool cancel = false;

  emit closing(&cancel);

  return !cancel;
}


//|---------------------- Core  ---------------------------------------------
//|--------------------------------------------------------------------------
//| Core
//|

///////////////////////// ViewFactory::Constructor //////////////////////////
ViewFactory::ViewFactory()
{
}


///////////////////////// ViewFactory::Constructor //////////////////////////
QWidget *ViewFactory::create_view(QString const &type)
{
  QWidget *view = nullptr;

  if (m_factories.find(type) != m_factories.end())
  {
    QMetaObject::invokeMethod(m_factories[type], "create_view", Q_RETURN_ARG(QWidget*, view), Q_ARG(QString, type));
  }

  return view;
}


///////////////////////// ViewFactory::register_factory /////////////////////
void ViewFactory::register_factory(QString const &type, QObject *factory)
{
  m_factories[type] = factory;
}


//|---------------------- Core  ---------------------------------------------
//|--------------------------------------------------------------------------
//| Core
//|

///////////////////////// Core::instance ////////////////////////////////////
Studio::Core *Studio::Core::instance()
{
  return ::Core::instance();
}


///////////////////////// Core::instance ////////////////////////////////////
Core *Core::instance()
{
  static Core g_core;

  return &g_core;
}


///////////////////////// Core::Constructor /////////////////////////////////
Core::Core()
{
}


///////////////////////// Core::objects /////////////////////////////////////
QList<QObject*> Core::objects() const
{
  return m_objects;
}


///////////////////////// Core::add_object //////////////////////////////////
void Core::add_object(QObject *object)
{
  m_objects.push_back(object);
}
