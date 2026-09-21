//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audialog.cpp,v 1.35 2022/02/19 18:33:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|


#include "audialog.h"
//
#include "auglobal.h"
#include <QSizePolicy>
#include <QApplication>
#include <QLayout>
#include <QStyle>

#include <QFrame>
#include <qevent.h>

#include <qdebug.h>

//constructer
AuDialog::AuDialog(QWidget* parent, bool bHasTitle) : QDialog(parent)
{
	m_hasFocus = false;
	m_IsResizing = false;
	m_IsResizingCursor = false;
	m_MoveBottom = false;
	m_MoveLeft = false;
	m_MoveRight = false;
	m_MoveTop = false;
	m_titleBar = 0;
    
	// Set appropriate window flags to remove dialog title bar.
	setWindowFlags(Qt::Dialog|Qt::CustomizeWindowHint|Qt::FramelessWindowHint);

	setMouseTracking(true);

    m_vbox = new QVBoxLayout(this);

	if(bHasTitle)
	{
		m_titleBar = new AuDlgTitleBar(this);
		m_vbox->addWidget(m_titleBar);
		m_vbox->setStretchFactor(m_titleBar, 0);
		m_vbox->insertStretch(1, 0);
	}

    m_vbox->setMargin(3);
    m_vbox->setSpacing(0);
	setLayout(m_vbox);
}

AuDialog::~AuDialog()
{
}

//paint event for title bar main widget
void AuDialog::paintEvent(QPaintEvent* pe)
{
    // Draw border when the window has focus.
	if(m_hasFocus && m_titleBar)
	{
		QPainter painter(this); 

		painter.setPen(QPen(QColor(93,135,221,255), 3));
		QRect outLine(0,0,width()-1,height()-1);
		painter.drawRect(outLine);      
	
		painter.end();
	}
    
    QDialog::paintEvent(pe);
}

void AuDialog::changeEvent(QEvent* e)
{
	if(e->type() == QEvent::ActivationChange)
	{
		if(m_titleBar)
		{
			m_hasFocus = !m_hasFocus;
			m_titleBar->SetHasFocus(m_hasFocus);
		}

		float opacity = (float)g_Au.WindowOpacity(m_hasFocus)/100.f;
		setWindowOpacity(opacity);

		if(m_titleBar)
			m_titleBar->repaint();
		repaint();
	}
	QDialog::changeEvent(e);
}

void AuDialog::setAuDlgLayout(QLayout* layout)
{
	m_vbox->insertLayout(1, layout , 1);

	// Set arrow cursor on all widgets that haven't set a cursor, otherwise my resize arrows will become the default for them
	// whenever I call setCursor.
	QObjectList childWidgets = children();
	for(QObjectList::iterator ci = childWidgets.begin(); ci != childWidgets.end(); ci++)
	{
		QWidget* widget = dynamic_cast<QWidget*>(*ci);
		if(widget && !widget->cursor().shape())
			widget->setCursor(Qt::ArrowCursor);
	}
}

void AuDialog::setAuDlgTitle(const QString& sTitle)
{
	if(m_titleBar)
		m_titleBar->SetText(sTitle);
	setWindowTitle(sTitle);
}

void AuDialog::setAuDlgIcon(QIcon Icon)
{
	if(m_titleBar)
		m_titleBar->setTitleIcon(Icon);
}

