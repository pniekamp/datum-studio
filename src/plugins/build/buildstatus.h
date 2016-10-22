//
// Build Status Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "api.h"
#include "buildapi.h"
#include <QLabel>
#include <QMovie>

//-------------------------- BuildStatus ------------------------------------
//---------------------------------------------------------------------------

class BuildStatus : public QWidget
{
  Q_OBJECT

  public:
    BuildStatus();

  protected slots:

    void on_build_started(Studio::Document *document);
    void on_build_completed(Studio::Document *document);

  private:

    QLabel *m_text;
    QLabel *m_loader;

    int m_building;
};

