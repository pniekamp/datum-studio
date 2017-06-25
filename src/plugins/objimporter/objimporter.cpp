//
// Obj Importer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "objimporter.h"
#include "contentapi.h"
#include "assetfile.h"
#include "datum/math.h"
#include <leap.h>
#include <QFileInfo>
#include <QtPlugin>
#include <unordered_map>

#include <QDebug>

using namespace std;
using namespace lml;
using namespace leap;

namespace
{
  vector<string> seperate(string const &str, const char *delimiters = " \t\r\n")
  {
    vector<string> result;

    size_t i = 0;
    size_t j = 0;

    while (j != string::npos)
    {
      j = str.find_first_of(delimiters, i);

      result.push_back(str.substr(i, j-i));

      i = j + 1;
    }

    return result;
  }

  void calculatetangents(vector<PackVertex> &vertices, vector<uint32_t> &indices)
  {
    vector<Vec3> tan1(vertices.size(), Vec3(0));
    vector<Vec3> tan2(vertices.size(), Vec3(0));

    for(size_t i = 0; i < indices.size(); i += 3)
    {
      auto &v1 = vertices[indices[i+0]];
      auto &v2 = vertices[indices[i+1]];
      auto &v3 = vertices[indices[i+2]];

      auto x1 = v2.position[0] - v1.position[0];
      auto x2 = v3.position[0] - v1.position[0];
      auto y1 = v2.position[1] - v1.position[1];
      auto y2 = v3.position[1] - v1.position[1];
      auto z1 = v2.position[2] - v1.position[2];
      auto z2 = v3.position[2] - v1.position[2];

      auto s1 = v2.texcoord[0] - v1.texcoord[0];
      auto s2 = v3.texcoord[0] - v1.texcoord[0];
      auto t1 = v1.texcoord[1] - v2.texcoord[1];
      auto t2 = v1.texcoord[1] - v3.texcoord[1];

      auto r = s1 * t2 - s2 * t1;

      if (r != 0)
      {
        auto sdir = Vec3(t2 * x1 - t1 * x2, t2 * y1 - t1 * y2, t2 * z1 - t1 * z2) / r;
        auto tdir = Vec3(s1 * x2 - s2 * x1, s1 * y2 - s2 * y1, s1 * z2 - s2 * z1) / r;

        auto uvarea = area(Vec2(v1.texcoord[0], v1.texcoord[1]), Vec2(v2.texcoord[0], v2.texcoord[1]), Vec2(v3.texcoord[0], v3.texcoord[1]));

        tan1[indices[i+0]] += sdir * uvarea;
        tan1[indices[i+1]] += sdir * uvarea;
        tan1[indices[i+2]] += sdir * uvarea;

        tan2[indices[i+0]] += tdir * uvarea;
        tan2[indices[i+1]] += tdir * uvarea;
        tan2[indices[i+2]] += tdir * uvarea;
      }
    }

    for(size_t i = 0; i < vertices.size(); ++i)
    {
      auto normal = Vec3(vertices[i].normal[0], vertices[i].normal[1], vertices[i].normal[2]);

      auto tangent = tan1[i];
      auto bitangent = tan2[i];

      orthonormalise(normal, tangent, bitangent);

      vertices[i].tangent[0] = tangent.x;
      vertices[i].tangent[1] = tangent.y;
      vertices[i].tangent[2] = tangent.z;
      vertices[i].tangent[3] = (dot(bitangent, tan2[i]) < 0.0f) ? -1.0f : 1.0f;
    }

  }

