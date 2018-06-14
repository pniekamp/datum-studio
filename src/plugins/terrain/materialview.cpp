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
#include <QTimer>

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

  camera.lookat(Vec3(0, 85, 0), m_focuspoint, Vec3(0, 1, 0));

  renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

  m_surface = resources.make_plane(257, 257);

  m_heightmap = resources.load<Texture>(CoreAsset::zero_depth, Texture::Format::HEIGHT);
  m_heightnormalmap = resources.load<Texture>(CoreAsset::nominal_normal, Texture::Format::RGBA);

  {
    m_blendmap = resources.create<Texture>(1024, 1024, 4, 1, Texture::Format::RGBA);

    if (auto lump = resources.acquire_lump(m_blendmap->size()))
    {
      uint32_t *bits = lump->memory<uint32_t>();

      for(int layer = 0; layer < m_blendmap->layers; ++layer)
      {
        for(int y = 0; y < m_blendmap->height; ++y)
        {
          for(int x = 0; x < m_blendmap->width; ++x)
          {
            uint32_t color = 0;

            int patch = (y / 256) * (m_blendmap->width / 256) + (x / 256);

            if (patch / 4 == layer)
            {
              if (patch % 4 == 0)
                color = 0x00FF0000;

              if (patch % 4 == 1)
                color = 0x0000FF00;

              if (patch % 4 == 2)
                color = 0x000000FF;

              if (patch % 4 == 3)
                color = 0xFF000000;
            }

            bits[layer * (m_blendmap->width * m_blendmap->height) + y * m_blendmap->width + x] = color;
          }
        }
      }

      resources.update(m_blendmap, lump);

      resources.release_lump(lump);
    }
  }

  m_material = resources.create<Material>(Color4(0.4f, 0.4f, 0.4f, 1.0f), 0.0f, 1.0f);

  m_buildhash = 0;

  setAcceptDrops(true);
}


///////////////////////// MaterialView::view ////////////////////////////////
void MaterialView::view(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &TerrainMaterialDocument::document_changed, this, &MaterialView::refresh);
  connect(&m_document, &TerrainMaterialDocument::dependant_changed, this, &MaterialView::refresh);

  refresh();
}


///////////////////////// MaterialView::invalidate //////////////////////////
void MaterialView::invalidate()
{
  update();
}


///////////////////////// MaterialView::refresh /////////////////////////////
void MaterialView::refresh()
{
  auto color = m_document.color();
  auto metalness = m_document.metalness();
  auto roughness = m_document.roughness();
  auto reflectivity = m_document.reflectivity();
  auto emissive = m_document.emissive();

  resources.update(m_material, color, metalness, roughness, reflectivity, emissive);

  size_t hash;
  TerrainMaterialDocument::build_hash(m_document, &hash);

  if (m_buildhash != hash)
  {
    auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

    buildmanager->request_build(m_document, this, &MaterialView::on_material_build_complete);

    m_buildhash = hash;
  }

  invalidate();
}


///////////////////////// MaterialView::material_build_complete /////////////
void MaterialView::on_material_build_complete(Studio::Document *document, QString const &path)
{
  if (m_buildpath != path)
  {
    ifstream fin(path.toUtf8(), ios::binary);

    auto color = m_document.color();
    auto metalness = m_document.metalness();
    auto roughness = m_document.roughness();
    auto reflectivity = m_document.reflectivity();
    auto emissive = m_document.emissive();

    m_albedomap = resources.load<Texture>(fin, 1, Texture::Format::SRGBA);
    m_surfacemap = resources.load<Texture>(fin, 2, Texture::Format::RGBA);
    m_normalmap = resources.load<Texture>(fin, 3, Texture::Format::RGBA);

    resources.update(m_material, color, metalness, roughness, reflectivity, emissive, *m_albedomap, *m_surfacemap, *m_normalmap);

    m_buildpath = path;

    invalidate();
  }
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

  m_skybox = resources.load<SkyBox>(fin, 1);

  renderparams.skybox = m_skybox;

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

    camera.lookat(Vec3(0, 85, 0), m_focuspoint, Vec3(0, 1, 0));

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

    m_yawsign = (camera.up().y > 0) ? -1.0f : 1.0f;

    event->accept();
  }
}


///////////////////////// MaterialView::mouseMoveEvent //////////////////////
void MaterialView::mouseMoveEvent(QMouseEvent *event)
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
      if (document->type() == "Image")
      {
        event->accept();
      }

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

  for(auto &url : event->mimeData()->urls())
  {
    if (auto document = documentmanager->open(url.toLocalFile()))
    {
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

  GeometryList geometry;
  GeometryList::BuildState buildstate;

  if (begin(geometry, buildstate))
  {
    if (m_surface)
    {
      if (m_material->albedomap && m_material->surfacemap && m_material->normalmap)
      {
        geometry.push_terrain(buildstate, Transform::rotation(Vec3(1, 0, 0), -pi<float>()/2), m_surface, m_heightmap, m_heightnormalmap, Rect2(Vec2(0), Vec2(1)), 50.0f, 40.0f, 50.0f, 4.0f/256, m_material, m_blendmap, m_document.layers(), Vec2(16, 16));
      }
    }

    geometry.finalise(buildstate);
  }

  push_geometry(geometry);

  render();
}
