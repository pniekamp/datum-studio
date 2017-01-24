//
// Curve Editor
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "particlesystem.h"
#include "ui_curveeditor.h"
#include <QDialog>

//-------------------------- CurveEditor ------------------------------------
//---------------------------------------------------------------------------

class CurveEditor : public QDialog
{
  Q_OBJECT

  public:
    CurveEditor(QWidget *parent = nullptr);

    template<typename T>
    ParticleSystemDocument::Distribution<T> distribution();

    template<typename T>
    void set_distribution(ParticleSystemDocument::Distribution<T> const &distribution, T const &minvalue, T const &maxvalue);

  signals:

    void distribution_changed();

  protected slots:

    void on_DistributionType_activated(int index);

    void on_ComponentList_currentRowChanged(int index);

    void on_ScaleMax_valueChanged(double value);
    void on_ScaleMax_editingFinished();

  protected:

    enum Type
    {
      Constant,
      Uniform,
      Curve,
      UniformCurve
    };

    void update();

    void keyPressEvent(QKeyEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void resizeEvent(QResizeEvent *event);

    void paintEvent(QPaintEvent *event);

  private:

    struct Component
    {
      struct Curve
      {
        std::vector<float> xa;
        std::vector<float> ya;

        QPolygonF points;
        QPolygonF polyline;

        float value(float u) const;
      };

      QString name;
      QColor color;

      float minvalue;
      float maxvalue;

      float scalemax;

      std::vector<Curve> curves;

      void add_curve(std::vector<float> const &xa, std::vector<float> const &ya);
    };

    std::vector<Component> m_components;

    void add_component(QString name, QColor const &color, float minvalue, float maxvalue);

    std::vector<float> timebase() const;

  private:

    Component *m_component;

    int m_selectedcurve;
    int m_selectedindex;

    QPoint m_mousepresspos, m_mousemovepos;

    Ui::CurveEditor ui;
};
