//
// Pack Model
//

//
// Copyright (C) 2016 Peter Niekamp
//

#pragma once

#include "documentapi.h"
#include <vector>
#include <memory>
#include <QObject>

//-------------------------- PackModel --------------------------------------
//---------------------------------------------------------------------------

class PackModel : public QObject
{
  Q_OBJECT

  public:

    enum class NodeType
    {
      Group,
      Asset,
    };

    enum class DataRole
    {
      None = 0,
      Name,
      Path,
    };

    class Node
    {
      public:
        virtual ~Node();
        virtual NodeType type() const = 0;
        virtual std::type_info const &type_info() const = 0;

        Node *parent() { return m_parent; }
        Node const *parent() const { return m_parent; }

        size_t children() const { return m_children.size(); }

        Node *child(size_t index) { return m_children[index]; }
        Node const *child(size_t index) const { return m_children[index]; }

      protected:
        Node();

        Node *insert(size_t index, Node *node);
        Node *remove(Node *node);

        virtual void set_data(DataRole role, QVariant const &data) = 0;

      protected:

        Node *m_parent;
        Node *m_nextsibling;
        std::vector<Node*> m_children;

        friend class PackModel;
    };

    class Asset : public Node
    {
      public:
        NodeType type() const { return NodeType::Asset; }
        std::type_info const &type_info() const { return typeid(*this); }

        QString name() const;
        QString fullname() const;

        QString path() const { return m_path; }

        Studio::Document *document() const { return m_document; }

      protected:
        Asset(QString const &path);

        void set_data(DataRole role, QVariant const &value);

        QString m_path;

        unique_document m_document;

        friend class PackModel;
    };

    class Group : public Node
    {
      public:
        NodeType type() const { return NodeType::Group; }
        std::type_info const &type_info() const { return typeid(*this); }

        QString name() const { return m_name; }

      private:
        Group(QString const &name);

        void set_data(DataRole role, QVariant const &value);

        QString m_name;

        friend class PackModel;
    };

  public:
    PackModel(QObject *parent = 0);

    void clear();

    bool modified() const { return m_modified; }

    Node *add_group(Node *parent, size_t index, QString const &name);
    Node *add_asset(Node *parent, size_t index, QString const &path);

    void move(Node *node, Node *parent, size_t index);

    void erase(Node *node);

  public:

    Node *root() { return m_root.get(); }
    Node const *root() const { return m_root.get(); }

    void set_data(Node *node, DataRole role, QVariant const &value);

  public:

    template<typename Node>
    class deep_normal_iterator
    {
      public:

        bool operator ==(deep_normal_iterator const &that) const { return node == that.node; }
        bool operator !=(deep_normal_iterator const &that) const { return node != that.node; }

        Node const &operator *() const { return node; }
        Node operator ->() const { return node; }

        deep_normal_iterator &operator++()
        {
          if (node->m_children.size() != 0)
          {
            node = node->m_children[0];

            return *this;
          }

          while (node)
          {
            if (node->m_nextsibling)
            {
              node = node->m_nextsibling;

              return *this;
            }

            node = node->m_parent;
          }

          return *this;
        }

        Node node;
    };

    template<typename Iterator>
    class iterator_pair : public std::pair<Iterator, Iterator>
    {
      public:
        using std::pair<Iterator, Iterator>::pair;

        Iterator begin() const { return this->first; }
        Iterator end() const { return this->second; }
    };

    typedef deep_normal_iterator<Node *> deep_iterator;
    typedef deep_normal_iterator<Node const *> const_deep_iterator;


    iterator_pair<deep_iterator> nodes()
    {
      return { deep_iterator{ root() }, deep_iterator{ nullptr } };
    }

    iterator_pair<const_deep_iterator> nodes() const
    {
      return { const_deep_iterator{ root() }, const_deep_iterator{ nullptr } };
    }

  public:

    QString signature() const { return m_parameters["signature"]; }
    QString version() const { return m_parameters["version"]; }

    void set_parameter(QString const &name, QString const &value);

  public:

    void load(std::string const &projectfile);
    void save(std::string const &projectfile);

  signals:

    void reset();
    void added(PackModel::Node *node);
    void changed(PackModel::Node *node, PackModel::DataRole role);
    void removed(PackModel::Node *node);

    void adding(PackModel::Node *parent, size_t index);
    void removing(PackModel::Node *parent, size_t index);

  protected:

    void on_document_renamed(Studio::Document *document, QString const &src, QString const &dst);

  private:

    bool m_modified;

    std::unique_ptr<Node> m_root;

    QMap<QString, QString> m_parameters;
};


///////////////////////// node_cast ////////////////////////////
template<typename T>
T *node_cast(PackModel::Node *node)
{
  return (node->type_info() == typeid(T)) ? static_cast<T*>(node) : nullptr;
}

template<typename T>
T const *node_cast(PackModel::Node const *node)
{
  return (node->type_info() == typeid(T)) ? static_cast<T const *>(node) : nullptr;
}
