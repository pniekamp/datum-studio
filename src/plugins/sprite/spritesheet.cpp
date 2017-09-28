//
// SpriteSheet Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "spritesheet.h"
#include "image.h"
#include "assetfile.h"
#include "atlaspacker.h"
#include <functional>
#include <cassert>
#include <QJsonDocument>
#include <QJsonArray>

#include <QDebug>

using namespace std;
using namespace lml;

namespace
{
  void hash_combine(size_t &seed, size_t key)
  {
    seed ^= key + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }

  size_t image_hash(QString const &path)
  {
    size_t key = 0;

    if (auto document = ImageDocument(path))
    {
      ImageDocument::hash(document, &key);
    }

    return key;
  }

  uint32_t write_catalog(ostream &fout, uint32_t id)
  {
    write_catl_asset(fout, id, 0, 0);

    return id + 1;
  }

  uint32_t write_spritesheet(ostream &fout, uint32_t id, vector<HDRImage> const &images)
  {
    int width = max_element(images.begin(), images.end(), [](auto &lhs, auto &rhs) { return lhs.width < rhs.width; })->width;
    int height = max_element(images.begin(), images.end(), [](auto &lhs, auto &rhs) { return lhs.height < rhs.height; })->height;
    int layers = images.size();
    int levels = min(4, image_maxlevels(width, height));

    vector<char> payload(image_datasize(width, height, layers, levels));

    uint32_t *dst = (uint32_t*)payload.data();

    for(auto &image : images)
    {
      if (image.bits.size() == 0)
        throw runtime_error("SpriteSheet build failed - null image");

      Vec2 area = Vec2(1.0f / min(width, image.width), 1.0f / min(height, image.height));

      for(int y = 0; y < height; ++y)
      {
        for(int x = 0; x < width; ++x)
        {
          *dst++ = srgba(clamp(image.sample(Vec2((x + 0.5f)/width, (y + 0.5f)/height), area), 0.0f, 1.0f));
        }
      }
    }

    image_premultiply_srgb(width, height, layers, levels, payload.data());

    image_buildmips_srgb(width, height, layers, levels, payload.data());

    write_imag_asset(fout, id, width, height, layers, levels, PackImageHeader::rgba, payload.data());

    return id + 1;
  }
}


///////////////////////// create ////////////////////////////////////////////
void SpriteSheetDocument::create(string const &path)
{
  QJsonObject metadata;
  metadata["type"] = "SpriteSheet";
  metadata["icon"] = encode_icon(QIcon(":/spriteplugin/icon.png"));

  QJsonObject definition;

  ofstream fout(path, ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_json(fout, 1, definition);

  write_asset_footer(fout);

  fout.close();
}


///////////////////////// hash //////////////////////////////////////////////
void SpriteSheetDocument::hash(Studio::Document *document, size_t *key)
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

  for(auto i : definition["layers"].toArray())
  {
    auto layer = i.toObject();

    hash_combine(*key, image_hash(fullpath(document, layer["path"].toString())));
  }
}


///////////////////////// hash //////////////////////////////////////////////
void SpriteSheetDocument::build_hash(Studio::Document *document, size_t *key)
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

  *key = 0;

  for(auto i : definition["layers"].toArray())
  {
    auto layer = i.toObject();

    hash_combine(*key, image_hash(fullpath(document, layer["path"].toString())));
  }
}


///////////////////////// build /////////////////////////////////////////////
void SpriteSheetDocument::build(Studio::Document *document, string const &path)
{
  auto spritesheetdocument = SpriteSheetDocument(document);

  if (spritesheetdocument.layers() == 0)
    throw runtime_error("Spritesheet build failed - no layers");

  vector<HDRImage> images;

  for(int i = 0; i < spritesheetdocument.layers(); ++i)
  {
    images.push_back(ImageDocument(spritesheetdocument.layer(i)).data());
  }

  ofstream fout(path, ios::binary | ios::trunc);

  write_header(fout);

  write_catalog(fout, 0);

  write_spritesheet(fout, 1, images);

  write_chunk(fout, "HEND", 0, nullptr);

  fout.close();
}


///////////////////////// pack //////////////////////////////////////////////
void SpriteSheetDocument::pack(Studio::PackerState &asset, ofstream &fout)
{
  ifstream fin(asset.buildpath, ios::binary);

  if (!fin)
    throw runtime_error("SpriteSheet Pack failed - no build file");

  PackImageHeader imag;

  if (read_asset_header(fin, 1, &imag))
  {
    vector<char> payload(pack_payload_size(imag));

    read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

    write_imag_asset(fout, asset.id, imag.width, imag.height, imag.layers, imag.levels, imag.format, payload.data());
  }
}


//|---------------------- SpriteSheetDocument -------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SpriteSheetDocument::Constructor //////////////////
SpriteSheetDocument::SpriteSheetDocument()
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_changed, this, &SpriteSheetDocument::touch);
}


///////////////////////// SpriteSheetDocument::Constructor //////////////////
SpriteSheetDocument::SpriteSheetDocument(QString const &path)
  : SpriteSheetDocument()
{
  if (path != "")
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
  }
}


///////////////////////// SpriteSheetDocument::Constructor //////////////////
SpriteSheetDocument::SpriteSheetDocument(Studio::Document *document)
  : SpriteSheetDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// SpriteSheetDocument::Constructor //////////////////
SpriteSheetDocument::SpriteSheetDocument(SpriteSheetDocument const &document)
  : SpriteSheetDocument(document.m_document)
{
}


///////////////////////// SpriteSheetDocument::Assignment ///////////////////
SpriteSheetDocument SpriteSheetDocument::operator =(SpriteSheetDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// SpriteSheetDocument::add_layer ////////////////////
void SpriteSheetDocument::add_layer(int position, QString const &path)
{
  QJsonObject layer;
  layer["path"] = relpath(m_document, path);

  auto layers = m_definition["layers"].toArray();

  layers.insert(position, layer);

  m_definition["layers"] = layers;

  update();
}


///////////////////////// SpriteSheetDocument::move_layer ///////////////////
void SpriteSheetDocument::move_layer(int index, int position)
{
  auto layers = m_definition["layers"].toArray();

  layers.insert((index < position) ? position-1 : position, layers.takeAt(index));

  m_definition["layers"] = layers;

  update();
}


///////////////////////// SpriteSheetDocument::erase_layer //////////////////
void SpriteSheetDocument::erase_layer(int index)
{
  auto layers = m_definition["layers"].toArray();

  layers.removeAt(index);

  m_definition["layers"] = layers;

  update();
}


///////////////////////// SpriteSheetDocument::attach ///////////////////////
void SpriteSheetDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &SpriteSheetDocument::refresh);

    refresh();
  }
}


///////////////////////// SpriteSheetDocument::touch ////////////////////////
void SpriteSheetDocument::touch(Studio::Document *document, QString const &path)
{
  for(auto &image : m_images)
  {
    if (document == image)
    {
      emit dependant_changed();
    }
  }
}


///////////////////////// SpriteSheetDocument::refresh //////////////////////
void SpriteSheetDocument::refresh()
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

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  m_images.clear();

  for(auto i : m_definition["layers"].toArray())
  {
    auto layer = i.toObject();
    auto path = fullpath(m_document, layer["path"].toString());

    m_images.push_back((path != "") ? documentmanager->open(path) : nullptr);
  }

  emit document_changed();
}


///////////////////////// SpriteSheetDocument::update ///////////////////////
void SpriteSheetDocument::update()
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
