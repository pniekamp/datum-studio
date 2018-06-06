//
// Material Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "terrainmaterial.h"
#include "ui_materialproperties.h"
#include <QDockWidget>

//-------------------------- MaterialProperties -----------------------------
//---------------------------------------------------------------------------

class MaterialProperties : public QDockWidget
{
  Q_OBJECT

  public:
    MaterialProperties(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

  protected slots:

    void refresh();

  protected:

    void on_material_build_complete(Studio::Document *document, QString const &path);

  private:

    Ui::Properties ui;

    TerrainMaterialDocument m_document;
};
