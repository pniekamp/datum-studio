//
// Curve Editor
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "curveeditor.h"
#include "leap/lml/interpolation.h"
#include "leap/lml/geometry.h"
#include "qpointtraits.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

using namespace std;
using namespace lml;
using namespace leap;

namespace
{
  /////////////////////// splice //////////////////////////
  vector<float> splice(vector<float> const &va, vector<float> const &vb, float epsilon)
  {
    vector<float> result;

    auto i = va.begin();
    auto j = vb.begin();

    while (i != va.end() && j != vb.end())
    {
      result.push_back((*i < *j) ? *i : *j);

      while (i != va.end() && fabs(*i - result.back()) < epsilon)
        ++i;

      while (j != vb.end() && fabs(*j - result.back()) < epsilon)
        ++j;
    }

    while (i != va.end())
      result.push_back(*i++);

    while (j != vb.end())
      result.push_back(*j++);

    return result;
  }
}


//|---------------------- CurveEditor ---------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// CurveEditor::Constructor //////////////////////////
CurveEditor::CurveEditor(QWidget *parent)
  : QDialog(parent)
{
  m_component = nullptr;

  m_selectedcurve = -1;
  m_selectedindex = -1;

  ui.setupUi(this);
}


///////////////////////// CurveEditor::timebase /////////////////////////////
vector<float> CurveEditor::timebase() const
{
  vector<float> times;

  for(auto &component : m_components)
  {
    for(auto &curve : component.curves)
    {
      times = splice(times, curve.xa, 0.005);
    }
  }

  return times;
}


///////////////////////// CurveEditor::distribution /////////////////////////
template<>
ParticleSystemDocument::Distribution<float> CurveEditor::distribution<float>()
{
  ParticleSystemDocument::Distribution<float> distribution;

  distribution.type = ui.DistributionType->currentIndex();

  switch(ui.DistributionType->currentIndex())
  {
    case Constant:
      distribution.ya.emplace_back(m_components[0].curves[0].ya[0]);
      break;

    case Uniform:
      distribution.ya.emplace_back(min(m_components[0].curves[0].ya[0], m_components[0].curves[1].ya[0]));
      distribution.ya.emplace_back(max(m_components[0].curves[0].ya[0], m_components[0].curves[1].ya[0]));
      break;

    case Curve:
      distribution.xa = m_component[0].curves[0].xa;
      distribution.ya = m_component[0].curves[0].ya;
      break;

    case UniformCurve:
      distribution.xa = timebase();
      distribution.ya.resize(2 * distribution.xa.size());
      transform(distribution.xa.begin(), distribution.xa.end(), distribution.ya.begin(), [&](auto &u) { return m_components[0].curves[0].value(u); });
      transform(distribution.xa.begin(), distribution.xa.end(), distribution.ya.begin() + distribution.xa.size(), [&](auto &u) { return m_components[0].curves[1].value(u); });
      break;
  }

  return distribution;
}

