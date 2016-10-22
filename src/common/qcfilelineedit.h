//
// File Line Edit
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include <QLineEdit>

class QPushButton;

//-------------------------- QcFileLineEdit ---------------------------------
//---------------------------------------------------------------------------

class QcFileLineEdit : public QWidget
{
  Q_OBJECT

  public:

    enum class BrowseType
    {
      OpenFile,
      SaveFile,
      Directory,
    };

  public:
    QcFileLineEdit(QWidget *parent = 0);

    void set_browsetype(BrowseType type, QString const &filter = "All Files (*.*)");

    void set_browsetext(QString const &text);

    void set_rememberkey(QString const &key);

    QString text() const { return m_path->text(); }

    void setText(QString const &text) { m_path->setText(text); }

    void setPlaceholderText(QString const &text) { m_path->setPlaceholderText(text); }

  signals:

    void textChanged(QString const &text);

  protected:

    void on_browse_clicked();

  private:

    class LineEdit : public QLineEdit
    {
      public:
        LineEdit(QWidget *parent = 0);

        virtual void dragEnterEvent(QDragEnterEvent *event);
        virtual void dropEvent(QDropEvent *event);
    };

  private:

    BrowseType m_type;

    QString m_filter;

    QString m_rememberkey;

    LineEdit *m_path;
    QPushButton *m_browse;
};
