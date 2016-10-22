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
  renderparams.ssaoscale = 0;
  renderparams.ssrstrength = 0;
  renderparams.bloomstrength = 0;  
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

  buildmanager->request_build(m_document, this, &SkyboxView::on_build_complete);
}


///////////////////////// SkyboxView::build_complete ////////////////////////
void SkyboxView::on_build_complete(Studio::Document *document, QString const &path)
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
