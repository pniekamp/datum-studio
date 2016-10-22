//
// QcDoubleSlider
//

#include "qcdoubleslider.h"
#include <cmath>

#include <QDebug>

using namespace std;

//-------------------- QcDoubleSlider ---------------------------------------
//---------------------------------------------------------------------------

////////////////////// QcDoubleSlider::Constructor //////////////////////////
QcDoubleSlider::QcDoubleSlider(QWidget *parent)
  : QSlider(parent)
{
  m_min = 0.0;
  m_max = 1.0;
  m_decimals = 2;

  connect(this, &QcDoubleSlider::valueChanged, this, &QcDoubleSlider::valueUpdated);
}


////////////////////// QcDoubleSlider::Constructor //////////////////////////
QcDoubleSlider::QcDoubleSlider(Qt::Orientation orientation, QWidget *parent)
  : QSlider(orientation, parent)
{
  m_min = 0.0;
  m_max = 1.0;
  m_decimals = 2;

  connect(this, &QcDoubleSlider::valueChanged, this, &QcDoubleSlider::valueUpdated);
}


////////////////////// QcDoubleSlider::value ////////////////////////////////
double QcDoubleSlider::value() const
{
  return m_min + QSlider::value() / std::pow(10.0, m_decimals);
}


////////////////////// QcDoubleSlider::setDecimals //////////////////////////
void QcDoubleSlider::setDecimals(int decimals)
{
  m_decimals = decimals;

  setRange(m_min, m_max);
}


////////////////////// QcDoubleSlider::setRange /////////////////////////////
void QcDoubleSlider::setRange(int min, int max)
{
  setRange(min / std::pow(10.0, m_decimals), max / std::pow(10.0, m_decimals));
}


////////////////////// QcDoubleSlider::setRange /////////////////////////////
void QcDoubleSlider::setRange(double min, double max)
{
  m_min = min;
  m_max = max;

  QSlider::setRange(0, (m_max - m_min) * std::pow(10.0, m_decimals));
}


////////////////////// QcDoubleSlider::setMinimum ///////////////////////////
void QcDoubleSlider::setMinimum(int min)
{
  setRange(min / std::pow(10.0, m_decimals), m_max);
}


////////////////////// QcDoubleSlider::setMinimum ///////////////////////////
void QcDoubleSlider::setMinimum(double min)
{
  setRange(min, m_max);
}


////////////////////// QcDoubleSlider::setMaximum ///////////////////////////
void QcDoubleSlider::setMaximum(int max)
{
  setRange(m_min, max / std::pow(10.0, m_decimals));
}


////////////////////// QcDoubleSlider::setMaximum ///////////////////////////
void QcDoubleSlider::setMaximum(double max)
{
  setRange(m_min, max);
}


////////////////////// QcDoubleSlider::setValue /////////////////////////////
void QcDoubleSlider::setValue(int value)
{
  QSlider::setValue(value);
}


////////////////////// QcDoubleSlider::setValue /////////////////////////////
void QcDoubleSlider::setValue(double value)
{
  QSlider::setValue((value - m_min) * std::pow(10.0, m_decimals) + 0.5);
}


////////////////////// QcDoubleSlider::updateValue //////////////////////////
void QcDoubleSlider::updateValue(double value)
{
  blockSignals(true);
  setValue(value);
  blockSignals(false);

  emit valueUpdated(value);
}


////////////////////// QcDoubleSlider::sliderChange /////////////////////////
void QcDoubleSlider::sliderChange(SliderChange change)
{
  QSlider::sliderChange(change);

  if (change == QAbstractSlider::SliderValueChange)
    emit valueChanged(value());
}
