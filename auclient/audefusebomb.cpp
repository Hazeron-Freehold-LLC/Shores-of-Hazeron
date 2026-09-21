//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audefusebomb.cpp,v 1.9 2022/04/15 17:01:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audefusebomb.h"
//
#include "auglobal.h"
#include "auapp.h"
#include "auframe.h"
#include "ausound.h"
#include "auview.h"
#include <QGridLayout>
#include <QMouseEvent>
#include <QPaintEvent>

AuDefuseBombWidget::AuDefuseBombWidget(const AuId& bombId, QWidget* parent) : QWidget(parent)
{
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setMouseTracking(true);

	m_ActiveWire = -1;
	m_BombId = bombId;
	m_NumVal = 0;
	m_WhizzerMs = 0;

	// Initialize wires.
	int defuseWire = bombId.GetID() % 4;						// So the server can determine which wire is the defuse wire.
	AuDice dice;
	dice.Seed(bombId.GetID());
	QList<AuDefuseBombWire::WIRECOLOR> colors;
	colors << AuDefuseBombWire::Black << AuDefuseBombWire::Green << AuDefuseBombWire::Purple << AuDefuseBombWire::Red;
	QList<AuDefuseBombWire::WIREEFFECT> effects;
	effects << AuDefuseBombWire::Detonate << AuDefuseBombWire::Nothing << AuDefuseBombWire::Whizzer;
	for(int wire=0; wire<4; wire++)
	{
		int coli = dice.Roll(1, colors.count(), -1);
		m_Wires[wire].m_Color = colors[coli];
		colors.removeAt(coli);

		if(wire == defuseWire)
			m_Wires[wire].m_Effect = AuDefuseBombWire::Defuse;
		else
		{
			int effi = dice.Roll(1, effects.count(), -1);
			m_Wires[wire].m_Effect = effects[effi];
			effects.removeAt(effi);
		}

		m_Wires[wire].m_IsCut = false;
		m_Wires[wire].m_Orientation = (AuDefuseBombWire::WIREORIENT)dice.Roll(1,4,-1);
	}
}

AuDefuseBombWidget::~AuDefuseBombWidget()
{
}

QSize AuDefuseBombWidget::sizeHint() const
{
	return QSize(400, 400);
}

void AuDefuseBombWidget::setVisible(bool visible)
{
	if(visible)
	{
		QWidget::setVisible(visible);

		QTimer* timer = new QTimer;
		connect(timer, SIGNAL(timeout()), SLOT(update()));
		timer->start(100);
		m_TimeUp.start();
	}
	else if(isVisible())
	{
		QWidget::setVisible(visible);
	}
}

void AuDefuseBombWidget::mousePressEvent(QMouseEvent* event)
{
	const int yrows = 4;
	m_ActiveWire = event->pos().y() / (height()/yrows);

	if(!m_Wires[m_ActiveWire].m_IsCut)
	{
		auApp->PlayClientSound(SOUND_SnipWire);

		m_WhizzerTime = AuElapsedTimer();								// Cutting any wire stops the whizzer timer.
		m_Wires[m_ActiveWire].m_IsCut = true;
		update();

		if(m_Wires[m_ActiveWire].m_Effect == AuDefuseBombWire::Defuse)
		{
			//qDebug() << "AuDefuseBombWidget::mousePressEvent cut wire" << m_ActiveWire << "defused bomb";
			emit WireCut(m_ActiveWire);
		}
		else if(m_Wires[m_ActiveWire].m_Effect == AuDefuseBombWire::Detonate)
		{
			//qDebug() << "AuDefuseBombWidget::mousePressEvent cut wire" << m_ActiveWire << "detonated bomb";
			emit WireCut(m_ActiveWire);
		}
		else if(m_Wires[m_ActiveWire].m_Effect == AuDefuseBombWire::Whizzer)
		{
			//qDebug() << "AuDefuseBombWidget::mousePressEvent cut wire" << m_ActiveWire << "started whizzer";
			m_WhizzerTime.start();
			auApp->PlayClientSound(SOUND_Whizzer);
		}
		//else
		//{
			//qDebug() << "AuDefuseBombWidget::mousePressEvent cut wire" << m_ActiveWire << "did nothing";
		//}
	}
}

void AuDefuseBombWidget::mouseMoveEvent(QMouseEvent* event)
{
	const int yrows = 4;
	int wire = event->pos().y() / (height()/yrows);
	if(wire != m_ActiveWire)
	{
		m_ActiveWire = wire;
		update();
	}
}

