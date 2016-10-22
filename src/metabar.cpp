//
// Meta Bar
//

//
// Copyright (C) 2016 Peter Niekamp
//


#include "metabar.h"
#include <QActionEvent>

#include <QtDebug>

using namespace std;


//|---------------------- MetaBar -------------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MetaBar::Constructor //////////////////////////////
MetaBar::MetaBar(QWidget *parent)
  : QToolBar(parent)
{
  setOrientation(Qt::Vertical);
  setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  setIconSize(QSize(32, 32));

  m_metamodes = new QActionGroup(this);

  m_metamodes->setExclusive(true);

  connect(m_metamodes, &QActionGroup::triggered, this, [=](QAction *action) { set_metamode(action->text()); });
}


///////////////////////// MetaBar::metamode /////////////////////////////////
QString MetaBar::metamode() const
{
  if (m_metamodes->checkedAction())
    return m_metamodes->checkedAction()->text();

  return "";
}


///////////////////////// MetaBar::set_metamode /////////////////////////////
void MetaBar::set_metamode(QString const &text)
{
  for(int i = 0; i < actions().size(); ++i)
  {
    QAction *action = actions().at(i);

    if (action->text() == text)
    {
      action->setChecked(true);

      emit metamode_changed(text);
    }
  }
}


///////////////////////// MetaBar::actionEvent //////////////////////////////
void MetaBar::actionEvent(QActionEvent *event)
{
  QAction *action = event->action();

  if (event->type() == QEvent::ActionAdded)
  {
    action->setCheckable(true);

    m_metamodes->addAction(action);
  }

  QToolBar::actionEvent(event);
}


//|---------------------- MetaBox -------------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MetaBox::Constructor //////////////////////////////
MetaBox::MetaBox(QWidget *parent)
  : QToolBar(parent)
{
  setOrientation(Qt::Vertical);
  setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  setIconSize(QSize(32, 32));
}



//|---------------------- ModeManager ---------------------------------------
//|--------------------------------------------------------------------------
//| Mode Manager
//|

///////////////////////// ModeManager::Constructor //////////////////////////
ModeManager::ModeManager(MetaBar *metabar, MetaBox *metabox, QStackedWidget *container)
  : m_metabar(metabar),
    m_metabox(metabox),
    m_container(container)
{
  connect(m_metabar, &MetaBar::metamode_changed, this, &ModeManager::metamode_changed);
}


///////////////////////// ModeManager::add_metamode /////////////////////////
void ModeManager::add_metamode(int index, QAction *action)
{
  QAction *before = nullptr;

  for(auto &action : m_metabar->actions())
  {
    if (index < action->property("Index").toInt())
    {
      before = action;
      break;
    }
  }

  action->setProperty("Index", index);

  m_metabar->insertAction(before, action);
}


///////////////////////// ModeManager::metamode /////////////////////////////
QString ModeManager::metamode() const
{
  return m_metabar->metamode();
}


///////////////////////// ModeManager::set_metamode /////////////////////////
void ModeManager::set_metamode(QString const &metamode)
{
  m_metabar->set_metamode(metamode);
}


///////////////////////// ModeManager::container ////////////////////////////
QStackedWidget *ModeManager::container()
{
  return m_container;
}

