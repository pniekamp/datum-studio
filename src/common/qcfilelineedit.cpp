//
// File Line Edit
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "qcfilelineedit.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>
#include <QSettings>

#include <QtDebug>

using namespace std;


////////////////// LineEdit::Constructor /////////////////
QcFileLineEdit::LineEdit::LineEdit(QWidget *parent)
  : QLineEdit(parent)
{
}


////////////////// LineEdit::dragEnterEvent //////////////
void QcFileLineEdit::LineEdit::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasUrls())
  {
    event->acceptProposedAction();
  }
}


////////////////// LineEdit::dropEvent ///////////////////
void QcFileLineEdit::LineEdit::dropEvent(QDropEvent *event)
{
  if (event->mimeData()->hasUrls())
  {
    setText(event->mimeData()->urls()[0].toLocalFile());
  }
}



//-------------------- QcFileLineEdit ---------------------------------------
//---------------------------------------------------------------------------


////////////////////// QcFileLineEdit::Constructor //////////////////////////
QcFileLineEdit::QcFileLineEdit(QWidget *parent)
  : QWidget(parent)
{
  m_type = BrowseType::OpenFile;

  m_path = new LineEdit(this);

  m_browse = new QPushButton(this);
  m_browse->setText("...");
  m_browse->setMaximumSize(QSize(25, 26));

  setLayout(new QHBoxLayout);
  layout()->setContentsMargins(0, 0, 0, 0);
  layout()->addWidget(m_path);
  layout()->addWidget(m_browse);

  connect(m_path, &QLineEdit::textChanged, this, &QcFileLineEdit::textChanged);
  connect(m_browse, &QPushButton::clicked, this, &QcFileLineEdit::on_browse_clicked);
}


////////////////////// QcFileLineEdit::set_browsetype ///////////////////////
void QcFileLineEdit::set_browsetype(BrowseType type, QString const &filter)
{
  m_type = type;
  m_filter = filter;
}


////////////////////// QcFileLineEdit::set_browsetext ///////////////////////
void QcFileLineEdit::set_browsetext(QString const &text)
{
  m_browse->setText(text);
  m_browse->setMaximumSize(QSize(text.size()*8+12, 26));
}


////////////////////// QcFileLineEdit::set_rememberkey //////////////////////
void QcFileLineEdit::set_rememberkey(QString const &key)
{
  m_rememberkey = key;
}


////////////////////// QcFileLineEdit::browse_clicked ///////////////////////
void QcFileLineEdit::on_browse_clicked()
{
  QSettings reg;

  QString basepath = m_path->text();

  if (basepath == "" && m_rememberkey != "")
    basepath = reg.value(m_rememberkey).toString();

  QString path;

  switch (m_type)
  {
    case BrowseType::OpenFile:
      path = QFileDialog::getOpenFileName(this, "Select Input", basepath, m_filter);
      break;

    case BrowseType::SaveFile:
      path = QFileDialog::getSaveFileName(this, "Select Output", basepath, m_filter);
      break;

    case BrowseType::Directory:
      path = QFileDialog::getExistingDirectory(this, "Select Location", basepath);
      break;
  }

  if (path != "")
  {
    m_path->setText(path);

    if (m_rememberkey != "")
      reg.setValue(m_rememberkey, QFileInfo(path).path());
  }
}
