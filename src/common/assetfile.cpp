//
// Asset File
//

//
// Copyright (C) 2016 Peter Niekamp
//


#include "assetfile.h"
#include <QJsonDocument>
#include <QDir>
#include <chrono>

#include <QtDebug>

using namespace std;

const int MetaDataBlockSize = 8192;


///////////////////////// encode_icon ///////////////////////////////////////
QString encode_icon(QIcon const &icon)
{
  QByteArray data;

  QDataStream datastream(&data, QIODevice::WriteOnly);

  datastream << icon;

  return data.toBase64();
}


///////////////////////// decode_icon ///////////////////////////////////////
QIcon decode_icon(QString const &str)
{
  QIcon icon;

  QByteArray data = QByteArray::fromBase64(str.toUtf8());

  QDataStream datastream(data);

  datastream >> icon;

  return icon;
}


///////////////////////// read_asset_header ////////////////////////////////
uint64_t read_asset_header(istream &fin, uint32_t id, uint32_t type, void *data, size_t size)
{
  fin.clear();
  fin.seekg(0);

  PackHeader header;
  fin.read((char*)&header, sizeof(header));

  if (header.signature[0] != 0xD9 || header.signature[1] != 'S' || header.signature[2] != 'V' || header.signature[3] != 'A')
    throw runtime_error("Invalid pack file");

  uint64_t position = sizeof(PackHeader);

  while (fin)
  {
    PackChunk chunk;

    fin.seekg(position);
    fin.read((char*)&chunk, sizeof(chunk));

    if (chunk.type == "HEND"_packchunktype)
      break;

    if (chunk.type == "ASET"_packchunktype)
    {
      PackAssetHeader aset;

      fin.seekg(position + sizeof(chunk));
      fin.read((char*)&aset, sizeof(aset));

      if (aset.id == id)
      {
        uint64_t imagpos = position + chunk.length + sizeof(chunk) + sizeof(uint32_t);

        fin.seekg(imagpos, ios::beg);

        PackChunk chunk;
        fin.read((char*)&chunk, sizeof(chunk));

        if (chunk.type != type)
          throw runtime_error("Invalid asset type");

        fin.read((char*)data, size);

        return position;
      }
    }

    position += chunk.length + sizeof(chunk) + sizeof(uint32_t);
  }

  return 0;
}


///////////////////////// read_asset_header ////////////////////////////////
uint64_t read_asset_header(istream &fin, uint32_t id, PackTextHeader *text)
{
  return read_asset_header(fin, id, "TEXT"_packchunktype, text, sizeof(*text));
}


///////////////////////// read_asset_header ////////////////////////////////
uint64_t read_asset_header(istream &fin, uint32_t id, PackFontHeader *font)
{
  return read_asset_header(fin, id, "FONT"_packchunktype, font, sizeof(*font));
}


///////////////////////// read_asset_header ////////////////////////////////
uint64_t read_asset_header(istream &fin, uint32_t id, PackImageHeader *imag)
{
  return read_asset_header(fin, id, "IMAG"_packchunktype, imag, sizeof(*imag));
}


///////////////////////// read_asset_header ////////////////////////////////
uint64_t read_asset_header(istream &fin, uint32_t id, PackMeshHeader *mesh)
{
  return read_asset_header(fin, id, "MESH"_packchunktype, mesh, sizeof(*mesh));
}


///////////////////////// read_asset_header /////////////////////////////////
uint64_t read_asset_header(istream &fin, uint32_t id, PackModelHeader *modl)
{
  return read_asset_header(fin, id, "MODL"_packchunktype, modl, sizeof(*modl));
}


///////////////////////// read_asset_payload ////////////////////////////////
uint64_t read_asset_payload(istream &fin, uint64_t offset, void *data, uint32_t size)
{
  fin.seekg(offset + sizeof(PackChunk));

  fin.read((char*)data, size);

  return fin.tellg();
}


///////////////////////// read_asset_image //////////////////////////////////
QImage read_asset_image(istream &fin, uint32_t id, int layer)
{
  PackImageHeader imag;

  if (read_asset_header(fin, id, &imag))
  {
    QImage image(imag.width, imag.height, QImage::Format_ARGB32);

    fin.seekg(imag.dataoffset + sizeof(PackChunk) + min(layer, (int)imag.layers - 1) * image_datasize(imag.width, imag.height, 1, 1));
    fin.read((char*)image.bits(), image.byteCount());

    return image;
  }

  return QImage();
}


