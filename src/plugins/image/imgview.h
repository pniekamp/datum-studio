//
// Image View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "viewport.h"
#include "documentapi.h"
#include <QAbstractScrollArea>

//-------------------------- ImageView --------------------------------------
//---------------------------------------------------------------------------

class ImageView : public QAbstractScrollArea
{
  Q_OBJECT

  public:
    ImageView(QWidget *parent = nullptr);

    Viewport *viewport() { return static_cast<Viewport*>(QAbstractScrollArea::viewport()); }

  public slots:

    void view(Studio::Document *document);

    void set_zoom(float value);
    void set_layer(int value);
    void set_exposure(float value);

  signals:

    void zoom_changed(float value);
    void layer_changed(int value);
    void exposure_changed(float value);

  protected slots:

    void refresh();

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

    int m_layer;
    float m_scale;
    float m_exposure;

    int m_width;
    int m_height;
    int m_layers;
    int m_border;

    unique_resource<Texture> m_image;

    Studio::Document *m_document;
};
