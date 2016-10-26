//
// Font Build
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "font.h"
#include "assetfile.h"
#include "atlaspacker.h"
#include <QPainter>
#include <QJsonDocument>
#include <functional>

#include <QDebug>

using namespace std;
using namespace lml;

namespace
{
  uint32_t write_catalog(ostream &fout, uint32_t id)
  {
    write_catl_asset(fout, id);

    return id + 1;
  }

  uint32_t write_font_atlas(ostream &fout, uint32_t id, QImage const &atlas)
  {
    int width = atlas.width();
    int height = atlas.height();
    int layers = 1;
    int levels = min(4, image_maxlevels(width, height));

    vector<char> payload(image_datasize(width, height, layers, levels));

    memcpy(payload.data(), atlas.bits(), atlas.byteCount());

    image_premultiply_srgb(width, height, layers, levels, payload.data());

    image_buildmips_srgb(width, height, layers, levels, payload.data());

    write_imag_asset(fout, id, width, height, layers, levels, PackImageHeader::rgba, payload.data());

    return id + 1;
  }

  uint32_t write_font(ostream &fout, uint32_t id, QFont font, int atlaswidth, int atlasheight)
  {
    QFontMetrics tm(font);

    int count = 127;

    vector<uint16_t> x(count);
    vector<uint16_t> y(count);
    vector<uint16_t> width(count);
    vector<uint16_t> height(count);
    vector<uint16_t> offsetx(count);
    vector<uint16_t> offsety(count);
    vector<uint8_t> advance(count*count);

    AtlasPacker packer(atlaswidth, atlasheight);

    for(int codepoint = 33; codepoint < count; ++codepoint)
    {
      auto position = packer.insert(codepoint, tm.width(QChar(codepoint))+2, tm.height()+2);

      if (!position)
        throw runtime_error("Font pack failed");

      x[codepoint] = position->x;
      y[codepoint] = position->y;
      width[codepoint] = position->width;
      height[codepoint] = position->height;
      offsetx[codepoint] = 1;
      offsety[codepoint] = 1 + tm.ascent();
    }

    for(int codepoint = 0; codepoint < count; ++codepoint)
    {
      advance[codepoint] = 0;

      for(int othercodepoint = 1; othercodepoint < count; ++othercodepoint)
      {
        advance[othercodepoint * count + codepoint] = tm.width(QString(QChar(othercodepoint)) + QString(QChar(codepoint))) - tm.width(QChar(codepoint));
      }
    }

    write_font_asset(fout, id, tm.ascent(), tm.descent(), tm.leading(), count, 1, x, y, width, height, offsetx, offsety, advance);

    QImage atlas(packer.width, packer.height, QImage::Format_ARGB32);

    atlas.fill(0x00000000);

    for(int codepoint = 33; codepoint < count; ++codepoint)
    {
      auto position = packer.find(codepoint);

      QPainter painter(&atlas);

      painter.setFont(font);
      painter.setPen(Qt::white);
      painter.drawText(QRectF(position->x + 1, position->y + 1, position->width - 2, position->height - 2), QString(QChar(codepoint)));
    }

    write_font_atlas(fout, id + 1, atlas);

    return id + 2;
  }
}


///////////////////////// create ////////////////////////////////////////////
void FontDocument::create(string const &path, string const &name, int size, int weight)
{
  QJsonObject metadata;
  metadata["type"] = "Font";
  metadata["icon"] = encode_icon(QIcon(":/fontplugin/icon.png"));

  QJsonObject definition;
  definition["name"] = name.c_str();
  definition["size"] = size;
  definition["weight"] = weight;
  definition["atlaswidth"] = 512;
  definition["atlasheight"] = 256;

  ofstream fout(path, ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_json(fout, 1, definition);

  write_asset_footer(fout);

  fout.close();
}


///////////////////////// hash //////////////////////////////////////////////
void FontDocument::hash(Studio::Document *document, size_t *key)
{
  *key = std::hash<double>{}(document->metadata("build", 0.0));
}


///////////////////////// build /////////////////////////////////////////////
void FontDocument::build(Studio::Document *document, string const &path)
{
  QJsonObject definition;

  document->lock();

  PackTextHeader text;

  if (read_asset_header(document, 1, &text))
  {
    QByteArray payload(pack_payload_size(text), 0);

    read_asset_payload(document, text.dataoffset, payload.data(), payload.size());

    definition = QJsonDocument::fromBinaryData(payload).object();
  }

  document->unlock();

  QFont font(definition["name"].toString(), definition["size"].toInt(), definition["weight"].toInt(), definition["italic"].toBool());

  int atlaswidth = definition["atlaswidth"].toInt(512);
  int atlasheight = definition["atlasheight"].toInt(256);

  ofstream fout(path, ios::binary | ios::trunc);

  write_header(fout);

  write_catalog(fout, 0);

  write_font(fout, 1, font, atlaswidth, atlasheight);

  write_chunk(fout, "HEND", 0, nullptr);

  fout.close();
}


//|---------------------- FontDocument --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// FontDocument::Constructor /////////////////////////
FontDocument::FontDocument(QString const &path)
{
  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path));
}


