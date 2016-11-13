//
// AssImp Importer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "assimporter.h"
#include "contentapi.h"
#include "assetfile.h"
#include <leap.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <QFileInfo>
#include <QtPlugin>

#include <QDebug>

using namespace std;
using namespace lml;
using namespace leap;

namespace
{
  struct Mesh
  {
    vector<PackVertex> vertices;
    vector<uint32_t> indices;
  };

  struct Model
  {
    vector<Mesh> meshdata;
    vector<PackModelPayload::Texture> textures;
    vector<PackModelPayload::Material> materials;
    vector<PackModelPayload::Mesh> meshes;
    vector<PackModelPayload::Instance> instances;
  };

  float dot(aiVector3D const &u, aiVector3D const &v)
  {
    return u * v;
  }

  aiVector3D cross(aiVector3D const &u, aiVector3D const &v)
  {
    return { u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x };
  }

  void build_model(Model &model, aiScene const *scene, aiNode const *node, Transform const &transform, float scale)
  {
    auto xaxis = Vector3(node->mTransformation.a1, node->mTransformation.b1, node->mTransformation.c1);
    auto yaxis = Vector3(node->mTransformation.a2, node->mTransformation.b2, node->mTransformation.c2);
    auto zaxis = Vector3(node->mTransformation.a3, node->mTransformation.b3, node->mTransformation.c3);

    auto rotation = Transform::rotation(Quaternion3f(xaxis, yaxis, zaxis));
    auto translation = Transform::translation(scale * node->mTransformation.a4, scale * node->mTransformation.b4, scale * node->mTransformation.c4);

    Transform local = transform * translation * rotation;

    for(size_t i = 0; i < node->mNumMeshes; ++i)
    {
      model.instances.push_back({ node->mMeshes[i], scene->mMeshes[node->mMeshes[i]]->mMaterialIndex, local.real.w, local.real.x, local.real.y, local.real.z, local.dual.w, local.dual.x, local.dual.y, local.dual.z, 0 });
    }

    for(size_t i = 0; i < node->mNumChildren; ++i)
    {
      build_model(model, scene, node->mChildren[i], local, scale);
    }
  }

  uint32_t write_mesh_asset(ostream &fout, uint32_t id, aiScene const *scene, float scale = 1.0f)
  {
    Model model;

    model.textures.push_back({ PackModelPayload::Texture::nullmap });

    for(size_t i = 0; i < scene->mNumMaterials; ++i)
    {
      aiColor4D diffuse = { 0.75f, 0.75f, 0.75f, 1.0f };
      aiGetMaterialColor(scene->mMaterials[i], AI_MATKEY_COLOR_DIFFUSE, &diffuse);

      model.materials.push_back({ diffuse.r, diffuse.g, diffuse.b, 0.0f, 1.0f, 0.5f, 0, 0, 0 });
    }

    for(size_t i = 0; i < scene->mNumMeshes; ++i)
    {
      if (!scene->mMeshes[i]->HasPositions())
        throw runtime_error("Mesh has no positions");

      if (!scene->mMeshes[i]->HasNormals())
        throw runtime_error("Mesh has no normals");

      if (!scene->mMeshes[i]->HasFaces())
        throw runtime_error("Mesh has no faces");

      Mesh mesh;

      mesh.vertices.resize(scene->mMeshes[i]->mNumVertices);

      for(size_t k = 0; k < scene->mMeshes[i]->mNumVertices; ++k)
      {
        if (scene->mMeshes[i]->HasPositions())
        {
          mesh.vertices[k].position[0] = scene->mMeshes[i]->mVertices[k].x * scale;
          mesh.vertices[k].position[1] = scene->mMeshes[i]->mVertices[k].y * scale;
          mesh.vertices[k].position[2] = scene->mMeshes[i]->mVertices[k].z * scale;
        }

        if (scene->mMeshes[i]->HasTextureCoords(0))
        {
          mesh.vertices[k].texcoord[0] = scene->mMeshes[i]->mTextureCoords[0][k].x;
          mesh.vertices[k].texcoord[1] = scene->mMeshes[i]->mTextureCoords[0][k].y;
        }

        if (scene->mMeshes[i]->HasNormals())
        {
          mesh.vertices[k].normal[0] = scene->mMeshes[i]->mNormals[k].x;
          mesh.vertices[k].normal[1] = scene->mMeshes[i]->mNormals[k].y;
          mesh.vertices[k].normal[2] = scene->mMeshes[i]->mNormals[k].z;
        }

        if (scene->mMeshes[i]->HasTangentsAndBitangents())
        {
          mesh.vertices[k].tangent[0] = scene->mMeshes[i]->mTangents[k].x;
          mesh.vertices[k].tangent[1] = scene->mMeshes[i]->mTangents[k].y;
          mesh.vertices[k].tangent[2] = scene->mMeshes[i]->mTangents[k].z;
          mesh.vertices[k].tangent[3] = (dot(cross(scene->mMeshes[i]->mNormals[k], scene->mMeshes[i]->mTangents[k]), scene->mMeshes[i]->mBitangents[k]) < 0.0f) ? -1.0f : 1.0f;
        }
      }

      mesh.indices.resize(scene->mMeshes[i]->mNumFaces * 3);

      for(size_t k = 0; k < scene->mMeshes[i]->mNumFaces; ++k)
      {
        assert(scene->mMeshes[i]->mFaces[k].mNumIndices == 3);

        mesh.indices[k*3 + 0] = scene->mMeshes[i]->mFaces[k].mIndices[0];
        mesh.indices[k*3 + 1] = scene->mMeshes[i]->mFaces[k].mIndices[1];
        mesh.indices[k*3 + 2] = scene->mMeshes[i]->mFaces[k].mIndices[2];
      }

      model.meshes.push_back({ (uint32_t)(1 + i) });
      model.meshdata.push_back(std::move(mesh));
    }

    build_model(model, scene, scene->mRootNode, Transform::identity(), scale);

    write_modl_asset(fout, id, model.textures, model.materials, model.meshes, model.instances);

    for(size_t i = 0; i < model.meshes.size(); ++i)
    {
      write_mesh_asset(fout, id + 1 + i, model.meshdata[i].vertices, model.meshdata[i].indices);
    }

    return id + 1 + model.meshes.size();
  }
}


