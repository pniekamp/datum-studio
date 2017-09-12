//
// Document Management
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "documentmanager.h"
#include "assetfile.h"
#include <QFileInfo>
#include <QFile>
#include <cassert>

#include <QtDebug>

using namespace std;
using namespace leap;
using namespace leap::threadlib;

//|---------------------- Document ------------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// Document::Constructor /////////////////////////////
Document::Document(QString const &path)
  : m_modified(false)
{
  lock_exclusive();

  attach(path);

  m_metadata = read_asset_json(m_file, 0);

#if 0
  uint64_t position = sizeof(PackHeader);

  position += sizeof(PackAssetHeader) + sizeof(PackChunk) + sizeof(uint32_t);
  position += sizeof(PackTextHeader) + sizeof(PackChunk) + sizeof(uint32_t);

  PackChunk chunk;

  m_file.seekg(position);
  m_file.read((char*)&chunk, sizeof(chunk));

  position += chunk.length + sizeof(chunk) + sizeof(uint32_t);
  position += sizeof(chunk) + sizeof(uint32_t);

  if (chunk.length < 16384)
  {
    ofstream fout(path.toStdString() + ".tmp", ios::binary);

    write_asset_header(fout, m_metadata);

    while (m_file)
    {
      m_file.seekg(position);
      m_file.read((char*)&chunk, sizeof(chunk));

      if (chunk.type == "HEND"_packchunktype)
        break;

      vector<char> buffer(chunk.length);

      m_file.read(buffer.data(), chunk.length);

      switch(chunk.type)
      {
        case "TEXT"_packchunktype:
          reinterpret_cast<PackTextHeader*>(buffer.data())->dataoffset += 8192;
          break;

        case "IMAG"_packchunktype:
          reinterpret_cast<PackImageHeader*>(buffer.data())->dataoffset += 8192;
          break;

        case "MODL"_packchunktype:
          reinterpret_cast<PackModelHeader*>(buffer.data())->dataoffset += 8192;
          break;

        case "MESH"_packchunktype:
          reinterpret_cast<PackMeshHeader*>(buffer.data())->dataoffset += 8192;
          break;

        case "ASET"_packchunktype:
        case "AEND"_packchunktype:
        case "DATA"_packchunktype:
          break;

        default:
          assert(false);
      }

      write_chunk(fout, (char*)&chunk.type, buffer.size(), buffer.data());

      position += chunk.length + sizeof(chunk) + sizeof(uint32_t);
    }

    write_asset_footer(fout);

    fout.close();

    detach();
    QFile::remove(path);
    QFile::rename(path + ".tmp", path);
    attach(path);
  }
#endif

  unlock_exclusive();
}


///////////////////////// Document::type ////////////////////////////////////
QString Document::type() const
{
  return metadata("type").toString();
}


///////////////////////// Document::icon ////////////////////////////////////
QIcon Document::icon() const
{
  return decode_icon(metadata("icon").toString());
}


///////////////////////// Document::set_icon ////////////////////////////////
void Document::set_icon(QIcon const &icon)
{
  set_metadata("icon", encode_icon(icon));
}


///////////////////////// Document::metadata ////////////////////////////////
QJsonObject Document::metadata() const
{
  SyncLock lock(m_mutex);

  return m_metadata;
}


///////////////////////// Document::metadata ////////////////////////////////
QVariant Document::metadata(QString const &name) const
{
  SyncLock lock(m_mutex);

  return m_metadata[name].toVariant();
}


///////////////////////// Document::set_metadata ////////////////////////////
void Document::set_metadata(QString const &name, QVariant const &value)
{
  assert(m_exclusive);

  SyncLock lock(m_mutex);

  m_metadata[name] = QJsonValue::fromVariant(value);

  m_modified = true;
}


///////////////////////// Document::modified ////////////////////////////////
bool Document::modified() const
{
  return m_modified;
}


///////////////////////// Document::attach ////////////////////////////////////
void Document::attach(QString const &path)
{
  assert(m_exclusive);

  m_file.open(path.toUtf8(), ios::in | ios::out | ios::binary);

  if (!m_file)
    throw runtime_error("Error Attaching Document");
}


///////////////////////// Document::detatch /////////////////////////////////
void Document::detach()
{
  assert(m_exclusive);

  m_file.close();
}


