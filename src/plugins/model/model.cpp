//
// Model Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "model.h"
#include "mesh.h"
#include "material.h"
#include "assetfile.h"
#include "atlaspacker.h"
#include <functional>
#include <cassert>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>

#include <QDebug>

using namespace std;
using namespace lml;
using leap::extentof;
using leap::indexof;

namespace
{
  void hash_combine(size_t &seed, size_t key)
  {
    seed ^= key + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }

  size_t mesh_hash(QString const &path)
  {
    size_t key = 0;

    if (auto document = MeshDocument(path))
    {
      MeshDocument::hash(document, &key);
    }

    return key;
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
}


///////////////////////// create ////////////////////////////////////////////
void ModelDocument::create(string const &path)
{
  QJsonObject metadata;
  metadata["type"] = "Model";
  metadata["icon"] = encode_icon(QIcon(":/modelplugin/icon.png"));

  QJsonObject definition;

  ofstream fout(path, ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_json(fout, 1, definition);

  write_asset_footer(fout);

  fout.close();
}


///////////////////////// hash //////////////////////////////////////////////
void ModelDocument::hash(Studio::Document *document, size_t *key)
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

  for(auto i : definition["meshes"].toArray())
  {
    auto mesh = i.toObject();

    hash_combine(*key, mesh_hash(fullpath(document, mesh["path"].toString())));

    for(auto j : mesh["materials"].toArray())
    {
      auto material = j.toObject();

      hash_combine(*key, material_hash(fullpath(document, material["path"].toString())));
    }
  }
}


///////////////////////// pack //////////////////////////////////////////////
void ModelDocument::pack(Studio::PackerState &asset, ofstream &fout)
{
  vector<PackModelPayload::Texture> textures;
  vector<PackModelPayload::Material> materials;
  vector<PackModelPayload::Mesh> meshes;
  vector<PackModelPayload::Instance> instances;

  map<std::tuple<Studio::Document*, size_t>, size_t> meshmap;
  map<std::tuple<Studio::Document*, int>, size_t> texturemap;
  map<std::tuple<Studio::Document*, float, float, float>, size_t> materialmap;

  auto make_meshkey = [](Studio::Document *document, size_t index) { return make_tuple(document, index); };
  auto make_texturekey = [](Studio::Document *document, size_t slot) { return make_tuple(document, slot); };
  auto make_materialkey = [](Studio::Document *document, Color3 const &tint) { return make_tuple(document, tint.r, tint.g, tint.b); };

  textures.push_back({ PackModelPayload::Texture::nullmap, 0 });

  auto modeldocument = ModelDocument(asset.document);

  for(auto &instance : modeldocument.instances())
  {
    auto mesh = meshmap.find(make_meshkey(instance.submesh->document, instance.submesh->index));

    if (mesh == meshmap.end())
    {
      PackModelPayload::Mesh entry;

      entry.mesh = asset.add_dependant(instance.submesh->document, instance.submesh->index, "Mesh");

      meshes.push_back(entry);

      mesh = meshmap.insert({ make_meshkey(instance.submesh->document, instance.submesh->index), meshes.size() - 1 }).first;
    }

    auto material = materialmap.find(make_materialkey(instance.material->document, instance.material->tint));

    if (material == materialmap.end())
    {
      PackModelPayload::Material entry;

      entry.color[0] = instance.material->tint.r;
      entry.color[1] = instance.material->tint.g;
      entry.color[2] = instance.material->tint.b;
      entry.color[3] = 1.0f;
      entry.metalness = 0.0f;
      entry.roughness = 1.0f;
      entry.reflectivity = 0.5f;
      entry.emissive = 0.0f;
      entry.albedomap = 0;
      entry.surfacemap = 0;
      entry.normalmap = 0;

      if (auto materialdocument = MaterialDocument(instance.material->document))
      {
        entry.color[0] = instance.material->tint.r * materialdocument.color().r;
        entry.color[1] = instance.material->tint.g * materialdocument.color().g;
        entry.color[2] = instance.material->tint.b * materialdocument.color().b;
        entry.color[3] = materialdocument.color().a;
        entry.metalness = materialdocument.metalness();
        entry.roughness = materialdocument.roughness();
        entry.reflectivity = materialdocument.reflectivity();
        entry.emissive = materialdocument.emissive();

        if (materialdocument.image(MaterialDocument::Image::AlbedoMap))
        {
          auto texture = texturemap.find(make_texturekey(instance.material->document, 1));

          if (texture == texturemap.end())
          {
            PackModelPayload::Texture entry;

            entry.type = PackModelPayload::Texture::albedomap;
            entry.texture = asset.add_dependant(instance.material->document, "Material.AlbedoMap");;

            textures.push_back(entry);

            texture = texturemap.insert({ make_texturekey(instance.material->document, 1), textures.size() - 1 }).first;
          }

          entry.albedomap = texture->second;
        }

        if (materialdocument.image(MaterialDocument::Image::MetalnessMap) || materialdocument.image(MaterialDocument::Image::RoughnessMap) || materialdocument.image(MaterialDocument::Image::ReflectivityMap))
        {
          auto texture = texturemap.find(make_texturekey(instance.material->document, 2));

          if (texture == texturemap.end())
          {
            PackModelPayload::Texture entry;

            entry.type = PackModelPayload::Texture::surfacemap;
            entry.texture = asset.add_dependant(instance.material->document, "Material.SurfaceMap");;

            textures.push_back(entry);

            texture = texturemap.insert({ make_texturekey(instance.material->document, 2), textures.size() - 1 }).first;
          }

          entry.surfacemap = texture->second;
        }

        if (materialdocument.image(MaterialDocument::Image::NormalMap))
        {
          auto texture = texturemap.find(make_texturekey(instance.material->document, 3));

          if (texture == texturemap.end())
          {
            PackModelPayload::Texture entry;

            entry.type = PackModelPayload::Texture::normalmap;
            entry.texture = asset.add_dependant(instance.material->document, "Material.NormalMap");;

            textures.push_back(entry);

            texture = texturemap.insert({ make_texturekey(instance.material->document, 3), textures.size() - 1 }).first;
          }

          entry.normalmap = texture->second;
        }
      }

      materials.push_back(entry);

      material = materialmap.insert({ make_materialkey(instance.material->document, instance.material->tint), materials.size() - 1 }).first;
    }

    PackModelPayload::Instance entry;

    entry.mesh = mesh->second;
    entry.material = material->second;
    memcpy(&entry.transform, &instance.transform, sizeof(Transform));
    entry.childcount = 0;

    instances.push_back(entry);
  }

  write_modl_asset(fout, asset.id, textures, materials, meshes, instances);
}


//|---------------------- ModelDocument -------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ModelDocument::Constructor ////////////////////////
ModelDocument::ModelDocument()
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_changed, this, &ModelDocument::touch);
}


