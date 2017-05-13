//
// Animation Properties
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "animation.h"
#include "ui_animationproperties.h"
#include <QDockWidget>

//-------------------------- AnimationProperties ----------------------------
//---------------------------------------------------------------------------

class AnimationProperties : public QDockWidget
{
  Q_OBJECT

  public:
    AnimationProperties(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

  protected slots:

    void refresh();

    void on_Reimport_clicked();

  private:

    Ui::Properties ui;

    AnimationDocument m_document;
};
