//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audlgtitle.cpp,v 1.30 2022/03/03 16:26:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audlgtitle.h"
//
#include "audef.h"
#include "auglobal.h"
#include "austylesheet.h"
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>
#include <QToolButton>

AuDlgTitleBar::AuDlgTitleBar(QWidget *parent) : QWidget(parent)
{	
    m_hasFocus = false;

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setCursor(Qt::OpenHandCursor);

    m_minimize = new QToolButton(this);
	m_minimize->setAutoRaise(true);
	m_minimize->setCursor(QCursor(Qt::ArrowCursor));

    m_maximize = new QToolButton(this);
	m_maximize->setAutoRaise(true);
	m_maximize->setCursor(QCursor(Qt::ArrowCursor));

    m_HelpButton = new QToolButton(this);
	m_HelpButton->setAutoRaise(true);
	m_HelpButton->setCursor(QCursor(Qt::ArrowCursor));
	m_HelpButton->setToolTip(tr("<html>"
				"Opens a relevant topic in the <i>%1</i> wiki. "
				"</html>").arg(g_Au.ApplicationName()));

    m_close = new QToolButton(this);
	m_close->setAutoRaise(true);
	m_close->setCursor(QCursor(Qt::ArrowCursor));
    
    m_label = new QLabel(this);
    m_label->setText(tr("Default Window Title"));
    parent->setWindowTitle(tr("Default Window Title"));
	m_label->setTextInteractionFlags(Qt::NoTextInteraction);
	QFont lblFont = m_label->font();
	if(lblFont.pixelSize() > 0)
		lblFont.setPixelSize(lblFont.pixelSize() - 1);
	else
		lblFont.setPointSize(lblFont.pointSize() - 1);
	m_label->setFont(lblFont);
	QFontMetrics fmet = m_label->fontMetrics();
	int fontHt = fmet.height();
    m_label->setFixedHeight(fontHt);
	setFixedHeight(fontHt + 2);
  
    m_Icon = new QToolButton(this);
    m_Icon->setBackgroundRole(QPalette::Text);
	m_Icon->setAutoRaise(true);
	m_Icon->setCursor(QCursor(Qt::ArrowCursor));
    m_Icon->setFixedSize(fontHt, fontHt);
    m_Icon->setFocusPolicy(Qt::NoFocus);
    
    // Use the style to set the button pixmaps
    QPixmap pix(":data/hazBlackClose.png");
    m_close->setIcon(pix);
    m_close->setFixedSize(fontHt, fontHt);
    m_close->setFocusPolicy(Qt::NoFocus);
    
    maxPix = QPixmap(":data/hazBlackMax.png");
    m_maximize->setIcon(maxPix);
    m_maximize->setFixedSize(fontHt, fontHt);
    m_maximize->setFocusPolicy(Qt::NoFocus);
    
    pix = QPixmap(":data/hazBlackMin.png");
    m_minimize->setIcon(pix);
    m_minimize->setFixedSize(fontHt, fontHt);
    m_minimize->setFocusPolicy(Qt::NoFocus);
    
    pix = QPixmap(":data/hazWiki.png");
    m_HelpButton->setIcon(pix);
    m_HelpButton->setFixedSize(fontHt, fontHt);
    m_HelpButton->setFocusPolicy(Qt::NoFocus);
	m_HelpButton->hide();

    restorePix = QPixmap(":data/hazBlackNormal.png");
    
  	//titlebar layout
    m_hbox_TitleBar = new QHBoxLayout(this);
    m_hbox_TitleBar->setContentsMargins( 2, 0, 2, 0);
    
    //titlebar icon
    m_hbox_TitleBar->addWidget(m_Icon);
    
    //titlebar text
    m_hbox_TitleBar->addWidget(m_label);
    
    m_hbox_TitleBar->addWidget(m_HelpButton);

    //min max close buttons
    m_hbox_TitleBar->addWidget(m_minimize);
    setMin(false);
    m_hbox_TitleBar->addWidget(m_maximize);
    //setMax(false);//default get max button
    m_hbox_TitleBar->addWidget(m_close);
    
    //spacing is mainly for the buttons
 	m_hbox_TitleBar->setSpacing(1);
	setLayout(m_hbox_TitleBar);
    
    //this is for restoring the dialog after maxmize has been hit
    maxNormal = false;
    
    //button message mapping
    connect(m_close, SIGNAL(clicked()), parent, SLOT(close()));
    connect(m_minimize, SIGNAL(clicked()), this, SLOT(showSmall()));
    connect(m_maximize, SIGNAL(clicked()), this, SLOT(showMaxRestore()));
    connect(m_Icon, SIGNAL(clicked()), this, SLOT(showIconMenu()));
    connect(m_HelpButton, SIGNAL(clicked()), this, SLOT(ShowHelp()));
   
    //system menu
    createActions();
}

void AuDlgTitleBar::contextMenuEvent(QContextMenuEvent *e)
{
   createSysMenu(e->globalPos());
   QWidget::contextMenuEvent(e);
}

