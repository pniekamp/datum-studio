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
#include <QApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStyleHints>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>
#include <QTimer>

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
  m_mode = ModeType::None;

  m_readonly = false;

  m_selection = -1;

  m_hitrotation = 0;

  m_focuspoint = Vec3(0, 0, 0);

//  renderparams.ssaoscale = 0;
//  renderparams.ssrstrength = 0;

  camera.lookat(Vec3(0, 1, 10), m_focuspoint, Vec3(0, 1, 0));

  renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

  m_linecube = resources.load<Mesh>(CoreAsset::line_cube);

  m_updatetimer = new QTimer(this);
  m_updatetimer->setSingleShot(true);

  connect(m_updatetimer, SIGNAL(timeout()), this, SLOT(update()));

  setMouseTracking(true);

  setAcceptDrops(true);
}


///////////////////////// ModelView::view ///////////////////////////////////
void ModelView::view(ModelDocument *document)
{
  m_document = document;

  connect(m_document, &ModelDocument::document_changed, this, &ModelView::refresh);
  connect(m_document, &ModelDocument::dependant_changed, this, &ModelView::refresh);

  m_readonly = true;

  refresh();
}


///////////////////////// ModelView::edit ///////////////////////////////////
void ModelView::edit(ModelDocument *document)
{
  m_document = document;

  connect(m_document, &ModelDocument::document_changed, this, &ModelView::refresh);
  connect(m_document, &ModelDocument::dependant_changed, this, &ModelView::refresh);

  m_readonly = false;

  refresh();
}


///////////////////////// ModelView::set_selection //////////////////////////
void ModelView::set_selection(int index)
{
  if (m_selection != index)
  {
    m_selection = index;

    m_selectedtransform = Transform::identity();

    emit selection_changed(index);

    invalidate();
  }
}


///////////////////////// ModelView::invalidate /////////////////////////////
void ModelView::invalidate()
{
  m_updatetimer->start();
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
Material const *ModelView::find_or_create_material(Studio::Document *document, Color4 const &tint)
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

    j = materialdata.tints.insert(materialdata.tints.end(), { tint, resources.create<Material>(color, metalness, roughness, reflectivity, emissive, *materialdata.albedomap, *materialdata.surfacemap, *materialdata.normalmap) });
  }

  return j->material;
}


///////////////////////// MaterialView::build_complete //////////////////////
void ModelView::on_material_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  auto &materialdata = m_materials[document];

  materialdata.albedomap = resources.load<Texture>(fin, 1, Texture::Format::SRGBA);
  materialdata.surfacemap = resources.load<Texture>(fin, 2, Texture::Format::RGBA);
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
      resources.update(tintdata.material, hada(tintdata.tint, color), metalness, roughness, reflectivity, emissive, *materialdata.albedomap, *materialdata.surfacemap, *materialdata.normalmap);
    }
  }

  invalidate();
}


///////////////////////// ModelView::refresh ////////////////////////////////
void ModelView::refresh()
{
  m_instances.clear();
  m_transparencies.clear();

  for(auto &instance : m_document->instances())
  {
    Instance id;

    id.index = instance.index;

    id.mesh = find_or_create_mesh(instance.submesh->document, instance.submesh->index);
    id.material = find_or_create_material(instance.material->document, instance.material->tint);

    id.transform = instance.transform;

    if (id.index == m_selection)
    {
      id.transform = m_selectedtransform * id.transform;
    }

    id.bound = id.transform * id.mesh->bound;

    auto shader = MaterialDocument::Shader::Deferred;

    if (auto materialdocument = MaterialDocument(instance.material->document))
      shader = materialdocument.shader();

    switch (shader)
    {
      case MaterialDocument::Shader::Deferred:
        m_instances.push_back(id);
        break;

      case MaterialDocument::Shader::Translucent:
        m_transparencies.push_back(id);
        break;
    }
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

  if (event->key() == Qt::Key_G && m_selection != -1)
  {
    m_wrapoffset = QPoint();

    m_mode = ModeType::Translating;
  }

  if (event->key() == Qt::Key_R && m_selection != -1)
  {
    m_wrapoffset = QPoint();

    m_mode = ModeType::Rotating;
  }

  if (event->key() == Qt::Key_Escape)
  {
    m_selectedtransform = Transform::identity();

    m_mode = ModeType::None;

    refresh();
  }

  m_keypresspos = m_mousemovepos;
}


///////////////////////// ModelView::mousePressEvent ////////////////////////
void ModelView::mousePressEvent(QMouseEvent *event)
{
  if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::MidButton))
  {
    m_mousepresspos = m_mousemovepos = event->pos();

    m_yawsign = (camera.up().y > 0) ? -1.0f : 1.0f;

    event->accept();
  }
}


///////////////////////// ModelView::mouseMoveEvent /////////////////////////
void ModelView::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_mousepresspos.isNull())
  {
    if (m_mode == ModeType::None)
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
  }

  if (m_mode == ModeType::Translating)
  {
    auto &mesh = m_document->mesh(m_selection);

    auto position = mesh.transform.translation();

    auto dx = event->pos().x() - m_keypresspos.x() + m_wrapoffset.x();
    auto dy = event->pos().y() - m_keypresspos.y() + m_wrapoffset.y();

    float scalex = 2 * dist(camera.position(), position) / camera.proj()(0, 0) / width();
    float scaley = 2 * dist(camera.position(), position) / camera.proj()(1, 1) / height();

    m_selectedtransform = Transform::translation(scalex*dx*camera.right() + scaley*dy*camera.up());

    refresh();
  }

  if (m_mode == ModeType::Rotating)
  {
    auto &mesh = m_document->mesh(m_selection);

    auto position = mesh.transform.translation();

    auto pt = camera.viewproj() * Vec4(position, 1.0f);

    auto bx = m_keypresspos.x() - (0.5f + 0.5f*pt.x/pt.w)*width();
    auto by = m_keypresspos.y() - (0.5f + 0.5f*pt.y/pt.w)*height();

    auto ax = event->pos().x() - (0.5f + 0.5f*pt.x/pt.w)*width() + m_wrapoffset.x();
    auto ay = event->pos().y() - (0.5f + 0.5f*pt.y/pt.w)*height() + m_wrapoffset.y();

    m_selectedtransform = Transform::translation(position) * Transform::rotation(camera.forward(), atan2(ay, ax) - atan2(by, bx)) * Transform::translation(-position);

    refresh();
  }

  m_mousemovepos = event->pos();
}


