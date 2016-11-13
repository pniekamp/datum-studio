//
// SkyboxView
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "skybox.h"
#include "viewport.h"

//-------------------------- SkyboxView -------------------------------------
//---------------------------------------------------------------------------

class SkyboxView : public Viewport
{
  Q_OBJECT

  public:
    SkyboxView(QWidget *parent = nullptr);

  public slots:

    void view(Studio::Document *document);

    void set_layer(int value);
    void set_exposure(float value);

  signals:

    void layer_changed(int value);
    void exposure_changed(float value);

  protected slots:

    void refresh();

  protected:

    void invalidate();

    void on_skybox_build_complete(Studio::Document *document, QString const &path);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void paintEvent(QPaintEvent *event);

  private:

    QPoint m_mousepresspos, m_mousemovepos;

    unique_resource<SkyBox> m_skybox;

    SkyboxDocument m_document;
};
