//
// Emitter List Widget
//

//
// Copyright (C) 2017 Peter Niekamp
//

#pragma once

#include "particlesystem.h"
#include <QListWidget>

//-------------------------- EmitterListWidget ------------------------------
//---------------------------------------------------------------------------

class EmitterListWidget : public QListWidget
{
  Q_OBJECT

  public:
    EmitterListWidget(QWidget *parent = nullptr);

  public slots:

    void edit(Studio::Document *document);

    void set_selection(int index);

  signals:

    void selection_changed(int index);

  protected slots:

    void refresh();

    void on_contextmenu_requested(QPoint pos);

    void on_Create_triggered();
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

    QAction *m_createaction;
    QAction *m_renameaction;
    QAction *m_deleteaction;

    ParticleSystemDocument m_document;
};
