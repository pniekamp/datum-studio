//
// ParticleSystem Build
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "particlesystem.h"
#include "spritesheet.h"
#include "assetfile.h"
#include "atlaspacker.h"
#include <leap/lml/interpolation.h>
#include <functional>
#include <cassert>
#include <QJsonDocument>
#include <QJsonArray>

#include <QDebug>

using namespace std;
using namespace lml;
using leap::indexof;
using leap::extentof;

namespace
{
  void hash_combine(size_t &seed, size_t key)
  {
    seed ^= key + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }

  size_t spritesheet_hash(QString const &path)
  {
    size_t key = 0;

    if (auto document = SpriteSheetDocument(path))
    {
      SpriteSheetDocument::hash(document, &key);
    }

    return key;
  }

  template<typename T>
  QJsonValue to_json(T const &value)
  {
    return value;
  }

  QJsonObject to_json(Vec2 const &value)
  {
    return QJsonObject({ { "x", value.x }, { "y", value.y } });
  }

  QJsonObject to_json(Vec3 const &value)
  {
    return QJsonObject({ { "x", value.x }, { "y", value.y }, { "z", value.z } });
  }

  QJsonObject to_json(Color4 const &value)
  {
    return QJsonObject({ { "r", value.r }, { "g", value.g }, { "b", value.b }, { "a", value.a } });
  }

  QJsonObject to_json(Transform const &value)
  {
    return QJsonObject({ { "real.x", value.real.x }, { "real.y", value.real.y }, { "real.z", value.real.z }, { "real.w", value.real.w }, { "dual.x", value.dual.x }, { "dual.y", value.dual.y }, { "dual.z", value.dual.z }, { "dual.w", value.dual.w } });
  }

  template<typename T>
  QJsonArray to_json(vector<T> const &values)
  {
    QJsonArray array;

    for(auto &value : values)
    {
      array.push_back(to_json(value));
    }

    return array;
  }

  template<typename T>
  T from_json(QJsonObject const &value);

  template<>
  Vec2 from_json<Vec2>(QJsonObject const &value)
  {
    return Vec2(value["x"].toDouble(), value["y"].toDouble());
  }

  template<>
  Vec3 from_json<Vec3>(QJsonObject const &value)
  {
    return Vec3(value["x"].toDouble(), value["y"].toDouble(), value["z"].toDouble());
  }

  template<>
  Color4 from_json<Color4>(QJsonObject const &value)
  {
    return Color4(value["r"].toDouble(), value["g"].toDouble(), value["b"].toDouble(), value["a"].toDouble());
  }

  template<>
  Transform from_json<Transform>(QJsonObject const &value)
  {
    return Transform{ { (float)value["real.w"].toDouble(), (float)value["real.x"].toDouble(), (float)value["real.y"].toDouble(), (float)value["real.z"].toDouble() }, { (float)value["dual.w"].toDouble(), (float)value["dual.x"].toDouble(), (float)value["dual.y"].toDouble(), (float)value["dual.z"].toDouble() } };
  }

  template<typename T>
  vector<T> from_json(QJsonArray const &values)
  {
    vector<T> array;

    for(auto &value : values.toVariantList())
    {
      array.push_back(value.value<T>());
    }

    return array;
  }

  template<>
  vector<Vec3> from_json<Vec3>(QJsonArray const &values)
  {
    vector<Vec3> array;

    for(auto const &value : values)
    {
      array.push_back(from_json<Vec3>(value.toObject()));
    }

    return array;
  }

  template<>
  vector<Color4> from_json<Color4>(QJsonArray const &values)
  {
    vector<Color4> array;

    for(auto const &value : values)
    {
      array.push_back(from_json<Color4>(value.toObject()));
    }

    return array;
  }
}


