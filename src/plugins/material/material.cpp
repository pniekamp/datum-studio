//
// Material Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "material.h"
#include "image.h"
#include "assetfile.h"
#include <QPainter>
#include <QJsonDocument>
#include <functional>
#include <cassert>

#include <QDebug>

using namespace std;
using namespace lml;

static const char *ImageNames[7] = { "albedomap", "albedomask", "metalnessmap", "roughnessmap", "reflectivitymap", "normalmap" };

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

  HDRImage image_data(QString const &path)
  {
    HDRImage image = {};

    if (auto document = ImageDocument(path))
    {
      image = document.data();
    }

    return image;
  }

  HDRImage image_data_raw(QString const &path)
  {
    HDRImage image = {};

    if (auto document = ImageDocument(path))
    {
      image = document.data(ImageDocument::raw);
    }

    return image;
  }

  HDRImage normalmap_from_image(HDRImage const &src, float strength = 1.0f)
  {
    HDRImage normalmap(src.width, src.height);

    for(int y = 0; y < src.height; ++y)
    {
      for(int x = 0; x < src.width; ++x)
      {
        Color4 grid[9] = {
          src.sample((x+src.width-1)%src.width, (y+src.height-1)%src.height),
          src.sample((x+src.width  )%src.width, (y+src.height-1)%src.height),
          src.sample((x+src.width+1)%src.width, (y+src.height-1)%src.height),
          src.sample((x+src.width-1)%src.width, (y+src.height  )%src.height),
          src.sample((x+src.width  )%src.width, (y+src.height  )%src.height),
          src.sample((x+src.width+1)%src.width, (y+src.height  )%src.height),
          src.sample((x+src.width-1)%src.width, (y+src.height+1)%src.height),
          src.sample((x+src.width  )%src.width, (y+src.height+1)%src.height),
          src.sample((x+src.width+1)%src.width, (y+src.height+1)%src.height)
        };

        float s[9];
        for(int i = 0; i < 9; ++i)
          s[i] = (grid[i].r + grid[i].g + grid[i].b) / 3.0;

        Vec3 normal = normalise(Vec3((s[2] + 2*s[5] + s[8]) - (s[0] + 2*s[3] + s[6]), (s[0] + 2*s[1] + s[2]) - (s[6] + 2*s[7] + s[8]), 1/strength));

        normalmap.bits[y * normalmap.width + x] = Color4(0.5f*normal.x+0.5f, 0.5f*normal.y+0.5f, 0.5f*normal.z+0.5f, 1);
      }
    }

    return normalmap;
  }

  uint32_t write_catalog(ostream &fout, uint32_t id)
  {
    write_catl_asset(fout, id);

    return id + 1;
  }

  uint32_t write_albedomap(ostream &fout, uint32_t id, HDRImage const &image)
  {
    int width = image.width;
    int height = image.height;
    int layers = 1;
    int levels = min(4, image_maxlevels(width, height));

    if (image.bits.size() == 0)
      throw runtime_error("AlbedoMap build failed - null image");

    vector<char> payload(image_datasize(width, height, layers, levels));

    uint32_t *dst = (uint32_t*)payload.data();

    for(int y = 0; y < height; ++y)
    {
      for(int x = 0; x < width; ++x)
      {
        *dst++ = srgba(image.sample(x, (height - 1) - y));
      }
    }

    image_buildmips_srgb_a(0.5, width, height, layers, levels, payload.data());

    write_imag_asset(fout, id, width, height, layers, levels, PackImageHeader::rgba, payload.data());

    return id + 1;
  }

  uint32_t write_specularmap(ostream &fout, uint32_t id, HDRImage const &image)
  {
    int width = image.width;
    int height = image.height;
    int layers = 1;
    int levels = min(4, image_maxlevels(width, height));

    if (image.bits.size() == 0)
      throw runtime_error("SpecularMap build failed - null image");

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
    int levels = min(4, image_maxlevels(width, height));

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
void MaterialDocument::create(string const &path, lml::Color3 const &color, float metalness, float roughness)
{
  QJsonObject metadata;
  metadata["type"] = "Material";
  metadata["icon"] = encode_icon(QIcon(":/materialplugin/icon.png"));

  QJsonObject definition;
  definition["color.r"] = color.r;
  definition["color.g"] = color.g;
  definition["color.b"] = color.b;
  definition["metalness"] = metalness;
  definition["roughness"] = roughness;

  ofstream fout(path, ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_json(fout, 1, definition);

  write_asset_footer(fout);

  fout.close();
}


///////////////////////// hash //////////////////////////////////////////////
void MaterialDocument::hash(Studio::Document *document, size_t *key)
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
void MaterialDocument::build_hash(Studio::Document *document, size_t *key)
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

  hash_combine(*key, std::hash<int>{}(definition["albedooutput"].toInt()));
  hash_combine(*key, std::hash<int>{}(definition["metalnessoutput"].toInt()));
  hash_combine(*key, std::hash<int>{}(definition["roughnessoutput"].toInt()));
  hash_combine(*key, std::hash<int>{}(definition["reflectivityoutput"].toInt()));
  hash_combine(*key, std::hash<int>{}(definition["normaloutput"].toInt()));

  for(auto &name : ImageNames)
  {
    hash_combine(*key, image_hash(fullpath(document, definition[name].toString())));
  }
}


///////////////////////// build /////////////////////////////////////////////
void MaterialDocument::build(Studio::Document *document, string const &path)
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

  ofstream fout(path, ios::binary | ios::trunc);

  write_header(fout);

  write_catalog(fout, 0);

  if (definition["albedomap"].toString() != "")
  {
    HDRImage albedomap = image_data(fullpath(document, definition["albedomap"].toString()));

    if (definition["albedomask"].toString() != "")
    {
      HDRImage albedomask = image_data(fullpath(document, definition["albedomask"].toString()));

      if (albedomap.width != albedomask.width || albedomap.height != albedomap.height)
        throw runtime_error("Material build failed - albedo mask size mismatch");

      for(int i = 0; i < albedomap.width * albedomap.height; ++i)
      {
        if (albedomask.bits[i].r < 0.5f)
          albedomap.bits[i].a = 0;
      }
    }

    write_albedomap(fout, 1, albedomap);
  }

  if (definition["metalnessmap"].toString() != "" || definition["roughnessmap"].toString() != "" || definition["reflectivitymap"].toString() != "")
  {
    HDRImage metalnessmap = image_data_raw(fullpath(document, definition["metalnessmap"].toString()));
    HDRImage roughnessmap = image_data_raw(fullpath(document, definition["roughnessmap"].toString()));
    HDRImage reflectivitymap = image_data_raw(fullpath(document, definition["reflectivitymap"].toString()));

    HDRImage specularmap;
    specularmap.width = max({ metalnessmap.width, roughnessmap.width, reflectivitymap.width });;
    specularmap.height = max({ metalnessmap.height, roughnessmap.height, reflectivitymap.height });
    specularmap.bits = vector<Color4>(specularmap.width * specularmap.height, Color4(1, 1, 1, 1));

    if (definition["metalnessmap"].toString() != "")
    {
      if (metalnessmap.width != specularmap.width || metalnessmap.height != specularmap.height)
        throw runtime_error("Material build failed - metalness map size mismatch");

      switch(definition["metalnessoutput"].toInt())
      {
        case 0: // r
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].r = metalnessmap.bits[i].r;
          break;

        case 1: // g
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].r = metalnessmap.bits[i].g;
          break;

        case 2: // b
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].r = metalnessmap.bits[i].b;
          break;

        case 3: // a
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].r = metalnessmap.bits[i].a;
          break;

        case 4: // invr
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].r = 1 - metalnessmap.bits[i].r;
          break;

        case 5: // invg
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].r = 1 - metalnessmap.bits[i].g;
          break;

        case 6: // invb
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].r = 1 - metalnessmap.bits[i].b;
          break;

        case 7: // inva
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].r = 1 - metalnessmap.bits[i].a;
          break;
      }
    }

    if (definition["roughnessmap"].toString() != "")
    {
      if (roughnessmap.width != specularmap.width || roughnessmap.height != specularmap.height)
        throw runtime_error("Material build failed - roughness map size mismatch");

      switch(definition["roughnessoutput"].toInt())
      {
        case 0: // r
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].a = roughnessmap.bits[i].r;
          break;

        case 1: // g
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].a = roughnessmap.bits[i].g;
          break;

        case 2: // b
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].a = roughnessmap.bits[i].b;
          break;

        case 3: // a
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].a = roughnessmap.bits[i].a;
          break;

        case 4: // invr
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].a = 1 - roughnessmap.bits[i].r;
          break;

        case 5: // invg
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].a = 1 - roughnessmap.bits[i].g;
          break;

        case 6: // invb
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].a = 1 - roughnessmap.bits[i].b;
          break;

        case 7: // inva
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].a = 1 - roughnessmap.bits[i].a;
          break;
      }
    }

    if (definition["reflectivitymap"].toString() != "")
    {
      if (reflectivitymap.width != specularmap.width || reflectivitymap.height != specularmap.height)
        throw runtime_error("Material build failed - reflectivity map size mismatch");

      switch(definition["reflectivityoutput"].toInt())
      {
        case 0: // r
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].g = reflectivitymap.bits[i].r;
          break;

        case 1: // g
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].g = reflectivitymap.bits[i].g;
          break;

        case 2: // b
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].g = reflectivitymap.bits[i].b;
          break;

        case 3: // a
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].g = reflectivitymap.bits[i].a;
          break;

        case 4: // invr
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].g = 1 - reflectivitymap.bits[i].r;
          break;

        case 5: // invg
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].g = 1 - reflectivitymap.bits[i].g;
          break;

        case 6: // invb
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].g = 1 - reflectivitymap.bits[i].b;
          break;

        case 7: // inva
          for(size_t i = 0; i < specularmap.bits.size(); ++i)
            specularmap.bits[i].g = 1 - reflectivitymap.bits[i].a;
          break;
      }
    }

    write_specularmap(fout, 2, specularmap);
  }

  if (definition["normalmap"].toString() != "")
  {
    HDRImage normalmap = image_data_raw(fullpath(document, definition["normalmap"].toString()));

    switch(definition["normaloutput"].toInt())
    {
      case 0: // xyz
        break;

      case 1: // xinyz
        for(size_t i = 0; i < normalmap.bits.size(); ++i)
          normalmap.bits[i].g = 1 - normalmap.bits[i].g;
        break;

      case 2: // bump
        normalmap = normalmap_from_image(normalmap);
        break;
    }

    write_normalmap(fout, 3, normalmap);
  }

  write_chunk(fout, "HEND", 0, nullptr);

  fout.close();
}


