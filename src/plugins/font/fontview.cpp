//
// FontView
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "fontview.h"
#include "buildapi.h"
#include "assetfile.h"
#include <QApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QPainter>

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- FontView ------------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// FontView::Constructor /////////////////////////////
FontView::FontView(QWidget *parent)
  : QAbstractScrollArea(parent)
{
  m_scale = 1.0f;

  m_border = 4;
}


///////////////////////// FontView::view ////////////////////////////////////
void FontView::view(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &FontDocument::document_changed, this, &FontView::refresh);

  refresh();
}


///////////////////////// FontView::invalidate //////////////////////////////
void FontView::invalidate()
{
  int width = m_scale * m_image.width();
  int height = m_scale * m_image.height();

  horizontalScrollBar()->setRange(0, width + 2*m_border - viewport()->width());
  horizontalScrollBar()->setPageStep(viewport()->width());

  verticalScrollBar()->setRange(0, height + 2*m_border - viewport()->height());
  verticalScrollBar()->setPageStep(viewport()->height());

  viewport()->update();
}


///////////////////////// FontView::refresh /////////////////////////////////
void FontView::refresh()
{
  auto buildmanager = Studio::Core::instance()->find_object<Studio::BuildManager>();

  buildmanager->request_build(m_document, this, &FontView::on_font_build_complete);
}


///////////////////////// FontView::font_build_complete /////////////////////
void FontView::on_font_build_complete(Studio::Document *document, QString const &path)
{
  ifstream fin(path.toUtf8(), ios::binary);

  m_image = read_asset_image(fin, 2, 0);

  invalidate();
}


///////////////////////// FontView::set_zoom ////////////////////////////////
void FontView::set_zoom(float value)
{
  set_scale(0.1 * pow(1 + value, 2), QPoint(viewport()->width()/2, viewport()->height()/2));
}


///////////////////////// FontView::set_scale ///////////////////////////////
void FontView::set_scale(float scale, QPoint const &focus)
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


///////////////////////// FontView::mousePressEvent /////////////////////////
void FontView::mousePressEvent(QMouseEvent *event)
{
  if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::MidButton))
  {
    m_mousepresspos = m_mousemovepos = event->pos();

    event->accept();
  }
}


///////////////////////// FontView::mouseMoveEvent //////////////////////////
void FontView::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_mousepresspos.isNull())
  {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + m_mousemovepos.x() - event->pos().x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() + m_mousemovepos.y() - event->pos().y());
  }

  m_mousemovepos = event->pos();
}


///////////////////////// FontView::mouseReleaseEvent ///////////////////////
void FontView::mouseReleaseEvent(QMouseEvent *event)
{
  m_mousepresspos = QPoint();
}


///////////////////////// FontView::wheelEvent //////////////////////////////
void FontView::wheelEvent(QWheelEvent *event)
{
  set_scale(m_scale * (1 + 0.001*event->angleDelta().y()), event->pos());
}


///////////////////////// FontView::resizeEvent ////////////////////////////
void FontView::resizeEvent(QResizeEvent *event)
{
  if (m_document)
  {
    invalidate();
  }
}


///////////////////////// FontView::paintEvent //////////////////////////////
void FontView::paintEvent(QPaintEvent *event)
{
  QPainter painter(viewport());

  int x = -horizontalScrollBar()->value();
  int y = -verticalScrollBar()->value();

  painter.fillRect(QRect(x + m_border, y + m_border, m_scale * m_image.width(), m_scale * m_image.height()), Qt::black);

  painter.drawImage(QRect(x + m_border, y + m_border, m_scale * m_image.width(), m_scale * m_image.height()), m_image);
}
