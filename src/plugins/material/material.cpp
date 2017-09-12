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
    write_catl_asset(fout, id, 0, 0);

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

  uint32_t write_surfacemap(ostream &fout, uint32_t id, HDRImage const &image)
  {
    int width = image.width;
    int height = image.height;
    int layers = 1;
    int levels = min(4, image_maxlevels(width, height));

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
void MaterialDocument::create(string const &path, Color4 const &color, float metalness, float roughness)
{
  QJsonObject metadata;
  metadata["type"] = "Material";
  metadata["icon"] = encode_icon(QIcon(":/materialplugin/icon.png"));

  QJsonObject definition;
  definition["color.r"] = color.r;
  definition["color.g"] = color.g;
  definition["color.b"] = color.b;
  definition["color.a"] = color.a;
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

  *key = 0;

  hash_combine(*key, std::hash<int>{}(definition["albedooutput"].toInt(0)));
  hash_combine(*key, std::hash<int>{}(definition["metalnessoutput"].toInt(0)));
  hash_combine(*key, std::hash<int>{}(definition["roughnessoutput"].toInt(3)));
  hash_combine(*key, std::hash<int>{}(definition["reflectivityoutput"].toInt(1)));
  hash_combine(*key, std::hash<int>{}(definition["normaloutput"].toInt(0)));

  for(auto &name : ImageNames)
  {
    hash_combine(*key, image_hash(fullpath(document, definition[name].toString())));
  }
}


///////////////////////// build /////////////////////////////////////////////
void MaterialDocument::build(Studio::Document *document, string const &path)
{
  auto materialdocument = MaterialDocument(document);

  ofstream fout(path, ios::binary | ios::trunc);

  write_header(fout);

  write_catalog(fout, 0);

  if (materialdocument.image(MaterialDocument::Image::AlbedoMap))
  {
    auto albedomap = ImageDocument(materialdocument.image(MaterialDocument::Image::AlbedoMap)).data();

    if (materialdocument.image(MaterialDocument::Image::AlbedoMask))
    {
      auto albedomask = ImageDocument(materialdocument.image(MaterialDocument::Image::AlbedoMask)).data();

      if (albedomask.width != albedomap.width || albedomask.height != albedomap.height)
        throw runtime_error("Material build failed - albedo mask size mismatch");

      for(int i = 0; i < albedomap.width * albedomap.height; ++i)
      {
        if (albedomask.bits[i].r < 0.5f)
          albedomap.bits[i].a = 0;
      }
    }

    write_albedomap(fout, 1, albedomap);
  }

  if (materialdocument.image(MaterialDocument::Image::MetalnessMap) || materialdocument.image(MaterialDocument::Image::RoughnessMap) || materialdocument.image(MaterialDocument::Image::ReflectivityMap))
  {
    auto metalnessmap = ImageDocument(materialdocument.image(MaterialDocument::Image::MetalnessMap)).data(ImageDocument::raw);
    auto roughnessmap = ImageDocument(materialdocument.image(MaterialDocument::Image::RoughnessMap)).data(ImageDocument::raw);
    auto reflectivitymap = ImageDocument(materialdocument.image(MaterialDocument::Image::ReflectivityMap)).data(ImageDocument::raw);

    HDRImage surfacemap;
    surfacemap.width = max({ metalnessmap.width, roughnessmap.width, reflectivitymap.width });;
    surfacemap.height = max({ metalnessmap.height, roughnessmap.height, reflectivitymap.height });
    surfacemap.bits = vector<Color4>(surfacemap.width * surfacemap.height, Color4(1, 1, 1, 1));

    if (materialdocument.image(MaterialDocument::Image::MetalnessMap))
    {
      if (metalnessmap.width != surfacemap.width || metalnessmap.height != surfacemap.height)
        throw runtime_error("Material build failed - metalness map size mismatch");

      switch(materialdocument.metalnessoutput())
      {
        case MaterialDocument::MetalnessOutput::r:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].r = metalnessmap.bits[i].r;
          break;

        case MaterialDocument::MetalnessOutput::g:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].r = metalnessmap.bits[i].g;
          break;

        case MaterialDocument::MetalnessOutput::b:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].r = metalnessmap.bits[i].b;
          break;

        case MaterialDocument::MetalnessOutput::a:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].r = metalnessmap.bits[i].a;
          break;

        case MaterialDocument::MetalnessOutput::invr:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].r = 1 - metalnessmap.bits[i].r;
          break;

        case MaterialDocument::MetalnessOutput::invg:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].r = 1 - metalnessmap.bits[i].g;
          break;

        case MaterialDocument::MetalnessOutput::invb:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].r = 1 - metalnessmap.bits[i].b;
          break;

        case MaterialDocument::MetalnessOutput::inva:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].r = 1 - metalnessmap.bits[i].a;
          break;
      }
    }

    if (materialdocument.image(MaterialDocument::Image::RoughnessMap))
    {
      if (roughnessmap.width != surfacemap.width || roughnessmap.height != surfacemap.height)
        throw runtime_error("Material build failed - roughness map size mismatch");

      switch(materialdocument.roughnessoutput())
      {
        case MaterialDocument::RoughnessOutput::r:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].a = roughnessmap.bits[i].r;
          break;

        case MaterialDocument::RoughnessOutput::g:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].a = roughnessmap.bits[i].g;
          break;

        case MaterialDocument::RoughnessOutput::b:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].a = roughnessmap.bits[i].b;
          break;

        case MaterialDocument::RoughnessOutput::a:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].a = roughnessmap.bits[i].a;
          break;

        case MaterialDocument::RoughnessOutput::invr:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].a = 1 - roughnessmap.bits[i].r;
          break;

        case MaterialDocument::RoughnessOutput::invg:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].a = 1 - roughnessmap.bits[i].g;
          break;

        case MaterialDocument::RoughnessOutput::invb:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].a = 1 - roughnessmap.bits[i].b;
          break;

        case MaterialDocument::RoughnessOutput::inva:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].a = 1 - roughnessmap.bits[i].a;
          break;
      }
    }

    if (materialdocument.image(MaterialDocument::Image::ReflectivityMap))
    {
      if (reflectivitymap.width != surfacemap.width || reflectivitymap.height != surfacemap.height)
        throw runtime_error("Material build failed - reflectivity map size mismatch");

      switch(materialdocument.reflectivityoutput())
      {
        case MaterialDocument::ReflectivityOutput::r:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].g = reflectivitymap.bits[i].r;
          break;

        case MaterialDocument::ReflectivityOutput::g:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].g = reflectivitymap.bits[i].g;
          break;

        case MaterialDocument::ReflectivityOutput::b:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].g = reflectivitymap.bits[i].b;
          break;

        case MaterialDocument::ReflectivityOutput::a:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].g = reflectivitymap.bits[i].a;
          break;

        case MaterialDocument::ReflectivityOutput::invr:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].g = 1 - reflectivitymap.bits[i].r;
          break;

        case MaterialDocument::ReflectivityOutput::invg:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].g = 1 - reflectivitymap.bits[i].g;
          break;

        case MaterialDocument::ReflectivityOutput::invb:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].g = 1 - reflectivitymap.bits[i].b;
          break;

        case MaterialDocument::ReflectivityOutput::inva:
          for(size_t i = 0; i < surfacemap.bits.size(); ++i)
            surfacemap.bits[i].g = 1 - reflectivitymap.bits[i].a;
          break;
      }
    }

    write_surfacemap(fout, 2, surfacemap);
  }

  if (materialdocument.image(MaterialDocument::Image::NormalMap))
  {
    auto normalmap = ImageDocument(materialdocument.image(MaterialDocument::Image::NormalMap)).data(ImageDocument::raw);

    switch(materialdocument.normaloutput())
    {
      case MaterialDocument::NormalOutput::xyz:
        break;

      case MaterialDocument::NormalOutput::xinvyz:
        for(size_t i = 0; i < normalmap.bits.size(); ++i)
          normalmap.bits[i].g = 1 - normalmap.bits[i].g;
        break;

      case MaterialDocument::NormalOutput::bump:
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
    auto materialdocument = MaterialDocument(asset.document);

    auto color = materialdocument.color();
    auto metalness = materialdocument.metalness();
    auto roughness = materialdocument.roughness();
    auto reflectivity = materialdocument.reflectivity();
    auto emissive = materialdocument.emissive();

    auto albedomap = 0;
    if (materialdocument.image(MaterialDocument::Image::AlbedoMap))
      albedomap = asset.add_dependant(asset.document, "Material.AlbedoMap");

    auto surfacemap = 0;
    if (materialdocument.image(MaterialDocument::Image::MetalnessMap) || materialdocument.image(MaterialDocument::Image::RoughnessMap) || materialdocument.image(MaterialDocument::Image::ReflectivityMap))
      surfacemap = asset.add_dependant(asset.document, "Material.SurfaceMap");

    auto normalmap = 0;
    if (materialdocument.image(MaterialDocument::Image::NormalMap))
      normalmap = asset.add_dependant(asset.document, "Material.NormalMap");

    write_matl_asset(fout, asset.id, color, metalness, roughness, reflectivity, emissive, albedomap, surfacemap, normalmap);
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

  if (asset.type == "Material.SurfaceMap")
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


///////////////////////// MaterialDocument::touch ///////////////////////////
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


///////////////////////// MaterialDocument::set_shader //////////////////////
void MaterialDocument::set_shader(Shader shader)
{
  m_definition["shader"] = static_cast<int>(shader);

  update();
}


///////////////////////// MaterialDocument::set_color ///////////////////////
void MaterialDocument::set_color(Color4 const &color)
{
  m_definition["color.r"] = color.r;
  m_definition["color.g"] = color.g;
  m_definition["color.b"] = color.b;
  m_definition["color.a"] = color.a;

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
