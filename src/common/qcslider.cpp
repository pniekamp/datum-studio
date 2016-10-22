//
// QcSlider
//

#include "qcslider.h"
#include <cmath>

#include <QDebug>

using namespace std;

//-------------------- QcSlider ---------------------------------------------
//---------------------------------------------------------------------------

////////////////////// QcSlider::Constructor ////////////////////////////////
QcSlider::QcSlider(QWidget *parent)
  : QSlider(parent)
{
  connect(this, &QSlider::valueChanged, this, &QcSlider::valueUpdated);
}


////////////////////// QcSlider::Constructor ////////////////////////////////
QcSlider::QcSlider(Qt::Orientation orientation, QWidget *parent)
  : QSlider(orientation, parent)
{
  connect(this, &QSlider::valueChanged, this, &QcSlider::valueUpdated);
}


////////////////////// QcSlider::updateValue ////////////////////////////////
void QcSlider::updateValue(int value)
{
  blockSignals(true);
  setValue(value);
  blockSignals(false);

  emit valueUpdated(value);
}
