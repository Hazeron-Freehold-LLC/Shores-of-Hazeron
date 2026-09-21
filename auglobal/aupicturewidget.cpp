//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupicturewidget.cpp,v 1.8 2022/04/11 20:17:31 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aupicturewidget.h"
//
#include <QPainter>
#include <QWheelEvent>

AuPictureWidget::AuPictureWidget(bool autoScale, QWidget* parent) : QWidget(parent)
{
	m_Auto = autoScale;
	m_BackgroundColor = QColor(0,0,0);
	m_LeftButtonMode = 0;
	m_Scale = 1;

	setAttribute(Qt::WA_OpaquePaintEvent, true);
	if(!m_Auto)
		setCursor(QCursor(Qt::OpenHandCursor));
}

AuPictureWidget::~AuPictureWidget()
{
}

void AuPictureWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
}

QSize AuPictureWidget::sizeHint() const
{
	return QSize(256,256);
}

void AuPictureWidget::paintEvent(QPaintEvent* pe)
{
	QPainter p(this);

	// Fill background with a dark color.
	QBrush brBack(m_BackgroundColor);
	p.fillRect(rect(), brBack);

	// Draw picture, if we have one.
	if(!m_Image.isNull())
	{
		if(m_Auto)
		{
			p.setRenderHint(QPainter::SmoothPixmapTransform);
			p.drawImage(rect(), m_Image);
		}
		else
		{
			// Determine target rectangle for the image.
			QRectF rc(m_Image.rect());
			if(m_Scale != 1)
			{
				// Scale the rectangle.
				rc.setWidth(rc.width() * m_Scale);
				rc.setHeight(rc.height() * m_Scale);
			}

			// Translate the rectangle so the middle pixel is in the middle of the window.
			float offx = width()/2.f - m_MiddlePixel.x() * m_Scale;
			float offy = height()/2.f - m_MiddlePixel.y() * m_Scale;
			rc = rc.translated(offx, offy);

			// Draw the image so it looks as nice as possible.
			p.setRenderHint(QPainter::SmoothPixmapTransform);
			p.drawImage(rc, m_Image);
		}
	}
	p.end();
	QWidget::paintEvent(pe);
}

void AuPictureWidget::mousePressEvent(QMouseEvent* e)
{
	QWidget::mousePressEvent(e);
	if(m_Auto)
		return;
	m_LeftButtonMode = 0;
	m_MouseDownPt = e->pos();
	if(e->button() == Qt::LeftButton)
	{
		setCursor(QCursor(Qt::ClosedHandCursor));
		m_LeftButtonMode = 1;
	}
}

void AuPictureWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
	QWidget::mouseDoubleClickEvent(e);
}

// NOTE: We only get this event while a button is held down.
void AuPictureWidget::mouseMoveEvent(QMouseEvent* e)
{
	QWidget::mouseMoveEvent(e);
	if(m_Auto)
		return;
	if(m_LeftButtonMode == 1)
	{
		float x = m_MiddlePixel.x() - (float)(e->pos().x() - m_MouseDownPt.x()) / m_Scale;
		float y = m_MiddlePixel.y() - (float)(e->pos().y() - m_MouseDownPt.y()) / m_Scale;
		m_MiddlePixel.setX(x);
		m_MiddlePixel.setY(y);
		m_MouseDownPt = e->pos();
		update();
	}
}

void AuPictureWidget::mouseReleaseEvent(QMouseEvent* e)
{
	if(!m_Auto)
	{
		setCursor(QCursor(Qt::OpenHandCursor));
		m_LeftButtonMode = 0;
	}
	QWidget::mouseReleaseEvent(e);
}

void AuPictureWidget::wheelEvent(QWheelEvent *e)
{
	if(m_Auto)
	{
		QWidget::wheelEvent(e);
		return;
	}

	float nuScale = m_Scale;
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	if(e->angleDelta().y() > 0)
		nuScale += .5f;
	else if(e->angleDelta().y() < 0)
		nuScale -= .5f;
#else
	if(e->delta() > 0)
		nuScale += .5f;
	else if(e->delta() < 0)
		nuScale -= .5f;
#endif

	QWidget::wheelEvent(e);
	SetScale(nuScale);
	e->setAccepted(true);
}

void AuPictureWidget::SetScale(float scale)
{
	if(scale < 1 || scale > 10)
		return;
	m_Scale = scale;
	update();
	emit ScaleChanged(scale);
}

void AuPictureWidget::setVisible(bool visible)
{
	QWidget::setVisible(visible);
}

void AuPictureWidget::SetImage(const QImage& image)
{
	m_Image = image;
	m_MiddlePixel = QPointF(m_Image.width()/2.f, m_Image.height()/2.f);
	update();
}

void AuPictureWidget::SetBackgroundColor(const QColor& bgcol)
{
	m_BackgroundColor = bgcol;
	update();
}

