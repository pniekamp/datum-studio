//
// Animation Build
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "datum/math.h"
#include "packapi.h"
#include <string>

//-------------------------- AnimationDocument ------------------------------
//---------------------------------------------------------------------------

class AnimationDocument : public QObject
{
  Q_OBJECT

  public:

    static void hash(Studio::Document *document, size_t *key);

    static void pack(Studio::PackerState &asset, std::ofstream &fout);

  public:
    AnimationDocument();
    AnimationDocument(QString const &path);
    AnimationDocument(Studio::Document *document);
    AnimationDocument(AnimationDocument const &document);
    AnimationDocument operator =(AnimationDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    float duration() const;

    int jointcount() const;

    struct Joint
    {
      char name[32];
      size_t parent;

      struct Transform
      {
        float time;
        lml::Transform transform;
      };

      std::vector<Transform> transforms;
    };

    std::vector<Joint> joints() const;

  signals:

    void document_changed();

  private:

    void attach(Studio::Document *document);

    unique_document m_document;
};
