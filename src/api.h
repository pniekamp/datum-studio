//
// Datum Studio API
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

#if defined(DATUMSTUDIO)
# define STUDIO_EXPORT Q_DECL_EXPORT
#else
# define STUDIO_EXPORT Q_DECL_IMPORT
#endif

namespace Studio
{
  static const char *ApiBuild = "0.1";

  enum Constants
  {
  };


  //-------------------------- Plugin -----------------------------------------
  //---------------------------------------------------------------------------

  class STUDIO_EXPORT Plugin : public QObject
  {
    Q_OBJECT

    public:
      virtual ~Plugin() { }

      virtual const char *build() const { return ApiBuild; }

      virtual bool initialise(QStringList const &arguments, QString *errormsg) = 0;

      virtual void shutdown() = 0;
  };


  //-------------------------- ActionContainer --------------------------------
  //---------------------------------------------------------------------------

  class STUDIO_EXPORT ActionContainer : public QObject
  {
    Q_OBJECT

    public:
      virtual ~ActionContainer() { }

      virtual void add_back(QAction *action) = 0;
      virtual void add_front(QAction *action) = 0;

      virtual void add_back(ActionContainer *menu) = 0;
  };


  //-------------------------- ActionManager ----------------------------------
  //---------------------------------------------------------------------------

  class STUDIO_EXPORT ActionManager : public QObject
  {
    Q_OBJECT

    public:
      virtual ~ActionManager() { }

      virtual QAction *action(QString const &id) = 0;

      virtual QAction *register_action(QString const &id, QAction *action) = 0;

      virtual ActionContainer *container(QString const &id) = 0;

      virtual ActionContainer *register_container(QString const &id, QMenu *menu) = 0;
      virtual ActionContainer *register_container(QString const &id, QMenuBar *menubar) = 0;
      virtual ActionContainer *register_container(QString const &id, QToolBar *toolbar) = 0;
  };


  //-------------------------- MainWindow -------------------------------------
  //---------------------------------------------------------------------------

  class STUDIO_EXPORT MainWindow : public QObject
  {
    Q_OBJECT

    public:
      virtual ~MainWindow() { }

      virtual QWidget *handle() = 0;

    signals:

      void activated();

      void closing(bool *cancel);
  };


  //-------------------------- ModeManager ------------------------------------
  //---------------------------------------------------------------------------

  class STUDIO_EXPORT ModeManager : public QObject
  {
    Q_OBJECT

    public:
      virtual ~ModeManager() { }

      virtual void add_metamode(int index, QAction *action) = 0;

      virtual QString metamode() const = 0;
      virtual void set_metamode(QString const &metamode) = 0;

      virtual QStackedWidget *container() = 0;

    signals:

      void metamode_changed(QString const &metamode);
  };


  //-------------------------- StatusManager ----------------------------------
  //---------------------------------------------------------------------------

  class STUDIO_EXPORT StatusManager : public QObject
  {
    Q_OBJECT

    public:
      virtual ~StatusManager() { }

      virtual void add_statusview(int index, QAction *action) = 0;

      virtual QString statusview() const = 0;
      virtual void set_statusview(QString const &statusview) = 0;

      virtual QStackedWidget *container() = 0;

    signals:

      void statusview_changed(QString const &statusview);
  };


  //-------------------------- ViewFactory ------------------------------------
  //---------------------------------------------------------------------------

  class STUDIO_EXPORT ViewFactory : public QObject
  {
    Q_OBJECT

    public:
      virtual ~ViewFactory() { }

      virtual QWidget *create_view(QString const &type) = 0;

      virtual void register_factory(QString const &type, QObject *factory) = 0;
  };


  //-------------------------- Core -------------------------------------------
  //---------------------------------------------------------------------------

  class STUDIO_EXPORT Core : public QObject
  {
    Q_OBJECT

    public:

      static Core *instance();

    public:
      virtual ~Core() { }

      virtual QList<QObject*> objects() const = 0;

      virtual void add_object(QObject *object) = 0;

    public:

      template <typename T>
      QList<T*> find_objects() const
      {
        QList<T*> result;

        for(auto &obj : objects())
        {
          if (qobject_cast<T*>(obj))
            result.push_back(qobject_cast<T*>(obj));
        }

        return result;
      }

      template <typename T>
      T *find_object() const
      {
        for(auto &obj : objects())
        {
          if (qobject_cast<T*>(obj))
            return qobject_cast<T*>(obj);
        }

        return nullptr;
      }
  };

}

Q_DECLARE_INTERFACE(Studio::Plugin, "studio.plugin/1.0")
