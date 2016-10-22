//
// QcDoubleSpinBox
//

#include "qcdoublespinbox.h"
#include <QKeyEvent>

#include <QDebug>

using namespace std;

//-------------------- QcDoubleSpinBox --------------------------------------
//---------------------------------------------------------------------------

////////////////////// QcDoubleSpinBox::Constructor /////////////////////////
QcDoubleSpinBox::QcDoubleSpinBox(QWidget *parent)
  : QDoubleSpinBox(parent)
{
  m_editing = false;

  connect(this, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &QcDoubleSpinBox::updateValue);
  connect(this, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &QcDoubleSpinBox::valueChanged);
}


////////////////////// QcDoubleSpinBox::keyPressEvent ///////////////////////
void QcDoubleSpinBox::keyPressEvent(QKeyEvent *event)
{
  m_editing = true;

  QDoubleSpinBox::keyPressEvent(event);

  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
  {
    if (QDoubleSpinBox::value() != m_value)
      updateValue(QDoubleSpinBox::value());

    m_editing = false;
  }
}


////////////////////// QcDoubleSpinBox::focusOutEvent ///////////////////////
void QcDoubleSpinBox::focusOutEvent(QFocusEvent *event)
{
  m_editing = false;

  if (QDoubleSpinBox::value() != m_value)
    updateValue(m_value);

  QDoubleSpinBox::focusOutEvent(event);
}


////////////////////// QcDoubleSpinBox::wheelEvent //////////////////////////
void QcDoubleSpinBox::wheelEvent(QWheelEvent *event)
{
  m_editing = false;

  QDoubleSpinBox::wheelEvent(event);
}


////////////////////// QcDoubleSpinBox::updateText //////////////////////////
void QcDoubleSpinBox::updateValue(double value)
{
  m_value = value;

  if (!m_editing)
  {
    blockSignals(true);
    setValue(value);
    blockSignals(false);
  }

  emit valueUpdated(value);
}
