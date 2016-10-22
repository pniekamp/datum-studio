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


///////////////////////// build /////////////////////////////////////////////
void MeshDocument::build(Studio::Document *document, string const &path)
{
  vector<PackVertex> vertices;
  vector<uint32_t> indices;

  document->lock();

  PackModelHeader modl;

  if (read_asset_header(document, 1, &modl))
  {
    vector<char> payload(pack_payload_size(modl));

    read_asset_payload(document, modl.dataoffset, payload.data(), payload.size());

    auto meshtable = PackModelPayload::meshtable(payload.data(), modl.texturecount, modl.materialcount, modl.meshcount, modl.instancecount);
    auto instancetable = PackModelPayload::instancetable(payload.data(), modl.texturecount, modl.materialcount, modl.meshcount, modl.instancecount);

    for(size_t i = 0; i < modl.instancecount; ++i)
    {
      PackMeshHeader mhdr;

      if (read_asset_header(document, 1 + meshtable[instancetable[i].mesh].mesh, &mhdr))
      {
        uint64_t position = mhdr.dataoffset + sizeof(PackChunk);

        vector<PackVertex> vertextable(mhdr.vertexcount);

        position += document->read(position, vertextable.data(), vertextable.size() * sizeof(PackVertex));

        vector<uint32_t> indextable(mhdr.indexcount);

        position += document->read(position, indextable.data(), indextable.size() * sizeof(uint32_t));

        auto transform = Transform{ { instancetable[i].transform[0], instancetable[i].transform[1], instancetable[i].transform[2], instancetable[i].transform[3] }, { instancetable[i].transform[4], instancetable[i].transform[5], instancetable[i].transform[6], instancetable[i].transform[7] } };

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
  }

  document->unlock();

  ofstream fout(path, ios::binary | ios::trunc);

  write_header(fout);

  write_catl_asset(fout, 0);

  write_mesh_asset(fout, 1, vertices, indices);

  write_chunk(fout, "HEND", 0, nullptr);

  fout.close();
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
  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
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
  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
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
