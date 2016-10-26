//
// Image Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "Image.h"
#include "assetfile.h"
#include "atlaspacker.h"
#include <functional>
#include <cassert>

#include <QDebug>

using namespace std;
using namespace lml;

///////////////////////// hash //////////////////////////////////////////////
void ImageDocument::hash(Studio::Document *document, size_t *key)
{
  *key = std::hash<double>{}(document->metadata("build", 0.0));
}


//|---------------------- ImageDocument -------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ImageDocument::Constructor ////////////////////////
ImageDocument::ImageDocument()
{
}


///////////////////////// ImageDocument::Constructor ////////////////////////
ImageDocument::ImageDocument(QString const &path)
  : ImageDocument()
{
  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
}


///////////////////////// ImageDocument::Constructor ////////////////////////
ImageDocument::ImageDocument(Studio::Document *document)
  : ImageDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// ImageDocument::Constructor ////////////////////////
ImageDocument::ImageDocument(ImageDocument const &document)
  : ImageDocument(document.m_document)
{
}


///////////////////////// ImageDocument::Assignment /////////////////////////
ImageDocument ImageDocument::operator =(ImageDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// ImageDocument::attach /////////////////////////////
void ImageDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &ImageDocument::document_changed);
  }
}


///////////////////////// data //////////////////////////////////////////////
HDRImage ImageDocument::data(long flags)
{
  HDRImage image = {};

  m_document->lock();

  PackImageHeader imag;

  if (read_asset_header(m_document, 1, &imag))
  {
    vector<char> payload(pack_payload_size(imag));

    read_asset_payload(m_document, imag.dataoffset, payload.data(), payload.size());

    image.width = imag.width;
    image.height = imag.height;
    image.bits.resize(image.width * image.height);

    uint32_t *src = (uint32_t*)payload.data();

    for(size_t i = 0; i < image.bits.size(); ++i)
    {
      switch(imag.format)
      {
        case PackImageHeader::rgba:
          image.bits[i] = (flags & srgb) ? srgba(*src) : rgba(*src);
          break;

        case PackImageHeader::rgbe:
          image.bits[i] = rgbe(*src);
          break;

        default:
          assert(false);
      }

      ++src;
    }
  }

  m_document->unlock();

  return image;
}