  uint32_t write_mesh_asset(ostream &fout, uint32_t id, string const &path, float scale = 1.0f)
  {
    vector<Vec3> points;
    vector<Vec3> normals;
    vector<Vec2> texcoords;

    vector<PackVertex> vertices;
    vector<uint32_t> indices;
    unordered_map<PackVertex, uint32_t> vertexmap;

    ifstream fin(path);

    if (!fin)
      throw runtime_error("unable to read obj file - " + path);

    string buffer;

    while (getline(fin, buffer))
    {
      buffer = trim(buffer);

      // skip comments
      if (buffer.empty() || buffer[0] == '#' || buffer[0] == '/')
        continue;

      auto fields = split(buffer);

      if (fields[0] == "v")
      {
        points.push_back({ ato<float>(fields[1]), ato<float>(fields[2]), ato<float>(fields[3]) });
      }

      if (fields[0] == "vn")
      {
        normals.push_back({ ato<float>(fields[1]), ato<float>(fields[2]), ato<float>(fields[3]) });
      }

      if (fields[0] == "vt")
      {
        texcoords.push_back({ ato<float>(fields[1]), ato<float>(fields[2]) });
      }

      if (fields[0] == "f")
      {
        vector<string> face[] = { seperate(fields[1], "/"), seperate(fields[2], "/"), seperate(fields[3], "/") };

        for(auto &v : face)
        {
          PackVertex vertex = {};

          if (v.size() > 0 && ato<int>(v[0]) != 0)
            memcpy(vertex.position, &points[ato<int>(v[0]) - 1], sizeof(vertex.position));

          if (v.size() > 1 && ato<int>(v[1]) != 0)
            memcpy(vertex.texcoord, &texcoords[ato<int>(v[1]) - 1], sizeof(vertex.texcoord));

          if (v.size() > 2 && ato<int>(v[2]) != 0)
            memcpy(vertex.normal, &normals[ato<int>(v[2]) - 1], sizeof(vertex.normal));

          if (vertexmap.find(vertex) == vertexmap.end())
          {
            vertices.push_back(vertex);

            vertexmap[vertex] = vertices.size() - 1;
          }

          indices.push_back(vertexmap[vertex]);
        }
      }
    }

    for(auto &vertex : vertices)
    {
      vertex.position[0] *= scale;
      vertex.position[1] *= scale;
      vertex.position[2] *= scale;
    }

    calculatetangents(vertices, indices);

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
    material.surfacemap = 0;
    material.normalmap = 0;

    PackModelPayload::Mesh mesh;
    mesh.mesh = 1;

    Transform transform = Transform::identity();

    PackModelPayload::Instance instance;
    instance.mesh = 0;
    instance.material = 0;
    memcpy(&instance.transform, &transform, sizeof(Transform));
    instance.childcount = 0;

    write_modl_asset(fout, id, { texture }, { material }, { mesh }, { instance });

    write_mesh_asset(fout, id + 1, vertices, indices);

    return id + 2;
  }
}


//|---------------------- ObjImporter ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ObjImporter::Constructor //////////////////////////
ObjImporter::ObjImporter()
{
}


///////////////////////// ObjImporter::Destructor ///////////////////////////
ObjImporter::~ObjImporter()
{
  shutdown();
}


///////////////////////// ObjImporter::initialise ///////////////////////////
bool ObjImporter::initialise(QStringList const &arguments, QString *errormsg)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_importer("Mesh", this);

  return true;
}


///////////////////////// ObjImporter::shutdown /////////////////////////////
void ObjImporter::shutdown()
{
}


///////////////////////// ObjImporter::try_import ///////////////////////////
bool ObjImporter::try_import(QString const &src, QString const &dst, QJsonObject metadata)
{
  if (QFileInfo(src).suffix().toLower() != "obj")
    return false;

  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();

  QProgressDialog progress("Import OBJ", "Abort", 0, 100, mainwindow->handle());

  metadata["src"] = src;
  metadata["type"] = "Mesh";
  metadata["icon"] = encode_icon(QIcon(":/objimporter/icon.png"));
  metadata["build"] = buildtime();

  float scale = metadata["importscale"].toDouble(1.0);

  ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_mesh_asset(fout, 1, src.toStdString(), scale);

  write_asset_footer(fout);

  fout.close();

  return true;
}
