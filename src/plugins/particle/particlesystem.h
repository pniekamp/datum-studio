//
// ParticleSystem Build
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "datum/math.h"
#include "datum/renderer.h"
#include "packapi.h"
#include <string>

//-------------------------- ParticleSystemDocument -------------------------
//---------------------------------------------------------------------------

class ParticleSystemDocument : public QObject
{
  Q_OBJECT

  public:

    static void create(std::string const &path);

    static void hash(Studio::Document *document, size_t *key);

    static void pack(Studio::PackerState &asset, std::ofstream &fout);

  public:
    ParticleSystemDocument();
    ParticleSystemDocument(QString const &path);
    ParticleSystemDocument(Studio::Document *document);
    ParticleSystemDocument(ParticleSystemDocument const &document);
    ParticleSystemDocument operator =(ParticleSystemDocument const &document);

    operator Studio::Document *() const { return m_document; }

    Studio::Document *operator *() const { return m_document; }
    Studio::Document *operator ->() const { return m_document; }

  public:

    lml::Bound3 bound() { return m_bound; }

    int maxparticles() const { return m_definition["maxparticles"].toInt(); }

    Studio::Document *spritesheet() const { return m_spritesheet; }

    template<typename T>
    struct Distribution
    {
      enum Type
      {
        Constant,
        Uniform,
        Curve,
        UniformCurve
      };

      int type;

      std::vector<T> ya;
      std::vector<float> xa;
    };

    struct Emitter
    {
      QString name;

      float duration;
      bool looping;

      float rate;
      std::vector<float> bursttime;
      std::vector<int> burstcount;

      lml::Vec2 size;

      Distribution<float> life;
      Distribution<float> scale;
      Distribution<float> rotation;
      Distribution<lml::Vec3> velocity;
      Distribution<lml::Color4> color;
      Distribution<float> layer;

      bool accelerated;
      lml::Vec3 acceleration;

      bool shaped;
      int shape;
      float shaperadius;
      float shapeangle;

      bool scaled;
      Distribution<float> scaleoverlife;

      bool rotated;
      Distribution<float> rotateoverlife;

      bool colored;
      Distribution<lml::Color4> coloroverlife;

      bool animated;
      float layerstart;
      float layercount;
      Distribution<float> layerrate;

      bool stretched;
      float velocitystretchmin;
      float velocitystretchmax;

      bool aligned;
      lml::Vec3 alignedaxis;
    };

    int emitters() const { return m_emitters.size(); }

    Emitter const &emitter(int index) const { return m_emitters[index]; }

  public:

    void set_bound(lml::Bound3 const &bound);

    void set_maxparticles(int maxparticles);

    void set_spritesheet(QString const &path);

    void add_emitter(int position, QString const &name);

    void move_emitter(int index, int position);

    void erase_emitter(int index);

    void update_emitter(int index, Emitter const &emitter);

  signals:

    void document_changed();
    void dependant_changed();

  private:

    void attach(Studio::Document *document);

    void touch(Studio::Document *document, QString const &path);

    void refresh();

    void update();

    QJsonObject m_definition;

    unique_document m_spritesheet;

    lml::Bound3 m_bound;

    std::vector<Emitter> m_emitters;

    unique_document m_document;
};

template<typename T>
Distribution<T> make_distribution(ParticleSystemDocument::Distribution<T> const &source);

ParticleEmitter make_emitter(ParticleSystemDocument::Emitter const &source);
