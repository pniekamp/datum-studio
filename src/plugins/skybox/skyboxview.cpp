//
// SkyboxView
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "skyboxview.h"
#include "buildapi.h"
#include "assetfile.h"
#include <QMouseEvent>
#include <QWheelEvent>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- SkyboxView ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SkyboxView::Constructor ///////////////////////////
SkyboxView::SkyboxView(QWidget *parent)
  : Viewport(8*1024, 2*1024*1024, parent)
{
}


///////////////////////// SkyboxView::view //////////////////////////////////
void SkyboxView::view(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &SkyboxDocument::document_changed, this, &SkyboxView::refresh);
  connect(&m_document, &SkyboxDocument::dependant_changed, this, &SkyboxView::refresh);

  refresh();
}


///////////////////////// SkyboxView::invalidate ////////////////////////////
void SkyboxView::invalidate()
{
  update();
}


///////////////////////// SkyboxView::refresh ///////////////////////////////
void SkyboxView::refresh()
{
  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->request_build(m_document, this, &SkyboxView::on_skybox_build_complete);
}


///////////////////////// SkyboxView::skybox_build_complete /////////////////
void SkyboxView::on_skybox_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  m_skybox = resources.load<SkyBox>(fin, 1);

  renderparams.skybox = m_skybox;

  invalidate();
}


///////////////////////// SkyboxView::set_layer /////////////////////////////
void SkyboxView::set_layer(int value)
{
  if (renderparams.skyboxlod != value)
  {
    renderparams.skyboxlod = value;

    emit layer_changed(renderparams.skyboxlod);

    invalidate();
  }
}


///////////////////////// SkyboxView::set_exposure //////////////////////////
void SkyboxView::set_exposure(float value)
{
  if (camera.exposure() != value)
  {
    camera.set_exposure(value);

    emit exposure_changed(camera.exposure());

    invalidate();
  }
}


///////////////////////// SkyboxView::mousePressEvent ///////////////////////
void SkyboxView::mousePressEvent(QMouseEvent *event)
{
  if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::MidButton))
  {
    m_mousepresspos = m_mousemovepos = event->pos();

    event->accept();
  }
}


///////////////////////// SkyboxView::mouseMoveEvent ////////////////////////
void SkyboxView::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_mousepresspos.isNull())
  {
    camera.yaw(0.005*(m_mousemovepos.x() - event->pos().x()), Vec3(0, 1, 0));
    camera.pitch(0.005*(m_mousemovepos.y() - event->pos().y()));

    invalidate();
  }

  m_mousemovepos = event->pos();
}


///////////////////////// SkyboxView::mouseReleaseEvent /////////////////////
void SkyboxView::mouseReleaseEvent(QMouseEvent *event)
{
  m_mousepresspos = QPoint();
}


///////////////////////// SkyboxView::paintEvent ////////////////////////////
void SkyboxView::paintEvent(QPaintEvent *event)
{
  prepare();

  render();
}
