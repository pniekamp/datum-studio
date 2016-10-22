//
// Project Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "projectapi.h"

//-------------------------- ProjectManager ---------------------------------
//---------------------------------------------------------------------------

class ProjectManager : public Studio::ProjectManager
{
  Q_OBJECT

  public:
    ProjectManager();

    QString basepath() const;

  public:

    void create_project(QString const &name, QString const &location, QProgressDialog *progress);

    void open_project(QString const &projectfile, QProgressDialog *progress);

    void save_project(QProgressDialog *progress);

    bool close_project();

  private:

    QString m_name;
    QString m_basepath;
    QString m_projectfile;
};
