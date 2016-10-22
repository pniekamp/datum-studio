//
// QcRibbon
//

#include "qcribbon.h"
#include <QStyle>
#include <QStyleOptionMenuItem>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabBar>
#include <QPushButton>
#include <QPainter>
#include <QActionEvent>
#include <QWidgetAction>
#include <QDesktopWidget>
#include <QFrame>
#include <QGroupBox>
#include <QToolButton>
#include <QCheckBox>
#include <QMainWindow>

#include <QDebug>

using namespace std;

static char const *up_xpm[] = {
"16 16 2 1",
" 	c None",
".	c #C9C9C9",
"                ",
"                ",
"                ",
"                ",
"       ..       ",
"      ....      ",
"     ......     ",
"    ........    ",
"    ...  ...    ",
"    ..    ..    ",
"    .      .    ",
"                ",
"                ",
"                ",
"                ",
"                "};


//---------------- QcRibbonItem -------------------------
//-------------------------------------------------------
class QcRibbonItem : public QWidgetItem
{
  public:
    QcRibbonItem(QAction *action, QWidget *widget)
      : QWidgetItem(widget),
        m_Action(action)
    {
    }

    QAction *action() const { return m_Action; }

  private:

    QAction *m_Action;
};


//---------------- QcRibbonMenu -------------------------
//-------------------------------------------------------
QcRibbonMenu::QcRibbonMenu(QWidget *parent)
  : QMenu(parent)
{
}

void QcRibbonMenu::setDefaultAction(QAction *action)
{
  QMenu::setDefaultAction(action);

  emit defaultActionChanged(action);
}


//---------------- QcRibbonSeparator --------------------
//-------------------------------------------------------
QcRibbonSeparator::QcRibbonSeparator(QWidget *parent)
  : QWidget(parent)
{
  setMinimumSize(10, 0);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}


//---------------- QcRibbonIcon -------------------------
//-------------------------------------------------------
QcRibbonIcon::QcRibbonIcon(QAction *action, QWidget *parent)
  : QToolButton(parent)
{
  setAutoRaise(true);
  setFocusPolicy(Qt::NoFocus);
  setIconSize(QSize(20, 20));
  setToolButtonStyle(Qt::ToolButtonIconOnly);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

  setDefaultAction(action);

  setPopupMode(QToolButton::InstantPopup);

  action->setProperty("RibbonRowSpan", 1.5);
}


//---------------- QcRibbonButton -----------------------
//-------------------------------------------------------
QcRibbonButton::QcRibbonButton(QAction *action, QWidget *parent)
  : QToolButton(parent)
{
  setAutoRaise(true);
  setFocusPolicy(Qt::NoFocus);
  setIconSize(QSize(16, 16));
  setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

  setDefaultAction(action);

  setPopupMode(QToolButton::InstantPopup);
}


//---------------- QcRibbonLargeButton ------------------
//-------------------------------------------------------
QcRibbonLargeButton::QcRibbonLargeButton(QAction *action, QWidget *parent)
  : QToolButton(parent)
{
  setAutoRaise(true);
  setFocusPolicy(Qt::NoFocus);
  setIconSize(QSize(28, 28));
  setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

  setDefaultAction(action);

  setPopupMode(QToolButton::InstantPopup);

  action->setProperty("RibbonRowSpan", 3);
}


//---------------- QcRibbonDropButton -------------------
//-------------------------------------------------------
QcRibbonDropButton::QcRibbonDropButton(QAction *action, QWidget *parent)
  : QToolButton(parent)
{
  setAutoRaise(true);
  setFocusPolicy(Qt::NoFocus);
  setIconSize(QSize(28, 28));
  setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

  if (action->menu())
  {
    setMenu(action->menu());

    connect(qobject_cast<QcRibbonMenu*>(menu()), &QcRibbonMenu::defaultActionChanged, this, &QcRibbonDropButton::defaultActionChanged);
  }

  setPopupMode(QToolButton::MenuButtonPopup);

  action->setProperty("RibbonRowSpan", 3);
}

