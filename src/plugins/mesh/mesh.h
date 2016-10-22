//
// Mesh Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include <string>

//-------------------------- MeshDocument -----------------------------------
//---------------------------------------------------------------------------

class MeshDocument : public QObject
{
  Q_OBJECT

  public:

    static void hash(Studio::Document *document, size_t *key);

    static void build(Studio::Document *document, std::string const &path);

  public:
    MeshDocument();
    MeshDocument(QString const &path);
    MeshDocument(Studio::Document *document);
    MeshDocument(MeshDocument const &document);
    MeshDocument operator =(MeshDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  signals:

    void document_changed();

  private:

    void attach(Studio::Document *document);

    unique_document m_document;
};