///////////////////////// ModelDocument::Constructor ////////////////////////
ModelDocument::ModelDocument(QString const &path)
  : ModelDocument()
{
  if (path != "")
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
  }
}


///////////////////////// ModelDocument::Constructor ////////////////////////
ModelDocument::ModelDocument(Studio::Document *document)
  : ModelDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// ModelDocument::Constructor ////////////////////////
ModelDocument::ModelDocument(ModelDocument const &document)
  : ModelDocument(document.m_document)
{
}


///////////////////////// ModelDocument::Assignment /////////////////////////
ModelDocument ModelDocument::operator =(ModelDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// ModelDocument::add_mesh ///////////////////////////
void ModelDocument::add_mesh(int position, QString const &path)
{
  QJsonObject mesh;
  mesh["path"] = relpath(m_document, path);
  mesh["name"] = QFileInfo(path).completeBaseName();

  auto meshes = m_definition["meshes"].toArray();

  meshes.insert(position, mesh);

  m_definition["meshes"] = meshes;

  update();
}


///////////////////////// ModelDocument::move_mesh //////////////////////////
void ModelDocument::move_mesh(int index, int position)
{
  auto meshes = m_definition["meshes"].toArray();

  meshes.insert((index < position) ? position-1 : position, meshes.takeAt(index));

  m_definition["meshes"] = meshes;

  update();
}


///////////////////////// ModelDocument::erase_mesh /////////////////////////
void ModelDocument::erase_mesh(int index)
{
  auto meshes = m_definition["meshes"].toArray();

  meshes.removeAt(index);

  m_definition["meshes"] = meshes;

  update();
}


///////////////////////// ModelDocument::set_mesh_name //////////////////////
void ModelDocument::set_mesh_name(int index, QString const &name)
{
  auto meshes = m_definition["meshes"].toArray();

  auto mesh = meshes[index].toObject();

  mesh["name"] = name;

  meshes[index] = mesh;

  m_definition["meshes"] = meshes;

  update();
}


///////////////////////// ModelDocument::set_mesh_transform //////////////////
void ModelDocument::set_mesh_transform(int index, Transform const &transform)
{
  auto meshes = m_definition["meshes"].toArray();

  auto mesh = meshes[index].toObject();

  mesh["transform.real.w"] = transform.real.w;
  mesh["transform.real.x"] = transform.real.x;
  mesh["transform.real.y"] = transform.real.y;
  mesh["transform.real.z"] = transform.real.z;
  mesh["transform.dual.w"] = transform.dual.w;
  mesh["transform.dual.x"] = transform.dual.x;
  mesh["transform.dual.y"] = transform.dual.y;
  mesh["transform.dual.z"] = transform.dual.z;

  meshes[index] = mesh;

  m_definition["meshes"] = meshes;

  update();
}


///////////////////////// ModelDocument::set_mesh_material //////////////////
void ModelDocument::set_mesh_material(int index, int slot, QString const &path)
{
  auto meshes = m_definition["meshes"].toArray();

  auto mesh = meshes[index].toObject();

  auto materials = mesh["materials"].toArray();

  auto j = find_if(materials.begin(), materials.end(), [&](auto entry) { return (entry.toObject()["slot"].toInt() == slot); });

  if (j == materials.end())
  {
    j = materials.insert(materials.end(), QJsonObject({ { "slot", slot } }));
  }

  auto material = j->toObject();

  material["path"] = relpath(m_document, path);

  *j = material;

  mesh["materials"] = materials;

  meshes[index] = mesh;

  m_definition["meshes"] = meshes;

  update();
}


///////////////////////// ModelDocument::set_mesh_material //////////////////
void ModelDocument::set_mesh_material_tint(int index, int slot, Color3 const &tint)
{
  auto meshes = m_definition["meshes"].toArray();

  auto mesh = meshes[index].toObject();

  auto materials = mesh["materials"].toArray();

  auto j = find_if(materials.begin(), materials.end(), [&](auto entry) { return (entry.toObject()["slot"].toInt() == slot); });

  if (j == materials.end())
  {
    j = materials.insert(materials.end(), QJsonObject({ { "slot", slot } }));
  }

  auto material = j->toObject();

  material["tint.r"] = floor(tint.r*255.0f)/255.0f;
  material["tint.g"] = floor(tint.g*255.0f)/255.0f;
  material["tint.b"] = floor(tint.b*255.0f)/255.0f;

  *j = material;

  mesh["materials"] = materials;

  meshes[index] = mesh;

  m_definition["meshes"] = meshes;

  update();
}


///////////////////////// ModelDocument::reset_mesh_material ////////////////
void ModelDocument::reset_mesh_material(int index, int slot)
{
  auto meshes = m_definition["meshes"].toArray();

  auto mesh = meshes[index].toObject();

  auto materials = mesh["materials"].toArray();

  materials.erase(find_if(materials.begin(), materials.end(), [&](auto entry) { return (entry.toObject()["slot"].toInt() == slot); }));

  mesh["materials"] = materials;

  meshes[index] = mesh;

  m_definition["meshes"] = meshes;

  update();
}


///////////////////////// ModelDocument::instances //////////////////////////
vector<ModelDocument::Instance> ModelDocument::instances() const
{
  vector<Instance> instances;

  for(auto &mesh : m_meshes)
  {
    if (mesh.document && mesh.document->type() == "Mesh")
    {
      auto meshdocument = MeshDocument(mesh.document);

      for(auto &meshinstance : meshdocument.instances())
      {
        Instance instance;

        instance.index = indexof(m_meshes, mesh);
        instance.submesh = &mesh.submeshes[meshinstance.index-2];
        instance.material = &mesh.materials[meshinstance.material];
        instance.transform = mesh.transform * meshinstance.transform;

        instances.push_back(instance);
      }
    }

    if (mesh.document && mesh.document->type() == "Model")
    {
      auto modeldocument = ModelDocument(mesh.document);

      for(auto &meshinstance : modeldocument.instances())
      {
        Instance instance;

        size_t meshbase = 0;
        size_t materialbase = 0;
        for(int k = 0; k < meshinstance.index; ++k)
        {
          meshbase += modeldocument.mesh(k).submeshes.size();
          materialbase += modeldocument.mesh(k).materials.size();
        }

        instance.index = indexof(m_meshes, mesh);
        instance.submesh = &mesh.submeshes[meshbase + indexof(modeldocument.mesh(meshinstance.index).submeshes, meshinstance.submesh)];
        instance.material = &mesh.materials[materialbase + indexof(modeldocument.mesh(meshinstance.index).materials, meshinstance.material)];
        instance.transform = mesh.transform * meshinstance.transform;

        instances.push_back(instance);
      }
    }
  }

  return instances;
}


///////////////////////// ModelDocument::attach /////////////////////////////
void ModelDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &ModelDocument::refresh);

    refresh();
  }
}


