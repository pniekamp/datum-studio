//
// Home Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "homeplugin.h"
#include "projectapi.h"
#include <QFileInfo>
#include <QSettings>
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- HomePlugin ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// HomePlugin::Constructor ///////////////////////////
HomePlugin::HomePlugin()
{
}


///////////////////////// HomePlugin::Destructor ////////////////////////////
HomePlugin::~HomePlugin()
{
  shutdown();
}


///////////////////////// HomePlugin::initialise ////////////////////////////
bool HomePlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  QIcon icon;
  icon.addFile(":/homeplugin/logo.png", QSize(), QIcon::Normal, QIcon::Off);
  icon.addFile(":/homeplugin/logo-selected.png", QSize(), QIcon::Normal, QIcon::On);

  m_metamode = new QAction(icon, "Home", this);
  m_metamode->setToolTip("Switch to Home\nctrl-1");
  m_metamode->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));

  auto metamode = actionmanager->register_action("Home.MetaMode", m_metamode);

  auto modemanager = Studio::Core::instance()->find_object<Studio::ModeManager>();

  modemanager->add_metamode(1, metamode);

  connect(modemanager, &Studio::ModeManager::metamode_changed, this, &HomePlugin::on_metamode_changed);

  m_container = new QWidget;

  ui.setupUi(m_container);

  connect(ui.NewProject, &QPushButton::clicked, this, &HomePlugin::on_NewProject_triggered);
  connect(ui.RecentProjects, &QLabel::linkActivated, this, &HomePlugin::on_OpenProject_triggered);

  modemanager->container()->addWidget(m_container);

  modemanager->set_metamode("Home");

  return true;
}


///////////////////////// HomePlugin::shutdown //////////////////////////////
void HomePlugin::shutdown()
{
}


///////////////////////// HomePlugin::metamode_changed //////////////////////
void HomePlugin::on_metamode_changed(QString const &mode)
{
  if (mode == "Home")
  {
    auto modemanager = Studio::Core::instance()->find_object<Studio::ModeManager>();

    modemanager->container()->setCurrentWidget(m_container);

    QString txt;

    foreach(QString recentproject, QSettings().value("project/recent").toStringList())
    {
      QString title = QFileInfo(recentproject).completeBaseName().toHtmlEscaped();
      QString path = recentproject.toHtmlEscaped();

      txt += "<p style='line-height: 125%;'>";
      txt += "<a href='" + path + "'><span style='font-size:10pt; color:#0000ff;'>" + title + "</span></a><br>";
      txt += "<span style='font-size:8pt; color:#606060;'>" + path + "</span>";
      txt += "</p>";
    }

    ui.RecentProjects->setText(txt);
  }
}


///////////////////////// HomePlugin::NewProject ////////////////////////////
void HomePlugin::on_NewProject_triggered()
{
  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  actionmanager->action("Studio.Menu.File.NewProject")->trigger();
}


///////////////////////// HomePlugin::OpenProject ///////////////////////////
void HomePlugin::on_OpenProject_triggered(QString const &link)
{
  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  QProgressDialog progress(m_container);

  projectmanager->open_project(link, &progress);

  Studio::Core::instance()->find_object<Studio::ModeManager>()->set_metamode("Pack");
}

