//
// QcSpinBox
//

#pragma once

#include <QSpinBox>

//---------------- QcSpinBox ---------------------------
//------------------------------------------------------

class QcSpinBox : public QSpinBox
{
  Q_OBJECT

  public:
    QcSpinBox(QWidget *parent = 0);

    int value() const { return m_value; }

  public slots:

    void updateValue(int value);

  signals:

    void valueChanged(int value);

    void valueUpdated(int value);

  protected:

    void keyPressEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void wheelEvent(QWheelEvent *event);

  private:

    bool m_editing;
    int m_value;
};
