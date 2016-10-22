//
// QcRibbon
//

#pragma once

#include <QMenu>
#include <QMenuBar>
#include <QWidgetItem>
#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>

class QTabBar;
class QGroupBox;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QStyleOptionTab;
class QcRibbonItem;


//---------------- QcRibbonMenu ------------------------
//------------------------------------------------------

class QcRibbonMenu : public QMenu
{
  Q_OBJECT

  public:
    QcRibbonMenu(QWidget *parent = 0);

    void setDefaultAction(QAction *action);

  signals:

    void defaultActionChanged(QAction *action);
};


//---------------- QcRibbonTab -------------------------
//------------------------------------------------------

class QcRibbonTab : public QMenu
{
  Q_OBJECT

  public:
    QcRibbonTab(QWidget *parent = 0);

    QAction *addWidget(QWidget *widget);
    QAction *insertWidget(QAction *before, QWidget *widget);

    void popup(const QPoint &pos, QAction *at = 0);

  protected slots:

    void actionTriggered();

  protected:

    virtual bool event(QEvent *event);

    virtual void actionEvent(QActionEvent *event);

    virtual void paintEvent(QPaintEvent *event);

    QcRibbonItem *create_item(QAction *action);

    void destroy_item(QcRibbonItem *item);

    void update_layout();

  private:

    QHBoxLayout *m_layout;

    QMap<QAction*, QcRibbonItem*> m_items;
};


//---------------- QcRibbonGroup -----------------------
//------------------------------------------------------

class QcRibbonGroup : public QMenu
{
  Q_OBJECT

  public:
    QcRibbonGroup(QWidget *parent = 0);

    QAction *addWidget(QWidget *widget);
    QAction *insertWidget(QAction *before, QWidget *widget);

    QSize minimumSizeHint() const;

  protected slots:

    void actionTriggered();

  protected:

    virtual bool event(QEvent *event);

    virtual void actionEvent(QActionEvent *event);

    virtual void paintEvent(QPaintEvent *event);

    QcRibbonItem *create_item(QAction *action);

    void destroy_item(QcRibbonItem *item);

    void update_layout();

  private:

    QGridLayout *m_layout;

    QMap<QAction*, QcRibbonItem*> m_items;
};


//---------------- QcRibbonSeparator --------------------
//-------------------------------------------------------
class QcRibbonSeparator : public QWidget
{
  Q_OBJECT

  public:
    QcRibbonSeparator(QWidget *parent);
};


//---------------- QcRibbonIcon -------------------------
//-------------------------------------------------------
class QcRibbonIcon : public QToolButton
{
  Q_OBJECT

  public:
    QcRibbonIcon(QAction *action, QWidget *parent);
};


//---------------- QcRibbonButton -----------------------
//-------------------------------------------------------
class QcRibbonButton : public QToolButton
{
  Q_OBJECT

  public:
    QcRibbonButton(QAction *action, QWidget *parent);
};


//---------------- QcRibbonLargeButton ------------------
//-------------------------------------------------------
class QcRibbonLargeButton : public QToolButton
{
  Q_OBJECT

  public:
    QcRibbonLargeButton(QAction *action, QWidget *parent);
};


//---------------- QcRibbonDropButton -------------------
//-------------------------------------------------------
class QcRibbonDropButton : public QToolButton
{
  Q_OBJECT

  public:
    QcRibbonDropButton(QAction *action, QWidget *parent);

  protected slots:

    void defaultActionChanged(QAction *action);
};


//---------------- QcRibbonCheckBox ---------------------
//-------------------------------------------------------
class QcRibbonCheckBox : public QCheckBox
{
  Q_OBJECT

  public:
    QcRibbonCheckBox(QAction *action, QWidget *parent);

    virtual void actionEvent(QActionEvent *event);
};


//---------------- QcRibbon ----------------------------
//------------------------------------------------------

class QcRibbon : public QMenuBar
{
  Q_OBJECT

  public:
    QcRibbon(QWidget *parent = 0);

    int collapse() const { return m_collapsemode; }

    void set_collapse(int mode);

  public:

    int heightForWidth(int width) const;

  signals:

    void collapse_changed(int mode);

  protected slots:

    void on_tab_changed(int index);

    void on_collapse();

    void actionTriggered(QAction *action);

    void controlbarHidden();

  protected:

    void initStyleOption(QStyleOptionTab *option, int index) const;

    void updateGeometries();

    virtual bool event(QEvent *event);

    virtual void actionEvent(QActionEvent *event);

    virtual void changeEvent(QEvent *event);

    virtual void resizeEvent(QResizeEvent *event);

    virtual void paintEvent(QPaintEvent *event);

  private:

    QSpacerItem *m_menubar;

    QPushButton *m_collapsebutton;

    int m_currenttab;
    int m_collapsemode;

    QAction *m_hoveraction;
};
