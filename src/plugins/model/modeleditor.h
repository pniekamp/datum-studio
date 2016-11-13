//
// Model Editor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "modelview.h"
#include "modelproperties.h"
#include "qcslider.h"
#include "qcdoubleslider.h"
#include <QMainWindow>
#include <QToolBar>

//-------------------------- ModelEditor ------------------------------------
//---------------------------------------------------------------------------

class ModelEditor : public QMainWindow
{
  Q_OBJECT

  public:
    ModelEditor(QWidget *parent = nullptr);
    virtual ~ModelEditor();

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  private:

    QToolBar *m_toolbar;
    QcDoubleSlider *m_exposureslider;

    ModelView *m_view;
    ModelProperties *m_properties;
};
