//
// Folder View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include <QTreeWidget>

//-------------------------- FolderView -------------------------------------
//---------------------------------------------------------------------------

class FolderView : public QTreeWidget
{
  Q_OBJECT

  public:
    FolderView(QWidget *parent = 0);

    QSize sizeHint() const { return QSize(190, 256); }

    QString selected_path() const;

  public slots:

    void set_basepath(QString const &basepath);

    void select_path(QString const &path);

    void trigger_rename(QString const &path);

    void update(QString const &path);

    void refresh();

  signals:

    void selection_changed(QString const &path);

    void item_renamed(QString const &src, QString const &dst);

  protected:

    void on_selection_changed();

    void on_item_changed(QTreeWidgetItem *item, int column);

    QStringList mimeTypes() const;
    QMimeData *mimeData(QList<QTreeWidgetItem *> const items) const;
    Qt::DropActions supportedDropActions() const;
    void dropEvent(QDropEvent *event);

  private:

    QString m_basepath;
};