void AuDialog::mouseMoveEvent(QMouseEvent* e)
{
	QDialog::mouseMoveEvent(e);

	if(!m_IsResizing)
	{
		if(rect().contains(e->pos()))
		{
			// Test to see if the cursor is in the 3 pixel resizing margin at the edges.
			QRect rc = rect().adjusted(+4,+4,-4,-4);
			if(!rc.contains(e->pos()))
			{
				// Cursor is in the resizing margin of the widget.
				// Set the appropriate resizing cursor.
				m_MoveLeft = e->pos().x() < 4;
				m_MoveTop = e->pos().y() < 4;
				m_MoveRight = width() - e->pos().x() < 4;
				m_MoveBottom = height() - e->pos().y() < 4;

				if((m_MoveLeft && m_MoveTop) || (m_MoveRight && m_MoveBottom))
				{
					setCursor(Qt::SizeFDiagCursor);
					m_IsResizingCursor = true;
				}
				else if((m_MoveLeft && m_MoveBottom) || (m_MoveRight && m_MoveTop))
				{
					setCursor(Qt::SizeBDiagCursor);
					m_IsResizingCursor = true;
				}
				else if(m_MoveLeft || m_MoveRight)
				{
					setCursor(Qt::SizeHorCursor);
					m_IsResizingCursor = true;
				}
				else if(m_MoveTop || m_MoveBottom)
				{
					setCursor(Qt::SizeVerCursor);
					m_IsResizingCursor = true;
				}
				else
				{
					UnsetResizeCursor();
				}
			}
			else
			{
				UnsetResizeCursor();
			}
		}
		else
		{
			// Restore default/arrow cursor.
			UnsetResizeCursor();
		}
	}
	else
	{
		// The left mouse button was pressed in the resizing zone and it is now moving. Resize my window.
		QPoint MouseXY = mapToGlobal(e->pos());
		QPoint DialogXYTopLeft = mapToGlobal(pos());
		QPoint DialogXYBottomRight = mapToGlobal(rect().bottomRight());
		
		int minHeight = minimumHeight();
		int minWidth = minimumWidth();
		
		QRect g = geometry();
		
		if(m_MoveTop && m_MoveLeft)
		{
			QPoint NewPoint = MouseXY - DialogXYTopLeft;
			NewPoint = mapToParent(NewPoint);
			
			g = g.adjusted(NewPoint.x(),NewPoint.y(),0,0);
			
			if(g.width() < minWidth)
				g = g.adjusted(g.width() - minWidth,0,0,0);
			
			if(g.height() < minHeight)
				g = g.adjusted(0,g.height() - minHeight,0,0);
		}
		else if(m_MoveTop && m_MoveRight)
		{
			QPoint NewPoint = MouseXY - DialogXYTopLeft;
			QPoint PointTemp = MouseXY - DialogXYBottomRight;
			NewPoint = mapToParent(NewPoint);
			
			g = g.adjusted(0,NewPoint.y(),PointTemp.x(),0);
			
			if(g.width() < minWidth)
				g.setWidth(minWidth);
			
			if(g.height() < minHeight)
				g = g.adjusted(0,g.height() - minHeight,0,0);
		}
		else if(m_MoveBottom && m_MoveLeft)
		{
			QPoint NewPoint = MouseXY - DialogXYBottomRight;
			QPoint PointTemp = MouseXY - DialogXYTopLeft;
			PointTemp = mapToParent(PointTemp);
			
			g = g.adjusted(PointTemp.x(),0,0,NewPoint.y());
			
			if(g.width() < minWidth)
				g = g.adjusted(g.width() - minWidth,0,0,0);
			
			if(g.height() < minHeight)
				g.setHeight(minHeight);
		}
		else if(m_MoveBottom && m_MoveRight)
		{
			QPoint NewPoint = MouseXY - DialogXYBottomRight;
			
			g = g.adjusted(0,0,NewPoint.x(),NewPoint.y());
			
			if(g.width() < minWidth)
				g.setWidth(minWidth);
			
			if(g.height() < minHeight)
				g.setHeight(minHeight);
		}
		else if(m_MoveLeft)
		{
			QPoint NewPoint = MouseXY - DialogXYTopLeft;
			NewPoint = mapToParent(NewPoint);
			
			g = g.adjusted(NewPoint.x(),0,0,0);	
			
			if(g.width() < minWidth)
				g = g.adjusted(g.width() - minWidth,0,0,0);
		}
		else if(m_MoveRight)
		{
			QPoint NewPoint = MouseXY - DialogXYBottomRight;
	
			g = g.adjusted(0,0,NewPoint.x(),0);	
			
			if(g.width() < minWidth)
				g.setWidth(minWidth);
		}
		else if(m_MoveTop)
		{
			QPoint NewPoint = MouseXY - DialogXYTopLeft;
			NewPoint = mapToParent(NewPoint);
			
			g = g.adjusted(0,NewPoint.y(),0,0);
			
			if(g.height() < minHeight)
				g = g.adjusted(0,g.height() - minHeight,0,0);
		}
		else if(m_MoveBottom)
		{
			QPoint NewPoint = MouseXY - DialogXYBottomRight;
		
			g = g.adjusted(0,0,0,NewPoint.y());
			
			if(g.height() < minHeight)
				g.setHeight(minHeight);
		}  
		setGeometry(g);
	}
}

void AuDialog::mousePressEvent(QMouseEvent* e)
{
	QDialog::mousePressEvent(e);

	m_IsResizing = false;
	if(e->button() == Qt::LeftButton)
	{
		// Test to see if the cursor is inside the resizing zone.
		if(rect().contains(e->pos()))
		{
			// Test to see if the cursor is in the 3 pixel resizing margin at the edges.
			QRect rc = rect().adjusted(+4,+4,-4,-4);
			if(!rc.contains(e->pos()))
			{
				// Set the appropriate resizing cursor.
				bool left = e->pos().x() < 4;
				bool top = e->pos().y() < 4;
				bool right = width() - e->pos().x() < 4;
				bool bottom = height() - e->pos().y() < 4;
				if(left || top || right || bottom)
					m_IsResizing = true;
			}
		}
	}
}

void AuDialog::mouseReleaseEvent(QMouseEvent* e)
{
	QDialog::mouseReleaseEvent(e);
	if(e->button() == Qt::LeftButton)
		m_IsResizing = false;
}

void AuDialog::UnsetResizeCursor()
{
	if(m_IsResizingCursor)
	{
		unsetCursor();
		m_IsResizingCursor = false;
	}
}

void AuDialog::SetHelpUrl(const QUrl& url)
{
	if(m_titleBar)
		m_titleBar->SetHelpUrl(url);
}

void AuDialog::SetWikiTopic(const QString& topic)
{
	if(m_titleBar)
		m_titleBar->SetWikiTopic(topic);
}