template<>
ParticleSystemDocument::Distribution<Vec3> CurveEditor::distribution<Vec3>()
{
  ParticleSystemDocument::Distribution<Vec3> distribution;

  distribution.type = ui.DistributionType->currentIndex();

  switch(ui.DistributionType->currentIndex())
  {
    case Constant:
      distribution.ya.emplace_back(m_components[0].curves[0].ya[0], m_components[1].curves[0].ya[0], m_components[2].curves[0].ya[0]);
      break;

    case Uniform:
      distribution.ya.emplace_back(min(m_components[0].curves[0].ya[0], m_components[0].curves[1].ya[0]), min(m_components[1].curves[0].ya[0], m_components[1].curves[1].ya[0]), min(m_components[2].curves[0].ya[0], m_components[2].curves[1].ya[0]));
      distribution.ya.emplace_back(max(m_components[0].curves[0].ya[0], m_components[0].curves[1].ya[0]), max(m_components[1].curves[0].ya[0], m_components[1].curves[1].ya[0]), max(m_components[2].curves[0].ya[0], m_components[2].curves[1].ya[0]));
      break;

    case Curve:
      distribution.xa = timebase();
      distribution.ya.resize(distribution.xa.size());
      transform(distribution.xa.begin(), distribution.xa.end(), distribution.ya.begin(), [&](auto &u) { return Vec3(m_components[0].curves[0].value(u), m_components[1].curves[0].value(u), m_components[2].curves[0].value(u)); });
      break;

    case UniformCurve:
      distribution.xa = timebase();
      distribution.ya.resize(2 * distribution.xa.size());
      transform(distribution.xa.begin(), distribution.xa.end(), distribution.ya.begin(), [&](auto &u) { return Vec3(m_components[0].curves[0].value(u), m_components[1].curves[0].value(u), m_components[2].curves[0].value(u)); });
      transform(distribution.xa.begin(), distribution.xa.end(), distribution.ya.begin() + distribution.xa.size(), [&](auto &u) { return Vec3(m_components[0].curves[1].value(u), m_components[1].curves[1].value(u), m_components[2].curves[1].value(u)); });
      break;
  }

  return distribution;
}

template<>
ParticleSystemDocument::Distribution<Color4> CurveEditor::distribution<Color4>()
{
  ParticleSystemDocument::Distribution<Color4> distribution;

  distribution.type = ui.DistributionType->currentIndex();

  switch(ui.DistributionType->currentIndex())
  {
    case Constant:
      distribution.ya.emplace_back(m_components[0].curves[0].ya[0], m_components[1].curves[0].ya[0], m_components[2].curves[0].ya[0], m_components[3].curves[0].ya[0]);
      break;

    case Uniform:
      distribution.ya.emplace_back(min(m_components[0].curves[0].ya[0], m_components[0].curves[1].ya[0]), min(m_components[1].curves[0].ya[0], m_components[1].curves[1].ya[0]), min(m_components[2].curves[0].ya[0], m_components[2].curves[1].ya[0]), min(m_components[3].curves[0].ya[0], m_components[3].curves[1].ya[0]));
      distribution.ya.emplace_back(max(m_components[0].curves[0].ya[0], m_components[0].curves[1].ya[0]), max(m_components[1].curves[0].ya[0], m_components[1].curves[1].ya[0]), max(m_components[2].curves[0].ya[0], m_components[2].curves[1].ya[0]), max(m_components[3].curves[0].ya[0], m_components[3].curves[1].ya[0]));
      break;

    case Curve:
      distribution.xa = timebase();
      distribution.ya.resize(distribution.xa.size());
      transform(distribution.xa.begin(), distribution.xa.end(), distribution.ya.begin(), [&](auto &u) { return Color4(m_components[0].curves[0].value(u), m_components[1].curves[0].value(u), m_components[2].curves[0].value(u), m_components[3].curves[0].value(u)); });
      break;

    case UniformCurve:
      distribution.xa = timebase();
      distribution.ya.resize(2 * distribution.xa.size());
      transform(distribution.xa.begin(), distribution.xa.end(), distribution.ya.begin(), [&](auto &u) { return Color4(m_components[0].curves[0].value(u), m_components[1].curves[0].value(u), m_components[2].curves[0].value(u), m_components[3].curves[0].value(u)); });
      transform(distribution.xa.begin(), distribution.xa.end(), distribution.ya.begin() + distribution.xa.size(), [&](auto &u) { return Color4(m_components[0].curves[1].value(u), m_components[1].curves[1].value(u), m_components[2].curves[1].value(u), m_components[3].curves[1].value(u)); });
      break;
  }

  return distribution;
}


