//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audocktitle.cpp,v 1.19 2022/03/03 16:30:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audocktitle.h"
//
#include "audef.h"
#include "auglobal.h"
#include "austylesheet.h"
#include <QDesktopServices>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>
#include <QToolButton>

AuDockTitle::AuDockTitle(QWidget* parent) : QWidget(parent)
{	
	//title bar sizing
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setCursor(Qt::OpenHandCursor);

    m_FloatButton = new QToolButton(this);
	m_FloatButton->setAutoRaise(true);
	m_FloatButton->setCursor(QCursor(Qt::ArrowCursor));
    //m_FloatButton->setFixedSize(14, 14);
    m_FloatButton->setFocusPolicy ( Qt::NoFocus );
    m_FloatButton->setIcon(QPixmap(":data/hazBlackFloat.png"));
	m_FloatButton->setToolTip(tr("<html>Dock/Float<br>"
				"Switches between docked and undocked window positions."
				"</html>"));
    connect(m_FloatButton, SIGNAL(clicked()), this, SLOT(ToggleFloat()));

    m_CloseButton = new QToolButton(this);
	m_CloseButton->setAutoRaise(true);
	m_CloseButton->setCursor(QCursor(Qt::ArrowCursor));
    //m_CloseButton->setFixedSize(14, 14);
    m_CloseButton->setFocusPolicy ( Qt::NoFocus );
    m_CloseButton->setIcon(QPixmap(":data/hazBlackClose.png"));
	m_CloseButton->setToolTip(tr("<html>Close<br>"
				"Closes the dockable window."
				"</html>"));
    connect(m_CloseButton, SIGNAL(clicked()), parent, SLOT(close()));

    m_HelpButton = new QToolButton(this);
	m_HelpButton->setAutoRaise(true);
	m_HelpButton->setCursor(QCursor(Qt::ArrowCursor));
    m_HelpButton->setIcon(QPixmap(":data/hazWiki.png"));
    //m_HelpButton->setFixedSize(14, 14);
    m_HelpButton->setFocusPolicy(Qt::NoFocus);
	m_HelpButton->hide();
	m_HelpButton->setToolTip(tr("<html>"
				"Opens a relevant topic in the <i>%1</i> wiki. "
				"</html>").arg(g_Au.ApplicationName()));
    connect(m_HelpButton, SIGNAL(clicked()), this, SLOT(ShowHelp()));
    
    m_Icon = new QToolButton(this);
    m_Icon->setBackgroundRole(QPalette::Text);
	m_Icon->setAutoRaise(true);
	m_Icon->setCursor(QCursor(Qt::ArrowCursor));
    //m_Icon->setFixedSize(14, 14);
    m_Icon->setFocusPolicy ( Qt::NoFocus );
    connect(m_Icon, SIGNAL(clicked()), this, SLOT(showIconMenu()));
    
    m_label = new QLabel(this);
    //the default window title
    m_label->setText(tr("Default Window Title"));
    //parent->setWindowTitle(tr("Default Window Title"));
	m_label->setTextInteractionFlags(Qt::NoTextInteraction);
	QFont lblFont = m_label->font();
	if(lblFont.pixelSize() > 0)
		lblFont.setPixelSize(lblFont.pixelSize() - 2);
	else
		lblFont.setPointSize(lblFont.pointSize() - 2);
	m_label->setFont(lblFont);
  
  	//titlebar layout
    m_hbox_TitleBar = new QHBoxLayout(this);
    m_hbox_TitleBar->setContentsMargins( 2, 0, 2, 0 );
    
    //titlebar icon
    m_hbox_TitleBar->addWidget(m_Icon);
    
    //titlebar text
    m_hbox_TitleBar->addWidget(m_label);

	m_hbox_TitleBar->addStretch(1);
    
    m_hbox_TitleBar->addWidget(m_HelpButton);
    
    //min max close buttons
    m_hbox_TitleBar->addWidget(m_FloatButton);
    m_hbox_TitleBar->addWidget(m_CloseButton);
    
    //spacing is mainly for the buttons
 	m_hbox_TitleBar->setSpacing(2);
 	m_hbox_TitleBar->setMargin(2);
	setLayout(m_hbox_TitleBar);

    //system menu
    createActions();
}

void AuDockTitle::contextMenuEvent(QContextMenuEvent *e)
{
   createSysMenu(e->globalPos());
   QWidget::contextMenuEvent(e);
}

void AuDockTitle::createSysMenu(QPoint point)
{
	QMenu systemMenu(this);
 
	Q_ASSERT(m_FloatButton);
	Q_ASSERT(m_CloseButton);
   	if(!m_HelpButton->isHidden())
   		systemMenu.addAction(m_HelpAction);
 	if(!m_FloatButton->isHidden())
   		systemMenu.addAction(m_FloatAction);
   	if(!m_CloseButton->isHidden())
   		systemMenu.addAction(m_CloseAction);
   
    systemMenu.exec(point);
}