void QcRibbonDropButton::defaultActionChanged(QAction *action)
{
  setDefaultAction(action);
}


//---------------- QcRibbonCheckBox ---------------------
//-------------------------------------------------------
QcRibbonCheckBox::QcRibbonCheckBox(QAction *action, QWidget *parent)
  : QCheckBox(parent)
{
  addAction(action);
}

void QcRibbonCheckBox::actionEvent(QActionEvent *event)
{
  if (event->type() == QEvent::ActionAdded)
  {
    connect(this, &QCheckBox::clicked, event->action(), &QAction::trigger);
  }

  if (event->type() == QEvent::ActionChanged)
  {
    setText(event->action()->text());
    setToolTip(event->action()->toolTip());
    setStatusTip(event->action()->statusTip());
    setWhatsThis(event->action()->whatsThis());
    setChecked(event->action()->isChecked());
    setEnabled(event->action()->isEnabled());
  }

  QCheckBox::actionEvent(event);
}



//-------------------- QcRibbonTab ------------------------------------------
//---------------------------------------------------------------------------

////////////////////// QcRibbonTab::Constructor /////////////////////////////
QcRibbonTab::QcRibbonTab(QWidget *parent)
  : QMenu(parent)
{
  setWindowFlags(0);

  m_layout = new QHBoxLayout;

  setLayout(m_layout);

  m_layout->setMargin(0);
  m_layout->setSpacing(0);
}


////////////////////// QcRibbonTab::addWidget ///////////////////////////////
QAction *QcRibbonTab::addWidget(QWidget *widget)
{
  QWidgetAction *action = new QWidgetAction(this);
  action->setDefaultWidget(widget);

  addAction(action);

  return action;
}


////////////////////// QcRibbonTab::insertWidget ////////////////////////////
QAction *QcRibbonTab::insertWidget(QAction *before, QWidget *widget)
{
  QWidgetAction *action = new QWidgetAction(this);
  action->setDefaultWidget(widget);

  insertAction(before, action);

  return action;
}


////////////////////// QcRibbonTab::event ///////////////////////////////////
bool QcRibbonTab::event(QEvent *event)
{
  switch(event->type())
  {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
      {
        hide();

        event->accept();
      }
      return true;

    case QEvent::MouseButtonPress:
      if (!rect().contains(static_cast<QMouseEvent*>(event)->pos()))
      {
        hide();
      }
      return true;

    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
      return true;

    case QEvent::Close:
      setAttribute(Qt::WA_NoMouseReplay);
      break;

    case QEvent::Wheel:
      return true;

    default:
      break;
  }

  return QWidget::event(event);
}


////////////////////// QcRibbonTab::popup ///////////////////////////////////
void QcRibbonTab::popup(const QPoint &pos, QAction *at)
{
  ensurePolished();

  emit aboutToShow();

  setGeometry(QRect(pos, QSize(parentWidget()->width(), max(height(), sizeHint().height()))));

  show();
}


////////////////////// QcRibbonTab::actionTriggered /////////////////////////
void QcRibbonTab::actionTriggered()
{
  if (QAction *action = qobject_cast<QAction*>(sender()))
    emit triggered(action);
}


////////////////////// QcRibbonTab::create_item /////////////////////////////
QcRibbonItem *QcRibbonTab::create_item(QAction *action)
{
  QWidget *widget = NULL;

  if (qobject_cast<QWidgetAction*>(action))
  {
    widget = qobject_cast<QWidgetAction*>(action)->requestWidget(this);

    widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
  }
  else if (action->isSeparator())
  {
    widget = new QWidget(this);
  }
  else
  {
    widget = action->menu();
  }

  if (action->menu())
  {
    connect(action->menu(), &QMenu::triggered, this, &QMenu::triggered);
  }

  connect(action, &QAction::triggered, this, &QcRibbonTab::actionTriggered);

  return new QcRibbonItem(action, widget);
}


