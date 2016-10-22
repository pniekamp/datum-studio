//
// Skybox Editor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "skyboxview.h"
#include "skyboxproperties.h"
#include "qcslider.h"
#include "qcdoubleslider.h"
#include <QMainWindow>
#include <QToolBar>

//-------------------------- SkyboxEditor -----------------------------------
//---------------------------------------------------------------------------

class SkyboxEditor : public QMainWindow
{
  Q_OBJECT

  public:
    SkyboxEditor(QWidget *parent = nullptr);
    virtual ~SkyboxEditor();

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  private:

    QToolBar *m_toolbar;
    QcSlider *m_layerslider;
    QcDoubleSlider *m_exposureslider;

    SkyboxView *m_view;
    SkyboxProperties *m_properties;
};
