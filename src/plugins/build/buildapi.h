//
// Build API
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "documentapi.h"
#include <functional>

#if defined(BUILDPLUGIN)
# define BUILDPLUGIN_EXPORT Q_DECL_EXPORT
#else
# define BUILDPLUGIN_EXPORT Q_DECL_IMPORT
#endif

namespace Studio
{
  //-------------------------- BuildManager -----------------------------------
  //---------------------------------------------------------------------------

  class BUILDPLUGIN_EXPORT BuildManager : public QObject
  {
    Q_OBJECT

    public:

      virtual QString basepath() const = 0;

      template<typename Object>
      void request_build(Document *document, Object *receiver, void (Object::*notify)(Document *, QString const &))
      {
        request_build(document, receiver, [=](Document *document, QString const &path) { (receiver->*notify)(document, path); });
      }

      template<typename Object>
      void request_build(Document *document, Object *receiver, void (Object::*notify)(Document *, QString const &), void (Object::*failure)(Document *))
      {
        request_build(document, receiver, [=](Document *document, QString const &path) { (receiver->*notify)(document, path); }, [=](Document *document) { (receiver->*failure)(document); });
      }

      virtual void request_build(Document *document, QObject *receiver, std::function<void (Document *, QString const &)> const &notify, std::function<void (Document *)> const &failure = nullptr) = 0;

      virtual void register_builder(QString const &type, QObject *builder) = 0;

    public:

      virtual bool build(Studio::Document *document, QString *path) = 0;

    signals:

      void builder_added(QString const &type);

      void build_started(Document *document);
      void build_completed(Document *document);

    protected:
      virtual ~BuildManager() { }
  };
}
