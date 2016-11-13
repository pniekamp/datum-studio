//
// Mesh Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "datum/math.h"
#include "packapi.h"
#include <string>

//-------------------------- MeshDocument -----------------------------------
//---------------------------------------------------------------------------

class MeshDocument : public QObject
{
  Q_OBJECT

  public:

    static void hash(Studio::Document *document, size_t *key);

    static void pack(Studio::PackerState &asset, std::ofstream &fout);

  public:
    MeshDocument();
    MeshDocument(QString const &path);
    MeshDocument(Studio::Document *document);
    MeshDocument(MeshDocument const &document);
    MeshDocument operator =(MeshDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    int meshcount() const;
    int materialcount() const;

    struct Instance
    {
      size_t index;
      size_t material;

      lml::Transform transform;
    };

    std::vector<Instance> instances() const;

  signals:

    void document_changed();

  private:

    void attach(Studio::Document *document);

    unique_document m_document;
};
