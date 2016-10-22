//
// Tree View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "packmodel.h"
#include <QTreeWidget>

//-------------------------- TreeView ---------------------------------------
//---------------------------------------------------------------------------

class TreeView : public QTreeWidget
{
  Q_OBJECT

  public:
    TreeView(QWidget *parent = 0);

    void set_model(PackModel *model);

    QSize sizeHint() const { return QSize(190, 256); }

  signals:

    void selection_changed(PackModel::Asset *asset);

    void item_triggered(PackModel::Asset *asset);

  protected:

    void on_model_reset();
    void on_model_added(size_t index, PackModel::Asset *asset);
    void on_model_changed(size_t index, PackModel::Asset *asset);
    void on_model_removed(size_t index, PackModel::Asset *asset);

  protected:

    void on_selection_changed();

    void on_item_triggered(QTreeWidgetItem *item, int column);

  protected:

    QStringList mimeTypes() const;
    QMimeData *mimeData(QList<QTreeWidgetItem *> const items) const;
    Qt::DropActions supportedDropActions() const;
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

  private:

    PackModel *m_model;
};