///////////////////////// FontDocument::Constructor /////////////////////////
FontDocument::FontDocument(Studio::Document *document)
{
  if (document)
  {
    attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));
  }
}


///////////////////////// FontDocument::Constructor /////////////////////////
FontDocument::FontDocument(FontDocument const &document)
  : FontDocument(document.m_document)
{
}


///////////////////////// FontDocument::Assignment //////////////////////////
FontDocument FontDocument::operator =(FontDocument const &document)
{
  disconnect();

  attach(Studio::Core::instance()->find_object<Studio::DocumentManager>()->dup(document));

  return *this;
}


///////////////////////// FontDocument::attach //////////////////////////////
void FontDocument::attach(Studio::Document *document)
{
  if (m_document)
  {
    disconnect(m_document, 0, this, 0);
  }

  m_document = document;

  if (m_document)
  {
    connect(m_document, &Studio::Document::document_changed, this, &FontDocument::refresh);

    refresh();
  }
}


///////////////////////// FontDocument::set_name ////////////////////////////
void FontDocument::set_name(QString const &name)
{
  m_definition["name"] = name;

  update();
}


///////////////////////// FontDocument::set_size ////////////////////////////
void FontDocument::set_size(int size)
{
  m_definition["size"] = size;

  update();
}


///////////////////////// FontDocument::set_weight //////////////////////////
void FontDocument::set_weight(int weight)
{
  m_definition["weight"] = weight;

  update();
}


///////////////////////// FontDocument::set_italic //////////////////////////
void FontDocument::set_italic(bool italic)
{
  m_definition["italic"] = italic;

  update();
}


///////////////////////// FontDocument::set_font ////////////////////////////
void FontDocument::set_font(QFont const &font)
{
  m_definition["name"] = font.family();
  m_definition["size"] = font.pointSize();
  m_definition["weight"] = font.weight();
  m_definition["italic"] = font.italic();

  update();
}


///////////////////////// FontDocument::set_atlaswidth //////////////////////
void FontDocument::set_atlaswidth(int atlaswidth)
{
  m_definition["atlaswidth"] = atlaswidth;

  update();
}


///////////////////////// FontDocument::set_atlasheight /////////////////////
void FontDocument::set_atlasheight(int atlasheight)
{
  m_definition["atlasheight"] = atlasheight;

  update();
}


///////////////////////// FontDocument::refresh /////////////////////////////
void FontDocument::refresh()
{
  m_document->lock();

  PackTextHeader text;

  if (read_asset_header(m_document, 1, &text))
  {
    QByteArray payload(pack_payload_size(text), 0);

    read_asset_payload(m_document, text.dataoffset, payload.data(), payload.size());

    m_definition = QJsonDocument::fromBinaryData(payload).object();
  }

  m_document->unlock();

  emit document_changed();
}


///////////////////////// FontDocument::update //////////////////////////////
void FontDocument::update()
{
  QByteArray data = QJsonDocument(m_definition).toBinaryData();

  m_document->lock_exclusive();

  PackTextHeader text;

  if (auto position = read_asset_header(m_document, 1, &text))
  {
    position += write_text_asset(m_document, position, 1, data.size(), data.data());

    position += write_footer(m_document, position);

    m_document->set_metadata("build", buildtime());
  }

  m_document->unlock_exclusive();
}
