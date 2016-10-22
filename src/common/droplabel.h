//
// Drop Label
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include <QLabel>

//-------------------------- DropLabel --------------------------------------
//---------------------------------------------------------------------------

class DropLabel : public QLabel
{
  Q_OBJECT

  public:
    DropLabel(QWidget *parent = 0);
    DropLabel(QString const &text, QWidget *parent = 0);
    virtual ~DropLabel();

    void set_droptype(QString const &droptype);

   public slots:

    void setText(QString const &text);

    void setPixmap(Studio::Document const *document);

  signals:

    void itemDropped(QString const &path);

  protected:

    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

  private:

    QString m_droptype;

    QString m_placeholder;
};
