//
// Tree View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "treeview.h"
#include <QDragMoveEvent>
#include <QLineEdit>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <cassert>

#include <QtDebug>

using namespace std;

namespace
{
  void add_folder_contents(PackModel *model, PackModel::Node *group, size_t index, QDir const &path)
  {
    auto folder = model->add_group(group, index, path.dirName());

    for(auto &entry : path.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
      add_folder_contents(model, folder, folder->children(), entry.filePath());
    }

    for(auto &entry : path.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name))
    {
      model->add_asset(folder, folder->children(), entry.filePath());
    }
  }

}


//|---------------------- TreeModel -----------------------------------------
//|--------------------------------------------------------------------------

class TreeModel : public QAbstractItemModel
{
  public:
    TreeModel(PackModel *model);

    PackModel::Node *node(QModelIndex const &index) const
    {
      if (!index.isValid())
        return m_model->root();

      return static_cast<PackModel::Node*>(index.internalPointer());
    }

    QModelIndex index(PackModel::Node *node) const
    {       
      if (node == m_model->root())
        return QModelIndex();

      int row = 0;
      while (node->parent()->child(row) != node)
        ++row;

      return createIndex(row, 0, node);
    }

    QModelIndex index(int row, int column, QModelIndex const &parent) const;
    QModelIndex parent(QModelIndex const &child) const;

    int rowCount(QModelIndex const &index) const;
    int columnCount(QModelIndex const &index) const;

    QVariant data(QModelIndex const &index, int role) const;

    Qt::ItemFlags flags(QModelIndex const &index) const;

  protected:

    void on_model_reset();
    void on_model_adding(PackModel::Node *parent, size_t index);
    void on_model_added(PackModel::Node *node);
    void on_model_changed(PackModel::Node *node, PackModel::DataRole role);
    void on_model_removing(PackModel::Node *parent, size_t index);
    void on_model_removed(PackModel::Node *node);

  protected:

    QStringList mimeTypes() const;
    QMimeData *mimeData(QModelIndexList const &indexes) const;
    Qt::DropActions supportedDropActions() const;
    bool dropMimeData(QMimeData const *data, Qt::DropAction action, int row, int column, QModelIndex const &parent);

  private:

    PackModel *m_model;
};


///////////////////////// TreeModel::Constructor ////////////////////////////
TreeModel::TreeModel(PackModel *model)
{
  m_model = model;

  connect(m_model, &PackModel::reset, this, &TreeModel::on_model_reset);
  connect(m_model, &PackModel::adding, this, &TreeModel::on_model_adding);
  connect(m_model, &PackModel::added, this, &TreeModel::on_model_added);
  connect(m_model, &PackModel::changed, this, &TreeModel::on_model_changed);
  connect(m_model, &PackModel::removing, this, &TreeModel::on_model_removing);
  connect(m_model, &PackModel::removed, this, &TreeModel::on_model_removed);

  on_model_reset();
}


///////////////////////// TreeModel::index //////////////////////////////////
QModelIndex TreeModel::index(int row, int column, QModelIndex const &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  return createIndex(row, column, node(parent)->child(row));
}


///////////////////////// TreeModel::parent /////////////////////////////////
QModelIndex TreeModel::parent(QModelIndex const &child) const
{
  if (!child.isValid() || node(child) == m_model->root())
    return QModelIndex();

  return index(node(child)->parent());
}


///////////////////////// TreeModel::rowCount ///////////////////////////////
int TreeModel::rowCount(QModelIndex const &index) const
{
  if (index.column() > 0)
    return 0;

  return node(index)->children();
}


///////////////////////// TreeModel::columnCount ////////////////////////////
int TreeModel::columnCount(QModelIndex const &index) const
{
  return 1;
}


///////////////////////// TreeModel::data ///////////////////////////////////
QVariant TreeModel::data(QModelIndex const &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  QVariant result;

  if (role == Qt::DecorationRole)
  {
    if (node_cast<PackModel::Group>(node(index)))
    {
      result = QIcon(":/packplugin/folder.png");
    }

    if (auto asset = node_cast<PackModel::Asset>(node(index)))
    {
      if (asset->document())
        result = QIcon(":/packplugin/asset.png");
      else
        result = QIcon(":/packplugin/error.png");
    }
  }

  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    if (auto group = node_cast<PackModel::Group>(node(index)))
    {
      result = group->name();
    }

    if (auto asset = node_cast<PackModel::Asset>(node(index)))
    {
      result = asset->name();
    }
  }

  return result;
}


///////////////////////// TreeModel::flags //////////////////////////////////
Qt::ItemFlags TreeModel::flags(QModelIndex const &index) const
{
  if (!index.isValid())
    return Qt::ItemIsDropEnabled;

  Qt::ItemFlags result;

  if (node_cast<PackModel::Group>(node(index)))
  {
    result = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  }

  if (node_cast<PackModel::Asset>(node(index)))
  {
    result = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
  }

  return result;
}


///////////////////////// TreeModel::on_model_reset /////////////////////////
void TreeModel::on_model_reset()
{
  beginResetModel();
  endResetModel();
}


///////////////////////// TreeModel::on_model_adding ////////////////////////
void TreeModel::on_model_adding(PackModel::Node *parent, size_t row)
{
  beginInsertRows(index(parent), row, row);
}


