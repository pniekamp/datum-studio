//
// Mesh Widget
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "model.h"
#include <QListWidget>

//-------------------------- MeshWidget -------------------------------------
//---------------------------------------------------------------------------

class MeshWidget : public QListWidget
{
  Q_OBJECT

  public:
    MeshWidget(QWidget *parent = nullptr);

  public slots:

    void edit(ModelDocument *document);

    void set_selection(int index);

  signals:

    void selection_changed(int index);

  protected slots:

    void refresh();

    void on_contextmenu_requested(QPoint pos);

    void on_Rename_triggered();
    void on_Delete_triggered();

  protected:

    void itemSelectionChanged();

    void commitData(QWidget *editor);

    QStringList mimeTypes() const;
    QMimeData *mimeData(QList<QListWidgetItem *> const items) const;
    Qt::DropActions supportedDropActions() const;
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

  private:

    QAction *m_renameaction;
    QAction *m_deleteaction;

    ModelDocument *m_document = nullptr;
};
