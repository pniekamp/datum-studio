//
// Document Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "documentapi.h"
#include <leap/threadcontrol.h>
#include <fstream>

//-------------------------- Document ---------------------------------------
//---------------------------------------------------------------------------
// read functions require lock
// write functions require lock_exclusive
// const functions are thread safe

class Document : public Studio::Document
{
  Q_OBJECT

  public:
    Document(QString const &path);

    QString type() const;

    QIcon icon() const;

    void set_icon(QIcon const &icon);

    QJsonObject metadata() const;

    QVariant metadata(QString const &name) const;

    void set_metadata(QString const &name, QVariant const &value);

    bool modified() const;

    size_t read(uint64_t position, void *buffer, size_t bytes);

    size_t write(uint64_t position, void const *buffer, size_t bytes);

    void discard();

    void save();

  public:

    void lock();
    void unlock();

    void lock_exclusive();
    void unlock_exclusive(bool changed = true);

  public:

    void attach(QString const &path);
    void detach();

  private:

    bool m_modified;

    QJsonObject m_metadata;

  private:

    static constexpr int MaxCacheBlocks = 64;

    struct Block
    {
      int index;
      bool modified;

      size_t size;
      uint8_t data[4096];

      std::list<Block*>::iterator lrunode;
    };

    std::list<Block*> m_lru;
    std::map<size_t, Block> m_blocks;

#ifndef NDEBUG
    std::atomic<int> m_locked{0};
    std::atomic<int> m_exclusive{0};
#endif

    std::fstream m_file;

    leap::threadlib::ReadWriteLock m_lock;

    mutable leap::threadlib::SpinLock m_mutex;
};


//-------------------------- DocumentManager --------------------------------
//---------------------------------------------------------------------------
// functions are thread safe

class DocumentManager : public Studio::DocumentManager
{
  Q_OBJECT

  public:
    DocumentManager();

    Document *open(QString const &path);
    Document *create(QString const &path);
    Document *dup(Studio::Document *document);

    void close(Studio::Document *document);

    QString path(Studio::Document *document) const;

    bool rename(Studio::Document *document, QString const &dst);

    bool rewrite(Studio::Document *document, QString const &src);

  private:

    struct DocInfo
    {
      QString path;

      Document *document;

      int refcount;
    };

    std::vector<DocInfo> m_documents;

    mutable leap::threadlib::CriticalSection m_mutex;
};
