//
// Material View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "materialview.h"
#include "buildapi.h"
#include "assetfile.h"
#include <leap/pathstring.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>

#include <QDebug>

using namespace std;
using namespace lml;
using namespace leap;

//|---------------------- MaterialView --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MaterialView::Constructor /////////////////////////
MaterialView::MaterialView(QWidget *parent)
  : Viewport(8*1024, 2*1024*1024, parent)
{
  m_focuspoint = Vec3(0, 0, 0);

  renderparams.ssaoscale = 0;
  renderparams.ssrstrength = 0;
  renderparams.bloomstrength = 0;

  camera.lookat(Vec3(0, 1, 2), m_focuspoint, Vec3(0, 1, 0));

  renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

  m_material = resources.create<Material>(Color3(0.4f, 0.4f, 0.4f), 0.0f, 1.0f);

  try
  {
    ifstream fin(pathstring("sphere.pack").c_str(), ios::binary);

    PackMeshHeader mhdr;

    if (read_asset_header(fin, 0, &mhdr))
    {
      auto mesh = resources.create<Mesh>(mhdr.vertexcount, mhdr.indexcount);

      if (auto lump = resources.acquire_lump(mesh->vertexbuffer.size))
      {
        fin.seekg(mhdr.dataoffset + sizeof(PackChunk));
        fin.read((char*)lump->transfermemory + mesh->vertexbuffer.verticiesoffset, mesh->vertexbuffer.vertexcount*mesh->vertexbuffer.vertexsize);
        fin.read((char*)lump->transfermemory + mesh->vertexbuffer.indicesoffset, mesh->vertexbuffer.indexcount*mesh->vertexbuffer.indexsize);

        resources.update<Mesh>(mesh, lump);

        resources.release_lump(lump);
      }

      m_meshes.push_back({ Transform::identity(), std::move(mesh) });
    }
  }
  catch(exception &e)
  {
    qDebug() << "Error loading default mesh:" << e.what();
  }

  setAcceptDrops(true);
}


///////////////////////// MaterialView::view ////////////////////////////////
void MaterialView::view(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &MaterialDocument::document_changed, this, &MaterialView::refresh);
  connect(&m_document, &MaterialDocument::dependant_changed, this, &MaterialView::refresh);

  refresh();
}


///////////////////////// MaterialView::set_mesh ////////////////////////////
void MaterialView::set_mesh(QString const &path)
{
  m_meshes.clear();

  if (m_meshdocument = MeshDocument(path))
  {
    m_meshdocument->lock();

    PackModelHeader modl;

    if (read_asset_header(m_meshdocument, 1, &modl))
    {
      vector<char> payload(pack_payload_size(modl));

      read_asset_payload(m_meshdocument, modl.dataoffset, payload.data(), payload.size());

      auto meshtable = PackModelPayload::meshtable(payload.data(), modl.texturecount, modl.materialcount, modl.meshcount, modl.instancecount);
      auto instancetable = PackModelPayload::instancetable(payload.data(), modl.texturecount, modl.materialcount, modl.meshcount, modl.instancecount);

      for(size_t i = 0; i < modl.instancecount; ++i)
      {
        PackMeshHeader mhdr;

        if (read_asset_header(m_meshdocument, 1 + meshtable[instancetable[i].mesh].mesh, &mhdr))
        {
          auto mesh = resources.create<Mesh>(mhdr.vertexcount, mhdr.indexcount);

          if (auto lump = resources.acquire_lump(mesh->vertexbuffer.size))
          {
            uint64_t position = mhdr.dataoffset + sizeof(PackChunk);

            position += m_meshdocument->read(position, (uint8_t*)lump->transfermemory + mesh->vertexbuffer.verticiesoffset, mesh->vertexbuffer.vertexcount*mesh->vertexbuffer.vertexsize);
            position += m_meshdocument->read(position, (uint8_t*)lump->transfermemory + mesh->vertexbuffer.indicesoffset, mesh->vertexbuffer.indexcount*mesh->vertexbuffer.indexsize);

            resources.update<Mesh>(mesh, lump);

            resources.release_lump(lump);
          }

          auto transform = Transform{ { instancetable[i].transform[0], instancetable[i].transform[1], instancetable[i].transform[2], instancetable[i].transform[3] }, { instancetable[i].transform[4], instancetable[i].transform[5], instancetable[i].transform[6], instancetable[i].transform[7] } };

          m_meshes.push_back({ transform, std::move(mesh) });
        }
      }
    }

    m_meshdocument->unlock();

    connect(&m_meshdocument, &MeshDocument::document_changed, [=]() { set_mesh(path); });
  }

  m_focuspoint = Vec3(0, 0, 0);

  camera.lookat(Vec3(0, 1, 10), m_focuspoint, Vec3(0, 1, 0));

  renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

  invalidate();
}


