//
// QcLineEdit
//

#include "qclineedit.h"
#include <QKeyEvent>

#include <QDebug>

using namespace std;

//-------------------- QcLineEdit -------------------------------------------
//---------------------------------------------------------------------------

////////////////////// QcLineEdit::Constructor //////////////////////////////
QcLineEdit::QcLineEdit(QWidget *parent)
  : QLineEdit(parent)
{
  m_editing = false;

  connect(this, &QLineEdit::textChanged, this, &QcLineEdit::updateText);
  connect(this, &QLineEdit::textEdited, this, &QcLineEdit::textChanged);
}


////////////////////// QcLineEdit::Constructor //////////////////////////////
QcLineEdit::QcLineEdit(QString const &contents, QWidget * parent)
  : QLineEdit(contents, parent)
{
  m_editing = false;

  connect(this, &QLineEdit::textChanged, this, &QcLineEdit::updateText);
  connect(this, &QLineEdit::textEdited, this, &QcLineEdit::textChanged);
}


////////////////////// QcLineEdit::keyPressEvent ////////////////////////////
void QcLineEdit::keyPressEvent(QKeyEvent *event)
{
  m_editing = true;

  QLineEdit::keyPressEvent(event);

  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
  {
    if (QLineEdit::text() != m_text)
      updateText(QLineEdit::text());

    m_editing = false;
  }
}


////////////////////// QcLineEdit::focusOutEvent ////////////////////////////
void QcLineEdit::focusOutEvent(QFocusEvent *event)
{
  m_editing = false;

  if (QLineEdit::text() != m_text)
    updateText(m_text);

  QLineEdit::focusOutEvent(event);
}


////////////////////// QcLineEdit::updateText ///////////////////////////////
void QcLineEdit::updateText(QString const &text)
{
  m_text = text;

  if (!m_editing)
  {
    blockSignals(true);
    setText(text);
    blockSignals(false);
  }

  emit textUpdated(text);
}

