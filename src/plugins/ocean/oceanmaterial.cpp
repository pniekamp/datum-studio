//
// Ocean Material Build
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "oceanmaterial.h"
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

static const char *ImageNames[2] = { "surfacemap", "normalmap" };

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

  uint32_t write_watermap(ostream &fout, uint32_t id, Color3 const &deepcolor, Color3 const &shallowcolor, float depthscale = 1.0f, Color3 const &fresnelcolor = { 0.0f, 0.0f, 0.0f }, float fresnelbias = 0.328f, float fresnelpower = 5.0f)
  {
    int width = 256;
    int height = 256;
    int layers = 1;
    int levels = 1;

    vector<char> payload(image_datasize(width, height, layers, levels));

    image_pack_watercolor(deepcolor, shallowcolor, depthscale, fresnelcolor, fresnelbias, fresnelpower, width, height, payload.data());

    write_imag_asset(fout, id, width, height, layers, levels, PackImageHeader::rgbe, payload.data());

    return id + 1;
  }

  uint32_t write_surfacemap(ostream &fout, uint32_t id, HDRImage const &image)
  {
    int width = image.width;
    int height = image.height;
    int layers = 1;
    int levels = image_maxlevels(width, height);

    if (image.bits.size() == 0)
      throw runtime_error("SurfaceMap build failed - null image");

    vector<char> payload(image_datasize(width, height, layers, levels));

    uint32_t *dst = (uint32_t*)payload.data();

    for(int y = 0; y < height; ++y)
    {
      for(int x = 0; x < width; ++x)
      {
        *dst++ = rgba(image.sample(x, (height - 1) - y));
      }
    }

    image_buildmips_rgb(width, height, layers, levels, payload.data());

    write_imag_asset(fout, id, width, height, layers, levels, PackImageHeader::rgba, payload.data());

    return id + 1;
  }

  uint32_t write_normalmap(ostream &fout, uint32_t id, HDRImage const &image)
  {
    int width = image.width;
    int height = image.height;
    int layers = 1;
    int levels = image_maxlevels(width, height);

    if (image.bits.size() == 0)
      throw runtime_error("NormalMap build failed - null image");

    vector<char> payload(image_datasize(width, height, layers, levels));

    uint32_t *dst = (uint32_t*)payload.data();

    for(int y = 0; y < height; ++y)
    {
      for(int x = 0; x < width; ++x)
      {
        *dst++ = rgba(image.sample(x, (height - 1) - y));
      }
    }

    image_buildmips_rgb(width, height, layers, levels, payload.data());

    write_imag_asset(fout, id, width, height, layers, levels, PackImageHeader::rgba, payload.data());

    return id + 1;
  }

}


///////////////////////// create ////////////////////////////////////////////
void OceanMaterialDocument::create(string const &path, Color4 const &color, float roughness)
{
  QJsonObject metadata;
  metadata["type"] = "Material\\Ocean";
  metadata["icon"] = encode_icon(QIcon(":/oceanplugin/icon.png"));

  QJsonObject definition;
  definition["color.r"] = color.r;
  definition["color.g"] = color.g;
  definition["color.b"] = color.b;
  definition["color.a"] = color.a;
  definition["roughness"] = roughness;

  definition["shallowcolor.r"] = 0.1f;
  definition["shallowcolor.g"] = 0.6f;
  definition["shallowcolor.b"] = 0.7f;

  definition["deepcolor.r"] = 0.0f;
  definition["deepcolor.g"] = 0.007f;
  definition["deepcolor.b"] = 0.005f;

  definition["fresnelcolor.r"] = 0.01f;
  definition["fresnelcolor.g"] = 0.05f;
  definition["fresnelcolor.b"] = 0.15f;

  ofstream fout(path, ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_json(fout, 1, definition);

  write_asset_footer(fout);

  fout.close();
}


///////////////////////// hash //////////////////////////////////////////////
void OceanMaterialDocument::hash(Studio::Document *document, size_t *key)
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


///////////////////////// hash //////////////////////////////////////////////
void OceanMaterialDocument::build_hash(Studio::Document *document, size_t *key)
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

  hash_combine(*key, std::hash<double>{}(definition["shallowcolor.r"].toDouble(1)));
  hash_combine(*key, std::hash<double>{}(definition["shallowcolor.g"].toDouble(1)));
  hash_combine(*key, std::hash<double>{}(definition["shallowcolor.b"].toDouble(1)));
  hash_combine(*key, std::hash<double>{}(definition["deepcolor.r"].toDouble(1)));
  hash_combine(*key, std::hash<double>{}(definition["deepcolor.g"].toDouble(1)));
  hash_combine(*key, std::hash<double>{}(definition["deepcolor.b"].toDouble(1)));
  hash_combine(*key, std::hash<double>{}(definition["fresnelcolor.r"].toDouble(1)));
  hash_combine(*key, std::hash<double>{}(definition["fresnelcolor.g"].toDouble(1)));
  hash_combine(*key, std::hash<double>{}(definition["fresnelcolor.b"].toDouble(1)));
  hash_combine(*key, std::hash<double>{}(definition["depthscale"].toDouble(1)));

  for(auto &name : ImageNames)
  {
    hash_combine(*key, image_hash(fullpath(document, definition[name].toString())));
  }
}


