//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austatushistogram.cpp,v 1.29 2022/02/24 16:31:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austatushistogram.h"
//
#include "auservermonapp.h"
#include <dttick.h>
#include <QPainter>

#define d_RangeMax	100

AuStatusHistogram::AuStatusHistogram(const QList<AuStatusReport>* status, int mode, QWidget* parent) : QWidget(parent)
{
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setMinimumHeight(104);	// 100 pixels (1 per ms) plus margin 2 top and bottom.
	m_Status = status;
	m_Mode = mode;
	m_TMinus = 0;
}

AuStatusHistogram::~AuStatusHistogram()
{
}

void AuStatusHistogram::paintEvent(QPaintEvent* pe)
{
	int refPos = width() - m_TMinus - 1;
	if(auServerMonApp->widgetAt(QCursor::pos()) == this)
	{
		QPoint cursorPos = mapFromGlobal(QCursor::pos());
		if(cursorPos.x() >= 0 && cursorPos.x() < width() && cursorPos.y() >= 0 && cursorPos.y() < height())
		{
			refPos = cursorPos.x();
			m_TMinus = width() - refPos - 1;
			emit TMinus(m_TMinus);
		}
	}

	QPainter p(this);

	// Fill background with a dark color.
	QBrush brBack(QColor(0,0,0));
	p.fillRect(0,0, width(),height(), brBack);

	// Draw grid
	int margin = 2;
	int drawHt = height() - margin*2;			// Allow a margin at the top and bottom.
	p.setPen(QPen(QColor(20,20,20), 0.f));
	for(int value=0; value<=d_RangeMax; value+=10)
	{
		int curY = value * drawHt / (d_RangeMax+1);				// Scale to a range of 0 to rangeMax. (1/2ms incs.)
		curY = (height()-1) - curY - margin;	// Invert so measure is from bottom.
		p.drawLine(0,curY, width()-1,curY);
	}
	int vertLines = 0;
	int ix = width()-1;
	while(ix >= 0)
	{
		if(vertLines%6 == 0)
			p.setPen(QPen(QColor(30,30,30), 0.f));
		else
			p.setPen(QPen(QColor(20,20,20), 0.f));
		p.drawLine(ix,0, ix, height()-1);
		ix -= 10;	// Every poll is 1 second. Grid marks every 10 seconds should be good.
		vertLines++;
	}
	if(refPos)
	{
		p.setPen(QPen(QColor(32,100,100), 0.f));
		p.drawLine(refPos,0, refPos, height()-1);
	}

	// Draw from right to left, newest to oldest.
	bool isFirst = true;
	unsigned int totalAtoms = 0;
	unsigned int maxAtomMs = 0;
	unsigned int avgAtomMs = 0;
	unsigned int minAtomMs = 0;
	unsigned int atomManagers = 0;
	quint32 maxAtomId = 0;
	quint32 minAtomId = 0;
	int curX = width()-1;
	for(QList<AuStatusReport>::const_iterator sai = m_Status->constEnd()-1; sai != m_Status->constBegin()-1; sai--)
	{
		// Determine the average, min, and max of the average.
		// These values are in 1/2ms increments. i.e. 100 = 50ms.
		unsigned int avg = 0;
		unsigned int min = 0;
		unsigned int max = 0;
		quint32 maxAtom = 0;
		quint32 minAtom = 0;
		if(!(*sai).m_Atoms.isEmpty())
		{
			if(m_Mode == 0)
			{
				for(QList<AuStatusAtom>::const_iterator ai = (*sai).m_Atoms.constBegin(); ai != (*sai).m_Atoms.constEnd(); ai++)
				{
					if(ai == (*sai).m_Atoms.constBegin())
					{
						avg = (*ai).m_AvgTick;
						min = (*ai).m_AvgTick;
						max = (*ai).m_AvgTick;
						maxAtom = (*ai).m_AtomId;
						minAtom = (*ai).m_AtomId;
					}
					else
					{
						avg += (*ai).m_AvgTick;
						if((*ai).m_AvgTick < min)
							min = (*ai).m_AvgTick;
						if((*ai).m_AvgTick > max)
						{
							max = (*ai).m_AvgTick;
							maxAtom = (*ai).m_AtomId;
						}
						else if((*ai).m_AvgTick < min)
						{
							min = (*ai).m_AvgTick;
							minAtom = (*ai).m_AtomId;
						}
					}
				}
			}
			else if(m_Mode == 1)
			{
				for(QList<AuStatusAtom>::const_iterator ai = (*sai).m_Atoms.constBegin(); ai != (*sai).m_Atoms.constEnd(); ai++)
				{
					if(ai == (*sai).m_Atoms.constBegin())
					{
						avg = (*ai).m_MaxTick;
						min = (*ai).m_MaxTick;
						max = (*ai).m_MaxTick;
						maxAtom = (*ai).m_AtomId;
						minAtom = (*ai).m_AtomId;
					}
					else
					{
						avg += (*ai).m_MaxTick;
						if((*ai).m_MaxTick < min)
							min = (*ai).m_MaxTick;
						if((*ai).m_MaxTick > max)
						{
							max = (*ai).m_MaxTick;
							maxAtom = (*ai).m_AtomId;
						}
						else if((*ai).m_MaxTick < min)
						{
							min = (*ai).m_MaxTick;
							minAtom = (*ai).m_AtomId;
						}
					}
				}
			}
			else if(m_Mode == 2)
			{
				for(QList<AuStatusAtom>::const_iterator ai = (*sai).m_Atoms.constBegin(); ai != (*sai).m_Atoms.constEnd(); ai++)
				{
					if(ai == (*sai).m_Atoms.constBegin())
					{
						avg = (*ai).m_AvgTickCycle;
						min = (*ai).m_AvgTickCycle;
						max = (*ai).m_AvgTickCycle;
						maxAtom = (*ai).m_AtomId;
						minAtom = (*ai).m_AtomId;
					}
					else
					{
						avg += (*ai).m_AvgTickCycle;
						if((*ai).m_AvgTickCycle < min)
							min = (*ai).m_AvgTickCycle;
						if((*ai).m_AvgTickCycle > max)
						{
							max = (*ai).m_AvgTickCycle;
							maxAtom = (*ai).m_AtomId;
						}
						else if((*ai).m_AvgTickCycle < min)
						{
							min = (*ai).m_AvgTickCycle;
							minAtom = (*ai).m_AtomId;
						}
					}
				}
			}
			avg /= (*sai).m_Atoms.count();
		}

		if((isFirst && !refPos) || (refPos && curX == refPos))
		{
			// First pass is actually the most recent data.
			// We will print some text about the most recent data.
			atomManagers = (*sai).m_AtomManagers;
			totalAtoms = (*sai).m_Atoms.count();
			maxAtomId = maxAtom;
			maxAtomMs = max;
			minAtomId = minAtom;
			minAtomMs = min;
			avgAtomMs = avg;
			isFirst = false;
		}

		// Draw average, min, and max.
		Plot(p, curX, margin, avg);
		Plot(p, curX, margin, min);
		Plot(p, curX, margin, max);

		curX--;
		if(curX < 0)
			break;
	}

	// Overlay some text.
	p.setPen(QPen(QColor(200,200,255), 0.f));
	if(totalAtoms || atomManagers)
		p.drawText(margin,16, QString("%1 Active Solar Systems, %2 Managers").arg(totalAtoms).arg(atomManagers));
	if(totalAtoms)
	{
		p.setPen(DtTick::Color(maxAtomMs, 128));
		if(m_Mode == 0)
			p.drawText(margin,32, QString("Longest Avg Tick: %2ms Atom %1").arg(maxAtomId).arg((float)maxAtomMs/2.f));
		else if(m_Mode == 1)
			p.drawText(margin,32, QString("Longest Max Tick: %2ms Atom %1").arg(maxAtomId).arg((float)maxAtomMs/2.f));
		else if(m_Mode == 2)
			p.drawText(margin,32, QString("Longest Avg Cycle: %2ms Atom %1").arg(maxAtomId).arg((float)maxAtomMs/2.f));

		p.setPen(DtTick::Color(avgAtomMs, 128));
		if(m_Mode == 0)
			p.drawText(margin,48, QString("Average Avg Tick: %2ms").arg((float)avgAtomMs/2.f));
		else if(m_Mode == 1)
			p.drawText(margin,48, QString("Average Max Tick: %2ms").arg((float)avgAtomMs/2.f));
		else if(m_Mode == 2)
			p.drawText(margin,48, QString("Average Avg Cycle: %2ms").arg((float)avgAtomMs/2.f));

		p.setPen(DtTick::Color(minAtomMs, 128));
		if(m_Mode == 0)
			p.drawText(margin,64, QString("Shortest Avg Tick: %2ms Atom %1").arg(minAtomId).arg((float)minAtomMs/2.f));
		else if(m_Mode == 1)
			p.drawText(margin,64, QString("Shortest Max Tick: %2ms Atom %1").arg(minAtomId).arg((float)minAtomMs/2.f));
		else if(m_Mode == 2)
			p.drawText(margin,64, QString("Shortest Avg Cycle: %2ms Atom %1").arg(minAtomId).arg((float)minAtomMs/2.f));
	}

	p.end();
	
	QWidget::paintEvent(pe);
}

void AuStatusHistogram::Plot(QPainter& p, int curX, int margin, int value)
{
	// Select an appropriate pen color.
	p.setPen(DtTick::Color(value, 128));

	// Plot the point.
	int drawHt = height() - margin*2;							// Allow a margin at the top and bottom.
	if(value > d_RangeMax*2)
		value = d_RangeMax*2;
	int curY = value * drawHt / (d_RangeMax*2+2);				// Scale to a range of 0 to rangeMax. (1/2ms incs)
	curY = (height()-1) - curY - margin;						// Invert so measure is from bottom.
	p.drawPoint(curX, curY);
}

QSize AuStatusHistogram::sizeHint() const
{
	return QSize(500,204);
}

void AuStatusHistogram::SetTMinus(int ms)
{
	m_TMinus = ms;
}
