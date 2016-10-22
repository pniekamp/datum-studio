//
// Pack Model
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include <vector>
#include <memory>
#include <QObject>

//-------------------------- PackModel --------------------------------------
//---------------------------------------------------------------------------

class PackModel : public QObject
{
  Q_OBJECT

  public:

    class Asset
    {
      public:
        Asset(QString const &path);

        QString path() const { return m_path; }

        Studio::Document *document() const { return m_document; }

      protected:

        QString m_path;
        unique_document m_document;

        friend class PackModel;
    };

  public:
    PackModel(QObject *parent = 0);

    void clear();

    bool modified() const { return m_modified; }

    void add_asset(size_t position, QString const &path);

    void move_asset(size_t index, size_t position);

    void erase_asset(size_t index);

  public:

    typedef std::vector<std::unique_ptr<Asset>>::iterator iterator;
    typedef std::vector<std::unique_ptr<Asset>>::const_iterator const_iterator;

    iterator begin() { return m_assets.begin(); }
    iterator end() { return m_assets.end(); }

    const_iterator begin() const { return m_assets.begin(); }
    const_iterator end() const { return m_assets.end(); }

  public:

    void load(std::string const &projectfile);
    void save(std::string const &projectfile);

  signals:

    void reset();
    void added(size_t index, PackModel::Asset *asset);
    void changed(size_t index, PackModel::Asset *asset);
    void removed(size_t index, PackModel::Asset *asset);

  protected:

    void on_document_renamed(Studio::Document *document, QString const &src, QString const &dst);

  private:

    bool m_modified;

    std::vector<std::unique_ptr<Asset>> m_assets;
};
