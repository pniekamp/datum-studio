//
// Animation Properties
//

//
// Copyright (C) 2016 Peter Niekamp
//

#include "animationproperties.h"
#include "contentapi.h"
#include "assetfile.h"

#include <QDebug>

using namespace std;
using namespace lml;

//|---------------------- AnimationProperties -------------------------------
//|--------------------------------------------------------------------------

///////////////////////// AnimationProperties::Constructor //////////////////
AnimationProperties::AnimationProperties(QWidget *parent)
  : QDockWidget(parent)
{
  ui.setupUi(this);

  ui.ImportSrc->set_browsetype(QcFileLineEdit::BrowseType::OpenFile);
}


///////////////////////// AnimationProperties::edit /////////////////////////
void AnimationProperties::edit(Studio::Document *document)
{
  m_document = document;

  connect(&m_document, &AnimationDocument::document_changed, this, &AnimationProperties::refresh);

  refresh();
}


///////////////////////// AnimationProperties::refresh //////////////////////
void AnimationProperties::refresh()
{
  int joints = 0;

  m_document->lock();

  PackAnimationHeader anim;

  if (read_asset_header(m_document, 1, &anim))
  {
    joints += anim.jointcount;
  }

  ui.Duration->setText(QString::number(anim.duration));

  ui.Joints->setText(QString::number(joints));

  ui.ImportSrc->setText(m_document->metadata("src").toString());

  m_document->unlock();
}


///////////////////////// AnimationProperties::Reimport /////////////////////
void AnimationProperties::on_Reimport_clicked()
{
  if (ui.ImportSrc->text() == "")
    return;

  m_document->lock_exclusive();

  m_document->set_metadata("src", ui.ImportSrc->text());

  m_document->save();

  m_document->unlock_exclusive(false);

  auto contentmanager = Studio::Core::instance()->find_object<Studio::ContentManager>();
  auto documentmanager = Studio::Core::instance()->find_object<Studio::DocumentManager>();

  contentmanager->reimport(documentmanager->path(m_document));
}
