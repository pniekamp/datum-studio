//
// Terrain Material Build
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "terrainmaterial.h"
#include "image.h"
#include "material.h"
#include "buildapi.h"
#include "assetfile.h"
#include <QPainter>
#include <QJsonDocument>
#include <QJsonArray>
#include <QThread>
#include <functional>
#include <cassert>

#include <QDebug>

using namespace std;
using namespace lml;

namespace
{
  void hash_combine(size_t &seed, size_t key)
  {
    seed ^= key + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }

  size_t material_hash(QString const &path)
  {
    size_t key = 0;

    if (auto document = MaterialDocument(path))
    {
      MaterialDocument::hash(document, &key);
    }

    return key;
  }

  HDRImage read_image(QString const &path, int index)
  {
    HDRImage image = {};

    ifstream fin(path.toUtf8(), ios::binary);

    PackImageHeader imag;

    if (read_asset_header(fin, index, &imag))
    {
      vector<char> payload(pack_payload_size(imag));

      read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

      image.width = imag.width;
      image.height = imag.height;
      image.bits.resize(image.width * image.height);

      assert(imag.format == PackImageHeader::rgba);

      uint32_t *src = (uint32_t*)payload.data();

      for(size_t i = 0; i < image.bits.size(); ++i)
      {
        image.bits[i] = rgba(*src);

        ++src;
      }
    }

    return image;
  }

  HDRImage scale_image(HDRImage const &image, Color4 const &scale)
  {
    HDRImage result = image;

    for(size_t i = 0; i < result.bits.size(); ++i)
    {
      result.bits[i] = hada(result.bits[i], scale);
    }

    return result;
  }

  uint32_t write_catalog(ostream &fout, uint32_t id)
  {
    write_catl_asset(fout, id, 0, 0);

    return id + 1;
  }

  uint32_t write_imagemap(ostream &fout, uint32_t id, vector<HDRImage> const &images)
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
        throw runtime_error("Terrain Material build failed - null image");

      Vec2 area = Vec2(1.0f / min(width, image.width), 1.0f / min(height, image.height));

      for(int y = 0; y < height; ++y)
      {
        for(int x = 0; x < width; ++x)
        {
          *dst++ = rgba(image.sample(Vec2((x + 0.5f)/width, (y + 0.5f)/height), area));
        }
      }
    }

    image_buildmips_rgb(width, height, layers, levels, payload.data());

    write_imag_asset(fout, id, width, height, layers, levels, PackImageHeader::rgba, payload.data());

    return id + 1;
  }
}


///////////////////////// create ////////////////////////////////////////////
void TerrainMaterialDocument::create(string const &path, Color4 const &color)
{
  QJsonObject metadata;
  metadata["type"] = "Material\\Terrain";
  metadata["icon"] = encode_icon(QIcon(":/terrainplugin/icon.png"));

  QJsonObject definition;
  definition["color.r"] = color.r;
  definition["color.g"] = color.g;
  definition["color.b"] = color.b;
  definition["color.a"] = color.a;

  ofstream fout(path, ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_json(fout, 1, definition);

  write_asset_footer(fout);

  fout.close();
}


///////////////////////// hash //////////////////////////////////////////////
void TerrainMaterialDocument::hash(Studio::Document *document, size_t *key)
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

    hash_combine(*key, material_hash(fullpath(document, layer["path"].toString())));
  }
}


///////////////////////// hash //////////////////////////////////////////////
void TerrainMaterialDocument::build_hash(Studio::Document *document, size_t *key)
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

    hash_combine(*key, material_hash(fullpath(document, layer["path"].toString())));
  }
}


///////////////////////// build /////////////////////////////////////////////
void TerrainMaterialDocument::build(Studio::Document *document, string const &path)
{
  auto materialdocument = TerrainMaterialDocument(document);

  if (materialdocument.layers() == 0)
    throw runtime_error("Terrain Material build failed - no layers");

  vector<HDRImage> albedomaps;
  vector<HDRImage> surfacemaps;
  vector<HDRImage> normalmaps;

  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  for(int i = 0; i < materialdocument.layers(); ++i)
  {
    HDRImage albedomap(1, 1, Color4(1, 1, 1, 1));
    HDRImage surfacemap(1, 1, Color4(1, 1, 1, 1));
    HDRImage normalmap(1, 1, Color4(0.5, 0.5, 1, 1));

    if (auto layerdocument = MaterialDocument(materialdocument.layer(i)))
    {
      QString buildpath;

      if (!buildmanager->build(layerdocument, &buildpath))
        throw runtime_error("Terrain Material build failed - material sub-build error");

      if (layerdocument.image(MaterialDocument::Image::AlbedoMap))
        albedomap = read_image(buildpath, 1);

      if (layerdocument.image(MaterialDocument::Image::MetalnessMap) || layerdocument.image(MaterialDocument::Image::RoughnessMap) || layerdocument.image(MaterialDocument::Image::ReflectivityMap))
        surfacemap = read_image(buildpath, 2);

      if (layerdocument.image(MaterialDocument::Image::NormalMap))
        normalmap = read_image(buildpath, 3);

      albedomap = scale_image(albedomap, layerdocument.color());
      surfacemap = scale_image(surfacemap, Color4(layerdocument.metalness(), layerdocument.reflectivity(), 1, layerdocument.roughness()));
    }

    albedomaps.push_back(albedomap);
    surfacemaps.push_back(surfacemap);
    normalmaps.push_back(normalmap);
  }

  ofstream fout(path, ios::binary | ios::trunc);

  write_header(fout);

  write_catalog(fout, 0);

  write_imagemap(fout, 1, albedomaps);
  write_imagemap(fout, 2, surfacemaps);
  write_imagemap(fout, 3, normalmaps);

  write_chunk(fout, "HEND", 0, nullptr);

  fout.close();
}