///////////////////////// pack //////////////////////////////////////////////
void MaterialDocument::pack(Studio::PackerState &asset, ofstream &fout)
{
  ifstream fin(asset.buildpath, ios::binary);

  if (!fin)
    throw runtime_error("Material Pack failed - no build file");

  if (asset.type == "Material")
  {
    MaterialDocument materialdocument(asset.document);

    auto color = materialdocument.color();
    auto metalness = materialdocument.metalness();
    auto roughness = materialdocument.roughness();
    auto reflectivity = materialdocument.reflectivity();
    auto emissive = materialdocument.emissive();

    auto albedomap = 0;
    if (materialdocument.image(MaterialDocument::Image::AlbedoMap))
      albedomap = asset.add_dependant(asset.document, "Material.AlbedoMap");

    auto specularmap = 0;
    if (materialdocument.image(MaterialDocument::Image::MetalnessMap) || materialdocument.image(MaterialDocument::Image::RoughnessMap) || materialdocument.image(MaterialDocument::Image::ReflectivityMap))
      specularmap = asset.add_dependant(asset.document, "Material.SpecularMap");

    auto normalmap = 0;
    if (materialdocument.image(MaterialDocument::Image::NormalMap))
      normalmap = asset.add_dependant(asset.document, "Material.NormalMap");

    write_matl_asset(fout, asset.id, color, metalness, roughness, reflectivity, emissive, albedomap, specularmap, normalmap);
  }

  if (asset.type == "Material.AlbedoMap")
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

  if (asset.type == "Material.SpecularMap")
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

  if (asset.type == "Material.NormalMap")
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


//|---------------------- MaterialDocument ----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MaterialDocument::Constructor /////////////////////
MaterialDocument::MaterialDocument()
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_changed, this, &MaterialDocument::touch);
}


