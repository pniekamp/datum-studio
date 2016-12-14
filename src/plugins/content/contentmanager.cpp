//
// Content Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "contentmanager.h"
#include "projectapi.h"
#include "documentapi.h"
#include <QDir>

#include <QtDebug>

using namespace std;

//|---------------------- ContentManager ------------------------------------
//|--------------------------------------------------------------------------
//| Content Manager
//|

///////////////////////// ContentManager::Constructor ///////////////////////
ContentManager::ContentManager()
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_changed, this, &ContentManager::on_document_changed);
  connect(documentmanager, &Studio::DocumentManager::document_renamed, this, &ContentManager::on_document_renamed);
}


///////////////////////// ContentManager::document_changed //////////////////
void ContentManager::on_document_changed(Studio::Document *document, QString const &path)
{
  emit content_changed(path);
}


///////////////////////// ContentManager::document_renamed //////////////////
void ContentManager::on_document_renamed(Studio::Document *document, QString const &src, QString const &dst)
{
  emit content_changed(src);
  emit content_changed(dst);
}


///////////////////////// ContentManager::basepath //////////////////////////
QString ContentManager::basepath() const
{
  auto projectmanager = Studio::Core::instance()->find_object<Studio::ProjectManager>();

  return projectmanager->basepath() + "/Content";
}


///////////////////////// ContentManager::create_folder /////////////////////
bool ContentManager::create(QString const &type, QString const &path)
{
  if (type == "Folder")
  {
    if (QDir().mkpath(path))
    {
      emit content_changed(path);

      return true;
    }
  }

  bool result = false;

  QObject *creator = m_creators[type];

  if (creator)
  {
    try
    {
      QMetaObject::invokeMethod(creator, "create", Q_RETURN_ARG(bool, result), Q_ARG(QString, type), Q_ARG(QString, path), Q_ARG(QJsonObject, QJsonObject()));
    }
    catch(exception &e)
    {
      qCritical() << "Create Error:" << e.what();
    }
  }

  if (result)
  {
    emit content_changed(path);
  }

  return result;
}


///////////////////////// ContentManager::import ////////////////////////////
bool ContentManager::import(QString const &src, QString const &dst)
{
  if (QDir(dst).absolutePath().left(QFileInfo(src).absoluteFilePath().length()) == QFileInfo(src).absoluteFilePath())
    return false;

  if (QFileInfo(src).isDir())
  {
    QString path = QDir(dst).filePath(QFileInfo(src).fileName());

    for(int k = 1; k < 256 && QFileInfo(path).exists(); ++k)
    {
      path = QDir(dst).filePath(QFileInfo(src).fileName() + "_" + QString::number(k));
    }

    if (!create("Folder", path))
      return false;

    for(auto &entry : QDir(src).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
      import(entry.filePath(), path);
    }

    return true;
  }

  bool result = false;

  QString path = QDir(dst).filePath(QFileInfo(src).completeBaseName() + ".asset");

  for(int k = 1; k < 256 && QFileInfo(path).exists(); ++k)
  {
    path = QDir(dst).filePath(QFileInfo(src).completeBaseName() + "_" + QString::number(k) + ".asset");
  }

  if (QFileInfo(src).suffix() == "asset")
  {
    QFile::copy(src, path);

    result = true;
  }

  try
  {
    for(auto &importer : m_importers)
    {
      if (!result)
      {
        QMetaObject::invokeMethod(importer, "try_import", Q_RETURN_ARG(bool, result), Q_ARG(QString, src), Q_ARG(QString, path), Q_ARG(QJsonObject, QJsonObject()));
      }
    }

    if (result)
    {
      emit content_changed(path);
    }
  }
  catch(exception &e)
  {
    qCritical() << "Import Error:" << e.what();

    QFile::remove(path);
  }

  return result;
}


///////////////////////// ContentManager::reimport //////////////////////////
bool ContentManager::reimport(QString const &path)
{
  if (QFileInfo(path).isDir())
  {
    for(auto &entry : QDir(path).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
      reimport(entry.filePath());
    }

    return true;
  }

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  if (auto document = documentmanager->open(path))
  {
    QString src = document->metadata("src").toString();
    QString dst = path + ".tmp";

    bool result = false;

    if (src != "")
    {
      try
      {
        for(auto &importer : m_importers)
        {
          if (!result)
          {
            QMetaObject::invokeMethod(importer, "try_import", Q_RETURN_ARG(bool, result), Q_ARG(QString, src), Q_ARG(QString, dst), Q_ARG(QJsonObject, document->metadata()));
          }
        }

        if (result)
        {
          documentmanager->rewrite(document, dst);
        }
      }
      catch(exception &e)
      {
        qCritical() << "Import Error:" << e.what();

        QFile::remove(dst);
      }
    }

    documentmanager->close(document);
  }

  return true;
}


///////////////////////// ContentManager::rename_content ////////////////////
bool ContentManager::rename_content(QString const &src, QString const &dst)
{
  if (QFileInfo(dst).exists())
    return false;

  if (QFileInfo(src).absoluteFilePath() == QFileInfo(dst).absoluteFilePath())
    return false;

  if (QFileInfo(dst).dir().absolutePath().left(QFileInfo(src).absoluteFilePath().length()) == QFileInfo(src).absoluteFilePath())
    return false;

  if (QFileInfo(src).isDir())
  {
    if (!create("Folder", dst))
      return false;

    for(auto &entry : QDir(src).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
      rename_content(entry.filePath(), QDir(dst).filePath(entry.fileName()));
    }

    delete_content(src);

    return true;
  }

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  if (auto document = documentmanager->open(src))
  {
    documentmanager->rename(document, dst);

    documentmanager->close(document);
  }

  return true;
}


///////////////////////// ContentManager::delete_content ////////////////////
bool ContentManager::delete_content(QString const &path)
{
  if (QFileInfo(path).isDir())
  {
    bool result = true;

    for(auto &entry : QDir(path).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
      result &= delete_content(entry.filePath());
    }

    if (result)
    {
      QDir(path).removeRecursively();
    }

    emit content_changed(path);

    return true;
  }

  if (QFile::remove(path))
  {
    emit content_changed(path);

    return true;
  }

  return false;
}


///////////////////////// ContentManager::register_creator //////////////////
void ContentManager::register_creator(QString const &type, QObject *creator)
{
  m_creators[type] = creator;

  emit creator_added(type);
}


///////////////////////// ContentManager::register_importer /////////////////
void ContentManager::register_importer(QString const &type, QObject *importer)
{
  m_importers.push_back(importer);

  emit importer_added(type);
}