///////////////////////// pack //////////////////////////////////////////////
void TerrainMaterialDocument::pack(Studio::PackerState &asset, ofstream &fout)
{
  ifstream fin(asset.buildpath, ios::binary);

  if (!fin)
    throw runtime_error("Material\\Terrain Pack failed - no build file");

  if (asset.type == "Material\\Terrain")
  {
    auto materialdocument = TerrainMaterialDocument(asset.document);

    auto color = materialdocument.color();
    auto metalness = 1;
    auto roughness = 1;
    auto reflectivity = 0.5f;
    auto emissive = 0;

    auto albedomap = asset.add_dependant(asset.document, "Material\\Terrain.AlbedoMap");
    auto surfacemap = asset.add_dependant(asset.document, "Material\\Terrain.SurfaceMap");
    auto normalmap = asset.add_dependant(asset.document, "Material\\Terrain.NormalMap");

    write_matl_asset(fout, asset.id, color, metalness, roughness, reflectivity, emissive, albedomap, surfacemap, normalmap);
  }

  if (asset.type == "Material\\Terrain.AlbedoMap")
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

  if (asset.type == "Material\\Terrain.SurfaceMap")
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

  if (asset.type == "Material\\Terrain.NormalMap")
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


//|---------------------- TerrainMaterialDocument -----------------------------
//|--------------------------------------------------------------------------

///////////////////////// TerrainMaterialDocument::Constructor ////////////////
TerrainMaterialDocument::TerrainMaterialDocument()
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_changed, this, &TerrainMaterialDocument::touch);
}


///////////////////////// TerrainMaterialDocument::Constructor ////////////////
TerrainMaterialDocument::TerrainMaterialDocument(QString const &path)
  : TerrainMaterialDocument()
{
  if (path != "")
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
  }
}


///////////////////////// TerrainMaterialDocument::Constructor ////////////////
TerrainMaterialDocument::TerrainMaterialDocument(Studio::Document *document)
  : TerrainMaterialDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// TerrainMaterialDocument::Constructor ////////////////
TerrainMaterialDocument::TerrainMaterialDocument(TerrainMaterialDocument const &document)
  : TerrainMaterialDocument(document.m_document)
{
}


///////////////////////// TerrainMaterialDocument::Assignment /////////////////
TerrainMaterialDocument TerrainMaterialDocument::operator =(TerrainMaterialDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// TerrainMaterialDocument::attach /////////////////////
void TerrainMaterialDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &TerrainMaterialDocument::refresh);

    refresh();
  }
}


///////////////////////// TerrainMaterialDocument::touch //////////////////////
void TerrainMaterialDocument::touch(Studio::Document *document, QString const &path)
{
  for(auto &material : m_materials)
  {
    if (document == material)
    {
      emit dependant_changed();
    }
  }
}


///////////////////////// TerrainMaterialDocument::set_color //////////////////
void TerrainMaterialDocument::set_color(Color4 const &color)
{
  m_definition["color.r"] = color.r;
  m_definition["color.g"] = color.g;
  m_definition["color.b"] = color.b;
  m_definition["color.a"] = color.a;

  update();
}


///////////////////////// TerrainMaterialDocument::set_metalness ////////////
void TerrainMaterialDocument::set_metalness(float metalness)
{
  m_definition["metalness"] = metalness;

  update();
}


///////////////////////// TerrainMaterialDocument::set_roughness ////////////
void TerrainMaterialDocument::set_roughness(float roughness)
{
  m_definition["roughness"] = roughness;

  update();
}


///////////////////////// TerrainMaterialDocument::set_reflectivity /////////
void TerrainMaterialDocument::set_reflectivity(float reflectivity)
{
  m_definition["reflectivity"] = reflectivity;

  update();
}


///////////////////////// TerrainMaterialDocument::set_emissive /////////////
void TerrainMaterialDocument::set_emissive(float emissive)
{
  m_definition["emissive"] = emissive;

  update();
}


///////////////////////// TerrainMaterialDocument::add_layer ////////////////
void TerrainMaterialDocument::add_layer(int position, QString const &path)
{
  QJsonObject layer;
  layer["path"] = relpath(m_document, path);

  auto layers = m_definition["layers"].toArray();

  layers.insert(position, layer);

  m_definition["layers"] = layers;

  update();
}


///////////////////////// TerrainMaterialDocument::move_layer ///////////////
void TerrainMaterialDocument::move_layer(int index, int position)
{
  auto layers = m_definition["layers"].toArray();

  layers.insert((index < position) ? position-1 : position, layers.takeAt(index));

  m_definition["layers"] = layers;

  update();
}


///////////////////////// TerrainMaterialDocument::erase_layer //////////////
void TerrainMaterialDocument::erase_layer(int index)
{
  auto layers = m_definition["layers"].toArray();

  layers.removeAt(index);

  m_definition["layers"] = layers;

  update();
}


///////////////////////// TerrainMaterialDocument::refresh ////////////////////
void TerrainMaterialDocument::refresh()
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

  m_materials.clear();

  for(auto i : m_definition["layers"].toArray())
  {
    auto layer = i.toObject();
    auto path = fullpath(m_document, layer["path"].toString());

    m_materials.push_back((path != "") ? documentmanager->open(path) : nullptr);
  }

  emit document_changed();
}


///////////////////////// TerrainMaterialDocument::update /////////////////////
void TerrainMaterialDocument::update()
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
