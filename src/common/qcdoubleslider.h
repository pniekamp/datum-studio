//
// QcDoubleSlider
//

#pragma once

#include <QSlider>

//---------------- QcDoubleSlider ----------------------
//------------------------------------------------------

class QcDoubleSlider : public QSlider
{
  Q_OBJECT

  public:
    QcDoubleSlider(QWidget *parent = 0);
    QcDoubleSlider(Qt::Orientation orientation, QWidget *parent = 0);

    double value() const;

    void setDecimals(int decimals);

    void setRange(int min, int max);
    void setRange(double min, double max);

    void setMinimum(int min);
    void setMinimum(double min);
    void setMaximum(int max);
    void setMaximum(double max);

  public slots:

    void setValue(int value);
    void setValue(double value);

    void updateValue(double value);

  signals:

    void valueChanged(double value);

    void valueUpdated(double value);

  protected:

    void sliderChange(SliderChange change);

  private:

    double m_min;
    double m_max;

    int m_decimals;
};