////////////////////// QcRibbonTab::destroy_item ////////////////////////////
void QcRibbonTab::destroy_item(QcRibbonItem *item)
{
  m_layout->removeItem(item);

  item->widget()->hide();

  if (item->action()->menu())
    item->action()->menu()->disconnect(this);

  item->action()->disconnect(this);

  if (qobject_cast<QWidgetAction*>(item->action()))
  {
    qobject_cast<QWidgetAction*>(item->action())->releaseWidget(item->widget());
  }
  else if (item->action()->isSeparator())
  {
    item->widget()->deleteLater();
  }
  else
  {
  }

  delete item;
}


////////////////////// QcRibbonTab::update_layout ///////////////////////////
void QcRibbonTab::update_layout()
{
  while (m_layout->count() != 0)
    m_layout->takeAt(0);

  foreach(QAction *action, actions())
  {
    m_layout->addItem(m_items[action]);
  }

  m_layout->addStretch();
}


////////////////////// QcRibbonTab::actionEvent /////////////////////////////
void QcRibbonTab::actionEvent(QActionEvent *event)
{
  QAction *action = event->action();

  switch (event->type())
  {
    case QEvent::ActionAdded:
      m_items.insert(action, create_item(action));
      break;

    case QEvent::ActionChanged:
      break;

    case QEvent::ActionRemoved:
      destroy_item(m_items.take(action));
      break;

    default:
      break;
  }

  update_layout();
}


//////////////////////// QcRibbonTab::paintEvent ////////////////////////////
void QcRibbonTab::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  QStyleOption opt;
  opt.initFrom(this);

  painter.fillRect(rect(), parentWidget()->palette().base());

  style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}



//-------------------- QcRibbonGroup ----------------------------------------
//---------------------------------------------------------------------------

////////////////////// QcRibbonGroup::Constructor ///////////////////////////
QcRibbonGroup::QcRibbonGroup(QWidget *parent)
  : QMenu(parent)
{
  setWindowFlags(0);

  m_layout = new QGridLayout;

  m_layout->setSpacing(0);
  m_layout->setMargin(3);

  setLayout(m_layout);

  setContentsMargins(0, 0, 0, 16);
}


////////////////////// QcRibbonGroup::addWidget /////////////////////////////
QAction *QcRibbonGroup::addWidget(QWidget *widget)
{
  QWidgetAction *action = new QWidgetAction(this);
  action->setDefaultWidget(widget);

  addAction(action);

  return action;
}


////////////////////// QcRibbonGroup::insertWidget //////////////////////////
QAction *QcRibbonGroup::insertWidget(QAction *before, QWidget *widget)
{
  QWidgetAction *action = new QWidgetAction(this);
  action->setDefaultWidget(widget);

  insertAction(before, action);

  return action;
}


////////////////////// QcRibbonGroup::minimumSizeHint ///////////////////////
QSize QcRibbonGroup::minimumSizeHint() const
{
  QStyleOptionGroupBox box;
  box.initFrom(this);

  QFontMetrics tm = fontMetrics();

  return style()->sizeFromContents(QStyle::CT_GroupBox, &box, QSize(tm.width(title())+16, tm.height()), this).expandedTo(QWidget::minimumSizeHint());
}


////////////////////// QcRibbonGroup::event /////////////////////////////////
bool QcRibbonGroup::event(QEvent *event)
{
  switch(event->type())
  {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::Wheel:
      return true;

    default:
      break;
  }

  return QWidget::event(event);
}


////////////////////// QcRibbonGroup::actionTriggered ///////////////////////
void QcRibbonGroup::actionTriggered()
{
  if (QAction *action = qobject_cast<QAction *>(sender()))
    emit triggered(action);
}


