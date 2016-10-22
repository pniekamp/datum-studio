//
// Status Bar
//

//
// Copyright (C) 2016 Peter Niekamp
//


#include "statusbar.h"
#include <QActionEvent>

#include <QtDebug>

using namespace std;

//|---------------------- StatusBar -----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// StatusBar::Constructor ////////////////////////////
StatusBar::StatusBar(QWidget *parent)
  : QToolBar(parent)
{
  m_selected = nullptr;

  setOrientation(Qt::Horizontal);
  setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  setIconSize(QSize(16, 16));
}


///////////////////////// StatusBar::statusview /////////////////////////////
QString StatusBar::statusview() const
{
  if (m_selected)
    return m_selected->text();

  return "";
}


///////////////////////// StatusBar::set_statusview /////////////////////////
void StatusBar::set_statusview(QString const &text)
{
  for(int i = 0; i < actions().size(); ++i)
  {
    QAction *action = actions().at(i);

    action->blockSignals(true);

    action->setChecked(false);

    if (action->text() == text)
    {
      action->setChecked(true);
    }

    action->blockSignals(false);
  }

  emit statusview_changed(text);
}


///////////////////////// StatusBar::actionEvent ////////////////////////////
void StatusBar::actionEvent(QActionEvent *event)
{
  QAction *action = event->action();

  if (event->type() == QEvent::ActionAdded)
  {
    action->setCheckable(true);

    connect(action, &QAction::toggled, this, [=](bool checked) { set_statusview(checked ? action->text() : ""); });
  }

  QToolBar::actionEvent(event);
}


//|---------------------- StatusBox -----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// StatusBox::Constructor ////////////////////////////
StatusBox::StatusBox(QWidget *parent)
  : CommandBar(parent)
{
}


//|---------------------- StatusReport --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// StatusReport::Constructor /////////////////////////
StatusReport::StatusReport(QWidget *parent)
  : CommandBar(parent)
{
}


//|---------------------- StatusManager -------------------------------------
//|--------------------------------------------------------------------------
//| Status View Manager
//|


///////////////////////// StatusManager::Constructor ////////////////////////
StatusManager::StatusManager(StatusBar *statusbar, StatusBox *statusbox, StatusReport *statusreport, QStackedWidget *container)
  : m_statusbar(statusbar),
    m_statusbox(statusbox),
    m_statusreport(statusreport),
    m_container(container)
{
  m_container->setVisible(false);

  connect(m_statusbar, &StatusBar::statusview_changed, this, &StatusManager::on_statusview_changed);
}


///////////////////////// StatusManager::add_statusview /////////////////////
void StatusManager::add_statusview(QAction *action)
{
  m_statusbar->addAction(action);
}


///////////////////////// StatusManager::statusview /////////////////////////
QString StatusManager::statusview() const
{
  return m_statusbar->statusview();
}


///////////////////////// StatusManager::set_statusview /////////////////////
void StatusManager::set_statusview(QString const &statusview)
{
  m_statusbar->set_statusview(statusview);
}


///////////////////////// StatusManager::container //////////////////////////
QStackedWidget *StatusManager::container()
{
  return m_container;
}


///////////////////////// StatusManager::on_statusview_changed //////////////
void StatusManager::on_statusview_changed(QString const &text)
{
  m_container->setVisible(text != "");

  emit statusview_changed(text);
}

