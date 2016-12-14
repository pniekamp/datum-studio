//
// Console Log Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "consoleplugin.h"
#include <iostream>
#include <QLabel>
#include <QtPlugin>

#include <QDebug>

using namespace std;

namespace
{
  ConsolePlugin *instance = nullptr;

  void message_handler(QtMsgType type, QMessageLogContext const &context, QString const &msg)
  {
    cout << msg.toLocal8Bit().constData() << endl;

    if (instance)
    {
      QMetaObject::invokeMethod(instance, "log_message", Q_ARG(QtMsgType, type), Q_ARG(QString, msg));
    }
  }
}


//|---------------------- ConsolePlugin -------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ConsolePlugin::Constructor ////////////////////////
ConsolePlugin::ConsolePlugin()
{
  qRegisterMetaType<QtMsgType>("QtMsgType");
}


///////////////////////// ConsolePlugin::Destructor /////////////////////////
ConsolePlugin::~ConsolePlugin()
{
  shutdown();
}


///////////////////////// ConsolePlugin::initialise /////////////////////////
bool ConsolePlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  m_statusview = new QAction("Console Log", this);
  m_statusview->setToolTip("Console Log\nalt-9");
  m_statusview->setShortcut(QKeySequence(Qt::ALT + Qt::Key_9));

  auto statusview = actionmanager->register_action("Console.Log", m_statusview);

  auto statusmanager = Studio::Core::instance()->find_object<Studio::StatusManager>();

  statusmanager->add_statusview(9, statusview);

  connect(statusmanager, &Studio::StatusManager::statusview_changed, this, &ConsolePlugin::on_statusview_changed);

  m_container = new QWidget;

  ui.setupUi(m_container);

  ui.Header->addWidget(new QLabel("Console Log"));
  ui.Header->addSeparator();

  statusmanager->container()->addWidget(m_container);

  instance = this;
  qInstallMessageHandler(message_handler);

  return true;
}


///////////////////////// ConsolePlugin::shutdown ///////////////////////////
void ConsolePlugin::shutdown()
{
  qInstallMessageHandler(nullptr);
}


///////////////////////// ConsolePlugin::statusview_changed /////////////////
void ConsolePlugin::on_statusview_changed(QString const &view)
{
  if (view == "Console Log")
  {
    auto statusmanager = Studio::Core::instance()->find_object<Studio::StatusManager>();

    statusmanager->container()->setCurrentWidget(m_container);
  }
}


///////////////////////// ConsolePlugin::log_message ////////////////////////
void ConsolePlugin::log_message(QtMsgType type, QString const &message)
{
  switch(type)
  {
    case QtDebugMsg:
      break;

    case QtWarningMsg:
    case QtCriticalMsg:
    case QtFatalMsg:
    case QtInfoMsg:
      ui.Log->addItem(message);
      ui.Log->setCurrentRow(ui.Log->count()-1);
      break;
  }
}