////////////////////// QcRibbonGroup::create_item ///////////////////////////
QcRibbonItem *QcRibbonGroup::create_item(QAction *action)
{
  QWidget *widget = NULL;

  if (action->menu())
  {
    foreach(QByteArray property, action->menu()->dynamicPropertyNames())
      action->setProperty(property, action->menu()->property(property));
  }

  if (action->isSeparator())
  {
    widget = new QcRibbonSeparator(this);
  }
  else if (qobject_cast<QWidgetAction*>(action))
  {
    widget = qobject_cast<QWidgetAction*>(action)->requestWidget(this);

    widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
  }
  else if (action->property("RibbonWidget").toString() == "QcRibbonIcon")
  {
    widget = new QcRibbonIcon(action, this);
  }
  else if (action->property("RibbonWidget").toString() == "QcRibbonButton")
  {
    widget = new QcRibbonButton(action, this);
  }
  else if (action->property("RibbonWidget").toString() == "QcRibbonLargeButton")
  {
    widget = new QcRibbonLargeButton(action, this);
  }
  else if (action->property("RibbonWidget").toString() == "QcRibbonDropButton")
  {
    widget = new QcRibbonDropButton(action, this);
  }
  else if (action->property("RibbonWidget").toString() == "QcRibbonCheckBox")
  {
    widget = new QcRibbonCheckBox(action, this);
  }
  else
  {
    widget = new QcRibbonButton(action, this);
  }

  foreach(QByteArray property, action->dynamicPropertyNames())
    widget->setProperty(property, action->property(property));

  if (action->menu())
  {
    connect(action->menu(), &QMenu::triggered, this, &QcRibbonGroup::triggered);
  }

  connect(action, &QAction::triggered, this, &QcRibbonGroup::actionTriggered);

  return new QcRibbonItem(action, widget);
}


////////////////////// QcRibbonGroup::destroy_item //////////////////////////
void QcRibbonGroup::destroy_item(QcRibbonItem *item)
{
  m_layout->removeItem(item);

  item->widget()->hide();

  if (item->action()->menu())
    item->action()->menu()->disconnect(this);

  item->action()->disconnect(this);

  if (qobject_cast<QWidgetAction*>(item->action()))
  {
    qobject_cast<QWidgetAction*>(item->action())->releaseWidget(item->widget());
  }
  else
  {
    item->widget()->deleteLater();
  }

  delete item;
}


////////////////////// QcRibbonGroup::update_layout /////////////////////////
void QcRibbonGroup::update_layout()
{
  while (m_layout->count() != 0)
    m_layout->takeAt(0);

  int row = 0, col = 0;

  foreach(QAction *action, actions())
  {
    float colspan = 1;
    float rowspan = 1;

    if (action->property("RibbonColSpan").isValid())
      colspan = action->property("RibbonColSpan").toFloat();

    if (action->property("RibbonRowSpan").isValid())
      rowspan = action->property("RibbonRowSpan").toFloat();

    if (row + rowspan > 3)
    {
      row = 0;
      col += 1;
    }

    m_layout->addItem(m_items[action], row, col, rowspan, colspan);

    row += rowspan;
  }
}


////////////////////// QcRibbonGroup::actionEvent ///////////////////////////
void QcRibbonGroup::actionEvent(QActionEvent *event)
{
  QAction *action = event->action();

  switch (event->type())
  {
    case QEvent::ActionAdded:
      m_items.insert(action, create_item(action));
      break;

    case QEvent::ActionChanged:
      break;

    case QEvent::ActionRemoved:
      destroy_item(m_items.take(action));
      break;

    default:
      break;
  }

  update_layout();
}


//////////////////////// QcRibbonGroup::paintEvent //////////////////////////
void QcRibbonGroup::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  QStyleOptionGroupBox opt;

  opt.initFrom(this);

  opt.text = title();
  opt.subControls = QStyle::SC_GroupBoxFrame | QStyle::SC_GroupBoxLabel;

  style()->drawComplexControl(QStyle::CC_GroupBox, &opt, &painter, this);
}



