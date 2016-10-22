//
// QcSlider
//

#pragma once

#include <QSlider>

//---------------- QcSlider ----------------------------
//------------------------------------------------------

class QcSlider : public QSlider
{
  Q_OBJECT

  public:
    QcSlider(QWidget *parent = 0);
    QcSlider(Qt::Orientation orientation, QWidget *parent = 0);

  public slots:

    void updateValue(int value);

  signals:

    void valueUpdated(int value);
};
