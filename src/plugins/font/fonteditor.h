//
// Font Editor
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "fontview.h"
#include "fontproperties.h"
#include "qcdoubleslider.h"
#include <QMainWindow>
#include <QToolBar>

//-------------------------- FontEditor -------------------------------------
//---------------------------------------------------------------------------

class FontEditor : public QMainWindow
{
  Q_OBJECT

  public:
    FontEditor(QWidget *parent = nullptr);
    virtual ~FontEditor();

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  private:

    QToolBar *m_toolbar;
    QcDoubleSlider *m_scaleslider;

    FontView *m_view;
    FontProperties *m_properties;
};