void AuDlgTitleBar::createSysMenu(QPoint point)
{
	QMenu systemMenu(this);
 
	Q_ASSERT(m_minimize);
	Q_ASSERT(m_maximize);
	Q_ASSERT(m_close);
   	if(!m_HelpButton->isHidden())
   		systemMenu.addAction(m_HelpAction);
 	if(!m_minimize->isHidden())
   		systemMenu.addAction(minAct);
   	if(!m_maximize->isHidden())
   		systemMenu.addAction(maxAct);
   	if(!m_close->isHidden())
   		systemMenu.addAction(closeAct);
   
    systemMenu.exec(point);
}

void AuDlgTitleBar::createActions()
{
	QIcon theMinIcon(":data/hazBlackMin.png");
	minAct = new QAction(theMinIcon, tr("Minimize"), this);	 
	//minAct->setShortcut(tr("Ctrl+N"));
	//minAct->setStatusTip(tr(""));
	connect(minAct, SIGNAL(triggered()), this, SLOT(showSmall()));

	QIcon theMaxIcon(":data/hazBlackMax.png");
	maxAct = new QAction(theMaxIcon, tr("Maximize"), this);
	//maxAct->setShortcut(tr("Ctrl+M"));
	//maxAct->setStatusTip(tr(""));
	connect(maxAct, SIGNAL(triggered()), this, SLOT(showMaxRestore()));

	QIcon theCloseIcon(":data/hazBlackClose.png");
	closeAct = new QAction(theCloseIcon, tr("Close"), this);
	closeAct->setShortcut(tr("Esc"));
	//closeAct->setStatusTip(tr(""));
	connect(closeAct, SIGNAL(triggered()), parentWidget(), SLOT(close()));

	QIcon wikiIcon(":data/hazWiki.png");
	m_HelpAction = new QAction(wikiIcon, tr("Wiki"), this);
	//m_HelpAction->setShortcut(tr("Esc"));
	//m_HelpAction->setStatusTip(tr(""));
	connect(m_HelpAction, SIGNAL(triggered()), this, SLOT(ShowHelp()));
}

// paint event for title bar main wiget
void AuDlgTitleBar::paintEvent(QPaintEvent* pe)
{
	QWidget::paintEvent(pe);
   
    QPainter painter(this); 
	painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw border
	if(m_hasFocus)
	{
		QImage image(":data/hazBlackTitleGrad.png");
		QSize size(image.width(), height());
		QImage scaledImage = image.scaled(size, Qt::IgnoreAspectRatio);
    	painter.drawImage(QPoint(0,0), scaledImage);
	}
		
	painter.setPen(QPen(QColor(93,135,221,255), 2));
	painter.drawRect(rect());      
}

void AuDlgTitleBar::showSmall()
{
    parentWidget()->showMinimized();
}

void AuDlgTitleBar::showMaxRestore()
{
    if(maxNormal) 
    {
        parentWidget()->showNormal();
        maxNormal = !maxNormal;
        m_maximize->setIcon(maxPix);
    } 
    else 
    {
        parentWidget()->showMaximized();
        maxNormal = !maxNormal;
        m_maximize->setIcon(restorePix);
    }
}

void AuDlgTitleBar::showIconMenu()
{
   createSysMenu(mapToGlobal(QPoint( rect().left(), rect().bottom())));
}

void AuDlgTitleBar::mousePressEvent(QMouseEvent* me)
{
	//qDebug("AuDlgTitleBar::mousePressEvent");
    startPos = me->globalPos();
    clickPos = mapToParent(me->pos());
	setCursor(Qt::ClosedHandCursor);
	QWidget::mousePressEvent(me);
}

void AuDlgTitleBar::mouseReleaseEvent(QMouseEvent* me)
{
	setCursor(Qt::OpenHandCursor);
	QWidget::mouseReleaseEvent(me);
}

void AuDlgTitleBar::mouseMoveEvent(QMouseEvent* me)
{
	//qDebug("AuDlgTitleBar::mouseMoveEvent maxNormal %c", maxNormal?'Y':'N');
    if(maxNormal)
        return;

    parentWidget()->move(me->globalPos() - clickPos);

	QWidget::mouseMoveEvent(me);
}

void AuDlgTitleBar::setMin(bool bSet /*= true*/)
{
	Q_ASSERT(m_minimize);
	if(bSet)
	    m_minimize->show();
	else
	    m_minimize->hide();
}

void AuDlgTitleBar::setMax(bool bSet /*= true*/)
{
	Q_ASSERT(m_maximize);
	if(bSet)
		m_maximize->show();
	else
		m_maximize->hide();	
}

void AuDlgTitleBar::setTitleIcon(QIcon Icon)
{
	Q_ASSERT(m_Icon);
	m_Icon->setIcon(Icon);
}

void AuDlgTitleBar::SetText(const QString& titleText)
{
	Q_ASSERT(m_label);
	m_label->setText(titleText);
}

void AuDlgTitleBar::SetHelpUrl(const QUrl& url)
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

void AuDlgTitleBar::SetWikiTopic(const QString& topic)
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

void AuDlgTitleBar::ShowHelp()
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

