//
// Animation View
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "animation.h"
#include "viewport.h"

//-------------------------- AnimationView ----------------------------------
//---------------------------------------------------------------------------

class AnimationView : public Viewport
{
  Q_OBJECT

  public:
    AnimationView(QWidget *parent = nullptr);

  public slots:

    void view(Studio::Document *document);

  protected slots:

    void refresh();

  protected:

    void invalidate();

    void keyPressEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    void timerEvent(QTimerEvent *event);

    void paintEvent(QPaintEvent *event);

  private:

    int m_timerid;

    float m_yawsign;

    lml::Vec3 m_focuspoint;

    QPoint m_mousepresspos, m_mousemovepos;

    float m_time;
    float m_duration;
    std::vector<lml::Transform> m_bones;

    std::vector<AnimationDocument::Joint> m_joints;

    AnimationDocument m_document;
};