///////////////////////// CurveEditor::set_distribution /////////////////////
template<>
void CurveEditor::set_distribution<float>(ParticleSystemDocument::Distribution<float> const &distribution, float const &minvalue, float const &maxvalue)
{
  add_component("", Qt::darkGray, minvalue, maxvalue);

  ui.DistributionType->setCurrentIndex(distribution.type);

  switch(distribution.type)
  {
    case Constant:
      m_components[0].add_curve({ 0.0f, 1.0f }, { distribution.ya[0], distribution.ya[0] });
      break;

    case Uniform:
      m_components[0].add_curve({ 0.0f, 1.0f }, { distribution.ya[0], distribution.ya[0] });
      m_components[0].add_curve({ 0.0f, 1.0f }, { distribution.ya[1], distribution.ya[1] });
      break;

    case Curve:
    case UniformCurve:
      for(auto j = distribution.ya.begin(); j != distribution.ya.end(); j += distribution.xa.size())
      {
        vector<float> curve(distribution.xa.size());

        copy(j, j + distribution.xa.size(), curve.begin());

        m_components[0].add_curve(distribution.xa, curve);
      }
      break;
  }

  ui.ComponentList->hide();
  ui.ComponentList->setCurrentRow(0);
}

template<>
void CurveEditor::set_distribution<Vec3>(ParticleSystemDocument::Distribution<Vec3> const &distribution, Vec3 const &minvalue, Vec3 const &maxvalue)
{
  add_component("x", Qt::red, minvalue.x, maxvalue.x);
  add_component("y", Qt::green, minvalue.y, maxvalue.y);
  add_component("z", Qt::blue, minvalue.z, maxvalue.z);

  ui.DistributionType->setCurrentIndex(distribution.type);

  for(size_t k = 0; k < m_components.size(); ++k)
  {
    switch(distribution.type)
    {
      case Constant:
        m_components[k].add_curve({ 0.0f, 1.0f }, { distribution.ya[0][k], distribution.ya[0][k] });
        break;

      case Uniform:
        m_components[k].add_curve({ 0.0f, 1.0f }, { distribution.ya[0][k], distribution.ya[0][k] });
        m_components[k].add_curve({ 0.0f, 1.0f }, { distribution.ya[1][k], distribution.ya[1][k] });
        break;

      case Curve:
      case UniformCurve:
        for(auto j = distribution.ya.begin(); j != distribution.ya.end(); j += distribution.xa.size())
        {
          vector<float> curve(distribution.xa.size());

          transform(j, j + distribution.xa.size(), curve.begin(), [&](auto &v) { return v[k]; });

          m_components[k].add_curve(distribution.xa, curve);
        }
        break;
    }
  }

  ui.ComponentList->show();
  ui.ComponentList->setCurrentRow(0);
}

template<>
void CurveEditor::set_distribution<Color4>(ParticleSystemDocument::Distribution<Color4> const &distribution, Color4 const &minvalue, Color4 const &maxvalue)
{
  add_component("r", Qt::red, minvalue.r, maxvalue.r);
  add_component("g", Qt::green, minvalue.g, maxvalue.g);
  add_component("b", Qt::blue, minvalue.b, maxvalue.b);
  add_component("a", Qt::darkYellow, minvalue.a, maxvalue.a);

  ui.DistributionType->setCurrentIndex(distribution.type);

  for(size_t k = 0; k < m_components.size(); ++k)
  {
    switch(distribution.type)
    {
      case Constant:
        m_components[k].add_curve({ 0.0f, 1.0f }, { distribution.ya[0][k], distribution.ya[0][k] });
        break;

      case Uniform:
        m_components[k].add_curve({ 0.0f, 1.0f }, { distribution.ya[0][k], distribution.ya[0][k] });
        m_components[k].add_curve({ 0.0f, 1.0f }, { distribution.ya[1][k], distribution.ya[1][k] });
        break;

      case Curve:
      case UniformCurve:
        for(auto j = distribution.ya.begin(); j != distribution.ya.end(); j += distribution.xa.size())
        {
          vector<float> curve(distribution.xa.size());

          transform(j, j + distribution.xa.size(), curve.begin(), [&](auto &v) { return v[k]; });

          m_components[k].add_curve(distribution.xa, curve);
        }
        break;
    }
  }

  ui.ComponentList->show();
  ui.ComponentList->setCurrentRow(3);
}


