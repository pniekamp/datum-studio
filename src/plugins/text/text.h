//
// Text Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "packapi.h"
#include <string>

//-------------------------- TextDocument -----------------------------------
//---------------------------------------------------------------------------

class TextDocument : public QObject
{
  Q_OBJECT

  public:

    static void hash(Studio::Document *document, size_t *key);

    static void pack(Studio::PackerState &asset, std::ofstream &fout);

  public:
    TextDocument();
    TextDocument(QString const &path);
    TextDocument(Studio::Document *document);
    TextDocument(TextDocument const &document);
    TextDocument operator =(TextDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  signals:

    void document_changed();

  private:

    void attach(Studio::Document *document);

    unique_document m_document;
};