///////////////////////// ModelView::mouseReleaseEvent //////////////////////
void ModelView::mouseReleaseEvent(QMouseEvent *event)
{
  if (m_mode == ModeType::None)
  {
    if (event->button() == Qt::LeftButton && (m_mousepresspos - event->pos()).manhattanLength() < qApp->styleHints()->startDragDistance())
    {
      if (!m_readonly)
      {
        vector<int> hits;

        auto cameraposition = camera.position();
        auto ray = normalise((inverse(camera.viewproj()) * Vec4((2.0f * m_mousepresspos.x()) / width() - 1.0f, (2.0f * m_mousepresspos.y()) / height() - 1.0f, 1.0f, 1.0f)).xyz);

        for(auto &instance : m_instances)
        {
          if (intersection(instance.bound, cameraposition, cameraposition + ray))
            hits.push_back(instance.index);
        }

        for(auto &instance : m_transparencies)
        {
          if (intersection(instance.bound, cameraposition, cameraposition + ray))
            hits.push_back(instance.index);
        }

        sort(hits.begin(), hits.end());
        hits.erase(unique(hits.begin(), hits.end()), hits.end());

        set_selection((hits.size() != 0) ? hits[m_hitrotation++ % hits.size()] : -1);
      }
    }
  }

  if (m_mode == ModeType::Translating || m_mode == ModeType::Rotating)
  {
    auto transform = normalise(m_selectedtransform * m_document->mesh(m_selection).transform);

    m_selectedtransform = Transform::identity();

    m_document->set_mesh_transform(m_selection, transform);

    m_mode = ModeType::None;
  }

  m_mousepresspos = QPoint();
}


///////////////////////// ModelView::leaveEvent /////////////////////////////
void ModelView::leaveEvent(QEvent *event)
{
  QWidget::leaveEvent(event);

  auto pos = mapFromGlobal(QCursor::pos());

  if (m_mode == ModeType::Translating || m_mode == ModeType::Rotating)
  {
    if (pos.x() <= 0)
    {
      m_wrapoffset -= QPoint(width(), 0);
      QCursor::setPos(mapToGlobal(QPoint(width(), pos.y())));
    }

    if (pos.x() >= width())
    {
      m_wrapoffset += QPoint(width(), 0);
      QCursor::setPos(mapToGlobal(QPoint(0, pos.y())));
    }

    if (pos.y() <= 0)
    {
      m_wrapoffset -= QPoint(0, height());
      QCursor::setPos(mapToGlobal(QPoint(pos.x(), height())));
    }

    if (pos.y() >= height())
    {
      m_wrapoffset += QPoint(0, height());
      QCursor::setPos(mapToGlobal(QPoint(pos.x(), 0)));
    }
  }
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

  if (m_instances.size() != 0)
  {
    GeometryList geometry;
    GeometryList::BuildState buildstate;

    if (begin(geometry, buildstate))
    {
      for(auto &instance : m_instances)
      {
        geometry.push_mesh(buildstate, instance.transform, instance.mesh, instance.material);
      }

      geometry.finalise(buildstate);
    }

    push_geometry(geometry);
  }

  if (m_transparencies.size() != 0)
  {
    ForwardList objects;
    ForwardList::BuildState buildstate;

    if (begin(objects, buildstate))
    {
      auto cameraposition = camera.position();

      auto draworder = [&](auto &lhs, auto &rhs) { return distsqr(cameraposition, rhs.bound.centre()) < distsqr(cameraposition, lhs.bound.centre()); };

      sort(m_transparencies.begin(), m_transparencies.end(), draworder);

      for(auto &instance : m_transparencies)
      {
        objects.push_translucent(buildstate, instance.transform, instance.mesh, instance.material);
      }

      objects.finalise(buildstate);
    }

    push_forward(objects);
  }

  if (m_selection != -1)
  {
    OverlayList selection;
    OverlayList::BuildState buildstate;

    if (begin(selection, buildstate))
    {
      for(auto &instance : m_instances)
      {
        if (instance.index == m_selection)
        {
          selection.push_stencilmask(buildstate, instance.transform, instance.mesh);
        }
      }

      for(auto &instance : m_instances)
      {
        if (instance.index == m_selection)
        {
          selection.push_stencilpath(buildstate, instance.transform, instance.mesh, Color4(1.0f, 0.5f, 0.15f, 1.0f));
        }
      }

#if 0
      auto bound = bound_limits<Bound3>::min();

      for(auto &instance : m_instances)
      {
        if (instance.index == m_selection)
        {
          bound = expand(bound, instance.bound);
        }
      }

      selection.push_volume(buildstate, bound, m_linecube, Color4(0.6f, 0.2f, 0.2f, 0.4f));
#endif

      selection.finalise(buildstate);
    }

    push_overlays(selection);
  }

  render();
}