///////////////////////// make_distribution /////////////////////////////////
template<typename T>
Distribution<T> make_distribution(ParticleSystemDocument::Distribution<T> const &source)
{
  using lml::min;
  using lml::max;

  switch(source.type)
  {
    case ParticleSystemDocument::Distribution<T>::Constant:
    {
      return make_constant_distribution(source.ya[0]);
    }

    case ParticleSystemDocument::Distribution<T>::Uniform:
    {
      return make_uniform_distribution(min(source.ya[0], source.ya[1]), max(source.ya[0], source.ya[1]));
    }

    case ParticleSystemDocument::Distribution<T>::Curve:
    {
      T values[Distribution<T>::TableSize];

      for(size_t i = 0; i < Distribution<T>::TableSize; ++i)
      {
        values[i] = interpolate<cubic>(source.xa.begin(), source.xa.end(), source.ya.begin(), i / (extentof(values) - 1.0f));
      }

      return make_table_distribution(values, extentof(values));
    }

    case ParticleSystemDocument::Distribution<T>::UniformCurve:
    {
      T minvalues[Distribution<T>::TableSize/2];
      T maxvalues[Distribution<T>::TableSize/2];

      for(size_t i = 0; i < Distribution<T>::TableSize/2; ++i)
      {
        auto a = interpolate<cubic>(source.xa.begin(), source.xa.end(), source.ya.begin(), i / (extentof(minvalues) - 1.0f));
        auto b = interpolate<cubic>(source.xa.begin(), source.xa.end(), source.ya.begin() + source.xa.size(), i / (extentof(maxvalues) - 1.0f));

        minvalues[i] = min(a, b);
        maxvalues[i] = max(a, b);
      }

      return make_uniformtable_distribution(minvalues, extentof(minvalues), maxvalues, extentof(maxvalues));
    }
  }

  assert(false);

  return make_constant_distribution(T());
}


///////////////////////// make_emitter //////////////////////////////////////
ParticleEmitter make_emitter(ParticleSystemDocument::Emitter const &source)
{
  ParticleEmitter emitter;

  emitter.duration = source.duration;
  emitter.looping = source.looping;

  emitter.transform = source.transform;

  emitter.rate = source.rate;
  emitter.bursts = source.bursttime.size();
  copy(source.bursttime.begin(), source.bursttime.end(), emitter.bursttime);
  copy(source.burstcount.begin(), source.burstcount.end(), emitter.burstcount);

  emitter.size = source.size;

  emitter.life = make_distribution(source.life);
  emitter.scale = make_distribution(source.scale);
  emitter.rotation = make_distribution(source.rotation);
  emitter.velocity = make_distribution(source.velocity);
  emitter.color = make_distribution(source.color);
  emitter.emissive = make_distribution(source.emissive);
  emitter.layer = make_distribution(source.layer);

  emitter.acceleration = source.accelerated ? source.acceleration : Vec3(0);

  emitter.modules = 0;

  if (source.shaped)
  {
    emitter.modules |= ParticleEmitter::ShapeEmitter;
    emitter.shape = static_cast<ParticleEmitter::Shape>(source.shape);
    emitter.shaperadius = source.shaperadius;
    emitter.shapeangle= source.shapeangle;
  }

  if (source.scaled)
  {
    emitter.modules |= ParticleEmitter::ScaleOverLife;
    emitter.scaleoverlife = make_distribution(source.scaleoverlife);
  }

  if (source.rotated)
  {
    emitter.modules |= ParticleEmitter::RotateOverLife;
    emitter.rotateoverlife = make_distribution(source.rotateoverlife);
  }

  if (source.colored)
  {
    emitter.modules |= ParticleEmitter::ColorOverLife;
    emitter.coloroverlife = make_distribution(source.coloroverlife);
  }

  if (source.animated)
  {
    emitter.modules |= ParticleEmitter::LayerOverLife;
    emitter.layerstart = source.layerstart;
    emitter.layercount = source.layercount;
    emitter.layerrate = make_distribution(source.layerrate);
  }

  if (source.stretched)
  {
    emitter.modules |= ParticleEmitter::StretchWithVelocity;
    emitter.velocitystretchmin = source.velocitystretchmin;
    emitter.velocitystretchmax = source.velocitystretchmax;
  }

  if (source.aligned)
  {
    emitter.modules |= ParticleEmitter::StretchWithAxis;
    emitter.stretchaxis = source.alignedaxis;
  }

  return emitter;
}


