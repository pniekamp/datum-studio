//
// Shader Plugin
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "shaderplugin.h"
#include "contentapi.h"
#include "buildapi.h"
#include "assetfile.h"
#include "atlaspacker.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "resourcelimits.h"
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

  size_t hash_shader(Studio::Document *document)
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
          auto path = string(buffer.begin() + buffer.find_first_of("\"") + 1, buffer.begin() + buffer.find_last_of("\"")) + ".asset";

          auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

          if (auto includedocument = documentmanager->open(fullpath(document, path.c_str())))
          {
            hash_combine(key, hash_shader(includedocument));

            documentmanager->close(includedocument);
          }
        }
      }
    }

    document->unlock();

    return key;
  }

  string load_shader(Studio::Document *document)
  {
    auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

    int line = 1;

    auto name = QFileInfo(documentmanager->path(document)).completeBaseName().toStdString();

    string shader = "#line " + to_string(line) + "\"" + name + "\"\n";

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

        if (buffer.substr(0, 8) == "#version")
        {
          shader = "";
          buffer += "\n#extension GL_GOOGLE_cpp_style_line_directive : enable\n";
          buffer += "\n#line " + to_string(line) + "\"" + name + "\"";
        }

        if (buffer.substr(0, 8) == "#include")
        {
          auto path = string(buffer.begin() + buffer.find_first_of("\"") + 1, buffer.begin() + buffer.find_last_of("\"")) + ".asset";

          auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

          if (auto includedocument = documentmanager->open(fullpath(document, path.c_str())))
          {
            buffer = load_shader(includedocument);

            buffer += "\n#line " + to_string(line) + "\"" + name + "\"";

            documentmanager->close(includedocument);
          }
        }

        shader += buffer + '\n';
      }
    }

    document->unlock();

    return shader;
  }
}

//|---------------------- ShaderPlugin --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ShaderPlugin::Constructor /////////////////////////
ShaderPlugin::ShaderPlugin()
{
}


///////////////////////// ShaderPlugin::Destructor //////////////////////////
ShaderPlugin::~ShaderPlugin()
{
  shutdown();
}


///////////////////////// ShaderPlugin::initialise /////////////////////////
bool ShaderPlugin::initialise(QStringList const &arguments, QString *errormsg)
{
  auto actionmanager = Studio::Core::instance()->find_object<Studio::ActionManager>();

  auto create = new QAction("Shader", this);

  actionmanager->register_action("Shader.Create", create);

  actionmanager->container("Content.Menu.Create")->add_back(create);

  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  viewfactory->register_factory("Shader", this);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();

  contentmanager->register_creator("Shader", this);

  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->register_builder("Shader", this);

  auto packmanager = Studio::Core::instance()->find_object<Studio::PackManager>();

  packmanager->register_packer("Shader", this);

  glslang::InitializeProcess();

  return true;
}


///////////////////////// ShaderPlugin::shutdown ////////////////////////////
void ShaderPlugin::shutdown()
{
  glslang::FinalizeProcess();
}


///////////////////////// ShaderPlugin::create_view /////////////////////////
QWidget *ShaderPlugin::create_view(QString const &type)
{
  auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

  return viewfactory->create_view("Text");
}


///////////////////////// ShaderPlugin::create //////////////////////////////
bool ShaderPlugin::create(QString const &type, QString const &path, QJsonObject metadata)
{
  metadata["type"] = "Shader";
  metadata["icon"] = encode_icon(QIcon(":/shaderplugin/icon.png"));

  const char *text = "";

  ofstream fout(path.toUtf8(), ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_text(fout, 1, strlen(text), text);

  write_asset_footer(fout);

  fout.close();

  return true;
}


///////////////////////// ShaderPlugin::hash ////////////////////////////////
bool ShaderPlugin::hash(Studio::Document *document, size_t *key)
{
  *key = hash_shader(document);

  return true;
}


///////////////////////// ShaderPlugin::build ///////////////////////////////
bool ShaderPlugin::build(Studio::Document *document, QString const &path)
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  auto name = QFileInfo(documentmanager->path(document)).completeBaseName().toStdString();

  auto suffix = QFileInfo(name.c_str()).suffix().toLower().toStdString();

  auto payload = load_shader(document);

  EShLanguage stage;

  if (suffix == "vert")
    stage = EShLangVertex;
  else if (suffix == "geom")
    stage = EShLangGeometry;
  else if (suffix == "frag")
    stage = EShLangFragment;
  else if (suffix == "comp")
    stage = EShLangCompute;
  else
    throw runtime_error("Invalid Shader Stage");

  glslang::TShader shader(stage);

  const char *strings[] = { payload.c_str() };
  int lengths[] = { (int)payload.size() };
  const char *names[] = { name.c_str() };

  shader.setStringsWithLengthsAndNames(strings, lengths, names, 1);

  EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

  if (!shader.parse(&DefaultTBuiltInResource, 110, false, messages))
  {
    qCritical() << shader.getInfoLog();

    throw runtime_error("Shader build failed - compile error");
  }

  glslang::TProgram program;

  program.addShader(&shader);

  if (!program.link(messages) || !program.getIntermediate(stage))
  {
    qCritical() << program.getInfoLog();

    throw runtime_error("Shader build failed - link error");
  }

  vector<unsigned int> spirv;

  glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

  ofstream fout(path.toStdString(), ios::binary | ios::trunc);

  write_header(fout);

  write_catl_asset(fout, 0, 0, 0);

  write_text_asset(fout, 1, spirv.size()*sizeof(unsigned int), spirv.data());

  write_chunk(fout, "HEND", 0, nullptr);

  fout.close();

  return true;
}


///////////////////////// ShaderPlugin::pack ////////////////////////////////
bool ShaderPlugin::pack(Studio::PackerState &asset, ofstream &fout)
{
  ifstream fin(asset.buildpath, ios::binary);

  if (!fin)
    throw runtime_error("Shader Pack failed - no build file");

  PackTextHeader text;;

  if (read_asset_header(fin, 1, &text))
  {
    vector<char> payload(pack_payload_size(text));

    read_asset_payload(fin, text.dataoffset, payload.data(), payload.size());

    write_text_asset(fout, asset.id, payload.size(), payload.data());
  }

  return true;
}
