//
// Document API
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include <QVariant>
#include <QJsonObject>

#if defined(DOCUMENTPLUGIN)
# define DOCUMENTPLUGIN_EXPORT Q_DECL_EXPORT
#else
# define DOCUMENTPLUGIN_EXPORT Q_DECL_IMPORT
#endif

namespace Studio
{
  //-------------------------- Document ---------------------------------------
  //---------------------------------------------------------------------------

  class DOCUMENTPLUGIN_EXPORT Document : public QObject
  {
    Q_OBJECT

    public:

      virtual QString type() const = 0;

      virtual QIcon icon() const = 0;

      virtual void set_icon(QIcon const &icon) = 0;

      virtual QJsonObject metadata() const = 0;

      virtual QVariant metadata(QString const &name) const = 0;

      virtual void set_metadata(QString const &name, QVariant const &value) = 0;

      virtual bool modified() const = 0;

      virtual size_t read(uint64_t position, void *buffer, size_t bytes) = 0;

      virtual size_t write(uint64_t position, void const *buffer, size_t bytes) = 0;

      virtual void discard() = 0;

      virtual void save() = 0;

    public:

      virtual void lock() = 0;
      virtual void unlock() = 0;

      virtual void lock_exclusive() = 0;
      virtual void unlock_exclusive(bool changed = true) = 0;

    public:

      template<typename T>
      T metadata(QString const &name, T const &defvalue) const
      {
        QVariant result = this->metadata(name);

        if (result.canConvert<T>())
          return result.value<T>();

        return defvalue;
      }

    signals:

      void document_changed();

    protected:
      virtual ~Document() { }
  };


  //-------------------------- DocumentManager --------------------------------
  //---------------------------------------------------------------------------

  class DOCUMENTPLUGIN_EXPORT DocumentManager : public QObject
  {
    Q_OBJECT

    public:

      virtual Document *open(QString const &path) = 0;
      virtual Document *create(QString const &path) = 0;
      virtual Document *dup(Document *document) = 0;

      virtual void close(Document *document) = 0;

      virtual QString path(Document *document) const = 0;

      virtual bool rename(Document *document, QString const &dst) = 0;

      virtual bool rewrite(Document *document, QString const &src) = 0;

    signals:

      void document_changed(Document *document, QString const &path);

      void document_renamed(Document *document, QString const &src, QString const &dst);

    protected:
      virtual ~DocumentManager() { }
  };
}

// unique_document

class unique_document
{
  public:
    unique_document(Studio::Document *document = nullptr) noexcept
      : m_document(document)
    {
    }

    unique_document(unique_document const &) = delete;

    unique_document(unique_document &&other) noexcept
      : unique_document()
    {
      std::swap(m_document, other.m_document);
    }

    ~unique_document() noexcept
    {     
      if (m_document)
      {
        auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

        documentmanager->close(m_document);
      }
    }

    unique_document &operator=(unique_document &&other) noexcept
    {
      std::swap(m_document, other.m_document);

      return *this;
    }

    Studio::Document *get() const noexcept { return m_document; }
    Studio::Document *operator *() const noexcept { return m_document; }
    Studio::Document *operator ->() const noexcept { return m_document; }

    operator Studio::Document *() const noexcept { return m_document; }

  private:

    Studio::Document *m_document;
};