///////////////////////// create ////////////////////////////////////////////
void ParticleSystemDocument::create(string const &path)
{
  QJsonObject metadata;
  metadata["type"] = "ParticleSystem";
  metadata["icon"] = encode_icon(QIcon(":/particleplugin/icon.png"));

  QJsonObject definition;
  definition["maxparticles"] = 1000;

  ofstream fout(path, ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_json(fout, 1, definition);

  write_asset_footer(fout);

  fout.close();
}


///////////////////////// hash //////////////////////////////////////////////
void ParticleSystemDocument::hash(Studio::Document *document, size_t *key)
{
  QJsonObject definition;

  document->lock();

  PackTextHeader text;

  if (read_asset_header(document, 1, &text))
  {
    QByteArray payload(pack_payload_size(text), 0);

    read_asset_payload(document, text.dataoffset, payload.data(), payload.size());

    definition = QJsonDocument::fromBinaryData(payload).object();
  }

  document->unlock();

  *key = std::hash<double>{}(document->metadata("build", 0.0));

  hash_combine(*key, spritesheet_hash(fullpath(document, definition["spritesheet"].toString())));
}


///////////////////////// pack //////////////////////////////////////////////
void ParticleSystemDocument::pack(Studio::PackerState &asset, ofstream &fout)
{
  using ::pack;

  auto particledocument = ParticleSystemDocument(asset.document);

  vector<uint8_t> emitters;

  for(int i = 0; i < particledocument.emitters(); ++i)
  {
    pack(emitters, make_emitter(particledocument.emitter(i)));
  }

  uint32_t spritesheet = asset.add_dependant(particledocument.spritesheet(), "SpriteSheet");

  write_part_asset(fout, asset.id, particledocument.bound(), spritesheet, particledocument.maxparticles(), particledocument.emitters(), emitters);
}


//|---------------------- ParticleSystemDocument ----------------------------
//|--------------------------------------------------------------------------

///////////////////////// ParticleSystemDocument::Constructor ///////////////
ParticleSystemDocument::ParticleSystemDocument()
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_changed, this, &ParticleSystemDocument::touch);
}


///////////////////////// ParticleSystemDocument::Constructor ///////////////
ParticleSystemDocument::ParticleSystemDocument(QString const &path)
  : ParticleSystemDocument()
{
  if (path != "")
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
  }
}


///////////////////////// ParticleSystemDocument::Constructor ///////////////
ParticleSystemDocument::ParticleSystemDocument(Studio::Document *document)
  : ParticleSystemDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// ParticleSystemDocument::Constructor ///////////////
ParticleSystemDocument::ParticleSystemDocument(ParticleSystemDocument const &document)
  : ParticleSystemDocument(document.m_document)
{
}