///////////////////////// MaterialDocument::Constructor /////////////////////
MaterialDocument::MaterialDocument(QString const &path)
  : MaterialDocument()
{
  if (path != "")
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
  }
}


///////////////////////// MaterialDocument::Constructor /////////////////////
MaterialDocument::MaterialDocument(Studio::Document *document)
  : MaterialDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// MaterialDocument::Constructor /////////////////////
MaterialDocument::MaterialDocument(MaterialDocument const &document)
  : MaterialDocument(document.m_document)
{
}


///////////////////////// MaterialDocument::Assignment //////////////////////
MaterialDocument MaterialDocument::operator =(MaterialDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// MaterialDocument::attach //////////////////////////
void MaterialDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &MaterialDocument::refresh);

    refresh();
  }
}


///////////////////////// MaterialDocument::touch /////////////////////////////
void MaterialDocument::touch(Studio::Document *document, QString const &path)
{
  for(auto &image : m_images)
  {
    if (document == image)
    {
      emit dependant_changed();
    }
  }
}


///////////////////////// MaterialDocument::set_color ///////////////////////
void MaterialDocument::set_color(Color3 const &color)
{
  m_definition["color.r"] = color.r;
  m_definition["color.g"] = color.g;
  m_definition["color.b"] = color.b;

  update();
}


