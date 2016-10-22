//
// Material Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "material.h"
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

  private:

    Ui::Properties ui;

    MaterialDocument m_document;
};
