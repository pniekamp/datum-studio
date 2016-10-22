//
// Command Bar
//

//
// Copyright (C) 2016 Peter Niekamp
//


#include "commandbar.h"
#include <QActionEvent>
#include <QToolButton>

#include <QtDebug>

using namespace std;

//|---------------------- CommandBar ----------------------------------------
//|--------------------------------------------------------------------------

///////////////////////// CommandBar::Constructor ///////////////////////////
CommandBar::CommandBar(QWidget *parent)
  : QToolBar(parent)
{
  setOrientation(Qt::Horizontal);
  setToolButtonStyle(Qt::ToolButtonIconOnly);
  setIconSize(QSize(14, 14));
}
