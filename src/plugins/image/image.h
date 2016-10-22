//
// Image Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "hdr.h"
#include <string>

//-------------------------- ImageDocument ----------------------------------
//---------------------------------------------------------------------------

class ImageDocument : public QObject
{
  Q_OBJECT

  public:

    static void hash(Studio::Document *document, size_t *key);

  public:
    ImageDocument();
    ImageDocument(QString const &path);
    ImageDocument(Studio::Document *document);
    ImageDocument(ImageDocument const &document);
    ImageDocument operator =(ImageDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    enum DataFlags
    {
      raw  = 0x00,
      srgb = 0x01
    };

    HDRImage data(long flags = srgb);

  signals:

    void document_changed();

  private:

    void attach(Studio::Document *document);

    unique_document m_document;
};
