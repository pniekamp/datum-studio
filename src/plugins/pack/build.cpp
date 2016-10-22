//
// Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "build.h"
#include "buildapi.h"
#include "assetfile.h"
#include <leap.h>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <QFileInfo>
#include <QJsonDocument>

#include <QtDebug>

using namespace std;
using namespace lml;
using namespace leap;

namespace
{
  struct BuildState : public QObject
  {
    struct Asset
    {
      QString type;
      unique_document document;

      QString buildpath;
      vector<size_t> dependants;
    };

    ofstream fout;

    vector<Asset> assets;

    void add(Studio::Document *document);
    void add_dependant(size_t asset, Studio::Document *document, QString type);
  };

  void BuildState::add(Studio::Document *document)
  {
    Asset asset;
    asset.type = document->metadata("type", QString("Text"));
    asset.document = Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document);

    assets.push_back(std::move(asset));
  }

  void BuildState::add_dependant(size_t asset, Studio::Document *document, QString type)
  {
    auto j = find_if(assets.begin() + asset, assets.end(), [&](Asset const &asset) { return (asset.document == document && asset.type == type); });

    if (j == assets.end())
    {
      Asset dependant;
      dependant.type = type;
      dependant.document = Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document);

      j = assets.insert(assets.end(), std::move(dependant));
    }

    assets[asset].dependants.push_back(j - assets.begin());
  }


  ///////////////////////// prepare /////////////////////////////////////////
  void prepare(BuildState &pack)
  {
    auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

    for(size_t i = 0; i < pack.assets.size(); ++i)
    {
      if (pack.assets[i].type == "Font")
      {
        pack.add_dependant(i, pack.assets[i].document, "Font.Atlas");
      }

      if (pack.assets[i].type == "Material")
      {
        pack.assets[i].document->lock();

        PackTextHeader text;

        if (read_asset_header(pack.assets[i].document, 1, &text))
        {
          QByteArray payload(pack_payload_size(text), 0);

          read_asset_payload(pack.assets[i].document, text.dataoffset, payload.data(), payload.size());

          QJsonObject definition = QJsonDocument::fromBinaryData(payload).object();

          if (definition["albedomap"].toString() != "")
            pack.add_dependant(i, pack.assets[i].document, "Material.AlbedoMap");

          if (definition["metalnessmap"].toString() != "" || definition["roughnessmap"].toString() != "" || definition["reflectivitymap"].toString() != "")
            pack.add_dependant(i, pack.assets[i].document, "Material.SpecularMap");

          if (definition["normalmap"].toString() != "")
            pack.add_dependant(i, pack.assets[i].document, "Material.NormalMap");
        }

        pack.assets[i].document->unlock();
      }

      buildmanager->request_build(pack.assets[i].document, &pack, [i,&pack](Studio::Document *document, QString const &path) { pack.assets[i].buildpath = path; });
    }
  }


  ///////////////////////// process /////////////////////////////////////////
  bool process(BuildState &pack, BuildState::Asset &asset)
  {
    if (asset.type == "Text")
    {
      return true;
    }

    if (asset.type == "Image")
    {
      return true;
    }

    if (asset.type == "Font" || asset.type == "Font.Atlas")
    {
      return (asset.buildpath != "");
    }

    if (asset.type == "Mesh")
    {
      return (asset.buildpath != "");
    }

    if (asset.type == "Material" || asset.type == "Material.AlbedoMap" || asset.type == "Material.SpecularMap" || asset.type == "Material.NormalMap")
    {
      return (asset.buildpath != "");
    }

    if (asset.type == "SkyBox")
    {
      return (asset.buildpath != "");
    }

    assert(false);

    return true;
  }


  ///////////////////////// text ////////////////////////////////////////////
  void write_text(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    asset.document->lock();

    PackTextHeader text;

    if (read_asset_header(asset.document, 1, &text))
    {
      vector<char> payload(pack_payload_size(text));

      read_asset_payload(asset.document, text.dataoffset, payload.data(), payload.size());

      write_text_asset(pack.fout, id, payload.size(), payload.data());
    }

    asset.document->unlock();
  }


  ///////////////////////// image ///////////////////////////////////////////
  void write_image(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    asset.document->lock();

    PackImageHeader imag;

    if (read_asset_header(asset.document, 1, &imag))
    {
      vector<char> payload(pack_payload_size(imag));

      read_asset_payload(asset.document, imag.dataoffset, payload.data(), payload.size());

      write_imag_asset(pack.fout, id, imag.width, imag.height, imag.layers, imag.levels, imag.format, payload.data());
    }

    asset.document->unlock();
  }


  ///////////////////////// font ////////////////////////////////////////////
  void write_font(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    ifstream fin(asset.buildpath.toUtf8(), ios::binary);

    PackFontHeader font;

    if (read_asset_header(fin, 1, &font))
    {
      vector<char> payload(pack_payload_size(font));

      read_asset_payload(fin, font.dataoffset, payload.data(), payload.size());

      reinterpret_cast<PackFontPayload*>(payload.data())->glyphatlas = asset.dependants[0] - (&asset - &pack.assets.front());

      write_font_asset(pack.fout, id, font.ascent, font.descent, font.leading, font.glyphcount, payload.data());
    }
  }

  void write_font_atlas(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    ifstream fin(asset.buildpath.toUtf8(), ios::binary);

    PackImageHeader imag;

    if (read_asset_header(fin, 2, &imag))
    {
      vector<char> payload(pack_payload_size(imag));

      read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

      write_imag_asset(pack.fout, id, imag.width, imag.height, imag.layers, imag.levels, imag.format, payload.data());
    }
  }


  ///////////////////////// mesh //////////////////////////////////////////
  void write_mesh(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    ifstream fin(asset.buildpath.toUtf8(), ios::binary);

    PackMeshHeader mesh;

    if (read_asset_header(fin, 1, &mesh))
    {
      vector<char> payload(pack_payload_size(mesh));

      read_asset_payload(fin, mesh.dataoffset, payload.data(), payload.size());

      write_mesh_asset(pack.fout, id, mesh.vertexcount, mesh.indexcount, Bound3(Vec3(mesh.mincorner[0], mesh.mincorner[1], mesh.mincorner[2]), Vec3(mesh.maxcorner[0], mesh.maxcorner[1], mesh.maxcorner[2])), payload.data());
    }
  }


  ///////////////////////// material ////////////////////////////////////////
  void write_material(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    asset.document->lock();

    PackTextHeader text;

    if (read_asset_header(asset.document, 1, &text))
    {
      QByteArray payload(pack_payload_size(text), 0);

      read_asset_payload(asset.document, text.dataoffset, payload.data(), payload.size());

      QJsonObject definition = QJsonDocument::fromBinaryData(payload).object();

      auto color = Color3(definition["color.r"].toDouble(), definition["color.g"].toDouble(), definition["color.b"].toDouble());
      auto metalness = definition["metalness"].toDouble(0);
      auto roughness = definition["roughness"].toDouble(1);
      auto reflectivity = definition["reflectivity"].toDouble(0.5);
      auto emissive = definition["emissive"].toDouble(0);

      int i = 0;

      auto albedomap = 0;
      if (definition["albedomap"].toString() != "")
        albedomap = asset.dependants[i++] - (&asset - &pack.assets.front());

      auto specularmap = 0;
      if (definition["metalnessmap"].toString() != "" || definition["roughnessmap"].toString() != "" || definition["reflectivitymap"].toString() != "")
        specularmap = asset.dependants[i++] - (&asset - &pack.assets.front());

      auto normalmap = 0;
      if (definition["normalmap"].toString() != "")
        normalmap = asset.dependants[i++] - (&asset - &pack.assets.front());

      write_matl_asset(pack.fout, id, color, metalness, roughness, reflectivity, emissive, albedomap, specularmap, normalmap);
    }

    asset.document->unlock();
  }

  void write_material_albedomap(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    ifstream fin(asset.buildpath.toUtf8(), ios::binary);

    PackImageHeader imag;

    if (read_asset_header(fin, 1, &imag))
    {
      vector<char> payload(pack_payload_size(imag));

      read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

      image_compress_bc3(imag.width, imag.height, imag.layers, imag.levels, payload.data());

      write_imag_asset(pack.fout, id, imag.width, imag.height, imag.layers, imag.levels, PackImageHeader::rgba_bc3, payload.data());
    }
  }

  void write_material_specularmap(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    ifstream fin(asset.buildpath.toUtf8(), ios::binary);

    PackImageHeader imag;

    if (read_asset_header(fin, 2, &imag))
    {
      vector<char> payload(pack_payload_size(imag));

      read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

      image_compress_bc3(imag.width, imag.height, imag.layers, imag.levels, payload.data());

      write_imag_asset(pack.fout, id, imag.width, imag.height, imag.layers, imag.levels, PackImageHeader::rgba_bc3, payload.data());
    }
  }

  void write_material_normalmap(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    ifstream fin(asset.buildpath.toUtf8(), ios::binary);

    PackImageHeader imag;

    if (read_asset_header(fin, 3, &imag))
    {
      vector<char> payload(pack_payload_size(imag));

      read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

      write_imag_asset(pack.fout, id, imag.width, imag.height, imag.layers, imag.levels, imag.format, payload.data());
    }
  }


  ///////////////////////// skybox //////////////////////////////////////////
  void write_skybox(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    ifstream fin(asset.buildpath.toUtf8(), ios::binary);

    PackImageHeader imag;

    if (read_asset_header(fin, 1, &imag))
    {
      vector<char> payload(pack_payload_size(imag));

      read_asset_payload(fin, imag.dataoffset, payload.data(), payload.size());

      write_imag_asset(pack.fout, id, imag.width, imag.height, imag.layers, imag.levels, imag.format, payload.data());
    }
  }


  ///////////////////////// write ///////////////////////////////////////////
  void write(BuildState &pack, uint32_t id, BuildState::Asset &asset)
  {
    if (asset.type == "Text")
    {
      write_text(pack, id, asset);
    }

    else if (asset.type == "Image")
    {
      write_image(pack, id, asset);
    }

    else if (asset.type == "Font")
    {
      write_font(pack, id, asset);
    }

    else if (asset.type == "Font.Atlas")
    {
      write_font_atlas(pack, id, asset);
    }

    else if (asset.type == "Mesh")
    {
      write_mesh(pack, id, asset);
    }

    else if (asset.type == "Material")
    {
      write_material(pack, id, asset);
    }

    else if (asset.type == "Material.AlbedoMap")
    {
      write_material_albedomap(pack, id, asset);
    }

    else if (asset.type == "Material.SpecularMap")
    {
      write_material_specularmap(pack, id, asset);
    }

    else if (asset.type == "Material.NormalMap")
    {
      write_material_normalmap(pack, id, asset);
    }

    else if (asset.type == "SkyBox")
    {
      write_skybox(pack, id, asset);
    }

    else
    {
      assert(false);
    }    
  }
}


