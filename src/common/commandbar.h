//
// Command Bar
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include <QToolBar>

//-------------------------- CommandBar -------------------------------------
//---------------------------------------------------------------------------

class CommandBar : public QToolBar
{
  Q_OBJECT

  public:
    CommandBar(QWidget *parent = nullptr);
};
