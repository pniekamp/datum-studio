//
// Dialog Factory
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include <QDialog>

//----------------------- DialogFactory -------------------------
//---------------------------------------------------------------

template<class UIT>
class DialogFactory : public QDialog
{
  public:
    DialogFactory(QWidget *parent)
      : QDialog(parent)
    {
      ui.setupUi(this);
    }

    ~DialogFactory()
    {
    }

    UIT ui;
}; 
