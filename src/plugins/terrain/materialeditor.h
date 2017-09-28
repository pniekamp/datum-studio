//
// Material Editor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "materialview.h"
#include "materialproperties.h"
#include "qcdoubleslider.h"
#include <QMainWindow>
#include <QToolBar>

//-------------------------- MaterialEditor ---------------------------------
//---------------------------------------------------------------------------

class MaterialEditor : public QMainWindow
{
  Q_OBJECT

  public:
    MaterialEditor(QWidget *parent = nullptr);
    virtual ~MaterialEditor();

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  private:

    QToolBar *m_toolbar;
    QcDoubleSlider *m_exposureslider;

    MaterialView *m_view;
    MaterialProperties *m_properties;
};
