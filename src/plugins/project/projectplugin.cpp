//
// Project Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "projectplugin.h"
#include "projectmanager.h"
#include "dialogfactory.h"
#include "ui_newproject.h"
#include <QMessageBox>
#include <QProgressDialog>
#include <QFileDialog>
#include <QSettings>
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- ProjectPlugin -------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ProjectPlugin::Constructor ////////////////////////
ProjectPlugin::ProjectPlugin()
{
}


///////////////////////// ProjectPlugin::Destructor /////////////////////////
ProjectPlugin::~ProjectPlugin()
{
  shutdown();
}


///////////////////////// ProjectPlugin::initialise /////////////////////////
bool ProjectPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  Studio::Core::instance()->add_object(new ProjectManager);

  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto saveproject = new QAction(QIcon(":/projectplugin/filesave.png"), "Save Project", this);
  actionmanager->register_action("Studio.Menu.File.SaveProject", saveproject);
  actionmanager->container("Studio.Menu.File")->add_front(saveproject);

  auto openproject = new QAction(QIcon(":/projectplugin/fileopen.png"), "Open Project", this);
  actionmanager->register_action("Studio.Menu.File.OpenProject", openproject);
  actionmanager->container("Studio.Menu.File")->add_front(openproject);

  auto newproject = new QAction(QIcon(":/projectplugin/filenew.png"), "New Project", this);
  actionmanager->register_action("Studio.Menu.File.NewProject", newproject);
  actionmanager->container("Studio.Menu.File")->add_front(newproject);

  connect(saveproject, &QAction::triggered, this, &ProjectPlugin::on_SaveProject_triggered);
  connect(openproject, &QAction::triggered, this, &ProjectPlugin::on_OpenProject_triggered);
  connect(newproject, &QAction::triggered, this, &ProjectPlugin::on_NewProject_triggered);

  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();

  connect(mainwindow, &Studio::MainWindow::closing, this, &ProjectPlugin::on_mainwindow_closing);

  return true;
}


///////////////////////// ProjectPlugin::shutdown ///////////////////////////
void ProjectPlugin::shutdown()
{
}


///////////////////////// ProjectPlugin::mainwindow_closing /////////////////
void ProjectPlugin::on_mainwindow_closing(bool *cancel)
{
  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  if (!projectmanager->close_project())
  {
    *cancel = true;
  }
}


///////////////////////// ProjectPlugin::NewProject /////////////////////////
void ProjectPlugin::on_NewProject_triggered()
{
  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();
  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  DialogFactory<Ui::NewProject> dlg(mainwindow->handle());

  dlg.ui.ProjectLocation->set_browsetype(QcFileLineEdit::BrowseType::Directory);

  dlg.ui.ProjectLocation->setText(QSettings().value("project/location").toString());

  if (dlg.exec() == QDialog::Accepted && projectmanager->close_project())
  {
    try
    {
      QProgressDialog progress("New Project", "Abort", 0, 100, mainwindow->handle());

      projectmanager->create_project(dlg.ui.ProjectName->text(), dlg.ui.ProjectLocation->text(), &progress);

      Studio::Core::instance()->find_object<Studio::ModeManager>()->set_metamode("Pack");
    }
    catch(exception &e)
    {
      QMessageBox::information(mainwindow->handle(), "Project Error", e.what());
    }

    QSettings().setValue("project/location", dlg.ui.ProjectLocation->text());
  }
}


///////////////////////// ProjectPlugin::OpenProject ////////////////////////
void ProjectPlugin::on_OpenProject_triggered()
{
  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();
  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  QString projectfile = QFileDialog::getOpenFileName(mainwindow->handle(), "Open Project", "", "Projects (*.project)");

  if (projectfile != "" && projectmanager->close_project())
  {
    try
    {
      QProgressDialog progress("Open Project", "Abort", 0, 100, mainwindow->handle());

      projectmanager->open_project(projectfile, &progress);

      Studio::Core::instance()->find_object<Studio::ModeManager>()->set_metamode("Pack");
    }
    catch(exception &e)
    {
      QMessageBox::information(mainwindow->handle(), "Project Error", e.what());
    }
  }
}


///////////////////////// ProjectPlugin::SaveProject ////////////////////////
void ProjectPlugin::on_SaveProject_triggered()
{
  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();
  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  try
  {
    QProgressDialog progress("Save Project", "Abort", 0, 100, mainwindow->handle());

    projectmanager->save_project(&progress);
  }
  catch(exception &e)
  {
    QMessageBox::information(mainwindow->handle(), "Project Error", e.what());
  }
}