///////////////////////// MaterialView::set_skybox //////////////////////////
void MaterialView::set_skybox(QString const &path)
{
  if (m_skyboxdocument = SkyboxDocument(path))
  {
    auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

    buildmanager->request_build(m_skyboxdocument, this, &MaterialView::on_skybox_build_complete);

    connect(&m_skyboxdocument, &SkyboxDocument::document_changed, [=]() { set_skybox(path); });
    connect(&m_skyboxdocument, &SkyboxDocument::dependant_changed, [=]() { set_skybox(path); });
  }
}


///////////////////////// MaterialView::skybox_build_complete ///////////////
void MaterialView::on_skybox_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  PackImageHeader imag;

  if (read_asset_header(fin, 1, &imag))
  {
    m_skybox = resources.create<SkyBox>(imag.width, imag.height, EnvMap::Format::RGBE);

    if (auto lump = resources.acquire_lump(imag.datasize))
    {
      read_asset_payload(fin, imag.dataoffset, lump->transfermemory, imag.datasize);

      resources.update<SkyBox>(m_skybox, lump);

      resources.release_lump(lump);
    }

    renderparams.skybox = m_skybox;
  }

  invalidate();
}


///////////////////////// MaterialView::invalidate //////////////////////////
void MaterialView::invalidate()
{
  update();
}


///////////////////////// MaterialView::refresh /////////////////////////////
void MaterialView::refresh()
{
  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->request_build(m_document, this, &MaterialView::on_build_complete);
}


///////////////////////// MaterialView::build_complete //////////////////////
void MaterialView::on_build_complete(Studio::Document *document, QString const &path)
{
  auto color = m_document.color();
  auto metalness = m_document.metalness();
  auto roughness = m_document.roughness();
  auto reflectivity = m_document.reflectivity();
  auto emissive = m_document.emissive();

  if (m_buildpath != path)
  {
    ifstream fin(path.toUtf8(), ios::binary);

    PackImageHeader imag;

    m_albedomap = {};

    if (read_asset_header(fin, 1, &imag))
    {
      m_albedomap = resources.create<Texture>(imag.width, imag.height, imag.layers, imag.levels, Texture::Format::SRGBA);

      if (auto lump = resources.acquire_lump(imag.datasize))
      {
        read_asset_payload(fin, imag.dataoffset, lump->transfermemory, imag.datasize);

        resources.update<Texture>(m_albedomap, lump);

        resources.release_lump(lump);
      }
    }

    m_specularmap = {};

    if (read_asset_header(fin, 2, &imag))
    {
      m_specularmap = resources.create<Texture>(imag.width, imag.height, imag.layers, imag.levels, Texture::Format::RGBA);

      if (auto lump = resources.acquire_lump(imag.datasize))
      {
        read_asset_payload(fin, imag.dataoffset, lump->transfermemory, imag.datasize);

        resources.update<Texture>(m_specularmap, lump);

        resources.release_lump(lump);
      }
    }

    m_normalmap = {};

    if (read_asset_header(fin, 3, &imag))
    {
      m_normalmap = resources.create<Texture>(imag.width, imag.height, imag.layers, imag.levels, Texture::Format::RGBA);

      if (auto lump = resources.acquire_lump(imag.datasize))
      {
        read_asset_payload(fin, imag.dataoffset, lump->transfermemory, imag.datasize);

        resources.update<Texture>(m_normalmap, lump);

        resources.release_lump(lump);
      }
    }

    m_material = resources.create<Material>(color, metalness, roughness, reflectivity, emissive, *m_albedomap, *m_specularmap, *m_normalmap);

    m_buildpath = path;
  }

  resources.update<Material>(*m_material, color, metalness, roughness, reflectivity, emissive);

  invalidate();
}


