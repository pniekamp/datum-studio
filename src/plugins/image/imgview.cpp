//
// Image View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "imgview.h"
#include "assetfile.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- ImageView -----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// ImageView::Constructor ////////////////////////////
ImageView::ImageView(QWidget *parent)
  : QAbstractScrollArea(parent)
{
  m_layer = 0;
  m_scale = 1.0f;
  m_exposure = 1.0f;

  m_border = 4;

  setViewport(new Viewport);

  viewport()->renderparams.ssaoscale = 0;
  viewport()->renderparams.ssrstrength = 0;
  viewport()->renderparams.bloomstrength = 0;

  m_image = viewport()->resources.create<Texture>(1, 1, 1, 1, Texture::Format::SRGBA);
}


///////////////////////// ImageView::view ///////////////////////////////////
void ImageView::view(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &ImageDocument::document_changed, this, &ImageView::refresh);

  refresh();
}


///////////////////////// ImageView::invalidate /////////////////////////////
void ImageView::invalidate()
{
  int width = m_scale * m_width;
  int height = m_scale * m_height;

  horizontalScrollBar()->setRange(0, width + 2*m_border - viewport()->width());
  horizontalScrollBar()->setPageStep(viewport()->width());

  verticalScrollBar()->setRange(0, height + 2*m_border - viewport()->height());
  verticalScrollBar()->setPageStep(viewport()->height());

  viewport()->update();
}


///////////////////////// ImageView::refresh ////////////////////////////////
void ImageView::refresh()
{
  m_document->lock();

  PackImageHeader imag;

  if (read_asset_header(m_document, 1, &imag))
  {
    m_width = imag.width;
    m_height = imag.height;
    m_layers = imag.layers;

    switch(imag.format)
    {
      case PackImageHeader::rgba:
        m_image = viewport()->resources.create<Texture>(imag.width, imag.height, imag.layers, imag.levels, Texture::Format::SRGBA);
        break;

      case PackImageHeader::rgbe:
        m_image = viewport()->resources.create<Texture>(imag.width, imag.height, imag.layers, imag.levels, Texture::Format::RGBE);
        break;

      default:
        assert(false);
    }

    if (auto lump = viewport()->resources.acquire_lump(imag.datasize))
    { 
      read_asset_payload(m_document, imag.dataoffset, lump->memory(), imag.datasize);

      viewport()->resources.update<Texture>(m_image, lump);

      viewport()->resources.release_lump(lump);
    }
  }

  m_document->unlock();

  invalidate();
}


///////////////////////// ImageView::set_zoom ///////////////////////////////
void ImageView::set_zoom(float value)
{
  set_scale(0.1 * pow(1 + value, 2), QPoint(viewport()->width()/2, viewport()->height()/2));
}


///////////////////////// ImageView::set_scale //////////////////////////////
void ImageView::set_scale(float scale, QPoint const &focus)
{
  if (m_scale != scale)
  {
    auto locus = QPointF(horizontalScrollBar()->value() + focus.x(), verticalScrollBar()->value() + focus.y()) / m_scale;

    m_scale = scale;

    invalidate();

    horizontalScrollBar()->setValue(locus.x()*m_scale - focus.x());
    verticalScrollBar()->setValue(locus.y()*m_scale - focus.y());

    emit zoom_changed(sqrt(10.0 * m_scale) - 1);
  }
}


///////////////////////// ImageView::set_layer //////////////////////////////
void ImageView::set_layer(int value)
{
  if (m_layer != value)
  {
    m_layer = value;

    emit layer_changed(m_layer);

    invalidate();
  }
}


///////////////////////// ImageView::set_exposure ///////////////////////////
void ImageView::set_exposure(float value)
{
  if (m_exposure != value)
  {
    m_exposure = value;

    emit exposure_changed(m_exposure);

    invalidate();
  }
}


///////////////////////// ImageView::mousePressEvent ////////////////////////
void ImageView::mousePressEvent(QMouseEvent *event)
{
  if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::MidButton))
  {
    m_mousepresspos = m_mousemovepos = event->pos();

    event->accept();
  }
}


///////////////////////// ImageView::mouseMoveEvent /////////////////////////
void ImageView::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_mousepresspos.isNull())
  {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + m_mousemovepos.x() - event->pos().x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() + m_mousemovepos.y() - event->pos().y());
  }

  m_mousemovepos = event->pos();
}


///////////////////////// ImageView::mouseReleaseEvent //////////////////////
void ImageView::mouseReleaseEvent(QMouseEvent *event)
{
  m_mousepresspos = QPoint();
}


///////////////////////// ImageView::wheelEvent /////////////////////////////
void ImageView::wheelEvent(QWheelEvent *event)
{
  set_scale(m_scale * (1 + 0.001*event->angleDelta().y()), event->pos());
}


///////////////////////// ImageView::resizeEvent ////////////////////////////
void ImageView::resizeEvent(QResizeEvent *event)
{
  if (m_document)
  {
    invalidate();
  }
}


///////////////////////// ImageView::paintEvent /////////////////////////////
void ImageView::paintEvent(QPaintEvent *event)
{
  int x = -horizontalScrollBar()->value();
  int y = -verticalScrollBar()->value();

  viewport()->prepare();

  SpriteList sprites;
  SpriteList::BuildState buildstate;

  auto sprite = viewport()->resources.create<Sprite>(*m_image, Rect2(Vec2(0, 0), Vec2(1, 1)), Vec2(0,0));

  if (viewport()->begin(sprites, buildstate))
  {
    sprites.viewport(buildstate, Rect2(Vec2(0, 0), Vec2(width(), height())));

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

    sprites.push_sprite(buildstate, Vec2(x + m_border, y + m_border), m_scale*m_height, sprite, m_layer, Color4(m_exposure, m_exposure, m_exposure, 1));

    sprites.finalise(buildstate);
  }

  viewport()->push_sprites(sprites);

  viewport()->render();
}
