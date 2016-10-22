//
// Mesh Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "mesh.h"
#include "ui_meshproperties.h"
#include <QDockWidget>

//-------------------------- MeshProperties ---------------------------------
//---------------------------------------------------------------------------

class MeshProperties : public QDockWidget
{
  Q_OBJECT

  public:
    MeshProperties(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

  protected slots:

    void refresh();

    void on_Reimport_clicked();

  private:

    Ui::Properties ui;

    MeshDocument m_document;
};
