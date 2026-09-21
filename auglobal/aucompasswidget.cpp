//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucompasswidget.cpp,v 1.2 2022/02/19 18:33:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucompasswidget.h"
//
#include <QPainter>
#include <QWheelEvent>

AuCompassWidget::AuCompassWidget(QWidget* parent) : QWidget(parent)
{
	m_Compass.load(":data/Compass.png");
	m_CompassShadow.load(":data/CompassShadow.png");
	m_Heading = 0;
	setAttribute(Qt::WA_OpaquePaintEvent, true);
}

AuCompassWidget::~AuCompassWidget()
{
}

QSize AuCompassWidget::sizeHint() const
{
	return m_Compass.size();
}

void AuCompassWidget::paintEvent(QPaintEvent* pe)
{
	QPainter p(this);

	// Fill background with a dark color.
	QBrush brBack(QColor(0,0,0));
	p.fillRect(0,0, width(),height(), brBack);

	// Draw compass image scaled to fit my rectangle, with a drop shadow.
	QImage imcompassshad;
	QImage imcompass;
	if(width() < height())
	{
		imcompassshad = m_CompassShadow.scaledToWidth(width(), Qt::SmoothTransformation);
		imcompass = m_Compass.scaledToWidth(width(), Qt::SmoothTransformation);
	}
	else
	{
		imcompassshad = m_CompassShadow.scaledToHeight(height(), Qt::SmoothTransformation);
		imcompass = m_Compass.scaledToHeight(height(), Qt::SmoothTransformation);
	}
	int compx = width()/2;
	int compy = height()/2;
	p.translate(compx+5, compy+5);
	p.rotate(-m_Heading*RAD2DEG);
	p.drawImage(-imcompassshad.width()/2,-imcompassshad.height()/2, imcompassshad);
	p.setTransform(QTransform());	// Reset to identity matrix.
	p.translate(compx, compy);
	p.rotate(-m_Heading*RAD2DEG);
	p.drawImage(-imcompass.width()/2,-imcompass.height()/2, imcompass);
	p.setWorldMatrixEnabled(false);

	// Draw compass image with a drop shadow.
	/*
	int compx = width()/2;
	int compy = height()/2;
	p.translate(compx+5, compy+5);
	p.rotate(-m_Heading*RAD2DEG);
	p.drawImage(-m_CompassShadow.width()/2,-m_CompassShadow.height()/2, m_CompassShadow);
	p.setTransform(QTransform());	// Reset to identity matrix.
	p.translate(compx, compy);
	p.rotate(-m_Heading*RAD2DEG);
	p.drawImage(-m_Compass.width()/2,-m_Compass.height()/2, m_Compass);
	p.setWorldMatrixEnabled(false);
	*/

	p.end();
	
	QWidget::paintEvent(pe);
}

