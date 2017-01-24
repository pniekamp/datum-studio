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
    qCritical() << "Error loading default mesh:" << e.what();
  }

  m_updatetimer = new QTimer(this);
  m_updatetimer->setSingleShot(true);

  connect(m_updatetimer, SIGNAL(timeout()), this, SLOT(update()));

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


///////////////////////// MaterialView::invalidate //////////////////////////
void MaterialView::invalidate()
{
  m_updatetimer->start();
}


///////////////////////// MaterialView::refresh /////////////////////////////
void MaterialView::refresh()
{
  auto color = m_document.color();
  auto metalness = m_document.metalness();
  auto roughness = m_document.roughness();
  auto reflectivity = m_document.reflectivity();
  auto emissive = m_document.emissive();

  resources.update<Material>(*m_material, color, metalness, roughness, reflectivity, emissive);

  size_t hash;
  MaterialDocument::build_hash(m_document, &hash);

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
  ifstream fin(path.toUtf8(), ios::binary);

  auto color = m_document.color();
  auto metalness = m_document.metalness();
  auto roughness = m_document.roughness();
  auto reflectivity = m_document.reflectivity();
  auto emissive = m_document.emissive();

  m_albedomap = resources.load<Texture>(fin, 1, Texture::Format::SRGBA);
  m_specularmap = resources.load<Texture>(fin, 2, Texture::Format::RGBA);
  m_normalmap = resources.load<Texture>(fin, 3, Texture::Format::RGBA);

  resources.update<Material>(m_material, color, metalness, roughness, reflectivity, emissive, *m_albedomap, *m_specularmap, *m_normalmap);

  invalidate();
}


///////////////////////// MaterialView::set_mesh ////////////////////////////
void MaterialView::set_mesh(QString const &path)
{
  m_meshes.clear();

  if (m_meshdocument = MeshDocument(path))
  {
    for(auto &instance : m_meshdocument.instances())
    {
      m_meshes.push_back({ instance.transform, resources.load<Mesh>(m_meshdocument, instance.index) });
    }

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

  if (m_document.shader() == MaterialDocument::Shader::Deferred)
  {
    GeometryList geometry;
    GeometryList::BuildState buildstate;

    if (begin(geometry, buildstate))
    {
      geometry.push_material(buildstate, m_material);

      for(auto &instance : m_meshes)
      {
        geometry.push_mesh(buildstate, instance.transform, instance.mesh);
      }

      geometry.finalise(buildstate);
    }

    push_geometry(geometry);
  }

  if (m_document.shader() == MaterialDocument::Shader::Translucent)
  {
    ForwardList objects;
    ForwardList::BuildState buildstate;

    if (begin(objects, buildstate))
    {
      for(auto &instance : m_meshes)
      {
        objects.push_translucent(buildstate, instance.transform, instance.mesh, m_material);
      }

      objects.finalise(buildstate);
    }

    push_objects(objects);
  }

  render();
}
