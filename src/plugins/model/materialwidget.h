//
// Material Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "model.h"
#include "ui_materialwidget.h"
#include <QWidget>

//-------------------------- MaterialWidget ---------------------------------
//---------------------------------------------------------------------------

class MaterialWidget : public QWidget
{
  Q_OBJECT

  public:
    MaterialWidget(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

    void set_mesh(int index);
    void set_slot(int index);

  protected slots:

    void refresh();

    void on_Material_itemDropped(QString const &path);

    void on_TintRedSlider_valueChanged(double value);
    void on_TintRedSpinner_valueChanged(double value);
    void on_TintGreenSlider_valueChanged(double value);
    void on_TintGreenSpinner_valueChanged(double value);
    void on_TintBlueSlider_valueChanged(double value);
    void on_TintBlueSpinner_valueChanged(double value);    

    void on_Reset_clicked();

  private:

    int m_mesh;
    int m_slot;

    Ui::MaterialWidget ui;

    ModelDocument m_document;
};
