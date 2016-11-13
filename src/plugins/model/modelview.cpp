//
// Model View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "modelview.h"
#include "buildapi.h"
#include "assetfile.h"
#include "mesh.h"
#include "material.h"
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

//|---------------------- ModelView -----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ModelView::Constructor ////////////////////////////
ModelView::ModelView(QWidget *parent)
  : Viewport(8*1024, 2*1024*1024, parent)
{
  m_focuspoint = Vec3(0, 0, 0);

  renderparams.ssaoscale = 0;
  renderparams.ssrstrength = 0;

  camera.lookat(Vec3(0, 1, 10), m_focuspoint, Vec3(0, 1, 0));

  renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

  setAcceptDrops(true);
}


///////////////////////// ModelView::view ///////////////////////////////////
void ModelView::view(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &ModelDocument::document_changed, this, &ModelView::refresh);
  connect(&m_document, &ModelDocument::dependant_changed, this, &ModelView::refresh);

  refresh();
}


///////////////////////// ModelView::edit ///////////////////////////////////
void ModelView::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &ModelDocument::document_changed, this, &ModelView::refresh);
  connect(&m_document, &ModelDocument::dependant_changed, this, &ModelView::refresh);

  refresh();
}


///////////////////////// ModelView::invalidate /////////////////////////////
void ModelView::invalidate()
{
  update();
}


///////////////////////// ModelView::find_or_create_mesh ////////////////////
Mesh const *ModelView::find_or_create_mesh(Studio::Document *document, size_t index)
{
  auto &meshdata = m_meshes[{ document, index }];

  size_t hash;
  MeshDocument::hash(document, &hash);

  if (meshdata.hash != hash)
  {
    meshdata.mesh = resources.load<Mesh>(document, index);

    meshdata.hash = hash;
  }

  return meshdata.mesh;
}


///////////////////////// ModelView::find_or_create_material ////////////////
Material const *ModelView::find_or_create_material(Studio::Document *document, Color3 const &tint)
{
  auto &materialdata = m_materials[document];

  if (document)
  {
    size_t hash;
    MaterialDocument::hash(document, &hash);

    if (materialdata.hash != hash)
    {
      auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

      buildmanager->request_build(document, this, &ModelView::on_material_build_complete);

      materialdata.hash = hash;
    }
  }

  auto j = find_if(materialdata.tints.begin(), materialdata.tints.end(), [&](auto &md) { return (md.tint == tint); });

  if (j == materialdata.tints.end())
  {
    auto color = tint;
    auto metalness = 0.0f;
    auto roughness = 1.0f;
    auto reflectivity = 0.5f;
    auto emissive = 0.0f;

    if (auto materialdocument = MaterialDocument(document))
    {
      color = hada(tint, materialdocument.color());
      metalness = materialdocument.metalness();
      roughness = materialdocument.roughness();
      reflectivity = materialdocument.reflectivity();
      emissive = materialdocument.emissive();
    }

    j = materialdata.tints.insert(materialdata.tints.end(), { tint, resources.create<Material>(color, metalness, roughness, reflectivity, emissive, *materialdata.albedomap, *materialdata.specularmap, *materialdata.normalmap) });
  }

  return j->material;
}


///////////////////////// MaterialView::build_complete //////////////////////
void ModelView::on_material_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  auto &materialdata = m_materials[document];

  materialdata.albedomap = resources.load<Texture>(fin, 1, Texture::Format::SRGBA);
  materialdata.specularmap = resources.load<Texture>(fin, 2, Texture::Format::RGBA);
  materialdata.normalmap = resources.load<Texture>(fin, 3, Texture::Format::RGBA);

  if (auto materialdocument = MaterialDocument(document))
  {
    auto color = materialdocument.color();
    auto metalness = materialdocument.metalness();
    auto roughness = materialdocument.roughness();
    auto reflectivity = materialdocument.reflectivity();
    auto emissive = materialdocument.emissive();

    for(auto &tintdata : materialdata.tints)
    {
      resources.update<Material>(tintdata.material, hada(tintdata.tint, color), metalness, roughness, reflectivity, emissive, *materialdata.albedomap, *materialdata.specularmap, *materialdata.normalmap);
    }
  }

  invalidate();
}