//-------------------- QcRibbon ---------------------------------------------
//---------------------------------------------------------------------------

////////////////////// QcRibbon::Constructor ////////////////////////////////
QcRibbon::QcRibbon(QWidget *parent)
  : QMenuBar(parent)
{
  setLayout(new QVBoxLayout);

  m_menubar = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);

  m_collapsebutton = new QPushButton(QIcon(QPixmap(up_xpm)), "", this);

  m_collapsebutton->setObjectName("RibbonCollapseButon");

  m_currenttab = 1;
  m_hoveraction = NULL;
  m_collapsemode = 0;

  layout()->setMargin(0);
  layout()->setSpacing(0);
  layout()->addItem(m_menubar);

  connect(m_collapsebutton, &QPushButton::clicked, this, &QcRibbon::on_collapse);
  connect(this, &QcRibbon::triggered, this, &QcRibbon::actionTriggered);
}


////////////////////// QcRibbon::set_collapse ///////////////////////////////
void QcRibbon::set_collapse(int mode)
{
  if (m_collapsemode != mode)
  {
    m_collapsemode = mode;

    switch (m_collapsemode)
    {
      case 0:
        m_currenttab = 1;
        break;

      case 1:
        m_currenttab = -1;
        break;
    }

    updateGeometries();

    resize(width(), layout()->sizeHint().height());

    emit collapse_changed(mode);
  }
}


////////////////////// QcRibbon::heightForWidth /////////////////////////////
int QcRibbon::heightForWidth(int width) const
{
  return layout()->sizeHint().height();
}


////////////////////// QcRibbon::initStyleOption ////////////////////////////
void QcRibbon::initStyleOption(QStyleOptionTab *option, int index) const
{
  QAction *action = actions()[index];

  option->initFrom(this);
  option->state &= ~(QStyle::State_HasFocus | QStyle::State_MouseOver);
  option->rect = actionGeometry(action);
  option->text = action->text();
  option->icon = action->icon();
  option->position = QStyleOptionTab::Middle;

  if (isActiveWindow())
    option->state |= QStyle::State_Active;

  if (index == m_currenttab)
    option->state |= QStyle::State_Selected;

  if (index == m_currenttab && hasFocus())
    option->state |= QStyle::State_HasFocus;

  if (action == m_hoveraction)
    option->state |= QStyle::State_MouseOver;

  if (index == 0)
    option->position = QStyleOptionTab::Beginning;
}


////////////////////// QcRibbon::updateGeometries ///////////////////////////
void QcRibbon::updateGeometries()
{
  for(int i = 0; i < actions().size(); ++i)
  {
    QAction *action = actions()[i];

    if (qobject_cast<QcRibbonTab*>(action->menu()))
    {
      action->menu()->setWindowFlags((m_collapsemode == 1) ? Qt::Popup : Qt::Widget);
    }

    action->menu()->setMinimumWidth(1);
    action->menu()->setVisible(i == m_currenttab);
  }

  if (QMainWindow *main = qobject_cast<QMainWindow*>(parentWidget()))
  {
    main->centralWidget()->setFocus();
  }

  QSize sz;

  for(int i = 0; i < actions().size(); ++i)
  {
    QStyleOptionTab opt;
    initStyleOption(&opt, i);

    sz = style()->sizeFromContents(QStyle::CT_TabBarTab, &opt, sz, this);
  }

  m_menubar->changeSize(0, sz.height(), QSizePolicy::Fixed, QSizePolicy::Fixed);
}


////////////////////// QcRibbon::event //////////////////////////////////////
bool QcRibbon::event(QEvent *event)
{
  switch(event->type())
  {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      return true;

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
      on_tab_changed(actions().indexOf(m_hoveraction));
      return true;

    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
      return true;

    case QEvent::Wheel:
      return true;

    case QEvent::HoverEnter:
    case QEvent::HoverMove:
      m_hoveraction = actionAt(static_cast<QHoverEvent *>(event)->pos());
      update();
      break;

    case QEvent::HoverLeave:
      m_hoveraction = NULL;
      update();
      break;

    default:
      break;
  }

  return QWidget::event(event);
}


