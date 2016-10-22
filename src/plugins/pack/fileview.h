//
// File View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "packmodel.h"
#include <QWidget>

//-------------------------- FileView ---------------------------------------
//---------------------------------------------------------------------------

class FileView : public QWidget
{
  Q_OBJECT

  public:
    FileView(QWidget *parent = 0);

  public slots:

    void set_asset(PackModel::Asset *asset);

  private:

    QWidget *m_view;
};
