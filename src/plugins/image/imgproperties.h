//
// Image Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include "ui_imgproperties.h"
#include <QDockWidget>

//-------------------------- ImageProperties --------------------------------
//---------------------------------------------------------------------------

class ImageProperties : public QDockWidget
{
  Q_OBJECT

  public:
    ImageProperties(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

  protected slots:

    void refresh();

    void on_Reimport_clicked();

  private:

    Ui::Properties ui;

    Studio::Document *m_document;
};
