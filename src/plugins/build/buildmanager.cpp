//
// Build Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "buildmanager.h"
#include "projectapi.h"
#include <leap.h>
#include <fstream>

#include <QtDebug>

using namespace std;
using namespace leap;
using namespace leap::threadlib;

//|---------------------- Builder -------------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// Builder::Constructor //////////////////////////////
Builder::Builder(BuildManager *manager, Studio::Document *document)
  : m_manager(manager)
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  m_document = documentmanager->dup(document);

  setAutoDelete(true);
}


///////////////////////// Builder::run //////////////////////////////////////
void Builder::run()
{
  QString path;

  if (m_manager->build(m_document, &path))
  {
    emit build_complete(m_document, path);
  }
  else
  {
    emit build_failure(m_document);
  }
}



//|---------------------- BuildManager --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// BuildManager::Constructor /////////////////////////
BuildManager::BuildManager()
{
  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  connect(projectmanager, &Studio::ProjectManager::project_changed, this, &BuildManager::on_project_changed);
  connect(projectmanager, &Studio::ProjectManager::project_closing, this, &BuildManager::on_project_closing);

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_renamed, this, &BuildManager::on_document_renamed);
}


///////////////////////// BuildManager::basepath ////////////////////////////
QString BuildManager::basepath() const
{
  SyncLock lock(m_mutex);

  return m_path.filePath("Build");
}


///////////////////////// BuildManager::on_project_changed //////////////////
void BuildManager::on_project_changed(QString const &projectfile)
{
  SyncLock lock(m_mutex);

  m_path = QFileInfo(projectfile).dir();

  m_path.mkdir("Build");

  m_builds.clear();

  ifstream fin(m_path.filePath("Build/buildstate.dat").toUtf8());

  string buffer;

  while (getline(fin, buffer))
  {
    auto line = trim(buffer);

    if (line == "[Builds]")
      break;
  }

  while (getline(fin, buffer))
  {
    auto line = trim(buffer);

    if (line.empty())
      break;

    if (line[0] == '#' || line[0] == '/')
      continue;

    auto i = line.find_first_of(' ');
    auto j = line.find_first_of(' ', i+1);

    QUuid id = line.substr(0, i).to_string().c_str();
    size_t hash = stoull(line.substr(i+1, j).to_string());
    QString file = m_path.filePath(line.substr(j+1).to_string().c_str());

    m_builds.push_back({ id, file, hash });
  }
}


///////////////////////// BuildManager::on_project_closing //////////////////
void BuildManager::on_project_closing(bool *cancel)
{
  SyncLock lock(m_mutex);

  ofstream fout(m_path.filePath("Build/buildstate.dat").toUtf8());

  fout << "[Builds]" << '\n';

  for(auto &build : m_builds)
  {
    fout << build.id.toString().toStdString() << " " << build.hash << " " << m_path.relativeFilePath(build.file).toStdString() << '\n';
  }

  fout << '\n';
}


///////////////////////// BuildManager::on_document_renamed /////////////////
void BuildManager::on_document_renamed(Studio::Document *document, QString const &src, QString const &dst)
{
  SyncLock lock(m_mutex);

  for(auto &build : m_builds)
  {
    if (build.file == src)
    {
      build.file = dst;
    }
  }
}


///////////////////////// BuildManager::request_build ///////////////////////
void BuildManager::request_build(Studio::Document *document, QObject *receiver, std::function<void (Studio::Document *, QString const &)> const &notify, std::function<void (Studio::Document *)> const &failure)
{
  SyncLock lock(m_mutex);

  auto builder = new Builder(this, document);

  connect(builder, &Builder::build_complete, receiver, notify, Qt::QueuedConnection);

  if (failure)
  {
    connect(builder, &Builder::build_failure, receiver, failure, Qt::QueuedConnection);
  }

  QThreadPool::globalInstance()->start(builder);
}


///////////////////////// BuildManager::register_builder ////////////////////
void BuildManager::register_builder(QString const &type, QObject *builder)
{
  m_builders[type] = builder;

  emit builder_added(type);
}


///////////////////////// BuildManager::find_build //////////////////////////
QUuid BuildManager::find_build(QString const &file, size_t hash) const
{
  SyncLock lock(m_mutex);

  for(auto &build : m_builds)
  {
    if (build.file == file && build.hash == hash)
    {
      return build.id;
    }
  }

  return QUuid();
}


///////////////////////// BuildManager::build ///////////////////////////////
bool BuildManager::build(Studio::Document *document, QString *path)
{
  while(true)
  {
    {
      SyncLock lock(m_mutex);

      if (find(m_pending.begin(), m_pending.end(), document) == m_pending.end())
      {
        m_pending.push_back(document);

        break;
      }
    }

    QThread::msleep(250);
  }

  bool result = false;

  QObject *builder = m_builders[document->metadata("type").toString()];

  if (builder)
  {
    size_t hash = 0;

    try
    {
      QMetaObject::invokeMethod(builder, "hash", Qt::DirectConnection, Q_ARG(Studio::Document*, document), Q_ARG(size_t*, &hash));
    }
    catch(exception &e)
    {
      qDebug() << "Hash Error:" << e.what();
    }

    document->lock();

    QString file = Studio::Core::instance()->find_object<Studio::DocumentManager>()->path(document);

    auto id = find_build(file, hash);

    if (id.isNull())
    {
      SyncLock lock(m_mutex);

      id = QUuid::createUuid();

      m_builds.push_back({ id, file, hash });
    }

    document->unlock();

    *path = basepath() + "/" + id.toString().mid(1, 36);

    if (QFile::exists(*path))
    {
      result = true;
    }

    if (!result)
    {
      qInfo() << "Building" << file;

      emit build_started(document);

      try
      {
        QMetaObject::invokeMethod(builder, "build", Qt::DirectConnection, Q_RETURN_ARG(bool, result), Q_ARG(Studio::Document*, document), Q_ARG(QString, *path + ".tmp"));

        QFile::rename(*path + ".tmp", *path);
      }
      catch(exception &e)
      {
        qCritical() << "Build Error:" << e.what();

        QFile::remove(*path + ".tmp");
      }

      emit build_completed(document);
    }
  }

  {
    SyncLock lock(m_mutex);

    m_pending.erase(find(m_pending.begin(), m_pending.end(), document));
  }

  return result;
}
