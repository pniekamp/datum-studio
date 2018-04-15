//
// DatumUi Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "datumuiplugin.h"
#include "contentapi.h"
#include "buildapi.h"
#include "assetfile.h"
#include "datum/ui/compiler.h"
#include <QFileInfo>
#include <QtPlugin>

#include <QDebug>

using namespace std;

namespace
{
  void hash_combine(size_t &seed, size_t key)
  {
    seed ^= key + 0x9e3779b9 + (seed<<6) + (seed>>2);
  }

  size_t hash_datumui(Studio::Document *document)
  {
    size_t key = std::hash<double>{}(document->metadata("build", 0.0));

    document->lock();

    PackTextHeader text;

    if (read_asset_header(document, 1, &text))
    {
      string payload(pack_payload_size(text), 0);

      read_asset_payload(document, text.dataoffset, &payload[0], payload.size());

      stringstream stream(payload);

      string buffer;

      while (getline(stream, buffer))
      {
        if (buffer.substr(0, 8) == "#include")
        {
          auto path = string(buffer.begin() + buffer.find_first_of('"') + 1, buffer.begin() + buffer.find_last_of('"')) + ".asset";

          auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

          if (auto includedocument = documentmanager->open(fullpath(document, path.c_str())))
          {
            hash_combine(key, hash_datumui(includedocument));

            documentmanager->close(includedocument);
          }
        }
      }
    }

    document->unlock();

    return key;
  }

  string load_datumui(Studio::Document *document)
  {
    auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

    int line = 1;

    auto name = QFileInfo(documentmanager->path(document)).completeBaseName().toStdString();

    string ui = "#line " + to_string(line) + "\"" + name + "\"\n";

    document->lock();

    PackTextHeader text;

    if (read_asset_header(document, 1, &text))
    {
      string payload(pack_payload_size(text), 0);

      read_asset_payload(document, text.dataoffset, &payload[0], payload.size());

      stringstream stream(payload);

      string buffer;

      while (getline(stream, buffer))
      {
        ++line;

        if (buffer.substr(0, 8) == "#include")
        {
          auto path = string(buffer.begin() + buffer.find_first_of('"') + 1, buffer.begin() + buffer.find_last_of('"')) + ".asset";

          auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

          if (auto includedocument = documentmanager->open(fullpath(document, path.c_str())))
          {
            buffer = load_datumui(includedocument);

            buffer += "\n#line " + to_string(line) + "\"" + name + "\"";

            documentmanager->close(includedocument);
          }
        }

        ui += buffer + '\n';
      }
    }

    document->unlock();

    return ui;
  }
}

//|---------------------- DatumUiPlugin -------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// DatumUiPlugin::Constructor ////////////////////////
DatumUiPlugin::DatumUiPlugin()
{
}


///////////////////////// DatumUiPlugin::Destructor /////////////////////////
DatumUiPlugin::~DatumUiPlugin()
{
  shutdown();
}


///////////////////////// DatumUiPlugin::initialise /////////////////////////
bool DatumUiPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("DatumUi", this);

  actionmanager->register_action("DatumUi.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("DatumUi", this);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("DatumUi", this);

  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->register_builder("DatumUi", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("DatumUi", this);

  return true;
}


///////////////////////// DatumUiPlugin::shutdown ///////////////////////////
void DatumUiPlugin::shutdown()
{
}


///////////////////////// DatumUiPlugin::create_view ////////////////////////
QWidget *DatumUiPlugin::create_view(QString const &type)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  return viewfactory->create_view("Text");
}


///////////////////////// DatumUiPlugin::create /////////////////////////////
bool DatumUiPlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  metadata["type"] = "DatumUi";
  metadata["icon"] = encode_icon(QIcon(":/datumuiplugin/icon.png"));

  const char *text = "";

  ofstream fout(path.toUtf8(), ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_text(fout, 1, strlen(text), text);

  write_asset_footer(fout);

  fout.close();

  return true;
}


///////////////////////// DatumUiPlugin::hash ///////////////////////////////
bool DatumUiPlugin::hash(Studio::Document *document, size_t *key)
{
  *key = hash_datumui(document);

  return true;
}


///////////////////////// DatumUiPlugin::build //////////////////////////////
bool DatumUiPlugin::build(Studio::Document *document, QString const &path)
{
  auto bytecode = parse_ui(load_datumui(document));

  ofstream fout(path.toStdString(), ios::binary | ios::trunc);

  write_header(fout);

  write_catl_asset(fout, 0, 0, 0);

  write_text_asset(fout, 1, bytecode.size(), bytecode.data());

  write_chunk(fout, "HEND", 0, nullptr);

  fout.close();

  return true;
}


///////////////////////// DatumUiPlugin::pack ///////////////////////////////
bool DatumUiPlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  ifstream fin(asset.buildpath, ios::binary);

  if (!fin)
    throw runtime_error("Ui Pack failed - no build file");

  PackTextHeader text;;

  if (read_asset_header(fin, 1, &text))
  {
    vector<char> payload(pack_payload_size(text));

    read_asset_payload(fin, text.dataoffset, payload.data(), payload.size());

    write_text_asset(fout, asset.id, payload.size(), payload.data());
  }

  return true;
}
