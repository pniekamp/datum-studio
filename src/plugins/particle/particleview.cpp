//
// Particle View
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "particleview.h"
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

//|---------------------- ParticleView --------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ParticleView::Constructor /////////////////////////
ParticleView::ParticleView(QWidget *parent)
  : Viewport(8*1024, 16*1024*1024, parent),
    m_particlememory{0, 2*1024*1024, new char[2*1024*1024]}
{
  m_focuspoint = Vec3(0, 0, 0);

  camera.lookat(Vec3(0, 1, 10), m_focuspoint, Vec3(0, 1, 0));

  renderparams.ssaoscale = 0;
  renderparams.ssrstrength = 0;

  m_showbound = true;

  m_spritesheet = resources.load<Texture>(CoreAsset::default_particle, Texture::Format::SRGBA);

  ParticleEmitter emitter;
  m_system = resources.create<ParticleSystem>(1000, Bound3(Vec3(-1, -1, -1), Vec3(1, 1, 1)), *m_spritesheet, 1, &emitter);

  m_instance = m_system->create(m_particlememory);

  m_transform = Transform::rotation(Vec3(0, 0, 1), pi<float>()/2);

  m_linecube = resources.load<Mesh>(CoreAsset::line_cube);

  setAcceptDrops(true);

  m_timerid = startTimer(16.67, Qt::PreciseTimer);
}


///////////////////////// ParticleView::Destructor //////////////////////////
ParticleView::~ParticleView()
{
  delete static_cast<char*>(m_particlememory.data);
}


///////////////////////// ParticleView::view ////////////////////////////////
void ParticleView::view(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &ParticleSystemDocument::document_changed, this, &ParticleView::refresh);
  connect(&m_document, &ParticleSystemDocument::dependant_changed, this, &ParticleView::refresh);

  refresh();
}


///////////////////////// ParticleView::invalidate //////////////////////////
void ParticleView::invalidate()
{
  update();
}


///////////////////////// ParticleView::refresh /////////////////////////////
void ParticleView::refresh()
{
  vector<ParticleEmitter> emitters(m_document.emitters());

  for(int i = 0; i < m_document.emitters(); ++i)
  {
    emitters[i] = make_emitter(m_document.emitter(i));
  }

  m_system = resources.create<ParticleSystem>(m_document.maxparticles(), m_document.bound(), *m_spritesheet, emitters.size(), emitters.data());

  if (m_system->maxparticles != m_document.maxparticles())
  {
    rewind(m_particlememory, 0);

    m_instance = m_system->create(m_particlememory);
  }

  if (m_document.spritesheet() != m_spritesheetdocument)
  {
    m_spritesheetdocument = m_document.spritesheet();

    if (m_spritesheetdocument)
    {
      auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

      buildmanager->request_build(m_spritesheetdocument, this, &ParticleView::on_spritesheet_build_complete);
    }
  }

  invalidate();
}


///////////////////////// ParticleView::spritesheet /////////////////////////
void ParticleView::on_spritesheet_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  PackImageHeader imag;

  if (read_asset_header(fin, 1, &imag))
  {
    m_spritesheet = resources.create<Texture>(imag.width, imag.height, imag.layers, imag.levels, Texture::Format::SRGBA);

    if (auto lump = resources.acquire_lump(imag.datasize))
    {
      read_asset_payload(fin, imag.dataoffset, lump->memory(), imag.datasize);

      resources.update<Texture>(m_spritesheet, lump);

      resources.release_lump(lump);
    }
  }

  refresh();
}


///////////////////////// ParticleView::set_skybox //////////////////////////
void ParticleView::set_skybox(QString const &path)
{
  if (m_skyboxdocument = SkyboxDocument(path))
  {
    auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

    buildmanager->request_build(m_skyboxdocument, this, &ParticleView::on_skybox_build_complete);

    connect(&m_skyboxdocument, &SkyboxDocument::document_changed, [=]() { set_skybox(path); });
    connect(&m_skyboxdocument, &SkyboxDocument::dependant_changed, [=]() { set_skybox(path); });
  }
}


///////////////////////// ParticleView::skybox_build_complete ///////////////
void ParticleView::on_skybox_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  m_skybox = resources.load<SkyBox>(fin, 1);

  renderparams.skybox = m_skybox;

  invalidate();
}


///////////////////////// ParticleView::set_exposure ////////////////////////
void ParticleView::set_exposure(float value)
{
  if (camera.exposure() != value)
  {
    camera.set_exposure(value);

    emit exposure_changed(camera.exposure());

    invalidate();
  }
}


///////////////////////// ParticleView::keyPressEvent ///////////////////////
void ParticleView::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Period && event->modifiers() == Qt::KeypadModifier)
  {
    m_focuspoint = Vec3(0, 0, 0);

    camera.lookat(m_focuspoint, Vec3(0, 1, 0));

    renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

    invalidate();
  }
}


///////////////////////// ParticleView::mousePressEvent /////////////////////
void ParticleView::mousePressEvent(QMouseEvent *event)
{
  if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::MidButton))
  {
    m_mousepresspos = m_mousemovepos = event->pos();

    m_yawsign = (camera.up().y > 0) ? -1.0f : 1.0f;

    event->accept();
  }
}


///////////////////////// ParticleView::mouseMoveEvent //////////////////////
void ParticleView::mouseMoveEvent(QMouseEvent *event)
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


///////////////////////// ParticleView::mouseReleaseEvent ///////////////////
void ParticleView::mouseReleaseEvent(QMouseEvent *event)
{
  m_mousepresspos = QPoint();
}


///////////////////////// ParticleView::wheelEvent //////////////////////////
void ParticleView::wheelEvent(QWheelEvent *event)
{
  camera.dolly(m_focuspoint, 0.01*event->angleDelta().y());

  invalidate();
}


///////////////////////// ParticleView::dragEnterEvent //////////////////////
void ParticleView::dragEnterEvent(QDragEnterEvent *event)
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


///////////////////////// ParticleView::dropEvent ///////////////////////////
void ParticleView::dropEvent(QDropEvent *event)
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


///////////////////////// ParticleView::timerEvent //////////////////////////
void ParticleView::timerEvent(QTimerEvent *event)
{
  if (event->timerId() == m_timerid)
  {
    if (isVisible())
    {
      m_system->update(m_instance, camera, m_transform, 1.0f/60.0f);

      update();
    }
  }

  Viewport::timerEvent(event);
}


///////////////////////// ParticleView::paintEvent //////////////////////////
void ParticleView::paintEvent(QPaintEvent *event)
{
  prepare();

  ForwardList objects;
  ForwardList::BuildState buildstate;

  if (begin(objects, buildstate))
  {
    objects.push_particlesystem(buildstate, m_system, m_instance);

    objects.finalise(buildstate);
  }

  push_objects(objects);

  if (m_showbound)
  {
    OverlayList overlay;
    OverlayList::BuildState buildstate;

    if (begin(overlay, buildstate))
    {
      overlay.push_volume(buildstate, m_transform * m_document.bound(), m_linecube, Color4(0.8f, 0.80f, 0.8f, 0.8f));

      overlay.finalise(buildstate);
    }

    push_overlays(overlay);
  }

  render();
}
