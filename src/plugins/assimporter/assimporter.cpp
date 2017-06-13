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
#include <leap/lml/matrixconstants.h>
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
    Vec3 scale;
    vector<PackVertex> vertices;
    vector<uint32_t> indices;

    vector<PackMeshPayload::Rig> rig;
    vector<PackMeshPayload::Bone> bones;
  };

  struct Model
  {
    vector<Mesh> meshdata;
    vector<PackModelPayload::Texture> textures;
    vector<PackModelPayload::Material> materials;
    vector<PackModelPayload::Mesh> meshes;
    vector<PackModelPayload::Instance> instances;
  };

  struct Animation
  {
    float duration;
    vector<PackAnimationPayload::Joint> joints;
    vector<PackAnimationPayload::Transform> transforms;
  };

  Vector3f vec(aiVector3D const &v)
  {
    return { v.x, v.y, v.z };
  }

  Matrix4f mat(aiMatrix4x4 const &m)
  {
    return { { m.a1, m.a2, m.a3, m.a4 }, { m.b1, m.b2, m.b3, m.b4 }, { m.c1, m.c2, m.c3, m.c4 }, { m.d1, m.d2, m.d3, m.d4 } };
  }

  void build_model(Model &model, aiScene const *scene, aiNode const *node, Matrix4f const &transform)
  {
    auto m = transform * mat(node->mTransformation);

    auto xaxis = Vec3(m(0, 0), m(1, 0), m(2, 0));
    auto yaxis = Vec3(m(0, 1), m(1, 1), m(2, 1));
    auto zaxis = Vec3(m(0, 2), m(1, 2), m(2, 2));
    auto offset = Vec3(m(0, 3), m(1, 3), m(2, 3));

    auto scale = Vec3(norm(xaxis), norm(yaxis), norm(zaxis));
    auto rotation = Transform::rotation(Quaternion3f(normalise(xaxis), normalise(yaxis), normalise(zaxis)));
    auto translation = Transform::translation(offset);

    Transform world = translation * rotation;

    for(size_t i = 0; i < node->mNumMeshes; ++i)
    {
      model.meshdata[node->mMeshes[i]].scale = scale;

      model.instances.push_back({ node->mMeshes[i], scene->mMeshes[node->mMeshes[i]]->mMaterialIndex, world.real.w, world.real.x, world.real.y, world.real.z, world.dual.w, world.dual.x, world.dual.y, world.dual.z, 0 });
    }

    for(size_t i = 0; i < node->mNumChildren; ++i)
    {
      build_model(model, scene, node->mChildren[i], m);
    }
  }

  void build_model(Model &model, aiScene const *scene, float scale = 1.0f)
  {
    //
    // Textures
    //

    model.textures.push_back({ PackModelPayload::Texture::nullmap });

    //
    // Materials
    //

    for(size_t i = 0; i < scene->mNumMaterials; ++i)
    {
      aiColor4D diffuse = { 0.75f, 0.75f, 0.75f, 1.0f };
      aiGetMaterialColor(scene->mMaterials[i], AI_MATKEY_COLOR_DIFFUSE, &diffuse);

      model.materials.push_back({ diffuse.r, diffuse.g, diffuse.b, diffuse.a, 0.0f, 1.0f, 0.5f, 0, 0, 0 });
    }

    //
    // Meshes
    //

    for(size_t i = 0; i < scene->mNumMeshes; ++i)
    {
      if (!scene->mMeshes[i]->HasPositions())
        throw runtime_error("Mesh has no positions");

      if (!scene->mMeshes[i]->HasNormals())
        throw runtime_error("Mesh has no normals");

      if (!scene->mMeshes[i]->HasFaces())
        throw runtime_error("Mesh has no faces");

      Mesh mesh;

      mesh.scale = Vec3(1.0f);

      mesh.vertices.resize(scene->mMeshes[i]->mNumVertices);

      for(size_t k = 0; k < scene->mMeshes[i]->mNumVertices; ++k)
      {
        if (scene->mMeshes[i]->HasPositions())
        {
          mesh.vertices[k].position[0] = scene->mMeshes[i]->mVertices[k].x;
          mesh.vertices[k].position[1] = scene->mMeshes[i]->mVertices[k].y;
          mesh.vertices[k].position[2] = scene->mMeshes[i]->mVertices[k].z;
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
          mesh.vertices[k].tangent[0] = isnormal(scene->mMeshes[i]->mTangents[k].x) ? scene->mMeshes[i]->mTangents[k].x : 0;
          mesh.vertices[k].tangent[1] = isnormal(scene->mMeshes[i]->mTangents[k].y) ? scene->mMeshes[i]->mTangents[k].y : 0;
          mesh.vertices[k].tangent[2] = isnormal(scene->mMeshes[i]->mTangents[k].z) ? scene->mMeshes[i]->mTangents[k].z : 0;
          mesh.vertices[k].tangent[3] = (dot(cross(vec(scene->mMeshes[i]->mNormals[k]), vec(scene->mMeshes[i]->mTangents[k])), vec(scene->mMeshes[i]->mBitangents[k])) < 0.0f) ? -1.0f : 1.0f;
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

      model.meshdata.push_back(std::move(mesh));

      model.meshes.push_back({ (uint32_t)(1 + i) });
    }

    build_model(model, scene, scene->mRootNode, ScaleMatrix(scale, scale, scale, 1.0f));

    for(auto &mesh : model.meshdata)
    {
      for(size_t k = 0; k < mesh.vertices.size(); ++k)
      {
        mesh.vertices[k].position[0] *= mesh.scale.x;
        mesh.vertices[k].position[1] *= mesh.scale.y;
        mesh.vertices[k].position[2] *= mesh.scale.z;
      }
    }

    //
    // Rig
    //

    for(size_t i = 0; i < scene->mNumMeshes; ++i)
    {
      auto &mesh = model.meshdata[i];

      if (scene->mMeshes[i]->HasBones())
      {
        mesh.bones.resize(scene->mMeshes[i]->mNumBones);

        for(size_t k = 0; k < scene->mMeshes[i]->mNumBones; ++k)
        {
          auto m = mat(scene->mMeshes[i]->mBones[k]->mOffsetMatrix);

          auto xaxis = Vec3(m(0, 0), m(1, 0), m(2, 0));
          auto yaxis = Vec3(m(0, 1), m(1, 1), m(2, 1));
          auto zaxis = Vec3(m(0, 2), m(1, 2), m(2, 2));
          auto scale = Vec3(mesh.scale.x / norm(xaxis), mesh.scale.y / norm(yaxis), mesh.scale.z / norm(zaxis));
          auto offset = Vec3(m(0, 3), m(1, 3), m(2, 3));

          auto transform = Transform::translation(hada(scale, offset)) * Transform::rotation(Quaternion3f(normalise(xaxis), normalise(yaxis), normalise(zaxis)));

          memcpy(mesh.bones[k].name, scene->mMeshes[i]->mBones[k]->mName.C_Str(), min(sizeof(mesh.bones[k].name)-1, scene->mMeshes[i]->mBones[k]->mName.length));
          memcpy(mesh.bones[k].transform, &transform, sizeof(transform));
        }

        mesh.rig.resize(mesh.vertices.size());

        for(size_t j = 0; j < scene->mMeshes[i]->mNumBones; ++j)
        {
          for(size_t k = 0; k < scene->mMeshes[i]->mBones[j]->mNumWeights; ++k)
          {
            assert(scene->mMeshes[i]->mBones[j]->mWeights[k].mVertexId < mesh.rig.size());

            int index = 0;

            while (index < 4 && mesh.rig[scene->mMeshes[i]->mBones[j]->mWeights[k].mVertexId].weight[index] != 0.0f)
              ++index;

            mesh.rig[scene->mMeshes[i]->mBones[j]->mWeights[k].mVertexId].bone[index] = j;
            mesh.rig[scene->mMeshes[i]->mBones[j]->mWeights[k].mVertexId].weight[index] = scene->mMeshes[i]->mBones[j]->mWeights[k].mWeight;
          }
        }
      }
    }
  }

  void build_heirarchy(Animation &anim, aiScene const *scene, aiNode const *node, int parent)
  {
    PackAnimationPayload::Joint joint = {};

    memcpy(joint.name, node->mName.C_Str(), min(sizeof(joint.name)-1, node->mName.length));

    joint.parent = parent;

    joint.index = 0;
    joint.count = 2;

    anim.joints.push_back(joint);

    auto k = anim.joints.size() - 1;

    for(size_t i = 0; i < node->mNumChildren; ++i)
    {
      build_heirarchy(anim, scene, node->mChildren[i], k);
    }
  }

  void build_animation(Animation &anim, aiScene const *scene, aiAnimation *animation)
  {
    anim.duration = animation->mDuration / animation->mTicksPerSecond;

    anim.transforms.push_back({ 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
    anim.transforms.push_back({ 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });

    build_heirarchy(anim, scene, scene->mRootNode, 0);

    for(size_t i = 0; i < animation->mNumChannels; ++i)
    {
      auto joint = find_if(anim.joints.begin(), anim.joints.end(), [&](auto &k) { return strcmp(k.name, animation->mChannels[i]->mNodeName.C_Str()) == 0; });

      if (joint == anim.joints.end())
        throw runtime_error("Animation joint not found in heirarchy");

      if (animation->mChannels[i]->mNumScalingKeys < 2 || animation->mChannels[i]->mNumRotationKeys < 2 || animation->mChannels[i]->mNumPositionKeys < 2)
        throw runtime_error("Insufficient Animation keyframes");

      if (animation->mChannels[i]->mNumScalingKeys != animation->mChannels[i]->mNumRotationKeys || animation->mChannels[i]->mNumRotationKeys != animation->mChannels[i]->mNumPositionKeys)
        throw runtime_error("Animation keyframes not baked");

      joint->count = 0;
      joint->index = anim.transforms.size();

      for(size_t k = 0; k < animation->mChannels[i]->mNumRotationKeys; ++k)
      {
        PackAnimationPayload::Transform transform;

        transform.time = animation->mChannels[i]->mRotationKeys[k].mTime / animation->mTicksPerSecond;

        auto t0 = Vec3(animation->mChannels[i]->mScalingKeys[k].mValue.x, animation->mChannels[i]->mScalingKeys[k].mValue.y, animation->mChannels[i]->mScalingKeys[k].mValue.z);
        auto t1 = Transform::translation(t0.x*animation->mChannels[i]->mPositionKeys[k].mValue.x, t0.y*animation->mChannels[i]->mPositionKeys[k].mValue.y, t0.z*animation->mChannels[i]->mPositionKeys[k].mValue.z);
        auto t2 = Transform::rotation({ animation->mChannels[i]->mRotationKeys[k].mValue.w, animation->mChannels[i]->mRotationKeys[k].mValue.x, animation->mChannels[i]->mRotationKeys[k].mValue.y, animation->mChannels[i]->mRotationKeys[k].mValue.z });
        auto t3 = t1 * t2;

        memcpy(transform.transform, &t3, sizeof(transform.transform));

        anim.transforms.push_back(transform);

        joint->count += 1;
      }
    }
  }

  uint32_t write_mesh_asset(ostream &fout, uint32_t id, Model const &model)
  {
    write_modl_asset(fout, id, model.textures, model.materials, model.meshes, model.instances);

    for(size_t i = 0; i < model.meshes.size(); ++i)
    {
      write_mesh_asset(fout, id + 1 + i, model.meshdata[i].vertices, model.meshdata[i].indices, model.meshdata[i].rig, model.meshdata[i].bones);
    }

    return id + 1 + model.meshes.size();
  }

  uint32_t write_animation_asset(ostream &fout, uint32_t id, Animation const &anim)
  {
    write_anim_asset(fout, id, anim.duration, anim.joints, anim.transforms);

    return id + 1;
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
  flags |= aiProcess_LimitBoneWeights;
//  flags |= aiProcess_OptimizeMeshes;
//  flags |= aiProcess_OptimizeGraph;

  importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_COLORS | aiComponent_CAMERAS | aiComponent_LIGHTS);
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,  aiPrimitiveType_POINT | aiPrimitiveType_LINE);

  auto scene = importer.ReadFile(src.toStdString(), flags);

  if (!scene)
    throw runtime_error("Scene failed to load");

  metadata["src"] = src;
  metadata["build"] = buildtime();

  if (scene->HasMeshes())
  {
    Model model;

    build_model(model, scene, metadata["importscale"].toDouble(1.0));

    metadata["type"] = "Mesh";
    metadata["icon"] = encode_icon(model.meshdata[0].bones.empty() ? QIcon(":/assimporter/model.png") : QIcon(":/assimporter/actor.png"));

    ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

    write_asset_header(fout, metadata);

    write_mesh_asset(fout, 1, model);

    write_asset_footer(fout);

    fout.close();
  }

  if (scene->HasAnimations() && !scene->HasMeshes())
  {
    Animation anim;

    build_animation(anim, scene, scene->mAnimations[0]);

    metadata["type"] = "Animation";
    metadata["icon"] = encode_icon(QIcon(":/assimporter/animation.png"));

    ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

    write_asset_header(fout, metadata);

    write_animation_asset(fout, 1, anim);

    write_asset_footer(fout);

    fout.close();
  }

  return true;
}