///////////////////////// ModelDocument::touch //////////////////////////////
void ModelDocument::touch(Studio::Document *document, QString const &path)
{
  bool touched = false;

  for(auto &mesh : m_meshes)
  {
    if (document == mesh.document)
    {
      refresh();

      touched = true;
    }

    for(auto &submesh : mesh.submeshes)
    {
      if (document == submesh.document)
      {
        touched = true;
      }
    }

    for(auto &material : mesh.materials)
    {
      if (document == material.document)
      {
        touched = true;
      }
    }
  }

  if (touched)
  {
    emit dependant_changed();
  }
}


///////////////////////// ModelDocument::refresh ////////////////////////////
void ModelDocument::refresh()
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

  vector<Mesh> meshes;

  for(auto i : m_definition["meshes"].toArray())
  {
    auto mesh = i.toObject();
    auto meshpath = fullpath(m_document, mesh["path"].toString());

    Mesh md;
    md.name = mesh["name"].toString();
    md.document = documentmanager->open(meshpath);
    md.transform.real.w = mesh["transform.real.w"].toDouble(1.0);
    md.transform.real.x = mesh["transform.real.x"].toDouble(0.0);
    md.transform.real.y = mesh["transform.real.y"].toDouble(0.0);
    md.transform.real.z = mesh["transform.real.z"].toDouble(0.0);
    md.transform.dual.w = mesh["transform.dual.w"].toDouble(0.0);
    md.transform.dual.x = mesh["transform.dual.x"].toDouble(0.0);
    md.transform.dual.y = mesh["transform.dual.y"].toDouble(0.0);
    md.transform.dual.z = mesh["transform.dual.z"].toDouble(0.0);

    if (md.document && md.document->type() == "Mesh")
    {
      auto meshdocument = MeshDocument(md.document);

      md.materials.resize(meshdocument.materialcount());

      for(size_t i = 0; i < md.materials.size(); ++i)
      {
        md.materials[i].name = QString("Slot %1").arg(i);
        md.materials[i].tint = Color3(1.0f, 1.0f, 1.0f);
      }

      md.submeshes.resize(meshdocument.meshcount());

      for(size_t i = 0; i < md.submeshes.size(); ++i)
      {
        md.submeshes[i].document = documentmanager->dup(md.document);
        md.submeshes[i].index = i + 2;
      }
    }

    if (md.document && md.document->type() == "Model")
    {
      auto modeldocument = ModelDocument(md.document);

      for(int k = 0; k < modeldocument.meshes(); ++k)
      {
        for(auto &material : modeldocument.mesh(k).materials)
        {
          md.materials.push_back({ material.name, material.tint, material.document ? documentmanager->dup(material.document) : nullptr });
        }

        for(auto &submesh : modeldocument.mesh(k).submeshes)
        {
          md.submeshes.push_back({ documentmanager->dup(submesh.document), submesh.index });
        }
      }
    }

    for(auto j : mesh["materials"].toArray())
    {
      auto material = j.toObject();

      size_t slot = material["slot"].toInt();

      if (slot < md.materials.size())
      {
        auto materialpath = fullpath(m_document, material["path"].toString());

        if (materialpath != "")
        {
          md.materials[slot].name = QFileInfo(materialpath).completeBaseName();
          md.materials[slot].document = documentmanager->open(materialpath);
        }

        md.materials[slot].tint.r = material["tint.r"].toDouble(1.0);
        md.materials[slot].tint.g = material["tint.g"].toDouble(1.0);
        md.materials[slot].tint.b = material["tint.b"].toDouble(1.0);
      }
    }

    meshes.push_back(std::move(md));
  }

  swap(meshes, m_meshes);

  emit document_changed();
}


///////////////////////// ModelDocument::update /////////////////////////////
void ModelDocument::update()
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
