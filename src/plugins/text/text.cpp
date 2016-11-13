//
// Text Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "text.h"
#include "assetfile.h"
#include <functional>
#include <cassert>

#include <QDebug>

using namespace std;
using namespace lml;

///////////////////////// hash //////////////////////////////////////////////
void TextDocument::hash(Studio::Document *document, size_t *key)
{
  *key = std::hash<double>{}(document->metadata("build", 0.0));
}


///////////////////////// pack //////////////////////////////////////////////
void TextDocument::pack(Studio::PackerState &asset, ofstream &fout)
{
  asset.document->lock();

  PackTextHeader text;

  if (read_asset_header(asset.document, 1, &text))
  {
    vector<char> payload(pack_payload_size(text));

    read_asset_payload(asset.document, text.dataoffset, payload.data(), payload.size());

    write_text_asset(fout, asset.id, payload.size(), payload.data());
  }

  asset.document->unlock();
}


//|---------------------- TextDocument --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// TextDocument::Constructor /////////////////////////
TextDocument::TextDocument()
{
}


///////////////////////// TextDocument::Constructor /////////////////////////
TextDocument::TextDocument(QString const &path)
  : TextDocument()
{
  if (path != "")
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
  }
}


///////////////////////// TextDocument::Constructor /////////////////////////
TextDocument::TextDocument(Studio::Document *document)
  : TextDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// TextDocument::Constructor /////////////////////////
TextDocument::TextDocument(TextDocument const &document)
  : TextDocument(document.m_document)
{
}


///////////////////////// TextDocument::Assignment //////////////////////////
TextDocument TextDocument::operator =(TextDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// TextDocument::attach //////////////////////////////
void TextDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &TextDocument::document_changed);
  }
}