///////////////////////// CurveEditor::add_component ////////////////////////
void CurveEditor::add_component(QString name, QColor const &color, float minvalue, float maxvalue)
{
  Component component;

  component.name = name;
  component.color = color;
  component.minvalue = minvalue;
  component.maxvalue = maxvalue;
  component.scalemax = 1.0f;

  m_components.push_back(component);

  auto item = new QListWidgetItem(name);

  item->setTextAlignment(Qt::AlignCenter);

  ui.ComponentList->addItem(item);
}

///////////////////////// CurveEditor::value ////////////////////////////////
float CurveEditor::Component::Curve::value(float u) const
{
  return interpolate<cubic>(xa, ya, u);
}


///////////////////////// CurveEditor::add_curve ////////////////////////////
void CurveEditor::Component::add_curve(vector<float> const &xa, vector<float> const &ya)
{
  Curve curve;

  curve.xa = xa;
  curve.ya = ya;

  scalemax = max(scalemax, *max_element(ya.begin(), ya.end()));

  curves.push_back(curve);
}


///////////////////////// CurveEditor::DistributionType /////////////////////
void CurveEditor::on_DistributionType_activated(int index)
{
  for(auto &component : m_components)
  {
    vector<Component::Curve> curves;

    swap(component.curves, curves);

    switch(index)
    {
      case Type::Constant:
        component.add_curve({ 0.0f, 1.0f }, { curves.front().ya[0], curves.front().ya[0] });
        break;

      case Type::Uniform:
        component.add_curve({ 0.0f, 1.0f }, { curves.front().ya[0], curves.front().ya[0] });
        component.add_curve({ 0.0f, 1.0f }, { curves.back().ya[0], curves.back().ya[0] });
        break;

      case Type::Curve:
        component.add_curve(curves.front().xa, curves.front().ya);
        break;

      case Type::UniformCurve:
        component.add_curve(curves.front().xa, curves.front().ya);
        component.add_curve(curves.back().xa, curves.back().ya);
        break;
    }
  }

  emit distribution_changed();

  m_selectedcurve = -1;
  m_selectedindex = -1;

  update();
}


///////////////////////// CurveEditor::ComponentList ////////////////////////
void CurveEditor::on_ComponentList_currentRowChanged(int index)
{
  m_component = &m_components[index];

  ui.ScaleMax->setValue(m_component->scalemax);

  m_selectedcurve = -1;
  m_selectedindex = -1;

  update();
}


///////////////////////// CurveEditor::ScaleMax /////////////////////////////
void CurveEditor::on_ScaleMax_valueChanged(double value)
{
  m_component->scalemax = value;

  update();
}


///////////////////////// CurveEditor::ScaleMax /////////////////////////////
void CurveEditor::on_ScaleMax_editingFinished()
{
  for(auto &curve : m_component->curves)
  {
    for(auto &y : curve.ya)
    {
      y = clamp(y, -m_component->scalemax, m_component->scalemax);
    }
  }

  emit distribution_changed();

  update();
}


