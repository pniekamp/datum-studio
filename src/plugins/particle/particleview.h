//
// Particle View
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "particlesystem.h"
#include "skybox.h"
#include "viewport.h"

//-------------------------- ParticleView -----------------------------------
//---------------------------------------------------------------------------

class ParticleView : public Viewport
{
  Q_OBJECT

  public:
    ParticleView(QWidget *parent = nullptr);
    ~ParticleView();

  public slots:

    void view(Studio::Document *document);

    void set_skybox(QString const &path);

    void set_exposure(float value);

  signals:

    void exposure_changed(float value);

  protected slots:

    void refresh();

  protected:

    void invalidate();

    void on_spritesheet_build_complete(Studio::Document *document, QString const &path);

    void on_skybox_build_complete(Studio::Document *document, QString const &path);

    void keyPressEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    void timerEvent(QTimerEvent *event);

    void paintEvent(QPaintEvent *event);

  private:

    SkyboxDocument m_skyboxdocument;

    unique_resource<SkyBox> m_skybox;

  private:

    int m_timerid;

    float m_yawsign;

    lml::Vec3 m_focuspoint;

    QPoint m_mousepresspos, m_mousemovepos;

    bool m_showbound;

    DatumPlatform::GameMemory m_particlememory;

    unique_resource<ParticleSystem> m_system;
    ParticleSystem::Instance *m_instance;

    Studio::Document *m_spritesheetdocument;

    unique_resource<Texture> m_spritesheet;

    unique_resource<Mesh> m_linecube;

    ParticleSystemDocument m_document;
};
