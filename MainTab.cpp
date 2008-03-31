#include "MainTab.h"
#include "AppTools.h"
#include <QDialog>

extern QList<AppTool> g_tools;
const int FIXED_TAB_COUNT = 4;

MainTab::MainTab(QWidget* parent) : QTabWidget(parent)
{
	m_toolTabClose = new QToolButton(this);
	m_toolTabClose->setIcon(QIcon(":/menu/tab_remove.png"));
	m_toolTabClose->setEnabled(false);
	setCornerWidget(m_toolTabClose);
	
	connect(m_toolTabClose, SIGNAL(clicked()), this, SLOT(closeTabBtn()));
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
	
	QToolButton* btn = new QToolButton(this);
	QMenu* tabOpenMenu = new QMenu(btn);
	
	initAppTools(tabOpenMenu);
	
	btn->setIcon(QIcon(":/menu/tab_new.png"));
	btn->setPopupMode(QToolButton::InstantPopup);
	btn->setMenu(tabOpenMenu);
	setCornerWidget(btn, Qt::TopLeftCorner);
}

void MainTab::currentTabChanged(int newTab)
{
	m_toolTabClose->setEnabled(newTab >= FIXED_TAB_COUNT);
}

void MainTab::initAppTools(QMenu* tabOpenMenu)
{
	for(int i=0;i<g_tools.size();i++)
	{
		QAction* action;
		
		action = tabOpenMenu->addAction(QIcon(":/menu/newtool.png"), g_tools[i].strName);
		action->setData(i);
		
		connect(action, SIGNAL(triggered()), this, SLOT(openAppTool()));
	}
}

void MainTab::openAppTool()
{
	QAction* action = (QAction*) sender();
	int tool = action->data().toInt();
	QWidget* widget = g_tools[tool].pfnCreate();
	
	if(QDialog* dlg = dynamic_cast<QDialog*>(widget))
	{
		dlg->exec();
		delete dlg;
	}
	else
	{
		connect(widget, SIGNAL(changeTabTitle(QString)), this, SLOT(changeTabTitle(QString)));
		setCurrentIndex( addTab(widget, g_tools[tool].strName) );
	}
}

void MainTab::changeTabTitle(QString newTitle)
{
	QWidget* widget = (QWidget*) sender();
	int i = indexOf(widget);
	
	if(i >= 0)
		setTabText(i, newTitle);
}

void MainTab::closeTabBtn()
{
	m_index = currentIndex();
	closeTab();
}

void MainTab::closeTab()
{
	if(m_index >= FIXED_TAB_COUNT)
	{
		QWidget* w;
		
		w = widget(m_index);
		
		if(m_index == currentIndex())
		{
			for(int i=m_index-1;i>=0;i--)
			{
				if(isTabEnabled(i))
				{
					setCurrentIndex(i);
					break;
				}
			}
		}
		removeTab(m_index);
		
		delete w;
	}
}
void MainTab::closeAllTabs()
{
	if(currentIndex() >= FIXED_TAB_COUNT)
		setCurrentIndex(0);
	
	for(int i=count()-1;i>FIXED_TAB_COUNT-1;i--)
	{
		QWidget* w = widget(i);
		removeTab(i);
		delete w;
	}
}

void MainTab::contextMenuEvent(QContextMenuEvent* event)
{
	QTabBar* bar = tabBar();
	
	m_index = bar->tabAt(bar->mapFrom(this, event->pos()));
	
	if(m_index < 0)
		return;
	
	QAction* action;
	QMenu menu(this);
	
	if(m_index > 3)
	{
		action = menu.addAction(tr("Close tab"));
		connect(action, SIGNAL(triggered()), this, SLOT(closeTab()));
	}
	
	action = menu.addAction(tr("Close all tabs"));
	connect(action, SIGNAL(triggered()), this, SLOT(closeAllTabs()));
	
	menu.exec(mapToGlobal(event->pos()));
}

void MainTab::mousePressEvent(QMouseEvent* event)
{
	if(event->buttons() == Qt::MidButton)
	{
		QTabBar* bar = tabBar();
		m_index = bar->tabAt(bar->mapFrom(this, event->pos()));
		closeTab();
	}
	QTabWidget::mousePressEvent(event);
}