///////////////////////// ModelView::refresh ////////////////////////////////
void ModelView::refresh()
{
  m_instances.clear();

  for(auto &instance : m_document.instances())
  {
    auto mesh = find_or_create_mesh(instance.mesh, instance.index);
    auto material = find_or_create_material(instance.material, instance.tint);

    m_instances.push_back({ mesh, material, instance.transform });
  }

  invalidate();
}


///////////////////////// ModelView::set_skybox /////////////////////////////
void ModelView::set_skybox(QString const &path)
{
  if (m_skyboxdocument = SkyboxDocument(path))
  {
    auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

    buildmanager->request_build(m_skyboxdocument, this, &ModelView::on_skybox_build_complete);

    connect(&m_skyboxdocument, &SkyboxDocument::document_changed, [=]() { set_skybox(path); });
    connect(&m_skyboxdocument, &SkyboxDocument::dependant_changed, [=]() { set_skybox(path); });
  }
}


///////////////////////// ModelView::skybox_build_complete //////////////////
void ModelView::on_skybox_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  m_skybox = resources.load<SkyBox>(fin, 1);

  renderparams.skybox = m_skybox;

  invalidate();
}


///////////////////////// ModelView::set_exposure ///////////////////////////
void ModelView::set_exposure(float value)
{
  if (camera.exposure() != value)
  {
    camera.set_exposure(value);

    emit exposure_changed(camera.exposure());

    invalidate();
  }
}


///////////////////////// ModelView::keyPressEvent //////////////////////////
void ModelView::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Period && event->modifiers() == Qt::KeypadModifier)
  {
    m_focuspoint = Vec3(0, 0, 0);

    camera.lookat(m_focuspoint, Vec3(0, 1, 0));

    renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

    invalidate();
  }
}


///////////////////////// ModelView::mousePressEvent ////////////////////////
void ModelView::mousePressEvent(QMouseEvent *event)
{
  if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::MidButton))
  {
    m_mousepresspos = m_mousemovepos = event->pos();

    m_yawsign = (camera.up().y < 0) ? -1.0f : 1.0f;

    event->accept();
  }
}


///////////////////////// ModelView::mouseMoveEvent /////////////////////////
void ModelView::mouseMoveEvent(QMouseEvent *event)
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


///////////////////////// ModelView::mouseReleaseEvent //////////////////////
void ModelView::mouseReleaseEvent(QMouseEvent *event)
{
  m_mousepresspos = QPoint();
}


///////////////////////// ModelView::wheelEvent /////////////////////////////
void ModelView::wheelEvent(QWheelEvent *event)
{
  camera.dolly(m_focuspoint, 0.01*event->angleDelta().y());

  invalidate();
}


///////////////////////// ModelView::dragEnterEvent /////////////////////////
void ModelView::dragEnterEvent(QDragEnterEvent *event)
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
      if (document->type() == "SkyBox")
      {
        event->accept();
      }

      documentmanager->close(document);
    }
  }
}


///////////////////////// ModelView::dropEvent //////////////////////////////
void ModelView::dropEvent(QDropEvent *event)
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


///////////////////////// ModelView::paintEvent /////////////////////////////
void ModelView::paintEvent(QPaintEvent *event)
{
  prepare();

  MeshList meshes;
  MeshList::BuildState meshstate;

  if (begin(meshes, meshstate))
  {
    for(auto &instance : m_instances)
    {
      meshes.push_mesh(meshstate, instance.transform, instance.mesh, instance.material);
    }

    meshes.finalise(meshstate);
  }

  push_meshes(meshes);

  render();
}
