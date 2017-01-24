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

    void edit(ModelDocument *document);

    void set_selection(int index);

  signals:

    void selection_changed(int index);

  protected slots:

    void refresh();

    void on_dx_valueChanged(double value);
    void on_dy_valueChanged(double value);
    void on_dz_valueChanged(double value);
    void on_ax_valueChanged(double value);
    void on_ay_valueChanged(double value);
    void on_az_valueChanged(double value);

  private:

    int m_mesh;

    Ui::Properties ui;

    ModelDocument *m_document = nullptr;
};
