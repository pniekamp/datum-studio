//
// Mesh Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "mesh.h"
#include "assetfile.h"
#include "atlaspacker.h"
#include <functional>

#include <QDebug>

using namespace std;
using namespace lml;

///////////////////////// hash //////////////////////////////////////////////
void MeshDocument::hash(Studio::Document *document, size_t *key)
{
  *key = std::hash<double>{}(document->metadata("build", 0.0));
}


///////////////////////// pack //////////////////////////////////////////////
void MeshDocument::pack(Studio::PackerState &asset, ofstream &fout)
{
  if (asset.index == 0)
  {
    vector<PackVertex> vertices;
    vector<uint32_t> indices;

    asset.document->lock();

    for(auto &instance : MeshDocument(asset.document).instances())
    {
      PackMeshHeader mesh;

      if (read_asset_header(asset.document, instance.index, &mesh))
      {
        uint64_t position = mesh.dataoffset + sizeof(PackChunk);

        vector<PackVertex> vertextable(mesh.vertexcount);

        position += asset.document->read(position, vertextable.data(), vertextable.size() * sizeof(PackVertex));

        vector<uint32_t> indextable(mesh.indexcount);

        position += asset.document->read(position, indextable.data(), indextable.size() * sizeof(uint32_t));

        auto transform = instance.transform;

        uint32_t base = vertices.size();

        for(auto &vertex : vertextable)
        {
          auto position = transform * Vec3(vertex.position[0], vertex.position[1], vertex.position[2]);
          auto normal = transform.rotation() * Vec3(vertex.normal[0], vertex.normal[1], vertex.normal[2]);
          auto tangent = transform.rotation() * Vec3(vertex.tangent[0], vertex.tangent[1], vertex.tangent[2]);

          vertices.push_back({ position.x, position.y, position.z, vertex.texcoord[0], vertex.texcoord[1], normal.x, normal.y, normal.z, tangent.x, tangent.y, tangent.z, vertex.tangent[3] });
        }

        for(auto &index: indextable)
        {
          indices.push_back(index + base);
        }
      }
    }

    asset.document->unlock();

    write_mesh_asset(fout, asset.id, vertices, indices);
  }

  if (asset.index > 0)
  {
    asset.document->lock();

    PackMeshHeader mesh;

    if (read_asset_header(asset.document, asset.index, &mesh))
    {
      vector<char> payload(pack_payload_size(mesh));

      read_asset_payload(asset.document, mesh.dataoffset, payload.data(), payload.size());

      write_mesh_asset(fout, asset.id, mesh.vertexcount, mesh.indexcount, Bound3(Vec3(mesh.mincorner[0], mesh.mincorner[1], mesh.mincorner[2]), Vec3(mesh.maxcorner[0], mesh.maxcorner[1], mesh.maxcorner[2])), payload.data());
    }

    asset.document->unlock();
  }
}


//|---------------------- MeshDocument --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MeshDocument::Constructor /////////////////////////
MeshDocument::MeshDocument()
{
}


///////////////////////// MeshDocument::Constructor /////////////////////////
MeshDocument::MeshDocument(QString const &path)
  : MeshDocument()
{
  if (path != "")
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
  }
}


///////////////////////// MeshDocument::Constructor /////////////////////////
MeshDocument::MeshDocument(Studio::Document *document)
  : MeshDocument()
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// MeshDocument::Constructor /////////////////////////
MeshDocument::MeshDocument(MeshDocument const &document)
  : MeshDocument(document.m_document)
{
}


///////////////////////// MeshDocument::Assignment //////////////////////////
MeshDocument MeshDocument::operator =(MeshDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// MeshDocument::meshcount ///////////////////////////
int MeshDocument::meshcount() const
{
  int meshes = 0;

  m_document->lock();

  PackModelHeader modl;

  if (read_asset_header(m_document, 1, &modl))
  {
    meshes = modl.meshcount;
  }

  m_document->unlock();

  return meshes;
}


///////////////////////// MeshDocument::materialcount ///////////////////////
int MeshDocument::materialcount() const
{
  int meshes = 0;

  m_document->lock();

  PackModelHeader modl;

  if (read_asset_header(m_document, 1, &modl))
  {
    meshes = modl.materialcount;
  }

  m_document->unlock();

  return meshes;
}


///////////////////////// MeshDocument::instances ///////////////////////////
vector<MeshDocument::Instance> MeshDocument::instances() const
{
  vector<Instance> instances;

  m_document->lock();

  PackModelHeader modl;

  if (read_asset_header(m_document, 1, &modl))
  {
    vector<char> payload(pack_payload_size(modl));

    read_asset_payload(m_document, modl.dataoffset, payload.data(), payload.size());

    auto meshtable = PackModelPayload::meshtable(payload.data(), modl.texturecount, modl.materialcount, modl.meshcount, modl.instancecount);
    auto instancetable = PackModelPayload::instancetable(payload.data(), modl.texturecount, modl.materialcount, modl.meshcount, modl.instancecount);

    for(size_t i = 0; i < modl.instancecount; ++i)
    {
      Instance instance;

      instance.index = 1 + meshtable[instancetable[i].mesh].mesh;
      instance.material = instancetable[i].material;
      instance.transform = Transform{ { instancetable[i].transform[0], instancetable[i].transform[1], instancetable[i].transform[2], instancetable[i].transform[3] }, { instancetable[i].transform[4], instancetable[i].transform[5], instancetable[i].transform[6], instancetable[i].transform[7] } };

      instances.push_back(instance);
    }
  }

  m_document->unlock();

  return instances;
}


///////////////////////// MeshDocument::attach //////////////////////////////
void MeshDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &MeshDocument::document_changed);
  }
}
