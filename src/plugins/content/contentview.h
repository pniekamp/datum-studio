//
// Content View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include <QListWidget>

//-------------------------- ContentView ------------------------------------
//---------------------------------------------------------------------------

class ContentView : public QListWidget
{
  Q_OBJECT

  public:
    ContentView(QWidget *parent = 0);

    QStringList selected_paths() const;

  public slots:

    void set_path(QString const &path);

    void trigger_rename(QString const &path);

    void update(QString const &path);

    void refresh();

  signals:

    void item_triggered(QString const &path);

    void item_renamed(QString const &src, QString const &dst);

  protected:

    void on_item_changed(QListWidgetItem *item);

    void on_item_triggered(QListWidgetItem *item);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QList<QListWidgetItem *> items) const;
    Qt::DropActions supportedDropActions() const;
    void dropEvent(QDropEvent *event);

  private:

    QString m_path;
};
