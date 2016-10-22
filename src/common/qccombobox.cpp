//
// QcComboBox
//

#include "qccombobox.h"
#include <QKeyEvent>

#include <QDebug>

using namespace std;

//-------------------- QcComboBox -------------------------------------------
//---------------------------------------------------------------------------

////////////////////// QcComboBox::Constructor //////////////////////////////
QcComboBox::QcComboBox(QWidget *parent)
  : QComboBox(parent)
{
  m_editing = false;

  connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QcComboBox::updateValue);
  connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QcComboBox::valueChanged);
}


////////////////////// QcComboBox::keyPressEvent ////////////////////////////
void QcComboBox::keyPressEvent(QKeyEvent *event)
{
  m_editing = true;

  QComboBox::keyPressEvent(event);

  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
  {
    if (QComboBox::currentIndex() != m_value)
      updateValue(QComboBox::currentIndex());

    m_editing = false;
  }
}


////////////////////// QcComboBox::focusOutEvent ////////////////////////////
void QcComboBox::focusOutEvent(QFocusEvent *event)
{
  m_editing = false;

  if (QComboBox::currentIndex() != m_value)
    updateValue(m_value);

  QComboBox::focusOutEvent(event);
}


////////////////////// QcComboBox::wheelEvent ///////////////////////////////
void QcComboBox::wheelEvent(QWheelEvent *event)
{
  m_editing = false;

  QComboBox::wheelEvent(event);
}


////////////////////// QcComboBox::setValue /////////////////////////////////
void QcComboBox::setValue(int value)
{
  setCurrentIndex(value);
}


////////////////////// QcComboBox::updateValue //////////////////////////////
void QcComboBox::updateValue(int value)
{
  m_value = value;

  if (!m_editing)
  {
    blockSignals(true);
    setCurrentIndex(value);
    blockSignals(false);
  }

  emit valueUpdated(value);
}
