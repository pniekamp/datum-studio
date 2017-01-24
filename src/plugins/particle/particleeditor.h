//
// Particle Editor
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "particleview.h"
#include "particleproperties.h"
#include "qcslider.h"
#include "qcdoubleslider.h"
#include <QMainWindow>
#include <QToolBar>

//-------------------------- ParticleEditor ---------------------------------
//---------------------------------------------------------------------------

class ParticleEditor : public QMainWindow
{
  Q_OBJECT

  public:
    ParticleEditor(QWidget *parent = nullptr);
    virtual ~ParticleEditor();

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  private:

    QToolBar *m_toolbar;
    QcDoubleSlider *m_exposureslider;

    ParticleView *m_view;
    ParticleProperties *m_properties;
};