///////////////////////// TreeModel::on_model_added /////////////////////////
void TreeModel::on_model_added(PackModel::Node *node)
{
  endInsertRows();
}


///////////////////////// TreeModel::on_model_changed ///////////////////////
void TreeModel::on_model_changed(PackModel::Node *node, PackModel::DataRole role)
{
  emit dataChanged(index(node), index(node));
}


///////////////////////// TreeModel::on_model_removing //////////////////////
void TreeModel::on_model_removing(PackModel::Node *parent, size_t row)
{
  beginRemoveRows(index(parent), row, row);
}


///////////////////////// TreeModel::on_model_removed ///////////////////////
void TreeModel::on_model_removed(PackModel::Node *node)
{
  endRemoveRows();
}


///////////////////////// TreeModel::mimeTypes ///////////////////////////////
QStringList TreeModel::mimeTypes() const
{
  return { "datumstudio/packmodelitem", "text/uri-list" };
}


///////////////////////// TreeModel::mimeData ////////////////////////////////
QMimeData *TreeModel::mimeData(QModelIndexList const &indexes) const
{
  QMimeData *mimedata = new QMimeData();

  QByteArray encoded;
  QDataStream stream(&encoded, QIODevice::WriteOnly);

  for(auto &index : indexes)
  {
    stream << index.row() << index.column() << index.internalId();
  }

  mimedata->setData("datumstudio/packmodelitem", encoded);

  return mimedata;
}


///////////////////////// TreeModel::supportedDropActions ////////////////////
Qt::DropActions TreeModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}


///////////////////////// TreeModel::dropMimeData ///////////////////////////
bool TreeModel::dropMimeData(QMimeData const *data, Qt::DropAction action, int row, int column, QModelIndex const &parent)
{
  int position = row;
  PackModel::Node *group = node(parent);

  if (row == -1)
  {
    position = group->children();
  }

  if (data->hasFormat("datumstudio/packmodelitem"))
  {
    QByteArray encoded = data->data("datumstudio/packmodelitem");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
      int row; stream >> row;
      int column; stream >> column;
      quintptr node; stream >> node;

      m_model->move(reinterpret_cast<PackModel::Node*>(node), group, position);

      position = (position <= row) ? position + 1 : position;
    }
  }

  if (data->hasUrls())
  {
    for(auto &url : data->urls())
    {
      QString src = url.toLocalFile();

      if (QFileInfo(src).isDir())
      {
        add_folder_contents(m_model, group, position, src);
      }
      else
      {
        m_model->add_asset(group, position, src);
      }

      position += 1;
    }
  }

  return false;
}


//|---------------------- TreeView ------------------------------------------
//|--------------------------------------------------------------------------
//| Tree View
//|

///////////////////////// TreeView::Constructor /////////////////////////////
TreeView::TreeView(QWidget *parent)
  : QTreeView(parent)
{
  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, &QTreeView::doubleClicked, this, &TreeView::itemTriggered);
}


///////////////////////// TreeView::set_model ///////////////////////////////
void TreeView::set_model(PackModel *model)
{
  setModel(new TreeModel(model));
}


///////////////////////// TreeView::current_node ////////////////////////////
PackModel::Node *TreeView::current_node() const
{
  return currentIndex().isValid() ? static_cast<PackModel::Node*>(currentIndex().internalPointer()) : nullptr;
}


///////////////////////// TreeView::selected_nodes //////////////////////////
QList<PackModel::Node*> TreeView::selected_nodes() const
{
  QList<PackModel::Node*> nodes;

  for(auto &index : selectedIndexes())
  {
    nodes.push_back(static_cast<PackModel::Node*>(index.internalPointer()));
  }

  return nodes;
}


///////////////////////// TreeView::trigger_rename //////////////////////////
void TreeView::trigger_rename(QModelIndex const &index)
{
  setFocus();

  setCurrentIndex(index);

  edit(index);
}


///////////////////////// TreeView::currentChanged //////////////////////////
void TreeView::currentChanged(QModelIndex const &current, QModelIndex const &previous)
{
  QAbstractItemView::currentChanged(current, previous);

  emit current_changed(current_node());
}


///////////////////////// TreeView::renamed /////////////////////////////////
void TreeView::commitData(QWidget *editor)
{
  if (currentIndex().isValid())
  {
    QString text = qobject_cast<QLineEdit*>(editor)->text();

    if (text != model()->data(currentIndex()))
    {
      closePersistentEditor(currentIndex());

      emit item_renamed(static_cast<PackModel::Node*>(currentIndex().internalPointer()), text);
    }
  }
}


///////////////////////// TreeView::triggered ///////////////////////////////
void TreeView::itemTriggered(QModelIndex const &index)
{
  emit item_triggered(static_cast<PackModel::Node*>(index.internalPointer()));
}


///////////////////////// TreeView::dragEnterEvent //////////////////////////
void TreeView::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("datumstudio/packmodelitem") && event->source() == this)
  {
    event->accept();
  }

  if (event->mimeData()->hasUrls() && event->source())
  {
    event->accept();
  }

  if (!event->isAccepted())
    return;

  QTreeView::dragEnterEvent(event);
}
