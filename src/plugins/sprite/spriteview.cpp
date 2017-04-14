//
// Sprite View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "spriteview.h"
#include "buildapi.h"
#include "assetfile.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- SpriteView ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// SpriteView::Constructor ///////////////////////////
SpriteView::SpriteView(QWidget *parent)
  : QAbstractScrollArea(parent)
{
  m_layer = 0;
  m_scale = 1.0f;

  m_border = 4;

  setViewport(new Viewport);

  viewport()->renderparams.ssaoscale = 0;
  viewport()->renderparams.ssrstrength = 0;
  viewport()->renderparams.bloomstrength = 0;

  m_image = viewport()->resources.create<Texture>(1, 1, 1, 1, Texture::Format::SRGBA);
}


///////////////////////// SpriteView::view //////////////////////////////////
void SpriteView::view(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &SpriteSheetDocument::document_changed, this, &SpriteView::refresh);
  connect(&m_document, &SpriteSheetDocument::dependant_changed, this, &SpriteView::refresh);

  refresh();
}


///////////////////////// SpriteView::invalidate ////////////////////////////
void SpriteView::invalidate()
{
  int width = m_scale * m_width;
  int height = m_scale * m_height;

  horizontalScrollBar()->setRange(0, width + 2*m_border - viewport()->width());
  horizontalScrollBar()->setPageStep(viewport()->width());

  verticalScrollBar()->setRange(0, height + 2*m_border - viewport()->height());
  verticalScrollBar()->setPageStep(viewport()->height());

  viewport()->update();
}


///////////////////////// SpriteView::refresh ///////////////////////////////
void SpriteView::refresh()
{ 
  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->request_build(m_document, this, &SpriteView::on_sprite_build_complete);
}


///////////////////////// SpriteView::sprite_build_complete /////////////////
void SpriteView::on_sprite_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  PackImageHeader imag;

  if (read_asset_header(fin, 1, &imag))
  {
    m_width = imag.width;
    m_height = imag.height;
    m_layers = imag.layers;

    m_image = viewport()->resources.create<Texture>(imag.width, imag.height, imag.layers, imag.levels, Texture::Format::SRGBA);

    if (auto lump = viewport()->resources.acquire_lump(imag.datasize))
    {
      read_asset_payload(fin, imag.dataoffset, lump->memory(), imag.datasize);

      viewport()->resources.update<Texture>(m_image, lump);

      viewport()->resources.release_lump(lump);
    }

    emit layers_changed(m_layers);
  }

  invalidate();
}


///////////////////////// SpriteView::set_zoom //////////////////////////////
void SpriteView::set_zoom(float value)
{
  set_scale(0.1 * pow(1 + value, 2), QPoint(viewport()->width()/2, viewport()->height()/2));
}


///////////////////////// SpriteView::set_scale /////////////////////////////
void SpriteView::set_scale(float scale, QPoint const &focus)
{
  if (m_scale != scale)
  {
    auto locus = QPointF(horizontalScrollBar()->value() + focus.x(), verticalScrollBar()->value() + focus.y()) / m_scale;

    m_scale = scale;

    horizontalScrollBar()->setValue(locus.x()*m_scale - focus.x());
    verticalScrollBar()->setValue(locus.y()*m_scale - focus.y());

    emit zoom_changed(sqrt(10.0 * m_scale) - 1);

    invalidate();
  }
}


///////////////////////// SpriteView::set_layer /////////////////////////////
void SpriteView::set_layer(int value)
{
  if (m_layer != value)
  {
    m_layer = value;

    emit layer_changed(m_layer);

    invalidate();
  }
}


///////////////////////// SpriteView::mousePressEvent ///////////////////////
void SpriteView::mousePressEvent(QMouseEvent *event)
{
  if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::MidButton))
  {
    m_mousepresspos = m_mousemovepos = event->pos();

    event->accept();
  }
}


///////////////////////// SpriteView::mouseMoveEvent ////////////////////////
void SpriteView::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_mousepresspos.isNull())
  {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + m_mousemovepos.x() - event->pos().x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() + m_mousemovepos.y() - event->pos().y());
  }

  m_mousemovepos = event->pos();
}


///////////////////////// SpriteView::mouseReleaseEvent /////////////////////
void SpriteView::mouseReleaseEvent(QMouseEvent *event)
{
  m_mousepresspos = QPoint();
}


///////////////////////// SpriteView::wheelEvent ////////////////////////////
void SpriteView::wheelEvent(QWheelEvent *event)
{
  set_scale(m_scale * (1 + 0.001*event->angleDelta().y()), event->pos());
}


///////////////////////// SpriteView::resizeEvent ///////////////////////////
void SpriteView::resizeEvent(QResizeEvent *event)
{
  if (m_document)
  {
    invalidate();
  }
}


///////////////////////// SpriteView::paintEvent ////////////////////////////
void SpriteView::paintEvent(QPaintEvent *event)
{
  int x = -horizontalScrollBar()->value();
  int y = -verticalScrollBar()->value();

  viewport()->prepare();

  SpriteList sprites;
  SpriteList::BuildState buildstate;

  auto sprite = viewport()->resources.create<Sprite>(*m_image, Rect2(Vec2(0, 0), Vec2(1, 1)), Vec2(0,0));

  if (viewport()->begin(sprites, buildstate))
  {
    for(int j = y % 32; j < height(); j += 32)
    {
      for(int i = x % 32; i < width(); i += 32)
      {
        sprites.push_rect(buildstate, Vec2(i, j), Rect2(Vec2(0, 0), Vec2(16, 16)), Color4(0.6f, 0.6f, 0.6f));
        sprites.push_rect(buildstate, Vec2(i+16, j), Rect2(Vec2(0, 0), Vec2(16, 16)), Color4(0.4f, 0.4f, 0.4f));
        sprites.push_rect(buildstate, Vec2(i, j+16), Rect2(Vec2(0, 0), Vec2(16, 16)), Color4(0.4f, 0.4f, 0.4f));
        sprites.push_rect(buildstate, Vec2(i+16, j+16), Rect2(Vec2(0, 0), Vec2(16, 16)), Color4(0.6f, 0.6f, 0.6f));
      }
    }

    sprites.push_sprite(buildstate, Vec2(x + m_border, y + m_border), m_scale*m_height, sprite, m_layer);

    sprites.finalise(buildstate);
  }

  viewport()->push_sprites(sprites);

  viewport()->render();
}
