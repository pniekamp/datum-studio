//
// Pack Model
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "packmodel.h"
#include <leap.h>
#include <QDir>
#include <fstream>
#include <cassert>

#include <QtDebug>

using namespace std;
using namespace leap;

//|---------------------- Asset -------------------------------------
//|------------------------------------------------------------------

///////////////////////// Asset::Constructor ////////////////////////
PackModel::Asset::Asset(QString const &path)
{
  m_path = path;
  m_document = Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path);
}


//|---------------------- PackModel -----------------------------------------
//|--------------------------------------------------------------------------
//| Pack Model
//|

///////////////////////// PackModel::Constructor ////////////////////////////
PackModel::PackModel(QObject *parent)
  : QObject(parent)
{
  m_modified = false;

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_renamed, this, &PackModel::on_document_renamed);
}


///////////////////////// PackModel::clear //////////////////////////////////
void PackModel::clear()
{
  m_assets.clear();

  emit reset();

  m_modified = true;
}


///////////////////////// PackModel::add_asset //////////////////////////////
void PackModel::add_asset(size_t position, QString const &path)
{
  auto i = m_assets.insert(m_assets.begin() + position, make_unique<Asset>(path));

  emit added(i - m_assets.begin(), i->get());

  m_modified = true;
}


///////////////////////// PackModel::move_asset /////////////////////////////
void PackModel::move_asset(size_t index, size_t position)
{
  assert(index < m_assets.size());

  auto asset = std::move(m_assets[index]);

  m_assets.erase(m_assets.begin() + index);

  emit removed(index, asset.get());

  if (position > index)
    --position;

  auto i = m_assets.insert(m_assets.begin() + position, std::move(asset));

  emit added(i - m_assets.begin(), i->get());

  m_modified = true;
}


///////////////////////// PackModel::erase_asset ////////////////////////////
void PackModel::erase_asset(size_t index)
{
  assert(index < m_assets.size());

  auto asset = m_assets[index].get();

  m_assets.erase(m_assets.begin() + index);

  emit removed(index, asset);

  m_modified = true;
}


///////////////////////// PackModel::document_renamed ///////////////////////
void PackModel::on_document_renamed(Studio::Document *document, QString const &src, QString const &dst)
{
  for(auto &asset: m_assets)
  {
    if (asset->m_document == document)
    {
      asset->m_path = dst;

      emit changed(&asset - &m_assets.front(), asset.get());

      m_modified = true;
    }
  }
}


///////////////////////// PackModel::load ///////////////////////////////////
void PackModel::load(string const &projectfile)
{
  clear();

  ifstream fin(projectfile);

  string buffer;

  while (getline(fin, buffer))
  {
    buffer = trim(buffer);

    if (buffer == "[Pack]")
      break;
  }

  QDir base = QFileInfo(projectfile.c_str()).dir();

  while (getline(fin, buffer))
  {
    buffer = trim(buffer);

    if (buffer.empty())
      break;

    if (buffer[0] == '#' || buffer[0] == '/')
      continue;

    add_asset(m_assets.size(), base.filePath(buffer.c_str()));
  }

  m_modified = false;
}


///////////////////////// PackModel::save ///////////////////////////////////
void PackModel::save(string const &projectfile)
{
  ofstream fout(projectfile, ios::app);

  fout << "[Pack]" << "\n";

  QDir base = QFileInfo(projectfile.c_str()).dir();

  for(auto &asset: m_assets)
  {
    fout << base.relativeFilePath(asset->path()).toStdString() << "\n";
  }

  fout << "\n";

  m_modified = false;
}
