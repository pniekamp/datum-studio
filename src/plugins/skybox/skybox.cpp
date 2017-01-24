//
// Skybox Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "skybox.h"
#include "image.h"
#include "assetfile.h"
#include "ibl.h"
#include <QPainter>
#include <QJsonDocument>
#include <functional>
#include <cassert>

#include <QDebug>

using namespace std;
using namespace lml;

static const char *ImageNames[7] = { "front", "left", "right", "back", "top", "bottom", "envmap" };

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

  uint32_t write_skybox(ostream &fout, uint32_t id, int width, int height, vector<HDRImage> const &images)
  {
    int layers = 6;
    int levels = 8;

    vector<char> payload(image_datasize(width, height, layers, levels));

    uint32_t *dst = (uint32_t*)payload.data();

    for(auto &image : images)
    {
      if (image.bits.size() == 0)
        throw runtime_error("Skybox build failed - null image");

      Vec2 area = Vec2(1.0f / min(width, image.width), 1.0f / min(height, image.height));

      for(int y = 0; y < height; ++y)
      {
        for(int x = 0; x < width; ++x)
        {
          *dst++ = rgbe(image.sample(Vec2((x + 0.5f)/width, 1.0f - (y + 0.5f)/height), area));
        }
      }
    }

    image_buildmips_cube_ibl(width, height, levels, payload.data());

    write_imag_asset(fout, id, width, height, layers, levels, PackImageHeader::rgbe, payload.data());

    return id + 1;
  }

  uint32_t write_skybox(ostream &fout, uint32_t id, int width, int height, HDRImage const &image)
  {
    int layers = 6;
    int levels = 8;

    if (image.bits.size() == 0)
      throw runtime_error("Skybox build failed - null image");

    vector<char> payload(image_datasize(width, height, layers, levels));

    image_pack_cube_ibl(image, width, height, levels, payload.data());

    write_imag_asset(fout, id, width, height, layers, levels, PackImageHeader::rgbe, payload.data());

    return id + 1;
  }

}


///////////////////////// create ////////////////////////////////////////////
void SkyboxDocument::create(string const &path)
{
  QJsonObject metadata;
  metadata["type"] = "SkyBox";
  metadata["icon"] = encode_icon(QIcon(":/skyboxplugin/icon.png"));

  QJsonObject definition;
  definition["width"] = 512;
  definition["height"] = 512;

  ofstream fout(path, ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_json(fout, 1, definition);

  write_asset_footer(fout);

  fout.close();
}


///////////////////////// hash //////////////////////////////////////////////
void SkyboxDocument::hash(Studio::Document *document, size_t *key)
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

  for(auto &name : ImageNames)
  {
    hash_combine(*key, image_hash(fullpath(document, definition[name].toString())));
  }
}


///////////////////////// build /////////////////////////////////////////////
void SkyboxDocument::build(Studio::Document *document, string const &path)
{
  auto skyboxdocument = SkyboxDocument(document);

  int width = skyboxdocument.width();
  int height = skyboxdocument.height();

  if (width == 0 || height == 0 || image_maxlevels(width, height) < 8)
    throw runtime_error("Skybox build failed - too small");

  ofstream fout(path, ios::binary | ios::trunc);

  write_header(fout);

  write_catalog(fout, 0);

  switch(skyboxdocument.type())
  {
    case SkyboxDocument::Type::FaceImages:
    {
      auto front = ImageDocument(skyboxdocument.image(SkyboxDocument::Image::Front)).data();
      auto left = ImageDocument(skyboxdocument.image(SkyboxDocument::Image::Left)).data();
      auto right = ImageDocument(skyboxdocument.image(SkyboxDocument::Image::Right)).data();
      auto back = ImageDocument(skyboxdocument.image(SkyboxDocument::Image::Back)).data();
      auto top = ImageDocument(skyboxdocument.image(SkyboxDocument::Image::Top)).data();
      auto bottom = ImageDocument(skyboxdocument.image(SkyboxDocument::Image::Bottom)).data();

      write_skybox(fout, 1, width, height, { right, left, bottom, top, front, back });

      break;
    }

    case SkyboxDocument::Type::SphericalMap:
    {
      auto envmap = ImageDocument(skyboxdocument.image(SkyboxDocument::Image::EnvMap)).data();

      write_skybox(fout, 1, width, height, envmap);

      break;
    }
  }

  write_chunk(fout, "HEND", 0, nullptr);

  fout.close();
}


///////////////////////// pack //////////////////////////////////////////////
void SkyboxDocument::pack(Studio::PackerState &asset, ofstream &fout)
{
  ifstream fin(asset.buildpath, ios::binary);

  if (!fin)
    throw runtime_error("Skybox Pack failed - no build file");

  PackImageHeader imag;

  if (read_asset_header(fin, 1, &imag))
  {
    vector<char> payload(pack_payload_size(imag));

    read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

    write_imag_asset(fout, asset.id, imag.width, imag.height, imag.layers, imag.levels, imag.format, payload.data());
  }
}


//|---------------------- SkyboxDocument ------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SkyboxDocument::Constructor ///////////////////////
SkyboxDocument::SkyboxDocument()
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_changed, this, &SkyboxDocument::touch);
}


///////////////////////// SkyboxDocument::Constructor ///////////////////////
SkyboxDocument::SkyboxDocument(QString const &path)
  : SkyboxDocument()
{
  if (path != "")
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
  }
}


///////////////////////// SkyboxDocument::Constructor ///////////////////////
SkyboxDocument::SkyboxDocument(Studio::Document *document)
  : SkyboxDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// SkyboxDocument::Constructor ///////////////////////
SkyboxDocument::SkyboxDocument(SkyboxDocument const &document)
  : SkyboxDocument(document.m_document)
{
}


///////////////////////// SkyboxDocument::Assignment ////////////////////////
SkyboxDocument SkyboxDocument::operator =(SkyboxDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// SkyboxDocument::attach ////////////////////////////
void SkyboxDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &SkyboxDocument::refresh);

    refresh();
  }
}


///////////////////////// SkyboxDocument::touch /////////////////////////////
void SkyboxDocument::touch(Studio::Document *document, QString const &path)
{
  for(auto &image : m_images)
  {
    if (document == image)
    {
      emit dependant_changed();
    }
  }
}


///////////////////////// SkyboxDocument::set_type //////////////////////////
void SkyboxDocument::set_type(Type type)
{
  m_definition["type"] = static_cast<int>(type);

  update();
}


///////////////////////// SkyboxDocument::set_width /////////////////////////
void SkyboxDocument::set_width(int width)
{
  m_definition["width"] = width;

  update();
}


///////////////////////// SkyboxDocument::set_height ////////////////////////
void SkyboxDocument::set_height(int height)
{
  m_definition["height"] = height;

  update();
}


///////////////////////// SkyboxDocument::set_image /////////////////////////
void SkyboxDocument::set_image(Image image, QString const &path)
{
  m_definition[ImageNames[static_cast<int>(image)]] = relpath(m_document, path);

  update();
}


///////////////////////// SkyboxDocument::refresh ///////////////////////////
void SkyboxDocument::refresh()
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

  for(auto &name : ImageNames)
  {
    auto path = fullpath(m_document, m_definition[name].toString());

    m_images.push_back((path != "") ? documentmanager->open(path) : nullptr);
  }

  emit document_changed();
}


///////////////////////// SkyboxDocument::update ////////////////////////////
void SkyboxDocument::update()
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
