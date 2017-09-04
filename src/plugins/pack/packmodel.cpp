//
// Pack Model
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "packmodel.h"
#include <leap.h>
#include <QDir>
#include <fstream>
#include <cassert>

#include <QtDebug>

using namespace std;
using namespace leap;

//|---------------------- Node --------------------------------------
//|------------------------------------------------------------------

///////////////////// Node::Constructor /////////////////////////////
PackModel::Node::Node()
{
  m_parent = nullptr;
  m_nextsibling = nullptr;
}


///////////////////// Node::Destructor //////////////////////////////
PackModel::Node::~Node()
{
  for(auto &child : m_children)
    delete child;
}


///////////////////// Node::insert //////////////////////////////////
PackModel::Node *PackModel::Node::insert(size_t index, Node *node)
{
  node->m_parent = this;
  node->m_nextsibling = nullptr;

  auto j = m_children.insert(m_children.begin() + index, node);

  if (j != m_children.begin())
    (*prev(j))->m_nextsibling = node;

  if (next(j) != m_children.end())
    node->m_nextsibling = *next(j);

  return node;
}


///////////////////// Node::remove //////////////////////////////////
PackModel::Node *PackModel::Node::remove(Node *node)
{
  auto j = find(m_children.begin(), m_children.end(), node);

  if (j != m_children.end())
  {
    if (j != m_children.begin())
      (*prev(j))->m_nextsibling = node->m_nextsibling;

    node->m_parent = nullptr;
    node->m_nextsibling = nullptr;

    m_children.erase(j);

    return node;
  }

  return nullptr;
}


//|---------------------- Asset -------------------------------------
//|------------------------------------------------------------------

///////////////////////// Asset::Constructor ////////////////////////
PackModel::Asset::Asset(QString const &path)
{
  m_path = path;
  m_document = Studio::Core::instance()->find_object<Studio::DocumentManager>()->open(path);
}


///////////////////////// Asset::name ///////////////////////////////
QString PackModel::Asset::name() const
{
  return QFileInfo(m_path).completeBaseName();
}


///////////////////////// Asset::fullname /////////////////////////
QString PackModel::Asset::fullname() const
{
  QString name = QFileInfo(m_path).completeBaseName();

  for(auto node = parent(); node && node->parent(); node = node->parent())
  {
    if (auto group = node_cast<Group>(node))
    {
      name = group->name() + "/" + name;
    }
  }

  return name;
}


///////////////////////// Asset::set_data ///////////////////////////
void PackModel::Asset::set_data(PackModel::DataRole role, QVariant const &value)
{
}


//|---------------------- Group -------------------------------------
//|------------------------------------------------------------------

///////////////////////// Group::Constructor ////////////////////////
PackModel::Group::Group(QString const &name)
{
  m_name = name;
}


///////////////////////// Group::set_data ///////////////////////////
void PackModel::Group::set_data(PackModel::DataRole role, QVariant const &value)
{
  if (role == DataRole::Name)
  {
    m_name = value.toString();
  }
}


//|---------------------- PackModel -----------------------------------------
//|--------------------------------------------------------------------------
//| Pack Model
//|

///////////////////////// PackModel::Constructor ////////////////////////////
PackModel::PackModel(QObject *parent)
  : QObject(parent)
{
  clear();

  m_modified = false;

  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  connect(documentmanager, &Studio::DocumentManager::document_renamed, this, &PackModel::on_document_renamed);
}


///////////////////////// PackModel::clear //////////////////////////////////
void PackModel::clear()
{
  m_root = unique_ptr<Group>(new Group("Root"));

  m_parameters.clear();

  emit reset();

  m_modified = true;
}


///////////////////////// PackModel::add_group //////////////////////////////
PackModel::Node *PackModel::add_group(Node *parent, size_t index, QString const &name)
{
  emit adding(parent, index);

  Node *node = parent->insert(index, new Group(name));

  emit added(node);

  m_modified = true;

  return node;
}



///////////////////////// PackModel::add_asset //////////////////////////////
PackModel::Node *PackModel::add_asset(Node *parent, size_t index, QString const &path)
{
  emit adding(parent, index);

  Node *node = parent->insert(index, new Asset(path));

  emit added(node);

  m_modified = true;

  return node;
}


