//
// FontView
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "font.h"
#include <leap/concurrentqueue.h>
#include <QAbstractScrollArea>

//-------------------------- FontView ---------------------------------------
//---------------------------------------------------------------------------

class FontView : public QAbstractScrollArea
{
  Q_OBJECT

  public:
    FontView(QWidget *parent = nullptr);

  public slots:

    void view(Studio::Document *document);

    void set_zoom(float value);

  signals:

    void zoom_changed(float value);

  protected slots:

    void refresh();

    void on_build_complete(Studio::Document *document, QString const &path);

    void set_scale(float scale, QPoint const &focus);

  protected:

    void invalidate();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void resizeEvent(QResizeEvent *event);

    void paintEvent(QPaintEvent *event);

  private:

    QPoint m_mousepresspos, m_mousemovepos;

    float m_scale;

    int m_border;

    QImage m_image;

    FontDocument m_document;
};
