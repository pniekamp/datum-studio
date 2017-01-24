//
// Slot List Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "model.h"
#include <QListWidget>

//-------------------------- SlotListWidget ---------------------------------
//---------------------------------------------------------------------------

class SlotListWidget : public QListWidget
{
  Q_OBJECT

  public:
    SlotListWidget(QWidget *parent = nullptr);

  public slots:

    void edit(ModelDocument *document);

    void set_mesh(int index);

  signals:

    void selection_changed(int index);

  protected slots:

    void refresh();

  protected:

    void itemSelectionChanged();

    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

  private:

    int m_mesh;

    ModelDocument *m_document = nullptr;
};
