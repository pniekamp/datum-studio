//
// Model Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "model.h"
#include "ui_modelproperties.h"
#include <QDockWidget>

//-------------------------- ModelProperties --------------------------------
//---------------------------------------------------------------------------

class ModelProperties : public QDockWidget
{
  Q_OBJECT

  public:
    ModelProperties(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

  protected slots:

    void refresh();

  private:

    Ui::Properties ui;

    ModelDocument m_document;
};
