//
// Sprite Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "spritesheet.h"
#include "ui_spriteproperties.h"
#include <QDockWidget>

//-------------------------- SpriteProperties -------------------------------
//---------------------------------------------------------------------------

class SpriteProperties : public QDockWidget
{
  Q_OBJECT

  public:
    SpriteProperties(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

  protected slots:

    void refresh();

  protected:

    void on_sprite_build_complete(Studio::Document *document, QString const &path);

  private:

    Ui::Properties ui;

    SpriteSheetDocument m_document;
};