///////////////////////// MaterialView::set_exposure ////////////////////////
void MaterialView::set_exposure(float value)
{
  if (camera.exposure() != value)
  {
    camera.set_exposure(value);

    emit exposure_changed(camera.exposure());

    invalidate();
  }
}


///////////////////////// MaterialView::keyPressEvent ///////////////////////
void MaterialView::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Period && event->modifiers() == Qt::KeypadModifier)
  {
    m_focuspoint = Vec3(0, 0, 0);

    camera.lookat(m_focuspoint, Vec3(0, 1, 0));

    renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

    invalidate();
  }
}


///////////////////////// MaterialView::mousePressEvent /////////////////////
void MaterialView::mousePressEvent(QMouseEvent *event)
{
  if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::MidButton))
  {
    m_mousepresspos = m_mousemovepos = event->pos();

    m_yawsign = (camera.up().y < 0) ? -1.0f : 1.0f;

    event->accept();
  }
}


///////////////////////// MaterialView::mouseMoveEvent //////////////////////
void MaterialView::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_mousepresspos.isNull())
  {
    auto dx = m_mousemovepos.x() - event->pos().x();
    auto dy = event->pos().y() - m_mousemovepos.y();

    if (event->modifiers() == Qt::NoModifier)
    {
      camera.orbit(m_focuspoint, Transform::rotation(camera.right(), -0.01f * dy).rotation());
      camera.orbit(m_focuspoint, Transform::rotation(Vec3(0, 1, 0), m_yawsign * 0.01f * dx).rotation());
    }

    if (event->modifiers() == Qt::ShiftModifier)
    {
      camera.pan(m_focuspoint, 0.05f * dx, 0.05f * dy);
    }

    renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

    invalidate();
  }

  m_mousemovepos = event->pos();
}


///////////////////////// MaterialView::mouseReleaseEvent ///////////////////
void MaterialView::mouseReleaseEvent(QMouseEvent *event)
{
  m_mousepresspos = QPoint();
}


///////////////////////// MaterialView::wheelEvent //////////////////////////
void MaterialView::wheelEvent(QWheelEvent *event)
{
  camera.dolly(m_focuspoint, 0.01*event->angleDelta().y());

  invalidate();
}


///////////////////////// MaterialView::dragEnterEvent //////////////////////
void MaterialView::dragEnterEvent(QDragEnterEvent *event)
{
  if (!event->source())
    return;

  if (!(event->possibleActions() & Qt::CopyAction))
    return;

  if (event->dropAction() != Qt::CopyAction)
  {
    event->setDropAction(Qt::CopyAction);
  }

  if (event->mimeData()->urls().size() == 1)
  {
    auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

    if (auto document = documentmanager->open(event->mimeData()->urls().at(0).toLocalFile()))
    {
      if (document->type() == "Mesh")
      {
        event->accept();
      }

      if (document->type() == "SkyBox")
      {
        event->accept();
      }

      documentmanager->close(document);
    }
  }
}


///////////////////////// MaterialView::dropEvent ///////////////////////////
void MaterialView::dropEvent(QDropEvent *event)
{
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  foreach(QUrl url, event->mimeData()->urls())
  {
    if (auto document = documentmanager->open(url.toLocalFile()))
    {
      if (document->type() == "Mesh")
      {
        set_mesh(url.toLocalFile());
      }

      if (document->type() == "SkyBox")
      {
        set_skybox(url.toLocalFile());
      }

      documentmanager->close(document);
    }
  }
}


///////////////////////// MaterialView::paintEvent //////////////////////////
void MaterialView::paintEvent(QPaintEvent *event)
{
  prepare();

  MeshList meshes;
  MeshList::BuildState meshstate;

  if (begin(meshes, meshstate))
  {
    meshes.push_material(meshstate, m_material);

    for(auto &instance : m_meshes)
    {
      meshes.push_mesh(meshstate, instance.transform, instance.mesh);
    }

    meshes.finalise(meshstate);
  }

  push_meshes(meshes);

  render();
}