////////////////////// QcRibbon::actionTriggered ////////////////////////////
void QcRibbon::actionTriggered(QAction *action)
{
  if (m_collapsemode == 1)
  {
    if (m_currenttab != -1 && action->menu() == NULL)
    {
      actions()[m_currenttab]->menu()->close();

      m_currenttab = -1;
    }
  }
}


////////////////////// QcRibbon::controlbarHidden ///////////////////////////
void QcRibbon::controlbarHidden()
{
  if (m_collapsemode == 1)
  {
    m_currenttab = -1;
  }

  m_hoveraction = NULL;

  update();
}


////////////////////// QcRibbon::on_tab_changed /////////////////////////////
void QcRibbon::on_tab_changed(int index)
{
  if (index >= 0)
  {
    QAction *action = actions()[index];

    if (isVisible() && action->isEnabled() && action->menu()->isEnabled())
    {
      if (qobject_cast<QcRibbonTab*>(action->menu()))
      {
        if (m_currenttab != -1)
          actions()[m_currenttab]->menu()->hide();

        qobject_cast<QcRibbonTab*>(action->menu())->popup(mapToGlobal(QPoint(0, m_menubar->sizeHint().height())));

        m_currenttab = index;
      }
      else
      {
        action->menu()->popup(mapToGlobal(QPoint(actionGeometry(action).left(), m_menubar->sizeHint().height())));
      }
    }
  }
}


////////////////////// QcRibbon::on_collapse ////////////////////////////////
void QcRibbon::on_collapse()
{
  switch (m_collapsemode)
  {
    case 0:
      set_collapse(1);
      break;

    case 1:
      set_collapse(0);
      break;
  }
}


////////////////////// QcRibbon::actionEvent ////////////////////////////////
void QcRibbon::actionEvent(QActionEvent *event)
{
  QMenuBar::actionEvent(event);

  QAction *action = event->action();

  switch (event->type())
  {
    case QEvent::ActionAdded:
      layout()->addWidget(action->menu());
      connect(action->menu(), &QMenu::triggered, this, &QcRibbon::triggered);
      connect(action->menu(), &QMenu::aboutToHide, this, &QcRibbon::controlbarHidden);
      break;

    case QEvent::ActionChanged:
      break;

    case QEvent::ActionRemoved:
      layout()->removeWidget(action->menu());
      action->menu()->disconnect(this);
      break;

    default:
      break;
  }

  updateGeometries();
}


////////////////////// QcRibbon::changeEvent ////////////////////////////////
void QcRibbon::changeEvent(QEvent *event)
{
  QMenuBar::changeEvent(event);

  if (event->type() == QEvent::StyleChange)
  {
    updateGeometries();
  }
}


////////////////////// QcRibbon::resizeEvent ////////////////////////////////
void QcRibbon::resizeEvent(QResizeEvent *event)
{
  QMenuBar::resizeEvent(event);

  m_collapsebutton->ensurePolished();

  m_collapsebutton->move(width()-m_collapsebutton->width(), 0);

  updateGeometries();
}


////////////////////// QcRibbon::paintEvent /////////////////////////////////
void QcRibbon::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);

  for(int i = 0; i < actions().size(); ++i)
  {
    QStyleOptionTab opt;
    initStyleOption(&opt, i);

    style()->drawControl(QStyle::CE_TabBarTab, &opt, &painter, this);
  }

  painter.setPen(QColor(187, 187, 187));
  painter.drawLine(actionGeometry(actions().last()).right(), m_menubar->geometry().height()-1, rect().right(), m_menubar->geometry().height()-1);
}
