//
// QPointF traits
//

#pragma once

#include <QPointF>
#include <QPolygonF>
#include <leap/lml/point.h>
#include <leap/lml/geometry2d.h>

//|-------------------- QPointF Traits ------------------------------------
//|------------------------------------------------------------------------

namespace leap { namespace lml
{
  template<>
  struct point_traits<QPointF>
  {
    static constexpr size_t dimension = 2;
  };
} }

template<size_t i>
constexpr auto get(QPointF const &pt);

template<>
constexpr auto get<0>(QPointF const &pt)
{
  return pt.x();
}

template<>
constexpr auto get<1>(QPointF const &pt)
{
  return pt.y();
}


//|///////////////////// operator + ///////////////////////////////////////
inline QPointF operator +(QPointF const &pt, leap::lml::Vector<double, 2> const &v)
{
  return QPointF(pt.x() + get<0>(v), pt.y() + get<1>(v));
}

//|///////////////////// operator - ///////////////////////////////////////
inline QPointF operator -(QPointF const &pt, leap::lml::Vector<double, 2> const &v)
{
  return QPointF(pt.x() - get<0>(v), pt.y() - get<1>(v));
}


//|-------------------- QPolygon Traits -----------------------------------
//|------------------------------------------------------------------------

namespace leap { namespace lml
{
  template<>
  struct ring_traits<QPolygonF>
  {
    static constexpr size_t dimension = 2;

    static constexpr int orientation = 1;
  };

} }
