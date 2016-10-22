//
// Datum Studio
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "core.h"
#include "ui_datumstudio.h"

//-------------------------- DatumStudio ------------------------------------
//---------------------------------------------------------------------------

class DatumStudio : public QMainWindow
{
  Q_OBJECT

  public:
    DatumStudio();
    virtual ~DatumStudio();

    void set_screen_geometry(std::string const &geometry);

  public slots:

    void on_About_triggered();

  protected:

    virtual void closeEvent(QCloseEvent *event) override;

  private:

    Ui::DatumStudio ui;
};
