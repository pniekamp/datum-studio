//
// QcComboBox
//

#pragma once

#include <QComboBox>

//---------------- QcComboBox ---------------------------
//------------------------------------------------------

class QcComboBox : public QComboBox
{
  Q_OBJECT

  public:
    QcComboBox(QWidget *parent = 0);

    int value() const { return m_value; }

  public slots:

    void setValue(int value);

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
