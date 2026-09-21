//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aunetworkactivity.cpp,v 1.2 2022/02/19 18:33:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aunetworkactivity.h"
//
#include "auglobal.h"
#include <QPainter>

AuNetworkActivity::AuNetworkActivity(QWidget* parent) : QPushButton(parent)
{
	m_Net1Down = 0;
	m_Net1Up = 0;
	m_Net2Down = 0;
	m_Net2Up = 0;
	m_Net3Down = 0;
	m_Net3Up = 0;
	m_Net4Down = 0;
	m_Net4Up = 0;
	m_NetImageDown.load(":data/NetActivityDown.png");
	m_NetImageUp.load(":data/NetActivityUp.png");
	m_Timer = 0;
	setAttribute(Qt::WA_OpaquePaintEvent);
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::ToolButton);
	setSizePolicy(sizePolicy);
}

QSize AuNetworkActivity::sizeHint() const
{
	return QSize(32,32);
}
void AuNetworkActivity::showEvent(QShowEvent* event)
{   
	QPushButton::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer) 
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(update()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(100);
}   
    
void AuNetworkActivity::hideEvent(QHideEvent* event)
{   
	QPushButton::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_Timer && m_Timer->isActive())
		m_Timer->stop();
}   

void AuNetworkActivity::paintEvent(QPaintEvent* ev)
{
	QWidget::paintEvent(ev);

	QPainter p(this);
	p.fillRect(rect(), QColor(0,0,0));

	int goingUp1 = g_Au.Net1Uploading();
	int goingDown1 = g_Au.Net1Downloading();
	int goingUp2 = g_Au.Net2Uploading();
	int goingDown2 = g_Au.Net2Downloading();
	int goingUp3 = g_Au.Net3Uploading();
	int goingDown3 = g_Au.Net3Downloading();
	int goingUp4 = g_Au.Net4Uploading();
	int goingDown4 = g_Au.Net4Downloading();
	//goingUp1 = 100;
	//goingDown1 = 100;
	//goingUp2 = 100;
	//goingDown2 = 100;
	//goingUp3 = 100;
	//goingDown3 = 100;
	//goingUp4 = 100;
	//goingDown4 = 100;
	if(!goingUp1 && !goingDown1 &&
			!goingUp2 && !goingDown2 &&
			!goingUp3 && !goingDown3 &&
			!goingUp4 && !goingDown4)
		return;

	const int frames = (width()+3)/4;	// Frames of animation along the movement route of the sprite.
	const int netDecay = 4;			// How many ticks per frame we decay from the network SetNet function values.
	if(goingUp1)
	{
		g_Au.DecayNet1Uploading(netDecay);
		m_Net1Up++;
		int frame = m_Net1Up%frames;
		float offset = (float)frame/(float)frames;
		QTransform model;
		model.translate(width()/2, height()/2);
		model.rotate(-(90-15));
		model.translate(-width()/2, -height()/2);
		qreal tx = cos(45*DEG2RAD)*(offset * width());
		qreal ty = -sin(45*DEG2RAD)*(offset * height());
		model.translate(tx, ty);
		p.setTransform(model);
		p.drawImage(rect(), m_NetImageUp);
	}
	else
		m_Net1Up = 0;

	if(goingDown1)
	{
		g_Au.DecayNet1Downloading(netDecay);
		m_Net1Down++;
		int frame = m_Net1Down%frames;
		float offset = (float)(frames - frame - 1)/(float)frames;
		QTransform model;
		model.translate(width()/2, height()/2);
		model.rotate(-(90+15));
		model.translate(-width()/2, -height()/2);
		qreal tx = cos(45*DEG2RAD)*(offset * width());
		qreal ty = -sin(45*DEG2RAD)*(offset * height());
		model.translate(tx, ty);
		p.setTransform(model);
		p.drawImage(rect(), m_NetImageDown);
	}
	else
		m_Net1Down = 0;

	if(goingUp2)
	{
		g_Au.DecayNet2Uploading(netDecay);
		m_Net2Up++;
		int frame = m_Net2Up%frames;
		float offset = (float)frame/(float)frames;
		QTransform model;
		model.translate(width()/2, height()/2);
		model.rotate(-15);
		model.translate(-width()/2, -height()/2);
		qreal tx = cos(45*DEG2RAD)*(offset * width());
		qreal ty = -sin(45*DEG2RAD)*(offset * height());
		model.translate(tx, ty);
		p.setTransform(model);
		p.drawImage(rect(), m_NetImageUp);
	}
	else
		m_Net2Up = 0;

	if(goingDown2)
	{
		g_Au.DecayNet2Downloading(netDecay);
		m_Net2Down++;
		int frame = m_Net2Down%frames;
		float offset = (float)(frames - frame - 1)/(float)frames;
		QTransform model;
		model.translate(width()/2, height()/2);
		model.rotate(15);
		model.translate(-width()/2, -height()/2);
		qreal tx = cos(45*DEG2RAD)*(offset * width());
		qreal ty = -sin(45*DEG2RAD)*(offset * height());
		model.translate(tx, ty);
		p.setTransform(model);
		p.drawImage(rect(), m_NetImageDown);
	}
	else
		m_Net2Down = 0;

	if(goingUp3)
	{
		g_Au.DecayNet3Uploading(netDecay);
		m_Net3Up++;
		int frame = m_Net3Up%frames;
		float offset = (float)frame/(float)frames;
		QTransform model;
		model.translate(width()/2, height()/2);
		model.rotate(-(180+15));
		model.translate(-width()/2, -height()/2);
		qreal tx = cos(45*DEG2RAD)*(offset * width());
		qreal ty = -sin(45*DEG2RAD)*(offset * height());
		model.translate(tx, ty);
		p.setTransform(model);
		p.drawImage(rect(), m_NetImageUp);
	}
	else
		m_Net3Up = 0;

	if(goingDown3)
	{
		g_Au.DecayNet3Downloading(netDecay);
		m_Net3Down++;
		int frame = m_Net3Down%frames;
		float offset = (float)(frames - frame - 1)/(float)frames;
		QTransform model;
		model.translate(width()/2, height()/2);
		model.rotate(-(180-15));
		model.translate(-width()/2, -height()/2);
		qreal tx = cos(45*DEG2RAD)*(offset * width());
		qreal ty = -sin(45*DEG2RAD)*(offset * height());
		model.translate(tx, ty);
		p.setTransform(model);
		p.drawImage(rect(), m_NetImageDown);
	}
	else
		m_Net3Down = 0;

	if(goingUp4)
	{
		g_Au.DecayNet4Uploading(netDecay);
		m_Net4Up++;
		int frame = m_Net4Up%frames;
		float offset = (float)frame/(float)frames;
		QTransform model;
		model.translate(width()/2, height()/2);
		model.rotate(-(-90-15));
		model.translate(-width()/2, -height()/2);
		qreal tx = cos(45*DEG2RAD)*(offset * width());
		qreal ty = -sin(45*DEG2RAD)*(offset * height());
		model.translate(tx, ty);
		p.setTransform(model);
		p.drawImage(rect(), m_NetImageUp);
	}
	else
		m_Net4Up = 0;

	if(goingDown4)
	{
		g_Au.DecayNet4Downloading(netDecay);
		m_Net4Down++;
		int frame = m_Net4Down%frames;
		float offset = (float)(frames - frame - 1)/(float)frames;
		QTransform model;
		model.translate(width()/2, height()/2);
		model.rotate(-(-90+15));
		model.translate(-width()/2, -height()/2);
		qreal tx = cos(45*DEG2RAD)*(offset * width());
		qreal ty = -sin(45*DEG2RAD)*(offset * height());
		model.translate(tx, ty);
		p.setTransform(model);
		p.drawImage(rect(), m_NetImageDown);
	}
	else
		m_Net4Down = 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

