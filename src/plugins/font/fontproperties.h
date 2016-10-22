//
// FontProperties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "font.h"
#include "ui_fontproperties.h"
#include <QDockWidget>

//-------------------------- FontProperties ---------------------------------
//---------------------------------------------------------------------------

class FontProperties : public QDockWidget
{
  Q_OBJECT

  public:
    FontProperties(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

  protected slots:

    void refresh();

    void on_FontButton_clicked();
    void on_AtlasWidth_textEdited(QString const &text);
    void on_AtlasHeight_textEdited(QString const &text);

  private:

    Ui::Properties ui;

    FontDocument m_document;
};
