//
// Animation Build
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "animation.h"
#include "assetfile.h"
#include "atlaspacker.h"
#include <functional>

#include <QDebug>

using namespace std;
using namespace lml;

///////////////////////// hash //////////////////////////////////////////////
void AnimationDocument::hash(Studio::Document *document, size_t *key)
{
  *key = std::hash<double>{}(document->metadata("build", 0.0));
}


///////////////////////// pack //////////////////////////////////////////////
void AnimationDocument::pack(Studio::PackerState &asset, ofstream &fout)
{
  asset.document->lock();

  PackAnimationHeader anim;

  if (read_asset_header(asset.document, 1, &anim))
  {
    vector<char> payload(pack_payload_size(anim));

    read_asset_payload(asset.document, anim.dataoffset, payload.data(), payload.size());

    write_anim_asset(fout, asset.id, anim.duration, anim.jointcount, anim.transformcount, payload.data());
  }

  asset.document->unlock();
}


//|---------------------- AnimationDocument ---------------------------------
//|--------------------------------------------------------------------------

///////////////////////// AnimationDocument::Constructor ////////////////////
AnimationDocument::AnimationDocument()
{
}


///////////////////////// AnimationDocument::Constructor ////////////////////
AnimationDocument::AnimationDocument(QString const &path)
  : AnimationDocument()
{
  if (path != "")
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
  }
}


///////////////////////// AnimationDocument::Constructor ////////////////////
AnimationDocument::AnimationDocument(Studio::Document *document)
  : AnimationDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// AnimationDocument::Constructor ////////////////////
AnimationDocument::AnimationDocument(AnimationDocument const &document)
  : AnimationDocument(document.m_document)
{
}


///////////////////////// AnimationDocument::Assignment /////////////////////
AnimationDocument AnimationDocument::operator =(AnimationDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// AnimationDocument::attach /////////////////////////
void AnimationDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &AnimationDocument::document_changed);
  }
}