///////////////////////// ParticleSystemDocument::Assignment ////////////////
ParticleSystemDocument ParticleSystemDocument::operator =(ParticleSystemDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// ParticleSystemDocument::set_bound /////////////////
void ParticleSystemDocument::set_bound(Bound3 const &bound)
{
  auto min = lml::min(bound.min, bound.max);
  auto max = lml::max(bound.min, bound.max);

  m_definition["bound"] = QJsonObject{ { "min.x", min.x }, { "min.y", min.y }, { "min.z", min.z }, { "max.x", max.x }, { "max.y", max.y }, { "max.z", max.z } };

  update();
}


///////////////////////// ParticleSystemDocument::set_maxparticles //////////
void ParticleSystemDocument::set_maxparticles(int maxparticles)
{
  m_definition["maxparticles"] = maxparticles;

  update();
}


///////////////////////// ParticleSystemDocument::set_spritesheet ///////////
void ParticleSystemDocument::set_spritesheet(QString const &path)
{
  m_definition["spritesheet"] = relpath(m_document, path);

  update();
}


///////////////////////// ParticleSystemDocument::add_emitter ///////////////
void ParticleSystemDocument::add_emitter(int position, QString const &name)
{
  QJsonObject emitter;
  emitter["name"] = name;

  auto emitters = m_definition["emitters"].toArray();

  emitters.insert(position, emitter);

  m_definition["emitters"] = emitters;

  update();
}


///////////////////////// ParticleSystemDocument::move_emitter //////////////
void ParticleSystemDocument::move_emitter(int index, int position)
{
  auto emitters = m_definition["emitters"].toArray();

  emitters.insert((index < position) ? position-1 : position, emitters.takeAt(index));

  m_definition["emitters"] = emitters;

  update();
}


///////////////////////// ParticleSystemDocument::erase_emitter /////////////
void ParticleSystemDocument::erase_emitter(int index)
{
  auto emitters = m_definition["emitters"].toArray();

  emitters.removeAt(index);

  m_definition["emitters"] = emitters;

  update();
}


///////////////////////// ParticleSystemDocument::update_emitter ////////////
void ParticleSystemDocument::update_emitter(int index, Emitter const &value)
{
  auto emitters = m_definition["emitters"].toArray();

  auto emitter = emitters[index].toObject();

  emitter["name"] = value.name;
  emitter["duration"] = value.duration;
  emitter["looping"] = value.looping;
  emitter["transform"] = to_json(value.transform);
  emitter["rate"] = value.rate;
  emitter["bursttime"] = to_json(value.bursttime);
  emitter["burstcount"] = to_json(value.burstcount);
  emitter["size"] = to_json(value.size);
  emitter["life.type"] = value.life.type;
  emitter["life.xa"] = to_json(value.life.xa);
  emitter["life.ya"] = to_json(value.life.ya);
  emitter["scale.type"] = value.scale.type;
  emitter["scale.xa"] = to_json(value.scale.xa);
  emitter["scale.ya"] = to_json(value.scale.ya);
  emitter["rotation.type"] = value.rotation.type;
  emitter["rotation.xa"] = to_json(value.rotation.xa);
  emitter["rotation.ya"] = to_json(value.rotation.ya);
  emitter["velocity.type"] = value.velocity.type;
  emitter["velocity.xa"] = to_json(value.velocity.xa);
  emitter["velocity.ya"] = to_json(value.velocity.ya);
  emitter["color.type"] = value.color.type;
  emitter["color.xa"] = to_json(value.color.xa);
  emitter["color.ya"] = to_json(value.color.ya);
  emitter["emissive.type"] = value.emissive.type;
  emitter["emissive.xa"] = to_json(value.emissive.xa);
  emitter["emissive.ya"] = to_json(value.emissive.ya);
  emitter["layer.type"] = value.layer.type;
  emitter["layer.xa"] = to_json(value.layer.xa);
  emitter["layer.ya"] = to_json(value.layer.ya);
  emitter["accelerated"] = value.accelerated;
  emitter["acceleration"] = to_json(value.acceleration);
  emitter["shaped"] = value.shaped;
  emitter["shape"] = value.shape;
  emitter["shaperadius"] = value.shaperadius;
  emitter["shapeangle"] = value.shapeangle;
  emitter["scaled"] = value.scaled;
  emitter["scaleoverlife.type"] = value.scaleoverlife.type;
  emitter["scaleoverlife.xa"] = to_json(value.scaleoverlife.xa);
  emitter["scaleoverlife.ya"] = to_json(value.scaleoverlife.ya);
  emitter["rotated"] = value.rotated;
  emitter["rotateoverlife.type"] = value.rotateoverlife.type;
  emitter["rotateoverlife.xa"] = to_json(value.rotateoverlife.xa);
  emitter["rotateoverlife.ya"] = to_json(value.rotateoverlife.ya);
  emitter["colored"] = value.colored;
  emitter["coloroverlife.type"] = value.coloroverlife.type;
  emitter["coloroverlife.xa"] = to_json(value.coloroverlife.xa);
  emitter["coloroverlife.ya"] = to_json(value.coloroverlife.ya);
  emitter["animated"] = value.animated;
  emitter["layerstart"] = value.layerstart;
  emitter["layercount"] = value.layercount;
  emitter["layerrate.type"] = value.layerrate.type;
  emitter["layerrate.xa"] = to_json(value.layerrate.xa);
  emitter["layerrate.ya"] = to_json(value.layerrate.ya);
  emitter["stretched"] = value.stretched;
  emitter["velocitystretchmin"] = value.velocitystretchmin;
  emitter["velocitystretchmax"] = value.velocitystretchmax;
  emitter["aligned"] = value.aligned;
  emitter["alignedaxis"] = to_json(value.alignedaxis);

  emitters[index] = emitter;

  m_definition["emitters"] = emitters;

  update();
}


///////////////////////// ParticleSystemDocument::attach ////////////////////
void ParticleSystemDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &ParticleSystemDocument::refresh);

    refresh();
  }
}


