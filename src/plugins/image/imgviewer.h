//
// Image Viewer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "imgview.h"
#include "imgproperties.h"
#include "qcslider.h"
#include "qcdoubleslider.h"
#include <QMainWindow>
#include <QToolBar>

//-------------------------- ImageViewer ------------------------------------
//---------------------------------------------------------------------------

class ImageViewer : public QMainWindow
{
  Q_OBJECT

  public:
    ImageViewer(QWidget *parent = nullptr);
    virtual ~ImageViewer();

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  private:

    QToolBar *m_toolbar;
    QcDoubleSlider *m_scaleslider;
    QcSlider *m_layerslider;
    QcDoubleSlider *m_exposureslider;

    ImageView *m_view;
    ImageProperties *m_properties;
};