///////////////////////// build /////////////////////////////////////////////
void OceanMaterialDocument::build(Studio::Document *document, string const &path)
{
  auto materialdocument = OceanMaterialDocument(document);

  ofstream fout(path, ios::binary | ios::trunc);

  write_header(fout);

  write_catalog(fout, 0);

  write_watermap(fout, 1, materialdocument.deepcolor(), materialdocument.shallowcolor(), materialdocument.depthscale(), materialdocument.fresnelcolor(), 0.015f, 5.0f);

  if (materialdocument.image(OceanMaterialDocument::Image::SurfaceMap))
  {
    auto surfacemap = ImageDocument(materialdocument.image(OceanMaterialDocument::Image::SurfaceMap)).data(ImageDocument::raw);

    write_surfacemap(fout, 1, surfacemap);
  }

  if (materialdocument.image(OceanMaterialDocument::Image::NormalMap))
  {
    auto normalmap = ImageDocument(materialdocument.image(OceanMaterialDocument::Image::NormalMap)).data(ImageDocument::raw);

    write_normalmap(fout, 3, normalmap);
  }

  write_chunk(fout, "HEND", 0, nullptr);

  fout.close();
}


///////////////////////// pack //////////////////////////////////////////////
void OceanMaterialDocument::pack(Studio::PackerState &asset, ofstream &fout)
{
  ifstream fin(asset.buildpath, ios::binary);

  if (!fin)
    throw runtime_error("Material\\Ocean Pack failed - no build file");

  if (asset.type == "Material\\Ocean")
  {
    auto materialdocument = OceanMaterialDocument(asset.document);

    auto color = materialdocument.color();
    auto metalness = materialdocument.metalness();
    auto roughness = materialdocument.roughness();
    auto reflectivity = materialdocument.reflectivity();
    auto emissive = materialdocument.emissive();

    auto albedomap = asset.add_dependant(asset.document, "Material\\Ocean.AlbedoMap");

    auto surfacemap = 0;
    if (materialdocument.image(OceanMaterialDocument::Image::SurfaceMap))
      surfacemap = asset.add_dependant(asset.document, "Material\\Ocean.SurfaceMap");

    auto normalmap = 0;
    if (materialdocument.image(OceanMaterialDocument::Image::NormalMap))
      normalmap = asset.add_dependant(asset.document, "Material\\Ocean.NormalMap");

    write_matl_asset(fout, asset.id, color, metalness, roughness, reflectivity, emissive, albedomap, surfacemap, normalmap);
  }

  if (asset.type == "Material\\Ocean.AlbedoMap")
  {
    PackImageHeader imag;

    if (read_asset_header(fin, 1, &imag))
    {
      vector<char> payload(pack_payload_size(imag));

      read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

      image_compress_bc3(imag.width, imag.height, imag.layers, imag.levels, payload.data());

      write_imag_asset(fout, asset.id, imag.width, imag.height, imag.layers, imag.levels, PackImageHeader::rgba_bc3, payload.data());
    }
  }

  if (asset.type == "Material\\Ocean.SurfaceMap")
  {
    PackImageHeader imag;

    if (read_asset_header(fin, 2, &imag))
    {
      vector<char> payload(pack_payload_size(imag));

      read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

      image_compress_bc3(imag.width, imag.height, imag.layers, imag.levels, payload.data());

      write_imag_asset(fout, asset.id, imag.width, imag.height, imag.layers, imag.levels, PackImageHeader::rgba_bc3, payload.data());
    }
  }

  if (asset.type == "Material\\Ocean.NormalMap")
  {
    PackImageHeader imag;

    if (read_asset_header(fin, 3, &imag))
    {
      vector<char> payload(pack_payload_size(imag));

      read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

      write_imag_asset(fout, asset.id, imag.width, imag.height, imag.layers, imag.levels, imag.format, payload.data());
    }
  }
}


