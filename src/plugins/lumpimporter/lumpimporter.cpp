//
// Lump Importer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "lumpimporter.h"
#include "contentapi.h"
#include "assetfile.h"
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- LumpImporter --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// LumpImporter::Constructor /////////////////////////
LumpImporter::LumpImporter()
{
}


///////////////////////// LumpImporter::Destructor //////////////////////////
LumpImporter::~LumpImporter()
{
  shutdown();
}


///////////////////////// LumpImporter::initialise //////////////////////////
bool LumpImporter::initialise(QStringList const &arguments, QString *errormsg)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_importer("Lump", this);

  return true;
}


///////////////////////// LumpImporter::shutdown ////////////////////////////
void LumpImporter::shutdown()
{
}


///////////////////////// LumpImporter::try_import //////////////////////////
bool LumpImporter::try_import(QString const &src, QString const &dst, QJsonObject metadata)
{
  vector<uint8_t> data;

  ifstream fin(src.toUtf8(), ios::binary);

  fin.seekg(0, ios::end);

  data.resize(fin.tellg());

  fin.seekg(0, ios::beg);

  fin.read((char*)data.data(), data.size());

  metadata["src"] = src;
  metadata["icon"] = encode_icon(QIcon(":/lumpimporter/icon.png"));
  metadata["build"] = buildtime();

  ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_text(fout, 1, data.size(), data.data());

  write_asset_footer(fout);

  fout.close();

  return true;
}