///////////////////////// PackModel::move ///////////////////////////////////
void PackModel::move(Node *node, Node *parent, size_t index)
{
  auto oldparent = node->parent();

  size_t oldindex = 0;
  while (oldparent->child(oldindex) != node)
    ++oldindex;

  emit removing(oldparent, oldindex);

  oldparent->remove(node);

  emit removed(node);

  if (parent == oldparent && oldindex < index)
    --index;

  emit adding(parent, index);

  parent->insert(index, node);

  emit added(node);

  m_modified = true;
}


///////////////////////// PackModel::erase //////////////////////////////////
void PackModel::erase(Node *node)
{
  auto parent = node->parent();

  size_t index = 0;
  while (parent->child(index) != node)
    ++index;

  emit removing(parent, index);

  parent->remove(node);

  emit removed(node);

  delete node;

  m_modified = true;
}


///////////////////////// PackModel::set_data ///////////////////////////////
void PackModel::set_data(Node *node, DataRole role, QVariant const &value)
{
  node->set_data(role, value);

  emit changed(node, role);
}


///////////////////////// PackModel::set_parameter //////////////////////////
void PackModel::set_parameter(QString const &name, QString const &value)
{
  if (m_parameters[name] != value)
  {
    m_parameters[name] = value;

    m_modified = true;
  }
}


///////////////////////// PackModel::document_renamed ///////////////////////
void PackModel::on_document_renamed(Studio::Document *document, QString const &src, QString const &dst)
{
  for(auto &node : nodes())
  {
    if (auto asset = node_cast<Asset>(node))
    {
      if (asset->document() == document)
      {
        asset->m_path = dst;

        emit changed(asset, DataRole::Path);

        m_modified = true;
      }
    }
  }
}


///////////////////////// PackModel::load ///////////////////////////////////
void PackModel::load(string const &projectfile)
{
  clear();

  ifstream fin(projectfile);

  string buffer;

  while (getline(fin, buffer))
  {
    buffer = trim(buffer);

    if (buffer == "[Pack]")
      break;
  }

  QDir base = QFileInfo(projectfile.c_str()).dir();

  vector<Node*> groupstack = { root() };

  while (getline(fin, buffer))
  {
    buffer = trim(buffer);

    if (buffer.empty())
      break;

    if (buffer[0] == '#' || buffer[0] == '/')
      continue;

    if (buffer[0] == '<' && buffer[buffer.length()-1] == '>')
    {
      if (buffer.substr(1, 9) == "Parameter")
      {
        auto name = string(buffer.begin() + buffer.find_first_of("'") + 1, buffer.begin() + buffer.find_last_of("'"));
        auto value = string(buffer.begin() + name.length() + 19, buffer.begin() + buffer.length() - 12);

        set_parameter(name.c_str(), value.c_str());
      }

      if (buffer.substr(1, 5) == "Group")
      {
        auto group = add_group(groupstack.back(), groupstack.back()->children(), buffer.substr(13, buffer.length()-15).c_str());

        groupstack.push_back(group);
      }

      if (buffer.substr(1, 6) == "/Group")
      {
        groupstack.pop_back();
      }

      continue;
    }

    add_asset(groupstack.back(), groupstack.back()->children(), base.filePath(buffer.c_str()));
  }

  m_modified = false;
}


///////////////////////// PackModel::save ///////////////////////////////////
void PackModel::save(string const &projectfile)
{
  ofstream fout(projectfile, ios::app);

  fout << "[Pack]" << '\n';

  for(auto i = m_parameters.begin(); i != m_parameters.end(); ++i)
  {
    fout << "<Parameter name='" << i.key().toStdString() << "'>" << i.value().toStdString() << "</Parameter>" << '\n';
  }

  QDir base = QFileInfo(projectfile.c_str()).dir();

  vector<Node*> groupstack = { root() };

  for(auto &node : nodes())
  {
    if (node == root())
      continue;

    while (node->parent() != groupstack.back())
    {
      fout << "</Group>" << '\n';

      groupstack.pop_back();
    }

    if (auto group = node_cast<Group>(node))
    {
      fout << "<Group name='" << group->name().toStdString() << "'>" << '\n';

      groupstack.push_back(group);
    }

    if (auto asset = node_cast<Asset>(node))
    {
      fout << base.relativeFilePath(asset->path()).toStdString() << '\n';
    }
  }

  while (groupstack.size() > 1)
  {
    fout << "</Group>" << '\n';

    groupstack.pop_back();
  }

  fout << '\n';

  m_modified = false;
}