void AuDefuseBombWidget::paintEvent(QPaintEvent* pe)
{
	QWidget::paintEvent(pe);

	QPainter p(this);

	// Backdrop.
	{
		QImage imgBack(":data/CircuitBoard.png");
		p.drawImage(QRect(0,0,width(),height()), imgBack, QRect(0,0,imgBack.width(),imgBack.height()));
	}

	// Whizzer
	{
		int size = height()/5;
		int top = height()/5;
		QRect rcwsec(width()*2/3, top, size, size);
		QRect rcwmsec(width()*2/3, top+size+4, size, size);

		int secrem;
		int msecrem;
		if(m_WhizzerTime.isValid())
			m_WhizzerMs = m_WhizzerTime.elapsed();				// Whizzer timer is running.
		secrem = 3000 - m_WhizzerMs;
		if(secrem > 0)
		{
			int wholesec = secrem / 1000;
			msecrem = secrem - (wholesec*1000);
		}
		else
		{
			secrem = 0;
			msecrem = 0;
			if(m_WhizzerTime.isValid())
			{
				// Emit a random wire index from 0..9. There is a 10% chance this will defuse the bomb. Otherwise it explodes.
				emit WireCut(g_Au.Dice().Roll(1,10,-1));
				m_WhizzerMs = 3000;							// Show zero time remaining on the whizzer.
				m_WhizzerTime = AuElapsedTimer();					// Stop the whizzer timer.
			}
		}

		// Backdrop.
		p.setPen(QPen(QColor(0,255,0,128), 0.f));
		p.setBrush(QBrush(QColor(0,32,0,128)));
		int clockStart = 90 * 16;
		p.drawPie(rcwsec, clockStart, 360*16);
		p.drawPie(rcwmsec, clockStart, 360*16);

		// Seconds
		p.setBrush(QBrush(QColor(0,128,0,128)));
		int secSweep = -360 * secrem / 3000 * 16;
		p.drawPie(rcwsec, clockStart, secSweep);

		// Milliseconds
		int msecSweep = -360 * msecrem / 1000 * 16;
		p.drawPie(rcwmsec, clockStart, msecSweep);
	}

	// Random number value.
	{
		if(AnyWireCut())
		{
			// Number stops changing when any wire has been cut.
			p.setPen(QPen(QColor(255,128,0), 0.));
		}
		else
		{
			// Number changes every second when operating normally.
			int sec = m_TimeUp.elapsed() / 1000;
			if(!m_NumVal || sec != m_NumSec)
			{
				m_NumVal = g_Au.Dice().Roll(1,0xfffffff);
				m_NumSec = sec;
			}

			p.setPen(QPen(QColor(0,255,0), 0.));
		}
		QString sval = QString("0x%1").arg(m_NumVal, 0, 16, QChar('0'));
		p.setFont(QFont("Helvetica", 20, QFont::DemiBold, false));
		p.drawText(width()/4, height()*3/4, sval);
	}

	// Blinking red light.
	{
		int fracsec;
		if(!m_WhizzerTime.isValid())
			fracsec = m_TimeUp.elapsed() / 600;
		else
			fracsec = m_TimeUp.elapsed() / 100;
		if(fracsec&0x01)
		{
			QImage imgLight(":data/TabClose.png");
			p.drawImage(QRect(width()/4,height()/4, width()/2, height()/2), imgLight);
		}
	}

	// Wires
	{
		for(int wire=0; wire<4; wire++)
		{
			QImage imgWire;
			switch(m_Wires[wire].m_Color)
			{
			case AuDefuseBombWire::Black:
				if(m_Wires[wire].m_IsCut)
					imgWire.load(":data/BlackWireCut.png");
				else
					imgWire.load(":data/BlackWire.png");
				break;
			case AuDefuseBombWire::Green:
				if(m_Wires[wire].m_IsCut)
					imgWire.load(":data/GreenWireCut.png");
				else
					imgWire.load(":data/GreenWire.png");
				break;
			case AuDefuseBombWire::Purple:
				if(m_Wires[wire].m_IsCut)
					imgWire.load(":data/PurpleWireCut.png");
				else
					imgWire.load(":data/PurpleWire.png");
				break;
			case AuDefuseBombWire::Red:
				if(m_Wires[wire].m_IsCut)
					imgWire.load(":data/RedWireCut.png");
				else
					imgWire.load(":data/RedWire.png");
				break;
			default:
				continue;
			}

			// Scale the wire image to the width of the widget.
			imgWire = imgWire.scaledToWidth(width(), Qt::SmoothTransformation);

			// Mirror to appropriate orientation.
			switch(m_Wires[wire].m_Orientation)
			{
			case AuDefuseBombWire::Normal:
			default:
				break;
			case AuDefuseBombWire::FlipX:
				imgWire = imgWire.mirrored(true, false);
				break;
			case AuDefuseBombWire::FlipY:
				imgWire = imgWire.mirrored(false, true);
				break;
			case AuDefuseBombWire::FlipXY:
				imgWire = imgWire.mirrored(true, true);
				break;
			}

			// Draw the wire image.
			int ypos = height() * wire / 4;
			p.drawImage(QPoint(0,ypos), imgWire);

			if(wire == m_ActiveWire)
				p.fillRect(QRect(0,ypos,width(),height()/4), QColor(255,128,0,64));
		}
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDefuseBomb::AuDefuseBomb(const AuId& bombId, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Defuse Bomb"));
	setAuDlgIcon(QIcon(":data/c_PlasticExplosive.png"));

	m_CutWireIndex = 0;

	AuDefuseBombWidget* dbw = new AuDefuseBombWidget(bombId);
	connect(dbw, SIGNAL(WireCut(qint8)), SLOT(WireCut(qint8)));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(dbw, row,0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuDefuseBomb::~AuDefuseBomb()
{
}

void AuDefuseBomb::WireCut(qint8 wireIndex)
{
	//qDebug() << "AuDefuseBomb::WireCut" << wireIndex;
	m_CutWireIndex = wireIndex;
	accept();
}