///////////////////////// Document::read ////////////////////////////////////
size_t Document::read(uint64_t position, void *buffer, size_t bytes)
{
  assert(m_locked);

  constexpr size_t blocksize = sizeof(Document::Block::data);

  size_t readbytes = 0;

  while (bytes != 0)
  {
    SyncLock lock(m_mutex);

    size_t index = position / blocksize;

    size_t offset = position - index * blocksize;

    size_t size = min(blocksize - offset, bytes);

    auto blk = m_blocks.find(index);

    if (blk == m_blocks.end())
    {
      Block block = {};

      block.index = index;
      block.modified = false;

      m_file.clear();
      m_file.seekg(index * blocksize);
      m_file.read((char*)block.data, blocksize);

      block.size = m_file.gcount();

      blk = m_blocks.insert({ index, block }).first;

      blk->second.lrunode = m_lru.insert(m_lru.end(), &blk->second);

      if (m_lru.size() > MaxCacheBlocks)
      {
        m_blocks.erase(m_lru.front()->index);

        m_lru.pop_front();
      }
    }

    if (blk->second.lrunode != std::list<Block*>::iterator())
    {
      m_lru.splice(m_lru.end(), m_lru, blk->second.lrunode);
    }

    memcpy(buffer, blk->second.data + offset, size);

    bytes -= size;
    position += size;
    buffer = (char*)buffer + size;    
    readbytes += min(blk->second.size - offset, size);
  }

  return readbytes;
}


///////////////////////// Document::write ///////////////////////////////////
size_t Document::write(uint64_t position, void const *buffer, size_t bytes)
{
  assert(m_exclusive);

  constexpr size_t blocksize = sizeof(Document::Block::data);

  size_t writebytes = 0;

  while (bytes != 0)
  {
    size_t index = position / blocksize;

    size_t offset = position - index * blocksize;

    size_t size = min(blocksize - offset, bytes);

    auto blk = m_blocks.find(index);

    if (blk == m_blocks.end())
    {
      Block block = {};

      block.index = index;
      block.modified = true;

      m_file.clear();
      m_file.seekg(index * blocksize);
      m_file.read((char*)block.data, blocksize);

      block.size = m_file.gcount();

      blk = m_blocks.insert({ index, block }).first;
    }

    if (!blk->second.modified)
    {
      blk->second.modified = true;
      m_lru.erase(blk->second.lrunode);
      blk->second.lrunode = std::list<Block*>::iterator();
    }

    memcpy(blk->second.data + offset, buffer, size);

    blk->second.size = max(blk->second.size, offset + size);

    bytes -= size;
    position += size;
    buffer = (char*)buffer + size;
    writebytes += size;
  }

  m_modified = true;

  return writebytes;
}


///////////////////////// Document::discard /////////////////////////////////
void Document::discard()
{
  assert(m_exclusive);

  SyncLock lock(m_mutex);

  m_lru.clear();
  m_blocks.clear();
  m_metadata = read_asset_json(m_file, 0);
  m_modified = false;
}


///////////////////////// Document::save ////////////////////////////////////
void Document::save()
{
  assert(m_exclusive);

  m_file.clear();
  m_file.seekg(0);

  write_asset_header(m_file, m_metadata);

  for(auto &blk : m_blocks)
  {
    auto &block = blk.second;

    if (block.modified)
    {
      m_file.seekg(block.index * sizeof(Document::Block::data));
      m_file.write((char*)block.data, block.size);

      block.modified = false;
      block.lrunode = m_lru.insert(m_lru.end(), &block);
    }
  }

  m_modified = false;
}


///////////////////////// Document::lock ////////////////////////////////////
void Document::lock()
{
  m_lock.readwait();

  assert((m_locked += 1) >= 0);
}


///////////////////////// Document::unlock //////////////////////////////////
void Document::unlock()
{
  assert((m_locked -= 1) >= 0);

  m_lock.readrelease();
}


///////////////////////// Document::lock_exclusive //////////////////////////
void Document::lock_exclusive()
{
  m_lock.writewait();

  assert((m_locked += 1) == 1);
  assert((m_exclusive += 1) == 1);
}


///////////////////////// Document::unlock_exclusive ////////////////////////
void Document::unlock_exclusive(bool changed)
{
  assert((m_locked -= 1) == 0);
  assert((m_exclusive -= 1) == 0);

  m_lock.writerelease();

  if (changed)
  {
    emit document_changed();
  }
}



//|---------------------- DocumentManager -----------------------------------
//|--------------------------------------------------------------------------

///////////////////////// DocumentManager::Constructor //////////////////////
DocumentManager::DocumentManager()
{
}


