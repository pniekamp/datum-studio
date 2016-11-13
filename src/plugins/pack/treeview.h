//
// Tree View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "packmodel.h"
#include <QTreeView>

//-------------------------- TreeView ---------------------------------------
//---------------------------------------------------------------------------

class TreeView : public QTreeView
{
  Q_OBJECT

  public:
    TreeView(QWidget *parent = 0);

    void set_model(PackModel *model);

    QSize sizeHint() const { return QSize(190, 256); }

  public:

    PackModel::Node *current_node() const;

    QList<PackModel::Node*> selected_nodes() const;

  public slots:

    void trigger_rename(QModelIndex const &index);

  signals:

    void current_changed(PackModel::Node *node);

    void item_triggered(PackModel::Node *node);

    void item_renamed(PackModel::Node *node, QString const &str);

  protected:

    void currentChanged(QModelIndex const &current, QModelIndex const &previous);

    void commitData(QWidget *editor);

    void itemTriggered(QModelIndex const &index);

    void dragEnterEvent(QDragEnterEvent *event);
};
