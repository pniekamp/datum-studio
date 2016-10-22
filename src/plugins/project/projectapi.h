//
// Project API
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include <QtGlobal>
#include <QObject>
#include <QAction>
#include <QStringList>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStackedWidget>
#include <QProgressDialog>

#if defined(PROJECTPLUGIN)
# define PROJECTPLUGIN_EXPORT Q_DECL_EXPORT
#else
# define PROJECTPLUGIN_EXPORT Q_DECL_IMPORT
#endif

namespace Studio
{
  //-------------------------- ProjectManager ---------------------------------
  //---------------------------------------------------------------------------

  class PROJECTPLUGIN_EXPORT ProjectManager : public QObject
  {
    Q_OBJECT

    public:

      virtual QString basepath() const = 0;

    public:

      virtual void create_project(QString const &name, QString const &location, QProgressDialog *progress) = 0;

      virtual void open_project(QString const &projectfile, QProgressDialog *progress) = 0;

      virtual void save_project(QProgressDialog *progress) = 0;

      virtual bool close_project() = 0;

    signals:

      void project_changed(QString const &projectfile);

      void project_closing(bool *cancel);

      void project_saving(QString const &projectfile);

      void project_closed();

    protected:
      virtual ~ProjectManager() { }
  };

}