///////////////////////// CurveEditor::update ///////////////////////////////
void CurveEditor::update()
{
  QRectF graph = ui.Graph->geometry();

  if (m_component)
  {
    float minx = 0.0;
    float maxx = 1.0;

    float miny = max(-m_component->scalemax, m_component->minvalue);
    float maxy = min(m_component->scalemax, m_component->maxvalue);

    float scalex = graph.width();
    float scaley = graph.height() / (2 * m_component->scalemax);

    for(auto &curve : m_component->curves)
    {
      curve.points.clear();
      curve.polyline.clear();

      for(size_t i = 0; i < curve.xa.size(); ++i)
      {
        auto x = graph.left() + clamp(curve.xa[i], minx, maxx) * scalex;
        auto y = graph.center().y() - clamp(curve.ya[i], miny, maxy) * scaley;

        curve.points.push_back(QPointF(x, y));
      }

      if (ui.DistributionType->currentIndex() == Constant || ui.DistributionType->currentIndex() == Uniform)
      {
        auto y = graph.center().y() - clamp(curve.ya[0], miny, maxy) * scaley;

        curve.polyline.push_back(QPointF(graph.left(), y));
        curve.polyline.push_back(QPointF(graph.right(), y));
      }

      if (ui.DistributionType->currentIndex() == Curve || ui.DistributionType->currentIndex() == UniformCurve)
      {
        for(float u = 0.0f; u < 1.0f + 1e-6f; u += 0.005f)
        {
          auto x = graph.left() + clamp(u, minx, maxx) * scalex;
          auto y = graph.center().y() - clamp(curve.value(u), miny, maxy) * scaley;

          curve.polyline.push_back(QPointF(x, y));
        }
      }
    }
  }

  QDialog::update();
}


///////////////////////// CurveEditor::keyPressEvent ////////////////////////
void CurveEditor::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Delete)
  {
    if (m_selectedcurve != -1 && m_selectedindex != -1)
    {
      auto &selectedcurve = m_component->curves[m_selectedcurve];

      if (selectedcurve.xa.size() > 2)
      {
        selectedcurve.xa.erase(selectedcurve.xa.begin() + m_selectedindex);
        selectedcurve.ya.erase(selectedcurve.ya.begin() + m_selectedindex);

        emit distribution_changed();

        m_selectedindex = -1;

        update();
      }
    }
  }
}


///////////////////////// CurveEditor::mousePressEvent //////////////////////
void CurveEditor::mousePressEvent(QMouseEvent *event)
{
  QRectF graph = ui.Graph->geometry();

  if (graph.adjusted(-8, -8, 8, 8).contains(event->pos()))
  {
    m_selectedcurve = -1;
    m_selectedindex = -1;

    if (m_component)
    {
      auto bestdist = 8.0f;

      for(auto &curve : m_component->curves)
      {
        auto curvedist = dist(nearest_on_polyline(curve.polyline, event->pos()), QPointF(event->pos()));

        if (curvedist < bestdist)
        {
          bestdist = curvedist;
          m_selectedcurve = indexof(m_component->curves, curve);
        }
      }

      if (m_selectedcurve != -1)
      {
        auto &selectedcurve = m_component->curves[m_selectedcurve];

        for(auto &pt : selectedcurve.points)
        {
          if (QRectF(pt.x()-4, pt.y()-4, 8, 8).contains(event->pos()))
          {
            m_selectedindex = indexof(selectedcurve.points, pt);
          }
        }

        if (m_selectedindex == -1)
        {
          if (ui.DistributionType->currentIndex() == Constant || ui.DistributionType->currentIndex() == Uniform)
          {
            m_selectedindex = 0;
          }

          if (ui.DistributionType->currentIndex() == Curve || ui.DistributionType->currentIndex() == UniformCurve)
          {
            double minx = 0.0;
            double maxx = 1.0;

            double miny = max(-m_component->scalemax, m_component->minvalue);
            double maxy = min(m_component->scalemax, m_component->maxvalue);

            double scalemax = m_component->scalemax;

            double x = clamp(remap(QPointF(event->pos()).x(), graph.left(), graph.right(), 0.0, 1.0), minx, maxx);
            double y = clamp(remap(QPointF(event->pos()).y(), graph.bottom(), graph.top(), -scalemax, scalemax), miny, maxy);

            size_t insertpos = 0;
            while (insertpos < selectedcurve.xa.size() && selectedcurve.xa[insertpos] < x)
              ++insertpos;

            selectedcurve.xa.insert(selectedcurve.xa.begin() + insertpos, x);
            selectedcurve.ya.insert(selectedcurve.ya.begin() + insertpos, y);

            m_selectedindex = insertpos;
          }
        }
      }
    }

    update();
  }

  m_mousepresspos = m_mousemovepos = event->pos();

  event->accept();
}


