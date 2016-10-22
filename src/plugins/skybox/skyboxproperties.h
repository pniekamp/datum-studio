//
// SkyboxProperties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "skybox.h"
#include "ui_skyboxproperties.h"
#include <QDockWidget>

//-------------------------- SkyboxProperties -------------------------------
//---------------------------------------------------------------------------

class SkyboxProperties : public QDockWidget
{
  Q_OBJECT

  public:
    SkyboxProperties(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

  protected slots:

    void refresh();

    void on_Width_textEdited(QString const &text);
    void on_Height_textEdited(QString const &text);

    void on_Type_valueChanged(int value);

    void on_Front_itemDropped(QString const &path);
    void on_Left_itemDropped(QString const &path);
    void on_Right_itemDropped(QString const &path);
    void on_Back_itemDropped(QString const &path);
    void on_Top_itemDropped(QString const &path);
    void on_Bottom_itemDropped(QString const &path);
    void on_EnvMap_itemDropped(QString const &path);

  private:

    Ui::Properties ui;

    SkyboxDocument m_document;
};
