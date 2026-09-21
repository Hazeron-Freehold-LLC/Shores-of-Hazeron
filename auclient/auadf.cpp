//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auadf.cpp,v 1.4 2022/02/19 18:03:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auadf.h"
//
#include "auglobal.h"

#define d_TitleBarMargin				26

AuADF::AuADF(const QString& title, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(title);
	setAuDlgIcon(QIcon(":data/Padlock.png"));

	setAttribute(Qt::WA_OpaquePaintEvent, true);

	quint16 lastcode = g_Au.Settings().readNumEntry("Widget/SystemAccessCode/LastEntry", 1000);
	if(lastcode < 1000)
	{
		m_Ones = lastcode;
		m_Hundreds = m_Ones / 100;
		m_Ones -= m_Hundreds * 100;
		m_Tens = m_Ones / 10;
		m_Ones -= m_Tens * 10;
	}
	else
	{
		m_Ones = 0;
		m_Tens = 0;
		m_Hundreds = 0;
	}
}

AuADF::~AuADF()
{
}

void AuADF::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

QSize AuADF::sizeHint() const
{
	QImage imgPanel(":data/ADFPanel.png");
	return QSize(imgPanel.size().width(), imgPanel.size().height() + d_TitleBarMargin);
}

void AuADF::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SystemAccessCode/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SystemAccessCode/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SystemAccessCode/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SystemAccessCode/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuADF::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SystemAccessCode/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SystemAccessCode/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SystemAccessCode/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SystemAccessCode/SizeY",sz.height());

	g_Au.Settings().writeEntry("Widget/SystemAccessCode/LastEntry", AccessCode());
}

quint16 AuADF::AccessCode() const
{
	quint16 sac = m_Hundreds * 100 + m_Tens * 10 + m_Ones;
	return sac;
}

void AuADF::paintEvent(QPaintEvent* pe)
{
	QPainter p(this);

	// Panel background.
	QRect rc(rect().left(), rect().top() + d_TitleBarMargin, rect().width(), rect().height() - d_TitleBarMargin);
	QImage imgPanel(":data/ADFPanel.png");
	p.drawImage(rc, imgPanel);

	float sx = (float)width() / (float)imgPanel.width();
	float sy = (float)(height() - d_TitleBarMargin) / (float)imgPanel.height();
	if(sx <= 0 || sy <= 0)
		return;

	// Frame around digital display.
	QImage imgDisplay(":data/ADFDisplay.png");
	int dl = 50 * sx;
	int dt = 78 * sy;
	int dw = imgDisplay.width() * sx;
	int dh = imgDisplay.height() * sy;
	QRect rcDisplay(dl, d_TitleBarMargin + dt, dw, dh);
	p.drawImage(rcDisplay, imgDisplay);

	// Knobs.
	QImage imgKnob(":data/ADFKnob.png");
	int hc = rc.width()/2;							// Center of panel from left to right.
	int hl = hc * 2 / 3;
	int hr = rc.width() - hl;
	int tc = (rc.top() + rcDisplay.top()) / 2;		// Y distance from top to center of top button row.
	int bc = (rcDisplay.top() + rcDisplay.height() + rc.bottom())/2;
	int kw = imgKnob.width() * sx;					// Scaled width of a knob.
	int kh = imgKnob.height() * sy;					// Scaled height of a knob.

	QRect rctl(hl - kw/2, tc - kh/2, kw, kh);
	p.drawImage(rctl, imgKnob);
	QRect rctc(hc - kw/2, tc - kh/2, kw, kh);
	p.drawImage(rctc, imgKnob);
	QRect rctr(hr - kw/2, tc - kh/2, kw, kh);
	p.drawImage(rctr, imgKnob);

	QRect rcbl(hl - kw/2, bc - kh/2, kw, kh);
	p.drawImage(rcbl, imgKnob);
	QRect rcbc(hc - kw/2, bc - kh/2, kw, kh);
	p.drawImage(rcbc, imgKnob);
	QRect rcbr(hr - kw/2, bc - kh/2, kw, kh);
	p.drawImage(rcbr, imgKnob);

	// Digits
	QRect rcDigits(56 * sx,38 * sy,190 * sx,40 * sy);	// Digital display area measured off the bitmap.
	rcDigits.translate(rcDisplay.left(), rcDisplay.top());
	int nc = (rcDigits.left() + rcDigits.right())/2;
	int nl = rcDigits.left() + rcDigits.width() / 4;
	int nr = rcDigits.right() - rcDigits.width() / 4;
	int nt = rcDigits.top();
	QImage imgOnes(QString(":data/ADF%1.png").arg(m_Ones));
	QRect rcOnes(nr - imgOnes.width()/2*sx, nt, imgOnes.width()*sx, imgOnes.height()*sy);
	p.drawImage(rcOnes, imgOnes);

	QImage imgTens(QString(":data/ADF%1.png").arg(m_Tens));
	QRect rcTens(nc - imgTens.width()/2*sx, nt, imgTens.width()*sx, imgTens.height()*sy);
	p.drawImage(rcTens, imgTens);

	QImage imgHundreds(QString(":data/ADF%1.png").arg(m_Hundreds));
	QRect rcHundreds(nl - imgHundreds.width()/2*sx, nt, imgHundreds.width()*sx, imgHundreds.height()*sy);
	p.drawImage(rcHundreds, imgHundreds);

	QDialog::paintEvent(pe);
}
