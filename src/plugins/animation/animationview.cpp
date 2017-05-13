//
// Animation View
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "animationview.h"
#include "assetfile.h"
#include "buildapi.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>

#include <QDebug>

using namespace std;
using namespace lml;
using leap::indexof;

//|---------------------- AnimationView -------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// AnimationView::Constructor ////////////////////////
AnimationView::AnimationView(QWidget *parent)
  : Viewport(8*1024, 2*1024*1024, parent)
{
  m_focuspoint = Vec3(0, 0, 0);

  camera.lookat(Vec3(0, 1, 10), m_focuspoint, Vec3(0, 1, 0));

  m_time = 0.0f;
  m_duration = 0.0f;

  renderparams.ssaoscale = 0;
  renderparams.ssrstrength = 0;
  renderparams.bloomstrength = 0;

  m_timerid = startTimer(16.67, Qt::PreciseTimer);
}


///////////////////////// AnimationView::view ///////////////////////////////
void AnimationView::view(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &AnimationDocument::document_changed, this, &AnimationView::refresh);

  refresh();
}


///////////////////////// AnimationView::invalidate /////////////////////////
void AnimationView::invalidate()
{
  update();
}


///////////////////////// AnimationView::refresh ////////////////////////////
void AnimationView::refresh()
{
  m_joints = m_document.joints();

  m_duration = m_document.duration();

  m_bones.resize(m_joints.size(), Transform::identity());

  invalidate();
}


///////////////////////// AnimationView::keyPressEvent //////////////////////
void AnimationView::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Period && event->modifiers() == Qt::KeypadModifier)
  {
    m_focuspoint = Vec3(0, 0, 0);

    camera.lookat(m_focuspoint, Vec3(0, 1, 0));

    invalidate();
  }
}


///////////////////////// AnimationView::mousePressEvent ////////////////////
void AnimationView::mousePressEvent(QMouseEvent *event)
{
  if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::MidButton))
  {
    m_mousepresspos = m_mousemovepos = event->pos();

    m_yawsign = (camera.up().y > 0) ? -1.0f : 1.0f;

    event->accept();
  }
}


///////////////////////// AnimationView::mouseMoveEvent /////////////////////
void AnimationView::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_mousepresspos.isNull())
  {
    auto dx = event->pos().x() - m_mousemovepos.x();
    auto dy = m_mousemovepos.y() - event->pos().y();

    if (event->modifiers() == Qt::NoModifier)
    {
      camera.orbit(m_focuspoint, Transform::rotation(camera.right(), 0.01f * dy).rotation());
      camera.orbit(m_focuspoint, Transform::rotation(Vec3(0, 1, 0), m_yawsign * 0.01f * dx).rotation());
    }

    if (event->modifiers() == Qt::ShiftModifier)
    {
      camera.pan(m_focuspoint, -0.05f * dx, -0.05f * dy);
    }

    renderparams.sundirection = normalise(camera.forward() - camera.right() - camera.up());

    invalidate();
  }

  m_mousemovepos = event->pos();
}


///////////////////////// AnimationView::mouseReleaseEvent //////////////////
void AnimationView::mouseReleaseEvent(QMouseEvent *event)
{
  m_mousepresspos = QPoint();
}


///////////////////////// AnimationView::wheelEvent /////////////////////////
void AnimationView::wheelEvent(QWheelEvent *event)
{
  camera.dolly(m_focuspoint, 0.01*event->angleDelta().y());

  invalidate();
}


///////////////////////// AnimationView::timerEvent /////////////////////////
void AnimationView::timerEvent(QTimerEvent *event)
{
  if (event->timerId() == m_timerid)
  {
    if (isVisible())
    {
      m_time += 1.0f/60.0f;

      float time = fmod(m_time, m_duration);

      for(size_t i = 1; i < m_joints.size(); ++i)
      {
        auto &joint = m_joints[i];

        size_t index = 0;

        while (index+2 < joint.transforms.size() && joint.transforms[index+1].time < time)
          ++index;

        auto alpha = remap(time, joint.transforms[index].time, joint.transforms[index+1].time, 0.0f, 1.0f);

        auto transform = lerp(joint.transforms[index].transform, joint.transforms[index+1].transform, alpha);

        m_bones[i] = m_bones[joint.parent] * transform;
      }

      update();
    }
  }

  Viewport::timerEvent(event);
}


///////////////////////// AnimationView::paintEvent /////////////////////////
void AnimationView::paintEvent(QPaintEvent *event)
{
  prepare();

  OverlayList overlays;
  OverlayList::BuildState buildstate;

  if (begin(overlays, buildstate))
  {
    for(size_t i = 1; i < m_joints.size(); ++i)
    {
      auto a = m_bones[m_joints[i].parent] * Vec3(0, 0, 0);
      auto b = m_bones[i] * Vec3(0, 0, 0);

      overlays.push_line(buildstate, a, b, Color4(1, 1, 1, 1));
    }

    overlays.finalise(buildstate);
  }

  push_overlays(overlays);

  render();
}