///////////////////////// ParticleSystemDocument::touch /////////////////////
void ParticleSystemDocument::touch(Studio::Document *document, QString const &path)
{
  if (document == m_spritesheet)
  {
    emit dependant_changed();
  }
}


///////////////////////// ParticleSystemDocument::refresh ///////////////////
void ParticleSystemDocument::refresh()
{
  m_document->lock();

  PackTextHeader text;

  if (read_asset_header(m_document, 1, &text))
  {
    QByteArray payload(pack_payload_size(text), 0);

    read_asset_payload(m_document, text.dataoffset, payload.data(), payload.size());

    m_definition = QJsonDocument::fromBinaryData(payload).object();
  }

  m_document->unlock();

  auto bound = m_definition["bound"].toObject();

  m_bound = Bound3(Vec3(bound["min.x"].toDouble(-1), bound["min.y"].toDouble(-1), bound["min.z"].toDouble(-1)), Vec3(bound["max.x"].toDouble(1), bound["max.y"].toDouble(1), bound["max.z"].toDouble(1)));

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  m_spritesheet = nullptr;

  if (m_definition["spritesheet"].toString() != "")
  {
    m_spritesheet = documentmanager->open(fullpath(m_document, m_definition["spritesheet"].toString()));
  }

  vector<Emitter> emitters;

  for(auto i : m_definition["emitters"].toArray())
  {
    auto emitter = i.toObject();

    Emitter ed;
    ed.name = emitter["name"].toString();
    ed.duration = emitter["duration"].toDouble(2.0);
    ed.looping = emitter["looping"].toBool(true);
    ed.transform = from_json<Transform>(emitter.value("transform").toObject(to_json(Transform::rotation(Vec3(0, 0, 1), pi<float>()/2))));
    ed.rate = emitter["rate"].toDouble(1.0);
    ed.bursttime = from_json<float>(emitter["bursttime"].toArray());
    ed.burstcount = from_json<int>(emitter["burstcount"].toArray());
    ed.size = from_json<Vec2>(emitter.value("size").toObject(to_json(Vec2(1.0f, 1.0f))));
    ed.life.type = emitter["life.type"].toInt(0);
    ed.life.xa = from_json<float>(emitter["life.xa"].toArray());
    ed.life.ya = from_json<float>(emitter.value("life.ya").toArray(QJsonArray({ 2.0f })));
    ed.scale.type = emitter["scale.type"].toInt(0);
    ed.scale.xa = from_json<float>(emitter["scale.xa"].toArray());
    ed.scale.ya = from_json<float>(emitter.value("scale.ya").toArray(QJsonArray({ 1.0f })));
    ed.rotation.type = emitter["rotation.type"].toInt(0);
    ed.rotation.xa = from_json<float>(emitter["rotation.xa"].toArray());
    ed.rotation.ya = from_json<float>(emitter.value("rotation.ya").toArray(QJsonArray({ 0.0f })));
    ed.velocity.type = emitter["velocity.type"].toInt(0);
    ed.velocity.xa = from_json<float>(emitter["velocity.xa"].toArray());
    ed.velocity.ya = from_json<Vec3>(emitter.value("velocity.ya").toArray(QJsonArray({ to_json(Vec3(1.0f, 0.0f, 0.0f)) })));
    ed.color.type = emitter["color.type"].toInt(0);
    ed.color.xa = from_json<float>(emitter["color.xa"].toArray());
    ed.color.ya = from_json<Color4>(emitter.value("color.ya").toArray(QJsonArray({ to_json(Color4(1.0f, 1.0f, 1.0f, 1.0f)) })));
    ed.emissive.type = emitter["emissive.type"].toInt(0);
    ed.emissive.xa = from_json<float>(emitter["emissive.xa"].toArray());
    ed.emissive.ya = from_json<float>(emitter.value("emissive.ya").toArray(QJsonArray({ 0.0f })));
    ed.layer.type = emitter["layer.type"].toInt(0);
    ed.layer.xa = from_json<float>(emitter["layer.xa"].toArray());
    ed.layer.ya = from_json<float>(emitter.value("layer.ya").toArray(QJsonArray({ 0.0f })));
    ed.accelerated = emitter["accelerated"].toBool(false);
    ed.acceleration = from_json<Vec3>(emitter.value("acceleration").toObject(to_json(Vec3(0.0f, -9.81f, 0.0f))));
    ed.shaped = emitter["shaped"].toBool(false);
    ed.shape = emitter["shape"].toInt(0);
    ed.shaperadius = emitter["shaperadius"].toDouble(1);
    ed.shapeangle = emitter["shapeangle"].toDouble(0);
    ed.scaled = emitter["scaled"].toBool(false);
    ed.scaleoverlife.type = emitter["scaleoverlife.type"].toInt(2);
    ed.scaleoverlife.xa = from_json<float>(emitter.value("scaleoverlife.xa").toArray(QJsonArray({ 0.0f, 1.0f })));
    ed.scaleoverlife.ya = from_json<float>(emitter.value("scaleoverlife.ya").toArray(QJsonArray({ 1.0f, 1.0f })));
    ed.rotated = emitter["rotated"].toBool(false);
    ed.rotateoverlife.type = emitter["rotateoverlife.type"].toInt(2);
    ed.rotateoverlife.xa = from_json<float>(emitter.value("rotateoverlife.xa").toArray(QJsonArray({ 0.0f, 1.0f })));
    ed.rotateoverlife.ya = from_json<float>(emitter.value("rotateoverlife.ya").toArray(QJsonArray({ 0.0f, 2*pi<float>() })));
    ed.colored = emitter["colored"].toBool(false);
    ed.coloroverlife.type = emitter["coloroverlife.type"].toInt(2);
    ed.coloroverlife.xa = from_json<float>(emitter.value("coloroverlife.xa").toArray(QJsonArray({ 0.0f, 1.0f })));
    ed.coloroverlife.ya = from_json<Color4>(emitter.value("coloroverlife.ya").toArray(QJsonArray({ to_json(Color4(1.0f, 1.0f, 1.0f, 1.0f)), to_json(Color4(0.0f, 0.0f, 0.0f, 0.0f)) })));
    ed.animated = emitter["animated"].toBool(false);
    ed.layerstart = emitter["layerstart"].toDouble(0.0);
    ed.layercount = emitter["layercount"].toDouble(1.0);
    ed.layerrate.type = emitter["layerrate.type"].toInt(0);
    ed.layerrate.xa = from_json<float>(emitter["layerrate.xa"].toArray());
    ed.layerrate.ya = from_json<float>(emitter.value("layerrate.ya").toArray(QJsonArray({ 25.0f })));
    ed.stretched = emitter["stretched"].toBool(false);
    ed.velocitystretchmin = emitter["velocitystretchmin"].toDouble(1.0);
    ed.velocitystretchmax = emitter["velocitystretchmax"].toDouble(5.0);
    ed.aligned = emitter["aligned"].toBool(false);
    ed.alignedaxis = from_json<Vec3>(emitter.value("alignedaxis").toObject(to_json(Vec3(0.0f, 1.0f, 0.0f))));

    emitters.push_back(std::move(ed));
  }

  swap(emitters, m_emitters);

  emit document_changed();
}


///////////////////////// ParticleSystemDocument::update ////////////////////
void ParticleSystemDocument::update()
{
  QByteArray data = QJsonDocument(m_definition).toBinaryData();

  m_document->lock_exclusive();

  PackTextHeader text;

  if (auto position = read_asset_header(m_document, 1, &text))
  {
    position += write_text_asset(m_document, position, 1, data.size(), data.data());

    position += write_footer(m_document, position);

    m_document->set_metadata("build", buildtime());
  }

  m_document->unlock_exclusive();
}
