//
// Content Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "contentapi.h"
#include "documentapi.h"

//-------------------------- ContentManager ---------------------------------
//---------------------------------------------------------------------------

class ContentManager : public Studio::ContentManager
{
  Q_OBJECT

  public:
    ContentManager();

    QString basepath() const;

    bool create(QString const &type, QString const &path);

    bool import(QString const &src, QString const &dst);

    bool reimport(QString const &path);

    bool rename_content(QString const &src, QString const &dst);

    bool delete_content(QString const &path);

    void register_creator(QString const &type, QObject *creator);
    void register_importer(QString const &type, QObject *importer);

  protected:

    void on_document_changed(Studio::Document *document, QString const &path);
    void on_document_renamed(Studio::Document *document, QString const &src, QString const &dst);

  private:

    QMap<QString, QObject*> m_creators;

    QVector<QObject*> m_importers;
};
