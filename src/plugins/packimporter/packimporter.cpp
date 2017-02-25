//
// Pack Importer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "packimporter.h"
#include "contentapi.h"
#include "assetfile.h"
#include "datum/math.h"
#include <leap/lz4.h>
#include <fstream>
#include <QDir>
#include <QFileInfo>
#include <QtPlugin>

#include <QDebug>

using namespace std;
using namespace lml;
using namespace leap;
using namespace leap::crypto;

namespace
{
  using ::write_text_asset;
  using ::write_imag_asset;
  using ::write_mesh_asset;
  using ::write_modl_asset;

  void read_asset_payload(string const &path, uint64_t offset, void *data, uint32_t size)
  {
    ifstream fin(path, ios::binary);

    PackChunk chunk;

    fin.seekg(offset);
    fin.read((char*)&chunk, sizeof(PackChunk));

    switch (chunk.type)
    {
      case "DATA"_packchunktype:
        {
          fin.read((char*)data, size);

          break;
        }

      case "CDAT"_packchunktype:
        {
          size_t remaining = chunk.length;
          uint8_t *datapos = (uint8_t*)data;

          while (remaining != 0)
          {
            PackBlock block;

            auto bytes = min(sizeof(block), remaining);

            fin.read((char*)&block, bytes);

            datapos += lz4_decompress(block.data, datapos, block.size, (uint8_t*)data + size - datapos);

            remaining -= bytes;
          }

          break;
        }
    }
  }

  void write_text_asset(QString const &src, QString const &dst, QJsonObject metadata, PackTextHeader *text)
  {
    vector<uint8_t> payload(pack_payload_size(*text));

    read_asset_payload(src.toStdString(), text->dataoffset, payload.data(), payload.size());

    metadata["icon"] = encode_icon(QIcon(":/packimporter/icon.png"));
    metadata["build"] = buildtime();

    ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

    write_asset_header(fout, metadata);

    write_text_asset(fout, 1, payload.size(), payload.data());

    write_asset_footer(fout);

    fout.close();
  }

  void write_imag_asset(QString const &src, QString const &dst, QJsonObject metadata, PackImageHeader *imag)
  {
    vector<uint8_t> payload(pack_payload_size(*imag));

    read_asset_payload(src.toStdString(), imag->dataoffset, payload.data(), payload.size());

    metadata["type"] = "Image";
    metadata["icon"] = encode_icon(QIcon(":/packimporter/icon.png"));
    metadata["build"] = buildtime();

    ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

    write_asset_header(fout, metadata);

    write_imag_asset(fout, 1, imag->width, imag->height, imag->layers, imag->levels, imag->format, payload.data());

    write_asset_footer(fout);

    fout.close();
  }

  void write_mesh_asset(QString const &src, QString const &dst, QJsonObject metadata, PackMeshHeader *mesh)
  {
    vector<uint8_t> payload(pack_payload_size(*mesh));

    read_asset_payload(src.toStdString(), mesh->dataoffset, payload.data(), payload.size());

    metadata["type"] = "Mesh";
    metadata["icon"] = encode_icon(QIcon(":/packimporter/icon.png"));
    metadata["build"] = buildtime();

    ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

    write_asset_header(fout, metadata);

    {
      PackModelPayload::Texture texture;
      texture.type = PackModelPayload::Texture::nullmap;

      PackModelPayload::Material material;
      material.color[0] = 0.75f;
      material.color[1] = 0.75f;
      material.color[2] = 0.75f;
      material.color[3] = 1.0f;
      material.metalness = 0.0f;
      material.roughness = 1.0f;
      material.reflectivity = 0.5f;
      material.emissive = 0.0f;
      material.albedomap = 0;
      material.specularmap = 0;
      material.normalmap = 0;

      PackModelPayload::Mesh mesh;
      mesh.mesh = 1;

      Transform transform = Transform::identity();

      PackModelPayload::Instance instance;
      instance.mesh = 0;
      instance.material = 0;
      memcpy(&instance.transform, &transform, sizeof(Transform));
      instance.childcount = 0;

      write_modl_asset(fout, 1, { texture }, { material }, { mesh }, { instance });
    }

    {
      Bound3 bound = Bound3(Vec3(mesh->mincorner[0], mesh->mincorner[1], mesh->mincorner[2]), Vec3(mesh->maxcorner[0], mesh->maxcorner[1], mesh->maxcorner[2]));

      write_mesh_asset(fout, 2, mesh->vertexcount, mesh->indexcount, bound, payload.data());
    }

    write_asset_footer(fout);

    fout.close();
  }

  void write_matl_asset(QString const &src, QString const &dst, QJsonObject metadata, PackMaterialHeader *matl)
  {

  }
}

//|---------------------- PackImporter --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// PackImporter::Constructor /////////////////////////
PackImporter::PackImporter()
{
}


///////////////////////// PackImporter::Destructor //////////////////////////
PackImporter::~PackImporter()
{
  shutdown();
}


///////////////////////// PackImporter::initialise //////////////////////////
bool PackImporter::initialise(QStringList const &arguments, QString *errormsg)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_importer("Pack", this);

  return true;
}


///////////////////////// PackImporter::shutdown ////////////////////////////
void PackImporter::shutdown()
{
}


///////////////////////// PackImporter::try_import //////////////////////////
bool PackImporter::try_import(QString const &src, QString const &dst, QJsonObject metadata)
{
  if (QFileInfo(src).suffix().toLower() != "pack")
    return false;

  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();

  QProgressDialog progress("Import Pack", "Abort", 0, 100, mainwindow->handle());

  QString path = QFileInfo(dst).dir().filePath(QFileInfo(dst).completeBaseName() + QString(".%1.") + QFileInfo(dst).suffix());

  ifstream fin(src.toStdString(), ios::binary);

  fin.seekg(sizeof(PackHeader), ios::beg);

  int id = 0;
  PackChunk chunk;

  while (fin.read((char*)&chunk, sizeof(chunk)))
  {
    vector<char> buffer(chunk.length);

    fin.read(buffer.data(), chunk.length);

    if (chunk.type == "ASET"_packchunktype)
    {
      id = reinterpret_cast<PackAssetHeader*>(buffer.data())->id;
    }

    switch (chunk.type)
    {
      case "TEXT"_packchunktype:
        write_text_asset(src, path.arg(id), metadata, reinterpret_cast<PackTextHeader*>(buffer.data()));
        break;

      case "IMAG"_packchunktype:
        write_imag_asset(src, path.arg(id), metadata, reinterpret_cast<PackImageHeader*>(buffer.data()));
        break;

      case "MESH"_packchunktype:
        write_mesh_asset(src, path.arg(id), metadata, reinterpret_cast<PackMeshHeader*>(buffer.data()));
        break;

      case "MATL"_packchunktype:
        write_matl_asset(src, path.arg(id), metadata, reinterpret_cast<PackMaterialHeader*>(buffer.data()));
        break;
    }

    fin.seekg(sizeof(uint32_t), ios::cur);
  }

  fin.close();

  return true;
}
