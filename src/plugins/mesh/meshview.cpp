//
// Mesh View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "meshview.h"
#include "assetfile.h"
#include "buildapi.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- MeshView ------------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// MeshView::Constructor /////////////////////////////
MeshView::MeshView(QWidget *parent)
  : Viewport(8*1024, 2*1024*1024, parent)
{
  m_focuspoint = Vec3(0, 0, 0);

  camera.lookat(Vec3(0, 1, 10), m_focuspoint, Vec3(0, 1, 0));

  renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

  m_material = resources.create<Material>(Color3(0.4f, 0.4f, 0.4f), 0.0f, 1.0f);

  renderparams.ssaoscale = 0;
  renderparams.ssrstrength = 0;
  renderparams.bloomstrength = 0;
}


///////////////////////// MeshView::view ////////////////////////////////////
void MeshView::view(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &MeshDocument::document_changed, this, &MeshView::refresh);

  refresh();
}


///////////////////////// MeshView::invalidate //////////////////////////////
void MeshView::invalidate()
{
  update();
}


///////////////////////// MeshView::refresh /////////////////////////////////
void MeshView::refresh()
{
  m_meshes.clear();

  m_document->lock();

  PackModelHeader modl;

  if (read_asset_header(m_document, 1, &modl))
  {
    vector<char> payload(pack_payload_size(modl));

    read_asset_payload(m_document, modl.dataoffset, payload.data(), payload.size());

    auto meshtable = PackModelPayload::meshtable(payload.data(), modl.texturecount, modl.materialcount, modl.meshcount, modl.instancecount);
    auto instancetable = PackModelPayload::instancetable(payload.data(), modl.texturecount, modl.materialcount, modl.meshcount, modl.instancecount);

    for(size_t i = 0; i < modl.instancecount; ++i)
    {
      PackMeshHeader mhdr;

      if (read_asset_header(m_document, 1 + meshtable[instancetable[i].mesh].mesh, &mhdr))
      {
        auto mesh = resources.create<Mesh>(mhdr.vertexcount, mhdr.indexcount);

        if (auto lump = resources.acquire_lump(mesh->vertexbuffer.size))
        {
          uint64_t position = mhdr.dataoffset + sizeof(PackChunk);

          position += m_document->read(position, (uint8_t*)lump->transfermemory + mesh->vertexbuffer.verticiesoffset, mesh->vertexbuffer.vertexcount*mesh->vertexbuffer.vertexsize);
          position += m_document->read(position, (uint8_t*)lump->transfermemory + mesh->vertexbuffer.indicesoffset, mesh->vertexbuffer.indexcount*mesh->vertexbuffer.indexsize);

          resources.update<Mesh>(mesh, lump);

          resources.release_lump(lump);
        }

        auto transform = Transform{ { instancetable[i].transform[0], instancetable[i].transform[1], instancetable[i].transform[2], instancetable[i].transform[3] }, { instancetable[i].transform[4], instancetable[i].transform[5], instancetable[i].transform[6], instancetable[i].transform[7] } };

        m_meshes.push_back({ transform, std::move(mesh) });
      }
    }
  }

  m_document->unlock();

  invalidate();
}


///////////////////////// MeshView::keyPressEvent ///////////////////////////
void MeshView::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Period && event->modifiers() == Qt::KeypadModifier)
  {
    m_focuspoint = Vec3(0, 0, 0);

    camera.lookat(m_focuspoint, Vec3(0, 1, 0));

    renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

    invalidate();
  }
}


///////////////////////// MeshView::mousePressEvent /////////////////////////
void MeshView::mousePressEvent(QMouseEvent *event)
{
  if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::MidButton))
  {
    m_mousepresspos = m_mousemovepos = event->pos();

    m_yawsign = (camera.up().y < 0) ? -1.0f : 1.0f;

    event->accept();
  }
}


///////////////////////// MeshView::mouseMoveEvent //////////////////////////
void MeshView::mouseMoveEvent(QMouseEvent *event)
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


///////////////////////// MeshView::mouseReleaseEvent ///////////////////////
void MeshView::mouseReleaseEvent(QMouseEvent *event)
{
  m_mousepresspos = QPoint();
}


///////////////////////// MeshView::wheelEvent //////////////////////////////
void MeshView::wheelEvent(QWheelEvent *event)
{
  camera.dolly(m_focuspoint, 0.01*event->angleDelta().y());

  invalidate();
}


///////////////////////// MeshView::paintEvent //////////////////////////////
void MeshView::paintEvent(QPaintEvent *event)
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