///////////////////////// MaterialDocument::set_metalness ///////////////////
void MaterialDocument::set_metalness(float metalness)
{
  m_definition["metalness"] = metalness;

  update();
}


///////////////////////// MaterialDocument::set_roughness ///////////////////
void MaterialDocument::set_roughness(float roughness)
{
  m_definition["roughness"] = roughness;

  update();
}


///////////////////////// MaterialDocument::set_reflectivity ////////////////
void MaterialDocument::set_reflectivity(float reflectivity)
{
  m_definition["reflectivity"] = reflectivity;

  update();
}


///////////////////////// MaterialDocument::set_emissive ////////////////////
void MaterialDocument::set_emissive(float emissive)
{
  m_definition["emissive"] = emissive;

  update();
}


///////////////////////// MaterialDocument::set_image ///////////////////////
void MaterialDocument::set_image(Image image, QString const &path)
{
  m_definition[ImageNames[static_cast<int>(image)]] = relpath(m_document, path);

  update();
}


///////////////////////// MaterialDocument::set_albedooutput ////////////////
void MaterialDocument::set_albedooutput(AlbedoOutput output)
{
  m_definition["albedooutput"] = static_cast<int>(output);

  update();
}


///////////////////////// MaterialDocument::set_albedooutput ////////////////
void MaterialDocument::set_metalnessoutput(MaterialDocument::MetalnessOutput output)
{
  m_definition["metalnessoutput"] = static_cast<int>(output);

  update();
}


///////////////////////// MaterialDocument::set_albedooutput ////////////////
void MaterialDocument::set_roughnessoutput(MaterialDocument::RoughnessOutput output)
{
  m_definition["roughnessoutput"] = static_cast<int>(output);

  update();
}


///////////////////////// MaterialDocument::set_albedooutput ////////////////
void MaterialDocument::set_reflectivityoutput(MaterialDocument::ReflectivityOutput output)
{
  m_definition["reflectivityoutput"] = static_cast<int>(output);

  update();
}


///////////////////////// MaterialDocument::set_albedooutput ////////////////
void MaterialDocument::set_normaloutput(MaterialDocument::NormalOutput output)
{
  m_definition["normaloutput"] = static_cast<int>(output);

  update();
}


///////////////////////// MaterialDocument::refresh /////////////////////////
void MaterialDocument::refresh()
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


///////////////////////// MaterialDocument::update //////////////////////////
void MaterialDocument::update()
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