///////////////////////// read_asset_text ///////////////////////////////////
QByteArray read_asset_text(istream &fin, uint32_t id)
{
  PackTextHeader text;

  if (read_asset_header(fin, id, &text))
  {
    QByteArray payload(pack_payload_size(text), 0);

    read_asset_payload(fin, text.dataoffset, payload.data(), payload.size());

    return payload;
  }

  return QByteArray();
}


///////////////////////// read_asset_json ///////////////////////////////////
QJsonObject read_asset_json(istream &fin, uint32_t id)
{
  PackTextHeader text;

  if (read_asset_header(fin, id, &text))
  {
    QByteArray payload(pack_payload_size(text), 0);

    read_asset_payload(fin, text.dataoffset, payload.data(), payload.size());

    return QJsonDocument::fromBinaryData(payload).object();
  }

  return QJsonObject();
}


///////////////////////// write_asset_header ////////////////////////////////
void write_asset_header(ostream &fout, QJsonObject const &metadata)
{
  write_header(fout);

  QByteArray data = QJsonDocument(metadata).toBinaryData();

  if (data.size() > MetaDataBlockSize)
    throw runtime_error("Metadata block size exceeded");

  PackAssetHeader aset = { 0 };

  write_chunk(fout, "ASET", sizeof(aset), &aset);

  PackTextHeader shdr = { (uint32_t)data.size(), (size_t)fout.tellp() + sizeof(shdr) + sizeof(PackChunk) + sizeof(uint32_t) };

  write_chunk(fout, "TEXT", sizeof(shdr), &shdr);

  data.resize(MetaDataBlockSize);

  write_chunk(fout, "DATA", data.size(), data.data());

  write_chunk(fout, "AEND", 0, nullptr);
}


///////////////////////// write_asset_text //////////////////////////////////
void write_asset_text(ostream &fout, uint32_t id, uint32_t length, void const *data)
{
  PackAssetHeader aset = { id };

  write_chunk(fout, "ASET", sizeof(aset), &aset);

  PackTextHeader shdr = { length, (size_t)fout.tellp() + sizeof(shdr) + sizeof(PackChunk) + sizeof(uint32_t) };

  write_chunk(fout, "TEXT", sizeof(shdr), &shdr);

  write_chunk(fout, "DATA", length, data);

  write_chunk(fout, "AEND", 0, nullptr);
}


///////////////////////// write_asset_image /////////////////////////////////
void write_asset_image(ostream &fout, uint32_t id, vector<QImage> const &images, uint32_t format)
{
  int width = images.front().width();
  int height = images.front().height();
  int layers = images.size();
  int levels = 1;

  vector<char> payload(image_datasize(width, height, layers, levels));

  char *dst = payload.data();

  for(size_t i = 0; i < images.size(); i++)
  {
    if (images[i].width() != width || images[i].height() != height)
      throw runtime_error("Layers with differing dimensions");

    memcpy(dst, images[i].bits(), images[i].byteCount());

    dst += images[i].byteCount();
  }

  write_imag_asset(fout, id, width, height, layers, levels, format, payload.data());
}


///////////////////////// write_asset_json //////////////////////////////////
void write_asset_json(ostream &fout, uint32_t id, QJsonObject const &json)
{
  QByteArray data = QJsonDocument(json).toBinaryData();

  write_asset_text(fout, id, data.size(), data.data());
}


///////////////////////// write_asset_footer ////////////////////////////////
void write_asset_footer(ostream &fout)
{
  write_chunk(fout, "HEND", 0, nullptr);
}


