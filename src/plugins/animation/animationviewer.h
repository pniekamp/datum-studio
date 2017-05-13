//
// Animation Viewer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "animationview.h"
#include "animationproperties.h"
#include <QMainWindow>
#include <QToolBar>
#include <QLabel>
#include <QSlider>

//-------------------------- AnimationViewer --------------------------------
//---------------------------------------------------------------------------

class AnimationViewer : public QMainWindow
{
  Q_OBJECT

  public:
    AnimationViewer(QWidget *parent = nullptr);
    virtual ~AnimationViewer();

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  private:

    QToolBar *m_toolbar;

    AnimationView *m_view;
    AnimationProperties *m_properties;
};