//|---------------------- OceanMaterialDocument -----------------------------
//|--------------------------------------------------------------------------

///////////////////////// OceanMaterialDocument::Constructor ////////////////
OceanMaterialDocument::OceanMaterialDocument()
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_changed, this, &OceanMaterialDocument::touch);
}


///////////////////////// OceanMaterialDocument::Constructor ////////////////
OceanMaterialDocument::OceanMaterialDocument(QString const &path)
  : OceanMaterialDocument()
{
  if (path != "")
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
  }
}


///////////////////////// OceanMaterialDocument::Constructor ////////////////
OceanMaterialDocument::OceanMaterialDocument(Studio::Document *document)
  : OceanMaterialDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// OceanMaterialDocument::Constructor ////////////////
OceanMaterialDocument::OceanMaterialDocument(OceanMaterialDocument const &document)
  : OceanMaterialDocument(document.m_document)
{
}


///////////////////////// OceanMaterialDocument::Assignment /////////////////
OceanMaterialDocument OceanMaterialDocument::operator =(OceanMaterialDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// OceanMaterialDocument::attach /////////////////////
void OceanMaterialDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &OceanMaterialDocument::refresh);

    refresh();
  }
}


///////////////////////// OceanMaterialDocument::touch //////////////////////
void OceanMaterialDocument::touch(Studio::Document *document, QString const &path)
{
  for(auto &image : m_images)
  {
    if (document == image)
    {
      emit dependant_changed();
    }
  }
}


///////////////////////// OceanMaterialDocument::set_color //////////////////
void OceanMaterialDocument::set_color(Color4 const &color)
{
  m_definition["color.r"] = color.r;
  m_definition["color.g"] = color.g;
  m_definition["color.b"] = color.b;
  m_definition["color.a"] = color.a;

  update();
}


///////////////////////// OceanMaterialDocument::set_shallowcolor ///////////
void OceanMaterialDocument::set_shallowcolor(Color3 const &color)
{
  m_definition["shallowcolor.r"] = color.r;
  m_definition["shallowcolor.g"] = color.g;
  m_definition["shallowcolor.b"] = color.b;

  update();
}


///////////////////////// OceanMaterialDocument::set_deepcolor //////////////
void OceanMaterialDocument::set_deepcolor(Color3 const &color)
{
  m_definition["deepcolor.r"] = color.r;
  m_definition["deepcolor.g"] = color.g;
  m_definition["deepcolor.b"] = color.b;

  update();
}


///////////////////////// OceanMaterialDocument::set_fresnelcolor ///////////
void OceanMaterialDocument::set_fresnelcolor(Color3 const &color)
{
  m_definition["fresnelcolor.r"] = color.r;
  m_definition["fresnelcolor.g"] = color.g;
  m_definition["fresnelcolor.b"] = color.b;

  update();
}


///////////////////////// OceanMaterialDocument::set_depthscale /////////////
void OceanMaterialDocument::set_depthscale(float depthscale)
{
  m_definition["depthscale"] = depthscale;

  update();
}


///////////////////////// OceanMaterialDocument::set_metalness //////////////
void OceanMaterialDocument::set_metalness(float metalness)
{
  m_definition["metalness"] = metalness;

  update();
}


///////////////////////// OceanMaterialDocument::set_roughness //////////////
void OceanMaterialDocument::set_roughness(float roughness)
{
  m_definition["roughness"] = roughness;

  update();
}


///////////////////////// OceanMaterialDocument::set_reflectivity ///////////
void OceanMaterialDocument::set_reflectivity(float reflectivity)
{
  m_definition["reflectivity"] = reflectivity;

  update();
}


///////////////////////// OceanMaterialDocument::set_emissive ///////////////
void OceanMaterialDocument::set_emissive(float emissive)
{
  m_definition["emissive"] = emissive;

  update();
}


///////////////////////// OceanMaterialDocument::set_image //////////////////
void OceanMaterialDocument::set_image(Image image, QString const &path)
{
  m_definition[ImageNames[static_cast<int>(image)]] = relpath(m_document, path);

  update();
}


///////////////////////// OceanMaterialDocument::refresh ////////////////////
void OceanMaterialDocument::refresh()
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


///////////////////////// OceanMaterialDocument::update /////////////////////
void OceanMaterialDocument::update()
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
