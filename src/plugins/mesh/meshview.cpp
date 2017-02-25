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

  m_material = resources.create<Material>(Color4(0.4f, 0.4f, 0.4f, 1.0f), 0.0f, 1.0f);

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

  for(auto &instance : m_document.instances())
  {
    m_meshes.push_back({ instance.transform, resources.load<Mesh>(m_document, instance.index) });
  }

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

    m_yawsign = (camera.up().y > 0) ? -1.0f : 1.0f;

    event->accept();
  }
}


///////////////////////// MeshView::mouseMoveEvent //////////////////////////
void MeshView::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_mousepresspos.isNull())
  {
    auto dx = event->pos().x() - m_mousemovepos.x();
    auto dy = m_mousemovepos.y() - event->pos().y();

    if (event->modifiers() == Qt::NoModifier)
    {
      camera.orbit(m_focuspoint, Transform::rotation(camera.right(), 0.01f * dy).rotation());
      camera.orbit(m_focuspoint, Transform::rotation(Vec3(0, 1, 0), m_yawsign * 0.01f * dx).rotation());
    }

    if (event->modifiers() == Qt::ShiftModifier)
    {
      camera.pan(m_focuspoint, -0.05f * dx, -0.05f * dy);
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

  GeometryList geometry;
  GeometryList::BuildState buildstate;

  if (begin(geometry, buildstate))
  {
    for(auto &instance : m_meshes)
    {
      geometry.push_mesh(buildstate, instance.transform, instance.mesh, m_material);
    }

    geometry.finalise(buildstate);
  }

  push_geometry(geometry);

  render();
}