///////////////////////// read_asset_header /////////////////////////////////
uint64_t read_asset_header(Studio::Document *document, uint32_t id, uint32_t type, void *data, size_t size)
{
  uint64_t position = sizeof(PackHeader);

  while (true)
  {
    PackChunk chunk;

    document->read(position, &chunk, sizeof(chunk));

    if (chunk.type == "HEND"_packchunktype)
      break;

    if (chunk.type == "ASET"_packchunktype)
    {
      PackAssetHeader aset;

      document->read(position + sizeof(chunk), &aset, sizeof(aset));

      if (aset.id == id)
      {
        uint64_t textpos = position + chunk.length + sizeof(chunk) + sizeof(uint32_t);

        document->read(textpos, &chunk, sizeof(chunk));

        if (chunk.type != type)
          throw runtime_error("Invalid asset type");

        document->read(textpos + sizeof(chunk), data, size);

        return position;
      }
    }

    position += chunk.length + sizeof(chunk) + sizeof(uint32_t);
  }

  return 0;
}


///////////////////////// read_asset_header /////////////////////////////////
uint64_t read_asset_header(Studio::Document *document, uint32_t id, PackTextHeader *text)
{
  return read_asset_header(document, id, "TEXT"_packchunktype, text, sizeof(*text));
}


///////////////////////// read_asset_header /////////////////////////////////
uint64_t read_asset_header(Studio::Document *document, uint32_t id, PackImageHeader *imag)
{
  return read_asset_header(document, id, "IMAG"_packchunktype, imag, sizeof(*imag));
}


///////////////////////// read_asset_header /////////////////////////////////
uint64_t read_asset_header(Studio::Document *document, uint32_t id, PackMeshHeader *mesh)
{
  return read_asset_header(document, id, "MESH"_packchunktype, mesh, sizeof(*mesh));
}


///////////////////////// read_asset_header /////////////////////////////////
uint64_t read_asset_header(Studio::Document *document, uint32_t id, PackModelHeader *modl)
{
  return read_asset_header(document, id, "MODL"_packchunktype, modl, sizeof(*modl));
}


///////////////////////// read_asset_payload ////////////////////////////////
uint64_t read_asset_payload(Studio::Document *document, uint64_t offset, void *data, uint32_t size)
{
  auto position = offset + sizeof(PackChunk);

  position += document->read(position, data, size);

  return position;
}


///////////////////////// write_chunk ///////////////////////////////////////
uint64_t write_chunk(Studio::Document *document, uint64_t position, const char type[4], uint32_t length, void const *data)
{
  uint32_t checksum = 0;

  for(size_t i = 0; i < length; ++i)
    checksum ^= static_cast<uint8_t const*>(data)[i] << (i % 4);

  auto writepos = position;

  writepos += document->write(writepos, &length, sizeof(length));
  writepos += document->write(writepos, type, 4);
  writepos += document->write(writepos, data, length);
  writepos += document->write(writepos, &checksum, sizeof(checksum));

  return writepos - position;
}


///////////////////////// write_text_asset //////////////////////////////////
uint64_t write_text_asset(Studio::Document *document, uint64_t position, uint32_t id, uint32_t length, void const *data)
{
  auto writepos = position;

  PackAssetHeader aset = { id };

  writepos += write_chunk(document, writepos, "ASET", sizeof(aset), &aset);

  PackTextHeader shdr = { length, writepos + sizeof(shdr) + sizeof(PackChunk) + sizeof(uint32_t) };

  writepos += write_chunk(document, writepos, "TEXT", sizeof(shdr), &shdr);

  writepos += write_chunk(document, writepos, "DATA", length, data);

  writepos += write_chunk(document, writepos, "AEND", 0, nullptr);

  return writepos - position;
}


///////////////////////// write_footer //////////////////////////////////////
uint64_t write_footer(Studio::Document *document, uint64_t position)
{
  auto writepos = position;

  writepos += write_chunk(document, writepos, "HEND", 0, nullptr);

  return writepos - position;
}


///////////////////////// buildtime /////////////////////////////////////////
double buildtime()
{
  return chrono::duration<double>(chrono::system_clock::now().time_since_epoch()).count();
}


///////////////////////// relpath ///////////////////////////////////////////
QString relpath(Studio::Document *document, QString const &file)
{
  if (file == "")
    return "";

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  return QFileInfo(documentmanager->path(document)).dir().relativeFilePath(file);
}


///////////////////////// fullpath //////////////////////////////////////////
QString fullpath(Studio::Document *document, QString const &file)
{
  if (file == "")
    return "";

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  return QFileInfo(documentmanager->path(document)).dir().absoluteFilePath(file);
}