///////////////////////// DocumentManager::open /////////////////////////////
Document *DocumentManager::open(QString const &path)
{
  assert(path == QFileInfo(path).absoluteFilePath());

  try
  {
    SyncLock lock(m_mutex);

    auto doc = find_if(m_documents.begin(), m_documents.end(), [&](auto &doc) { return doc.path == path; });

    if (doc == m_documents.end())
    {
      DocInfo docinfo;
      docinfo.path = path;
      docinfo.document = new Document(path);
      docinfo.refcount = 0;

      doc = m_documents.insert(m_documents.end(), docinfo);

      connect(docinfo.document, &Document::document_changed, this, [this,document=docinfo.document,path]() { emit document_changed(document, path); });
    }

    doc->refcount += 1;

    return doc->document;
  }
  catch(exception &e)
  {
    qDebug() << QString("Invalid Asset File (%1): %2").arg(path).arg(e.what());

    return nullptr;
  }
}


///////////////////////// DocumentManager::create ///////////////////////////
Document *DocumentManager::create(QString const &path)
{
  QJsonObject metadata;

  fstream fout(path.toUtf8(), ios::out | ios::binary | ios::trunc);

  write_asset_header(fout, metadata);

  write_asset_footer(fout);

  fout.close();

  return open(path);
}


///////////////////////// DocumentManager::dup //////////////////////////////
Document *DocumentManager::dup(Studio::Document *document)
{
  SyncLock lock(m_mutex);

  auto doc = find_if(m_documents.begin(), m_documents.end(), [&](auto &doc) { return doc.document == document; });

  assert(doc != m_documents.end());

  doc->refcount += 1;

  return doc->document;
}


///////////////////////// DocumentManager::close ////////////////////////////
void DocumentManager::close(Studio::Document *document)
{
  SyncLock lock(m_mutex);

  auto doc = find_if(m_documents.begin(), m_documents.end(), [&](auto &doc) { return doc.document == document; });

  assert(doc != m_documents.end());

  doc->refcount -= 1;

  if (doc->refcount == 0)
  {
    delete doc->document;

    m_documents.erase(doc);
  }
}


///////////////////////// DocumentManager::rewrite //////////////////////////
QString DocumentManager::path(Studio::Document *document) const
{
  SyncLock lock(m_mutex);

  auto doc = find_if(m_documents.begin(), m_documents.end(), [&](auto &doc) { return doc.document == document; });

  assert(doc != m_documents.end());

  return doc->path;
}


///////////////////////// DocumentManager::rename ///////////////////////////
bool DocumentManager::rename(Studio::Document *document, QString const &dst)
{
  m_mutex.wait();

  bool result = false;

  auto doc = find_if(m_documents.begin(), m_documents.end(), [&](auto &doc) { return doc.document == document; });

  assert(doc != m_documents.end());

  QString src = doc->path;

  doc->document->lock_exclusive();

  try
  {
    doc->document->detach();

    if (QFile::rename(src, dst))
    {
      disconnect(doc->document, &Document::document_changed, this, 0);

      doc->path = dst;

      connect(doc->document, &Document::document_changed, this, [this,document=doc->document,path=doc->path]() { emit document_changed(document, path); });

      result = true;
    }

    doc->document->attach(doc->path);
  }
  catch(exception &e)
  {
    qDebug() << QString("Error Renaming File (%1): %2").arg(dst).arg(e.what());
  }

  m_mutex.release();

  document->unlock_exclusive(false);

  if (result)
  {
    emit document_renamed(doc->document, src, dst);
  }

  return result;
}


///////////////////////// DocumentManager::rewrite //////////////////////////
bool DocumentManager::rewrite(Studio::Document *document, QString const &src)
{
  m_mutex.wait();

  bool result = false;

  auto doc = find_if(m_documents.begin(), m_documents.end(), [&](auto &doc) { return doc.document == document; });

  assert(doc != m_documents.end());

  doc->document->lock_exclusive();

  try
  {
    doc->document->detach();

    if (QFile::remove(doc->path))
    {
      if (QFile::rename(src, doc->path))
      {
        result = true;
      }
    }

    doc->document->attach(doc->path);

    doc->document->discard();
  }
  catch(exception &e)
  {
    qDebug() << QString("Error ReWriting File (%1): %2").arg(src).arg(e.what());
  }

  m_mutex.release();

  document->unlock_exclusive();

  return result;
}
