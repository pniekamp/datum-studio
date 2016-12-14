//
// Pack
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "pack.h"
#include "buildapi.h"
#include "assetfile.h"
#include <QFileInfo>

#include <QtDebug>

using namespace std;

namespace
{
  //|---------------------- BuildState --------------------------------------
  //|------------------------------------------------------------------------

  struct BuildState : public QObject
  {
    struct Asset : public Studio::PackerState
    {
      BuildState *buildstate;

      enum { Pending, Building, Packing, Done, Failed } status;

      uint32_t add_dependant(Studio::Document *document, QString type) override
      {
        return buildstate->add_dependant(this, document, 0, type) - id + 1;
      }

      uint32_t add_dependant(Studio::Document *document, uint32_t index, QString type) override
      {
        return buildstate->add_dependant(this, document, index, type) - id + 1;
      }
    };

    uint32_t signature;
    uint32_t version;

    vector<unique_ptr<Asset>> assets;

    size_t add(Studio::Document *document, uint32_t index = 0);
    size_t add_dependant(Asset *asset, Studio::Document *document, uint32_t index, QString type);
  };

  size_t BuildState::add(Studio::Document *document, uint32_t index)
  {
    auto asset = make_unique<Asset>();

    asset->id = assets.size() + 1;
    asset->name = QFileInfo(Studio::Core::instance()->find_object<Studio::DocumentManager>()->path(document)).completeBaseName();
    asset->type = document->metadata("type", QString("Text"));
    asset->document = Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document);
    asset->index = index;
    asset->buildstate = this;
    asset->status = Asset::Pending;

    assets.push_back(std::move(asset));

    return assets.size() - 1;
  }

  size_t BuildState::add_dependant(Asset *asset, Studio::Document *document, uint32_t index, QString type)
  {
    auto j = find_if(assets.begin() + asset->id, assets.end(), [&](auto &asset) { return (asset->document == document && asset->index == index && asset->type == type); });

    if (j == assets.end())
    {
      auto asset = make_unique<Asset>();

      asset->id = assets.size() + 1;
      asset->name = QFileInfo(Studio::Core::instance()->find_object<Studio::DocumentManager>()->path(document)).completeBaseName();
      asset->type = type;
      asset->document = Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document);
      asset->index = index;
      asset->buildstate = this;
      asset->status = Asset::Pending;

      j = assets.insert(assets.end(), std::move(asset));
    }

    return j - assets.begin();
  }

}


//|---------------------- PackManager ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// PackManager::Constructor //////////////////////////
PackManager::PackManager()
{
}


///////////////////////// PackManager::register_packer //////////////////////
void PackManager::register_packer(QString const &type, QObject *packer)
{
  m_packers[type] = packer;
}


///////////////////////// PackManager::build ///////////////////////////////
void PackManager::build(PackModel const *model, QString const &filename, Ui::Build *dlg)
{
  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  dlg->Close->setText("Cancel");
  dlg->Message->setText("Preparing...");

  bool cancel = false;
  auto closesignal = QObject::connect(dlg->Close, &QPushButton::clicked, [&] { cancel = true; });

  qApp->processEvents();

  BuildState pack;

  pack.signature = model->signature().toUInt(nullptr, 0);
  pack.version = model->version().toUInt(nullptr, 0);

  for(auto &node : model->nodes())
  {
    if (auto asset = node_cast<PackModel::Asset>(node))
    {
      pack.add(asset->document());
    }
  }

  ofstream fout(filename.toUtf8(), ios::binary | ios::trunc);

  if (!fout)
    throw runtime_error("Unable to create output pack");

  write_header(fout);

  write_catl_asset(fout, 0, pack.signature, pack.version);

  dlg->Message->setText("Building...");

  uint32_t head = 0;

  while (head < pack.assets.size())
  {
    auto &asset = *pack.assets[head];

    dlg->Message->setText(QString("Building: %1").arg(asset.name));
    dlg->TotalProgress->setValue(100 * head / pack.assets.size());

    for(size_t i = head; i < pack.assets.size(); ++i)
    {
      auto &asset = *pack.assets[i];

      if (asset.status == BuildState::Asset::Pending)
      {
        asset.status = BuildState::Asset::Building;

        buildmanager->request_build(asset.document, &pack, [&](Studio::Document *document, QString const &path) { asset.buildpath = path.toStdString(); asset.status = BuildState::Asset::Packing; }, [&](Studio::Document *document) { asset.status = BuildState::Asset::Packing; });
      }
    }

    if (asset.status == BuildState::Asset::Packing)
    {
      bool result = false;

      if (QObject *packer = m_packers[asset.type])
      {
        try
        {
          QMetaObject::invokeMethod(packer, "pack", Qt::DirectConnection, Q_RETURN_ARG(bool, result), Q_ARG(Studio::PackerState&, asset), Q_ARG(std::ofstream&, fout));
        }
        catch(exception &e)
        {
          qCritical() << "Pack Error:" << asset.name << e.what();
        }
      }
      else
      {
        qCritical() << "Pack Error: No Packer for" << asset.type;
      }

      asset.status = (result) ? BuildState::Asset::Done : BuildState::Asset::Failed;
    }

    if (asset.status == BuildState::Asset::Failed)
    {
      dlg->Message->setText("Build Failed");
      break;
    }

    if (asset.status == BuildState::Asset::Done)
    {
      ++head;
    }

    if (cancel)
    {
      dlg->Message->setText("Build Cancelled");
      break;
    }

    qApp->processEvents();
  }

  write_chunk(fout, "HEND", 0, nullptr);

  if (head == pack.assets.size())
  {
    dlg->Message->setText("Build Complete...");
    dlg->TotalProgress->setValue(100);
    dlg->Export->setEnabled(true);
  }

  dlg->Close->setText("Close");

  QObject::disconnect(closesignal);
}
