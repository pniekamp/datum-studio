//
// Content API
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"

#if defined(CONTENTPLUGIN)
# define CONTENTPLUGIN_EXPORT Q_DECL_EXPORT
#else
# define CONTENTPLUGIN_EXPORT Q_DECL_IMPORT
#endif

namespace Studio
{

  //-------------------------- ContentManager ---------------------------------
  //---------------------------------------------------------------------------

  class CONTENTPLUGIN_EXPORT ContentManager : public QObject
  {
    Q_OBJECT

    public:

      virtual QString basepath() const = 0;

      virtual bool create(QString const &type, QString const &path) = 0;

      virtual bool import(QString const &src, QString const &dst) = 0;

      virtual bool reimport(QString const &path) = 0;

      virtual bool rename_content(QString const &src, QString const &dst) = 0;

      virtual bool delete_content(QString const &path) = 0;

      virtual void register_creator(QString const &type, QObject *creator) = 0;
      virtual void register_importer(QString const &type, QObject *importer) = 0;

    signals:

      void content_changed(QString const &path);

      void creator_added(QString const &type);
      void importer_added(QString const &type);

    protected:
      virtual ~ContentManager() { }
  };
}
