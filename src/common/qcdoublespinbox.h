//
// QcDoubleSpinBox
//

#pragma once

#include <QDoubleSpinBox>

//---------------- QcDoubleSpinBox ---------------------
//------------------------------------------------------

class QcDoubleSpinBox : public QDoubleSpinBox
{
  Q_OBJECT

  public:
    QcDoubleSpinBox(QWidget *parent = 0);

    double value() const { return m_value; }

  public slots:

    void updateValue(double value);

  signals:

    void valueChanged(double value);

    void valueUpdated(double value);

  protected:

    void keyPressEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void wheelEvent(QWheelEvent *event);

  private:

    bool m_editing;
    double m_value;
};
