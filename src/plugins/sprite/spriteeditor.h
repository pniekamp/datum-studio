//
// Sprite Editor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "spriteview.h"
#include "spriteproperties.h"
#include "qcslider.h"
#include "qcdoubleslider.h"
#include <QMainWindow>
#include <QToolBar>

//-------------------------- SpriteEditor -----------------------------------
//---------------------------------------------------------------------------

class SpriteEditor : public QMainWindow
{
  Q_OBJECT

  public:
    SpriteEditor(QWidget *parent = nullptr);
    virtual ~SpriteEditor();

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  private:

    QToolBar *m_toolbar;
    QcDoubleSlider *m_scaleslider;
    QcSlider *m_layerslider;

    SpriteView *m_view;
    SpriteProperties *m_properties;
};
