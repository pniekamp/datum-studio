//
// Mesh Viewer
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "meshview.h"
#include "meshproperties.h"
#include <QMainWindow>
#include <QToolBar>
#include <QLabel>
#include <QSlider>

//-------------------------- MeshViewer -------------------------------------
//---------------------------------------------------------------------------

class MeshViewer : public QMainWindow
{
  Q_OBJECT

  public:
    MeshViewer(QWidget *parent = nullptr);
    virtual ~MeshViewer();

  public slots:

    QToolBar *toolbar() const;

    void view(Studio::Document *document);
    void edit(Studio::Document *document);

  private:

    QToolBar *m_toolbar;

    MeshView *m_view;
    MeshProperties *m_properties;
};

