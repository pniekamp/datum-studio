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


///////////////////////// AnimationDocument::duration ///////////////////////
float AnimationDocument::duration() const
{
  float duration = 0;

  m_document->lock();

  PackAnimationHeader anim;

  if (read_asset_header(m_document, 1, &anim))
  {
    duration = anim.duration;
  }

  m_document->unlock();

  return duration;
}


///////////////////////// AnimationDocument::jointcount /////////////////////
int AnimationDocument::jointcount() const
{
  int joints = 0;

  m_document->lock();

  PackAnimationHeader anim;

  if (read_asset_header(m_document, 1, &anim))
  {
    joints = anim.jointcount;
  }

  m_document->unlock();

  return joints;
}


///////////////////////// AnimationDocument::joints//////////////////////////
vector<AnimationDocument::Joint> AnimationDocument::joints() const
{
  vector<Joint> joints;

  m_document->lock();

  PackAnimationHeader anim;

  if (read_asset_header(m_document, 1, &anim))
  {
    vector<char> payload(pack_payload_size(anim));

    read_asset_payload(m_document, anim.dataoffset, payload.data(), payload.size());

    auto jointtable = PackAnimationPayload::jointtable(payload.data(), anim.jointcount, anim.transformcount);
    auto transformtable = PackAnimationPayload::transformtable(payload.data(), anim.jointcount, anim.transformcount);

    for(size_t i = 0; i < anim.jointcount; ++i)
    {
      Joint joint;

      memcpy(joint.name, jointtable[i].name, sizeof(joint.name));
      joint.parent = jointtable[i].parent;

      for(size_t j = jointtable[i].index; j < jointtable[i].index + jointtable[i].count; ++j)
      {
        joint.transforms.push_back({ transformtable[j].time, Transform{ { transformtable[j].transform[0], transformtable[j].transform[1], transformtable[j].transform[2], transformtable[j].transform[3] }, { transformtable[j].transform[4], transformtable[j].transform[5], transformtable[j].transform[6], transformtable[j].transform[7] } } });
      }

      joints.push_back(joint);
    }
  }

  m_document->unlock();

  return joints;
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
