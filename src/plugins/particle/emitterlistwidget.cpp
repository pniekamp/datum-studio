//
// Emitter List Widget
//

//
// Copyright (C) 2017 Peter Niekamp
//

#include "emitterlistwidget.h"
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QLineEdit>
#include <QDebug>

using namespace std;

//|---------------------- EmitterListWidget ---------------------------------
//|--------------------------------------------------------------------------

///////////////////////// EmitterListWidget::Constructor ////////////////////
EmitterListWidget::EmitterListWidget(QWidget *parent)
  : QListWidget(parent)
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  m_createaction = new QAction("Create", this);

  m_renameaction = new QAction("Rename", this);
  m_renameaction->setShortcut(QKeySequence(Qt::Key_F2));
  m_renameaction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  m_deleteaction = new QAction("Delete", this);
  m_deleteaction->setShortcut(QKeySequence(Qt::Key_Delete));
  m_deleteaction->setShortcutContext(Qt::WidgetWithChildrenShortcut);

  addAction(m_createaction);
  addAction(m_renameaction);
  addAction(m_deleteaction);

  connect(m_createaction, &QAction::triggered, this, &EmitterListWidget::on_Create_triggered);
  connect(m_renameaction, &QAction::triggered, this, &EmitterListWidget::on_Rename_triggered);
  connect(m_deleteaction, &QAction::triggered, this, &EmitterListWidget::on_Delete_triggered);
  connect(this, &QListWidget::itemSelectionChanged, this, &EmitterListWidget::itemSelectionChanged);
  connect(this, &QListWidget::customContextMenuRequested, this, &EmitterListWidget::on_contextmenu_requested);
}


///////////////////////// EmitterListWidget::edit ///////////////////////////
void EmitterListWidget::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &ParticleSystemDocument::document_changed, this, &EmitterListWidget::refresh);

  refresh();
}


///////////////////////// EmitterListWidget::set_selection //////////////////
void EmitterListWidget::set_selection(int index)
{
  if (currentRow() != index)
  {
    setCurrentRow(index);
  }
}


///////////////////////// EmitterListWidget::refresh ////////////////////////
void EmitterListWidget::refresh()
{
  auto currentrow = currentRow();

  clear();

  for(int i = 0; i < m_document.emitters(); ++i)
  {
    auto &emitter = m_document.emitter(i);

    QListWidgetItem *item = new QListWidgetItem(emitter.name, this);

    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled);

    item->setIcon(QIcon(":/particleplugin/icon.png"));
  }

  setCurrentRow(currentrow);
}


///////////////////////// EmitterListWidget::itemSelectionChanged ///////////
void EmitterListWidget::itemSelectionChanged()
{ 
  emit selection_changed(currentRow());
}


///////////////////////// EmitterListWidget::renamed ////////////////////////
void EmitterListWidget::commitData(QWidget *editor)
{
  if (currentItem())
  {
    QString text = qobject_cast<QLineEdit*>(editor)->text();

    if (text != currentItem()->text())
    {
      closePersistentEditor(currentItem());

      auto emitter = m_document.emitter(currentRow());

      emitter.name = text;

      m_document.update_emitter(currentRow(), emitter);
    }
  }
}


///////////////////////// EmitterListWidget::dragEnterEvent /////////////////
QStringList EmitterListWidget::mimeTypes() const
{
  return { "datumstudio/particleemittermodelitem" };
}


///////////////////////// EmitterListWidget::mimeData ///////////////////////
QMimeData *EmitterListWidget::mimeData(QList<QListWidgetItem *> const items) const
{
  QMimeData *mimedata = new QMimeData();

  QByteArray encoded;
  QDataStream stream(&encoded, QIODevice::WriteOnly);

  for(auto &item : items)
  {
    stream << indexFromItem(item).row();
  }

  mimedata->setData("datumstudio/particleemittermodelitem", encoded);

  return mimedata;
}


///////////////////////// EmitterListWidget::supportedDropActions ///////////
Qt::DropActions EmitterListWidget::supportedDropActions() const
{
  return Qt::MoveAction;
}


////////////////////// EmitterListWidget::dragEnterEvent ////////////////////
void EmitterListWidget::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("datumstudio/particleemittermodelitem") && event->source() == this)
  {
    event->accept();
  }

  if (!event->isAccepted())
    return;

  QListWidget::dragEnterEvent(event);
}


///////////////////////// EmitterListWidget::dropEvent //////////////////////
void EmitterListWidget::dropEvent(QDropEvent *event)
{
  size_t position = 0;

  switch(dropIndicatorPosition())
  {
    case QAbstractItemView::OnItem:
      break;

    case QAbstractItemView::AboveItem:
      position = indexAt(event->pos()).row();
      break;

    case QAbstractItemView::BelowItem:
      position = indexAt(event->pos()).row() + 1;
      break;

    case QAbstractItemView::OnViewport:
      position = count();
      break;
  }

  if (event->mimeData()->hasFormat("datumstudio/particleemittermodelitem"))
  {
    QByteArray encoded = event->mimeData()->data("datumstudio/particleemittermodelitem");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
      int index;

      stream >> index;

      m_document.move_emitter(index, position);
    }
  }

  event->setDropAction(Qt::IgnoreAction);
  event->accept();

  QListWidget::dropEvent(event);
}


///////////////////////// EmitterListWidget::contextmenu ////////////////////
void EmitterListWidget::on_contextmenu_requested(QPoint pos)
{
  QMenu menu;

  menu.addAction(m_createaction);

  if (currentRow() != -1)
  {
    menu.addAction(m_renameaction);
    menu.addAction(m_deleteaction);
  }

  menu.exec(QCursor::pos());
}


///////////////////////// EmitterListWidget::Create /////////////////////////
void EmitterListWidget::on_Create_triggered()
{
  m_document.add_emitter(count(), "New Emitter");

  setCurrentRow(count() - 1);

  editItem(currentItem());
}


///////////////////////// EmitterListWidget::Rename /////////////////////////
void EmitterListWidget::on_Rename_triggered()
{
  setFocus();

  editItem(currentItem());
}


///////////////////////// EmitterListWidget::Delete /////////////////////////
void EmitterListWidget::on_Delete_triggered()
{
  if (currentRow() != -1)
  {
    if (QMessageBox::question(this, "Remove Emitter", "Remove Selected Emitter\n\nSure ?", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
    {
      m_document.erase_emitter(currentRow());

      emit selection_changed(currentRow());
    }
  }
}
