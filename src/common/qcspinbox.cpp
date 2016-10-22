//
// QcSpinBox
//

#include "qcspinbox.h"
#include <QKeyEvent>

#include <QDebug>

using namespace std;

//-------------------- QcSpinBox --------------------------------------------
//---------------------------------------------------------------------------

////////////////////// QcSpinBox::Constructor ///////////////////////////////
QcSpinBox::QcSpinBox(QWidget *parent)
  : QSpinBox(parent)
{
  m_editing = false;

  connect(this, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &QcSpinBox::updateValue);
  connect(this, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &QcSpinBox::valueChanged);
}


////////////////////// QcSpinBox::keyPressEvent /////////////////////////////
void QcSpinBox::keyPressEvent(QKeyEvent *event)
{
  m_editing = true;

  QSpinBox::keyPressEvent(event);

  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
  {
    if (QSpinBox::value() != m_value)
      updateValue(QSpinBox::value());

    m_editing = false;
  }
}


////////////////////// QcSpinBox::focusOutEvent /////////////////////////////
void QcSpinBox::focusOutEvent(QFocusEvent *event)
{
  m_editing = false;

  if (QSpinBox::value() != m_value)
    updateValue(m_value);

  QSpinBox::focusOutEvent(event);
}


////////////////////// QcSpinBox::wheelEvent ////////////////////////////////
void QcSpinBox::wheelEvent(QWheelEvent *event)
{
  m_editing = false;

  QSpinBox::wheelEvent(event);
}


////////////////////// QcSpinBox::updateValue ///////////////////////////////
void QcSpinBox::updateValue(int value)
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
