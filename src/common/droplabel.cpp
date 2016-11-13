//
// Drop Label
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "droplabel.h"
#include "documentapi.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>

#include <QtDebug>

using namespace std;

//-------------------- DropLabel --------------------------------------------
//---------------------------------------------------------------------------

////////////////////// DropLabel::Constructor ///////////////////////////////
DropLabel::DropLabel(QWidget *parent)
  : QLabel(parent)
{
}


////////////////////// DropLabel::Constructor ///////////////////////////////
DropLabel::DropLabel(QString const &text, QWidget *parent)
  : QLabel(text, parent)
{
  m_placeholder = text;
}


////////////////////// DropLabel::Destructor ////////////////////////////////
DropLabel::~DropLabel()
{
}


////////////////////// DropLabel::set_droptype //////////////////////////////
void DropLabel::set_droptype(QString const &droptype)
{
  m_droptype = droptype;
}


////////////////////// DropLabel::setText ///////////////////////////////////
void DropLabel::setText(QString const &text)
{
  m_placeholder = text;
}


////////////////////// DropLabel::setPixmap /////////////////////////////////
void DropLabel::setPixmap(Studio::Document const *document)
{
  if (document)
  {
    QLabel::setPixmap(document->icon().pixmap(48, 48));
  }
  else
  {
    QLabel::setText(m_placeholder);
  }
}


////////////////////// DropLabel::dragEnterEvent ////////////////////////////
void DropLabel::dragEnterEvent(QDragEnterEvent *event)
{
  if (!event->source())
    return;

  if (!(event->possibleActions() & Qt::CopyAction))
    return;

  if (event->dropAction() != Qt::CopyAction)
  {
    event->setDropAction(Qt::CopyAction);
  }

  if (event->mimeData()->hasUrls() && m_droptype == "")
  {
    event->accept();
  }

  if (event->mimeData()->urls().size() == 1 && m_droptype != "")
  {
    auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

    if (auto document = documentmanager->open(event->mimeData()->urls().at(0).toLocalFile()))
    {
      if (document->type() == m_droptype)
      {
        event->accept();
      }

      documentmanager->close(document);
    }
  }
}


////////////////////// DropLabel::dropEvent ////////////////////////////////
void DropLabel::dropEvent(QDropEvent *event)
{
  for(auto &url : event->mimeData()->urls())
  {
    emit itemDropped(url.toLocalFile());
  }
}
