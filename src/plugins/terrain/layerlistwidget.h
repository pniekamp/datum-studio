//
// Layer List Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "terrainmaterial.h"
#include <QListWidget>

//-------------------------- LayerListWidget --------------------------------
//---------------------------------------------------------------------------

class LayerListWidget : public QListWidget
{
  Q_OBJECT

  public:
    LayerListWidget(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

  protected slots:

    void refresh();

    void on_contextmenu_requested(QPoint pos);

    void on_Delete_triggered();

  protected:

    QStringList mimeTypes() const;
    QMimeData *mimeData(QList<QListWidgetItem *> const items) const;
    Qt::DropActions supportedDropActions() const;
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

  private:

    QAction *m_deleteaction;

    TerrainMaterialDocument m_document;
};
