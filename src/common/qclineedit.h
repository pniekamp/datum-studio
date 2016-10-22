//
// QcLineEdit
//

#pragma once

#include <QLineEdit>

//---------------- QcLineEdit --------------------------
//------------------------------------------------------

class QcLineEdit : public QLineEdit
{
  Q_OBJECT

  public:
    QcLineEdit(QWidget *parent = 0);
    QcLineEdit(const QString &contents, QWidget * parent = 0);

    QString const &text() const { return m_text; }

  public slots:

    void updateText(QString const &text);

  signals:

    void textChanged(QString const &text);

    void textUpdated(QString const &text);

  protected:

    void keyPressEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *event);

  private:

    bool m_editing;
    QString m_text;
};
