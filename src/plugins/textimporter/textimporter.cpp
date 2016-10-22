//
// Text Importer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "textimporter.h"
#include "contentapi.h"
#include "assetfile.h"
#include <QMimeDatabase>
#include <QtPlugin>

#include <QDebug>

using namespace std;

//|---------------------- TextImporter --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// TextImporter::Constructor /////////////////////////
TextImporter::TextImporter()
{
}


///////////////////////// TextImporter::Destructor //////////////////////////
TextImporter::~TextImporter()
{
  shutdown();
}


///////////////////////// TextImporter::initialise //////////////////////////
bool TextImporter::initialise(QStringList const &arguments, QString *errormsg)
{
  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_importer("Text", this);

  return true;
}


///////////////////////// TextImporter::shutdown ////////////////////////////
void TextImporter::shutdown()
{
}


///////////////////////// TextImporter::try_import //////////////////////////
bool TextImporter::try_import(QString const &src, QString const &dst, QJsonObject metadata)
{
  QMimeDatabase db;
  QMimeType type = db.mimeTypeForFile(src);

  if (!type.inherits("text/plain"))
    return false;

  vector<uint8_t> data;

  ifstream fin(src.toUtf8());

  string buffer;

  while (getline(fin, buffer))
  {
    data.insert(data.end(), buffer.begin(), buffer.end());

    data.push_back('\n');
  }

  metadata["src"] = src;
  metadata["type"] = "Text";
  metadata["icon"] = encode_icon(QIcon(":/textimporter/icon.png"));
  metadata["build"] = buildtime();

  ofstream fout(dst.toUtf8(), ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_text(fout, 1, data.size(), data.data());

  write_asset_footer(fout);

  fout.close();

  return true;
}
