//
// Build Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "buildapi.h"
#include <leap/threadcontrol.h>
#include <QDir>
#include <QUuid>
#include <QThreadPool>

class BuildManager;

//-------------------------- Builder ----------------------------------------
//---------------------------------------------------------------------------

class Builder : public QObject, public QRunnable
{
  Q_OBJECT

  public:
    Builder(BuildManager *manager, Studio::Document *document);

    void run();

  signals:

    void build_failure(Studio::Document *document);

    void build_complete(Studio::Document *document, QString const &path);

  private:

    BuildManager *m_manager;

    unique_document m_document;
};


//-------------------------- BuildManager -----------------------------------
//---------------------------------------------------------------------------

class BuildManager : public Studio::BuildManager
{
  Q_OBJECT

  public:
    BuildManager();

    QString basepath() const;

    void request_build(Studio::Document *document, QObject *receiver, std::function<void (Studio::Document *, QString const &)> const &notify, std::function<void (Studio::Document *)> const &failure = nullptr);

    void register_builder(QString const &type, QObject *builder);

  public:

    bool build(Studio::Document *document, QString *path);

  protected:

    void on_project_changed(QString const &projectfile);

    void on_project_closing(bool *cancel);

    void on_document_renamed(Studio::Document *document, QString const &src, QString const &dst);

  private:

    QDir m_path;

    struct Build
    {
      QUuid id;
      QString doc;
      size_t hash;
    };

    std::vector<Build> m_builds;

    QUuid find_build(QString const &doc, size_t key) const;

    std::vector<Studio::Document*> m_pending;

    QMap<QString, QObject*> m_builders;

    mutable leap::threadlib::CriticalSection m_mutex;
};