///////////////////////// CurveEditor::mouseMoveEvent ///////////////////////
void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_mousepresspos.isNull())
  {
    QRectF graph = ui.Graph->geometry();

    if (m_selectedcurve != -1 && m_selectedindex != -1)
    {
      auto &selectedcurve = m_component->curves[m_selectedcurve];

      double minx = 0.0;
      double maxx = 1.0;

      if (m_selectedindex != 0)
        minx = selectedcurve.xa[m_selectedindex-1] + 0.01;

      if (selectedcurve.xa.begin() + m_selectedindex + 1 != selectedcurve.xa.end())
        maxx = selectedcurve.xa[m_selectedindex+1] - 0.01;

      double miny = max(-m_component->scalemax, m_component->minvalue);
      double maxy = min(m_component->scalemax, m_component->maxvalue);

      double scalemax = m_component->scalemax;

      double x = clamp(remap(QPointF(event->pos()).x(), graph.left(), graph.right(), 0.0, 1.0), minx, maxx);
      double y = clamp(remap(QPointF(event->pos()).y(), graph.bottom(), graph.top(), -scalemax, scalemax), miny, maxy);

      x = floor(x / 0.005) * 0.005;

      if (-0.05 < y && y < 0.05)
        y = 0.0f;

      switch(ui.DistributionType->currentIndex())
      {
        case Constant:
        case Uniform:
          selectedcurve.ya[0] = selectedcurve.ya[1] = y;
          break;

        case Curve:
        case UniformCurve:
          selectedcurve.xa[m_selectedindex] = x;
          selectedcurve.ya[m_selectedindex] = y;
          break;
      }

      emit distribution_changed();

      update();
    }
  }

  m_mousemovepos = event->pos();
}


///////////////////////// CurveEditor::mouseReleaseEvent ////////////////////
void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
  m_mousepresspos = QPoint();
}


///////////////////////// CurveEditor::resizeEvent //////////////////////////
void CurveEditor::resizeEvent(QResizeEvent *event)
{
  update();
}


///////////////////////// CurveEditor::paintEvent ///////////////////////////
void CurveEditor::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  QRect graph = ui.Graph->geometry();

  painter.setPen(Qt::black);
  painter.drawLine(graph.topLeft(), graph.bottomLeft());
  painter.drawLine(graph.left(), graph.center().y(), graph.right(), graph.center().y());
  painter.drawText(graph.left()-50, graph.center().y()-25, 44, 50, Qt::AlignRight | Qt::AlignVCenter, "0.0");

  if (m_component)
  {
    auto selectedcurve = (m_selectedcurve != -1) ? &m_component->curves[m_selectedcurve] : nullptr;
    auto selectedpoint = (m_selectedcurve != -1 && m_selectedindex != -1) ? &m_component->curves[m_selectedcurve].points[m_selectedindex] : nullptr;

    if (ui.DistributionType->currentIndex() == Uniform || ui.DistributionType->currentIndex() == UniformCurve)
    {
      QPolygonF top = m_component->curves[0].polyline;
      QPolygonF bot = m_component->curves[1].polyline; reverse(bot.begin(), bot.end());

      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor(m_component->color.red(), m_component->color.green(), m_component->color.blue(), 64));

      painter.drawPolygon(top + bot);
    }

    for(auto &curve : m_component->curves)
    {
      painter.setPen(m_component->color);

      painter.drawPolyline(curve.polyline);

      painter.setPen(Qt::black);

      for(auto &pt : curve.points)
      {
        painter.setBrush((&curve == selectedcurve && &pt == selectedpoint) ? Qt::black : Qt::transparent);

        painter.drawEllipse(pt, 3, 3);
      }
    }
  }
}