//|---------------------- AssImporter ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// AssImporter::Constructor //////////////////////////
AssImporter::AssImporter()
{
}


///////////////////////// AssImporter::Destructor ///////////////////////////
AssImporter::~AssImporter()
{
  shutdown();
}


///////////////////////// AssImporter::initialise ///////////////////////////
bool AssImporter::initialise(QStringList const &arguments, QString *errormsg)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_importer("Mesh", this);

  return true;
}


///////////////////////// AssImporter::shutdown /////////////////////////////
void AssImporter::shutdown()
{
}


///////////////////////// AssImporter::try_import ///////////////////////////
bool AssImporter::try_import(QString const &src, QString const &dst, QJsonObject metadata)
{
  Assimp::Importer importer;

  if (!importer.IsExtensionSupported(QFileInfo(src).suffix().toLower().toUtf8()))
    return false;

  auto mainwindow = Studio::Core::instance()->find_object<Studio::MainWindow>();

  QProgressDialog progress("Import Mesh", "Abort", 0, 100, mainwindow->handle());

  Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDOUT);

  unsigned int flags = 0;

  flags |= aiProcess_CalcTangentSpace;
  flags |= aiProcess_Triangulate;
  flags |= aiProcess_JoinIdenticalVertices;
  flags |= aiProcess_TransformUVCoords;
  flags |= aiProcess_SortByPType;
  flags |= aiProcess_RemoveComponent;
  flags |= aiProcess_RemoveRedundantMaterials;
  flags |= aiProcess_FindInstances;
//  flags |= aiProcess_OptimizeMeshes;
//  flags |= aiProcess_OptimizeGraph;

  importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_COLORS | aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_BONEWEIGHTS | aiComponent_ANIMATIONS);
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,  aiPrimitiveType_POINT | aiPrimitiveType_LINE);

  auto scene = importer.ReadFile(src.toStdString(), flags);

  if (!scene)
    throw runtime_error("Scene failed to load");

  metadata["src"] = src;
  metadata["type"] = "Mesh";
  metadata["build"] = buildtime();
  metadata["icon"] = encode_icon(QIcon(":/assimporter/icon.png"));

  float scale = metadata["importscale"].toDouble(1.0);

  ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_mesh_asset(fout, 1, scene, scale);

  write_asset_footer(fout);

  fout.close();

  return true;
}
