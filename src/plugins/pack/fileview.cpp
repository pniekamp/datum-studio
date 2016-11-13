//
// File View
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "fileview.h"
#include <QVBoxLayout>

#include <QtDebug>

using namespace std;

//|---------------------- FileView ------------------------------------------
//|--------------------------------------------------------------------------
//| File View
//|

///////////////////////// FileView::Constructor /////////////////////////////
FileView::FileView(QWidget *parent)
  : QWidget(parent)
{
  m_view = nullptr;

  setLayout(new QVBoxLayout);

  layout()->setMargin(0);
}


///////////////////////// FileView::set_asset ///////////////////////////////
void FileView::set_asset(PackModel::Node *node)
{
  if (m_view)
  {
    delete m_view;

    m_view = nullptr;
  }

  if (node)
  {
    auto asset = node_cast<PackModel::Asset>(node);

    if (asset && asset->document())
    {
      auto viewfactory = Studio::Core::instance()->find_object<Studio::ViewFactory>();

      m_view = viewfactory->create_view(asset->document()->metadata("type", QString("Binary")));

      if (m_view)
      {
        layout()->addWidget(m_view);

        QMetaObject::invokeMethod(m_view, "view", Q_ARG(Studio::Document*, asset->document()));
      }
    }
  }
}
