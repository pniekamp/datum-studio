//
// Project Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "projectmanager.h"
#include <QDir>
#include <QSettings>
#include <fstream>

#include <QtDebug>

using namespace std;

namespace
{
  void add_recent_projects(QString const &path)
  {
    QSettings reg;

    auto projects = reg.value("project/recent").toStringList();

    projects.removeOne(path);

    projects.push_front(path);

    projects = projects.mid(0, 12);

    reg.setValue("project/recent", projects);
  }
}


//|---------------------- ProjectManager ------------------------------------
//|--------------------------------------------------------------------------
//| Project Manager
//|

///////////////////////// ProjectManager::Constructor ///////////////////////
ProjectManager::ProjectManager()
{
}


///////////////////////// ProjectManager::basepath //////////////////////////
QString ProjectManager::basepath() const
{
  return m_basepath;
}


///////////////////////// ProjectManager::create_project ////////////////////
void ProjectManager::create_project(QString const &name, QString const &location, QProgressDialog *progress)
{
  QDir base(location);

  if (!base.mkdir(name))
    throw runtime_error("Unable to create project path");

  if (!base.mkpath(name + "/Content" ))
    throw runtime_error("Unable to create content path");

  m_name = name;
  m_basepath = base.absolutePath() + "/" + m_name;
  m_projectfile = m_basepath + "/" + m_name + ".project";  

  save_project(progress);

  add_recent_projects(m_projectfile);

  emit project_changed(m_projectfile);
}


///////////////////////// ProjectManager::open_project //////////////////////
void ProjectManager::open_project(QString const &projectfile, QProgressDialog *progress)
{
  m_name = QFileInfo(projectfile).completeBaseName();
  m_basepath = QFileInfo(projectfile).dir().absolutePath();
  m_projectfile = projectfile; 

  add_recent_projects(m_projectfile);

  emit project_changed(m_projectfile);
}


///////////////////////// ProjectManager::save_project //////////////////////
void ProjectManager::save_project(QProgressDialog *progress)
{
  ofstream fout(m_projectfile.toUtf8());

  fout << "[Datum Studio]" << '\n';
  fout << "version = 1.0" << '\n';
  fout << '\n';

  fout.close();

  emit project_saving(m_projectfile);
}


///////////////////////// ProjectManager::close_project /////////////////////
bool ProjectManager::close_project()
{
  bool cancel = false;

  if (m_basepath != "")
  {
    emit project_closing(&cancel);

    if (!cancel)
    {
      emit project_closed();
    }
  }

  return !cancel;
}