///////////////////////// build ///////////////////////////////////////////
void Pack::build(PackModel const *model, QString const &filename, Ui::Build *dlg)
{
  dlg->Close->setText("Cancel");
  dlg->Message->setText("Preparing...");

  bool cancel = false;
  auto closesignal = QObject::connect(dlg->Close, &QPushButton::clicked, [&] { cancel = true; });

  qApp->processEvents();

  BuildState pack;

  pack.fout.open(filename.toUtf8(), ios::binary | ios::trunc);

  if (!pack.fout)
    throw runtime_error("Unable to create output pack");

  write_header(pack.fout);

  write_catl_asset(pack.fout, 0);

  for(auto &asset : *model)
  {
    pack.add(asset->document());
  }

  prepare(pack);

  dlg->Message->setText("Building...");

  qApp->processEvents();

  uint32_t head = 0;

  while (head < pack.assets.size())
  {
    auto &asset = pack.assets[head];

    dlg->Message->setText(QString("Building: %1").arg(QFileInfo(Studio::Core::instance()->find_object<Studio::DocumentManager>()->path(asset.document)).completeBaseName()));
    dlg->TotalProgress->setValue(100 * head / pack.assets.size());

    if (process(pack, asset))
    {
      write(pack, head + 1, asset);

      head += 1;
    }

    qApp->processEvents();

    if (cancel)
    {
      dlg->Message->setText("Build Cancelled");
      break;
    }
  }

  write_chunk(pack.fout, "HEND", 0, nullptr);

  if (!cancel)
  {
    dlg->Message->setText("Build Complete...");
    dlg->TotalProgress->setValue(100);
    dlg->Export->setEnabled(true);
  }

  dlg->Close->setText("Close");

  QObject::disconnect(closesignal);
}