void AuDockTitle::createActions()
{
	QIcon theMaxIcon(":data/hazBlackFloat.png");
	m_FloatAction = new QAction(theMaxIcon, tr("Dock/Float"), this);	 
	//minAct->setShortcut(tr("Ctrl+N"));
	//minAct->setStatusTip(tr(""));
	m_FloatAction->setToolTip(tr("<html>Dock/Float<br>"
				"Switches between docked and undocked window positions."
				"</html>"));
	connect(m_FloatAction, SIGNAL(triggered()), this, SLOT(ToggleFloat()));

	QIcon theCloseIcon(":data/hazBlackClose.png");
	m_CloseAction = new QAction(theCloseIcon, tr("Close"), this);
	//m_CloseAction->setShortcut(tr("Esc"));
	//m_CloseAction->setStatusTip(tr(""));
	m_CloseAction->setToolTip(tr("<html>Close<br>"
				"Closes the dockable window."
				"</html>"));
	connect(m_CloseAction, SIGNAL(triggered()), parentWidget(), SLOT(close()));

	QIcon wikiIcon(":data/hazWiki.png");
	m_HelpAction = new QAction(wikiIcon, tr("Wiki"), this);
	//m_HelpAction->setShortcut(tr("Esc"));
	//m_HelpAction->setStatusTip(tr(""));
	connect(m_HelpAction, SIGNAL(triggered()), this, SLOT(ShowHelp()));
}

// paint event for title bar main wiget
void AuDockTitle::paintEvent(QPaintEvent* pe)
{
    QPainter painter(this); 
	painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw border
	QDockWidget* dockWidget = qobject_cast<QDockWidget*>(parentWidget());
	if(dockWidget && dockWidget->isFloating())
	{
		QImage image(":data/hazBlackTitleGrad.png");
		QSize size(image.width(), height());
		QImage scaledImage = image.scaled(size, Qt::IgnoreAspectRatio);
    	painter.drawImage(QPoint(0,0), scaledImage);
	}
		
	painter.setPen(QPen(QColor(93,135,221,255), 2));
	painter.drawRect(rect());      
	QWidget::paintEvent(pe);
}

void AuDockTitle::showIconMenu()
{
   createSysMenu(mapToGlobal(QPoint( rect().left(), rect().bottom())));
}

void AuDockTitle::mouseDoubleClickEvent(QMouseEvent* me)
{
	//qDebug("AuDockTitle::mouseDoubleClickEvent");
	QWidget::mouseDoubleClickEvent(me);
	me->ignore();
}

void AuDockTitle::mousePressEvent(QMouseEvent* me)
{
	//qDebug("AuDockTitle::mousePressEvent");
	//setCursor(Qt::ClosedHandCursor);
	QWidget::mousePressEvent(me);
	me->ignore();
}

void AuDockTitle::mouseReleaseEvent(QMouseEvent* me)
{
	//setCursor(Qt::OpenHandCursor);
	QWidget::mouseReleaseEvent(me);
	me->ignore();
}

void AuDockTitle::mouseMoveEvent(QMouseEvent* me)
{
	QWidget::mouseMoveEvent(me);
	me->ignore();
}

void AuDockTitle::setTitleIcon(QIcon Icon)
{
	Q_ASSERT(m_Icon);
	m_Icon->setIcon(Icon);
}

void AuDockTitle::SetText(const QString& titleText)
{
	Q_ASSERT(m_label);
	m_label->setText(titleText);
}

void AuDockTitle::resizeEvent(QResizeEvent* event)
{
	// From QDockWidget::setTitleBarWidget documentation.
	QDockWidget* dockWidget = qobject_cast<QDockWidget*>(parentWidget());
	if(dockWidget)
	{
		if(dockWidget->features() & QDockWidget::DockWidgetVerticalTitleBar)
			m_Orientation = Qt::Vertical;						// I need to be vertical
		else
			m_Orientation = Qt::Horizontal;						// I need to be horizontal
	}

	QWidget::resizeEvent(event);
}

QSize AuDockTitle::sizeHint() const
{
	/*
	if(m_Orientation == Qt::Horizontal)
	{
	}
	else
	{
	}
	*/
	//return m_hbox_TitleBar->sizeHint();
	QFontMetrics fmet = m_label->fontMetrics();
	int barHt = fmet.height() + 2;
	return QSize(barHt, barHt);
}

QSize AuDockTitle::minimumSizeHint() const
{
	/*
	if(m_Orientation == Qt::Horizontal)
	{
	}
	else
	{
	}
	*/
	//return m_hbox_TitleBar->minimumSize();
	return QSize(20,20);
}

void AuDockTitle::ToggleFloat()
{
	QDockWidget* dockWidget = qobject_cast<QDockWidget*>(parentWidget());
	if(dockWidget)
		dockWidget->setFloating(!dockWidget->isFloating());
}

void AuDockTitle::SetHelpUrl(const QUrl& url)
{
	m_Help = QString();
	m_HelpUrl = url;

	if(m_HelpUrl == QUrl())
		m_HelpButton->hide();
	else
	{
		m_HelpButton->show();
		m_HelpButton->setToolTip(tr("<html>"
					"%1"
					"</html>").arg(m_HelpUrl.toDisplayString()));
	}
}

void AuDockTitle::SetWikiTopic(const QString& topic)
{
	m_Help = topic;
	m_Help.replace(' ', '_');
	m_HelpUrl = QUrl();

	if(m_Help.isEmpty())
		m_HelpButton->hide();
	else
	{
		m_HelpButton->show();
		m_HelpButton->setToolTip(tr("<html>"
					"Wiki <b>%1</b>"
					"</html>").arg(topic));
	}
}

void AuDockTitle::ShowHelp()
{
	// Decide what url to open.
	QUrl url(m_HelpUrl);
	if(url == QUrl() && !m_Help.isEmpty())
	{
		url = UrlFromString(m_Help);
		if(url == QUrl())
			url = UrlFromString(QString("https://www.hazeron.com/wiki/index.php/%1").arg(m_Help));
	}

	// Open the url.
	if(url != QUrl())
		QDesktopServices::openUrl(url);
}

