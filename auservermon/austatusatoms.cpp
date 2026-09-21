//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austatusatoms.cpp,v 1.23 2022/02/19 18:40:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austatusatoms.h"
//
#include "auservermonapp.h"
#include "dttick.h"
#include "dttock.h"
#include <QPainter>

#define d_RangeMax	100

AuStatusAtoms::AuStatusAtoms(const QList<AuStatusReport>* status, QWidget* parent) : QWidget(parent)
{
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setMinimumHeight(104);	// 100 pixels (1 per ms) plus margin 2 top and bottom.
	m_Status = status;
	m_TMinus = 0;
}

AuStatusAtoms::~AuStatusAtoms()
{
}

void AuStatusAtoms::paintEvent(QPaintEvent* pe)
{
	int refPos = width() - 1;
	if(auServerMonApp->widgetAt(QCursor::pos()) == this)
	{
		QPoint cursorPos = mapFromGlobal(QCursor::pos());
		if(cursorPos.x() >= 0 && cursorPos.x() < width() && cursorPos.y() >= 0 && cursorPos.y() < height())
			refPos = cursorPos.x();
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
	if(refPos)
	{
		p.setPen(QPen(QColor(32,100,100), 0.f));
		p.drawLine(refPos,0, refPos, height()-1);
	}

	AuId systemId;
	if(!m_Status->isEmpty())
	{
		// We will be making a row of vertical bars, one for each atom.
		QList<AuStatusReport>::const_iterator sai;
		if(m_Status->count() > m_TMinus)
			sai = m_Status->constEnd() - 1 - m_TMinus;
		else
			sai = m_Status->constEnd() - 1;
		if((*sai).m_Atoms.count())
		{
			// Calculate the width of each vertical bar.
			int groupwid = width() / (*sai).m_Atoms.count();
			int gutter = 0;	// Extra space added between column groups.

			const int columnsPerGroup = 2;	// avg and cycle
			int barwid = groupwid / columnsPerGroup;
			if(!barwid)
				barwid = 1;
			else if(barwid > 20)
				barwid = 20;

			if(barwid >= 3)
			{
				// The gutter takes one pixel from each of our bars.
				gutter = columnsPerGroup;
				barwid--;
			}

			// Draw vertical background grid lines now that we know the spacing.
			p.setPen(QPen(QColor(20,20,20), 0.f));
			int gwid = barwid*2 + gutter;
			int drawWid = width();			// Allow a margin at the left and right.
			for(int value=0; value<=drawWid; value+=gwid)
			{
				int curX = value - 1;
				p.drawLine(curX,0, curX,height()-1);
			}

			int drawwid = barwid-1;
			if(!drawwid)
				drawwid = 1;

			int totalAvg = 0;
			int totalMax = 0;
			int totalAvgCycle = 0;
			int totalMaxCycle = 0;
			int cycles = 0;
			QList<AuStatusAtom>::const_iterator refai = (*sai).m_Atoms.constEnd();
			int xpos = 0;
			int drawHt = height() - margin*2;			// Allow a margin at the top and bottom.
			for(QList<AuStatusAtom>::const_iterator ai = (*sai).m_Atoms.constBegin(); ai != (*sai).m_Atoms.constEnd(); ai++)
			{
				// Remember iterator of the atom under the cursor.
				if(refPos >= xpos && refPos < xpos+barwid*3)
					refai = ai;

				int avgTick = (*ai).m_AvgTick;
				int maxTick = (*ai).m_MaxTick;
				int avgCycle = (*ai).m_AvgTickCycle;
				int maxCycle = (*ai).m_MaxTickCycle;
				int avgTock = (*ai).m_AvgTock;
				int maxTock = (*ai).m_MaxTock;

				// Accumulate totals.
				totalAvg += avgTick;
				totalMax += maxTick;

				if(avgCycle)
				{
					totalAvgCycle += avgCycle;
					cycles++;
				}

				if(maxCycle > totalMaxCycle)
					totalMaxCycle = maxCycle;

				{
					// Select an appropriate pen color.
					p.setPen(DtTick::Color(avgTick, 128));

					// Plot the vertical bar.
					if(avgTick > d_RangeMax*2)
						avgTick = d_RangeMax*2;
					int curY = avgTick * drawHt / (d_RangeMax*2+2);	// Scale to a range of 0 to rangeMax. (1/2ms incs)
					curY = (height()-1) - curY - margin;	// Invert so measure is from bottom.
					for(int x=0; x<drawwid; x++)
						p.drawLine(xpos + x,(height()-1)-margin, xpos + x,curY);

					// Select an appropriate pen color.
					p.setPen(DtTick::Color(maxTick, 128));

					// Plot the horizontal bar at the max.
					if(maxTick > d_RangeMax*2)
						maxTick = d_RangeMax*2;
					curY = maxTick * drawHt / (d_RangeMax*2+2);		// Scale to a range of 0 to rangeMax. (1/2ms incs)
					curY = (height()-1) - curY - margin;	// Invert so measure is from bottom.
					p.drawLine(xpos,curY, xpos+drawwid-1,curY);

					xpos += barwid;
				}

				{
					// Select an appropriate pen color.
					p.setPen(DtTick::Color(avgCycle, 128));

					// Plot the vertical bar.
					if(avgCycle > d_RangeMax*2)
						avgCycle = d_RangeMax*2;
					int curY = avgCycle * drawHt / (d_RangeMax*2+2);	// Scale to a range of 0 to rangeMax. (1/2ms incs)
					curY = (height()-1) - curY - margin;	// Invert so measure is from bottom.
					for(int x=0; x<drawwid; x++)
						p.drawLine(xpos + x,(height()-1)-margin, xpos + x,curY);

					// Select an appropriate pen color.
					p.setPen(DtTick::Color(maxCycle, 128));

					// Plot the horizontal bar at the max.
					if(maxCycle > d_RangeMax*2)
						maxCycle = d_RangeMax*2;
					curY = maxCycle * drawHt / (d_RangeMax*2+2);		// Scale to a range of 0 to rangeMax. (1/2ms incs)
					curY = (height()-1) - curY - margin;	// Invert so measure is from bottom.
					p.drawLine(xpos,curY, xpos+drawwid-1,curY);

					xpos += barwid;
				}

				{
					// Select an appropriate pen color.
					p.setPen(DtTock::Color(avgTock, 128));
					avgTock /= 10;	// Squish into a range that works beside the tick bars.

					// Plot the vertical bar.
					if(avgTock > d_RangeMax*2)
						avgTock = d_RangeMax*2;
					int curY = avgTock * drawHt / (d_RangeMax*2+2);	// Scale to a range of 0 to rangeMax. (1/2ms incs)
					curY = (height()-1) - curY - margin;	// Invert so measure is from bottom.
					for(int x=0; x<drawwid; x++)
						p.drawLine(xpos + x,(height()-1)-margin, xpos + x,curY);

					// Select an appropriate pen color.
					p.setPen(DtTock::Color(maxTock, 128));
					maxTock /= 10;	// Squish into a range that works beside the tick bars.

					// Plot the horizontal bar at the max.
					if(maxTock > d_RangeMax*2)
						maxTock = d_RangeMax*2;
					curY = maxTock * drawHt / (d_RangeMax*2+2);		// Scale to a range of 0 to rangeMax. (1/2ms incs)
					curY = (height()-1) - curY - margin;	// Invert so measure is from bottom.
					p.drawLine(xpos,curY, xpos+drawwid-1,curY);

					xpos += barwid;
				}

				xpos += gutter;
			}

			if(cycles)
				totalAvgCycle /= cycles;

			// Overlay some text.
			p.setPen(QPen(QColor(200,200,255), 0.f));
			int row = 1;
			p.drawText(margin,row*16, QString("Sum Max Tick %1ms / %2 Managers = %3ms per Manager")
					.arg(totalMax)
					.arg((*sai).m_AtomManagers)
					.arg((totalMax/2.f) / (*sai).m_AtomManagers));
			row++;
			p.drawText(margin,row*16, QString("Sum Avg Tick %1ms / %2 Managers = %3ms per Manager")
					.arg(totalAvg/2.f)
					.arg((*sai).m_AtomManagers)
					.arg((totalAvg/2.f) / (*sai).m_AtomManagers));
			row++;
			p.drawText(margin,row*16, QString("Max Cycle %1ms")
					.arg(totalMaxCycle/2.f));
			row++;
			p.drawText(margin,row*16, QString("Avg Cycle %1ms")
					.arg(totalAvgCycle/2.f));
			row++;

			// Display text about the atom under the cursor.
			if(refai != (*sai).m_Atoms.constEnd())
			{
				systemId = (*refai).m_AtomId;
				p.drawText(margin,row*16, QString("Solar System %1").arg((*refai).m_AtomId));
				row++;

				int avgTick = (*refai).m_AvgTick;
				int maxTick = (*refai).m_MaxTick;
				int avgCycle = (*refai).m_AvgTickCycle;
				int maxCycle = (*refai).m_MaxTickCycle;
				int avgTock = (*refai).m_AvgTock;
				int maxTock = (*refai).m_MaxTock;

				p.setPen(DtTick::Color(maxTick, 128));
				p.drawText(margin,row*16, QString("Max Tick: %1ms").arg((float)maxTick/2.f));
				row++;

				p.setPen(DtTick::Color(avgTick, 128));
				p.drawText(margin,row*16, QString("Avg Tick: %1ms").arg((float)avgTick/2.f));
				row++;

				p.setPen(DtTick::Color(maxCycle, 128));
				p.drawText(margin,row*16, QString("Max Cycle: %1ms").arg((float)maxCycle/2.f));
				row++;

				p.setPen(DtTick::Color(avgCycle, 128));
				p.drawText(margin,row*16, QString("Avg Cycle: %1ms").arg((float)avgCycle/2.f));
				row++;

				p.setPen(DtTock::Color(maxTock, 128));
				p.drawText(margin,row*16, QString("Max Tock: %1ms").arg((float)maxTock/2.f));
				row++;

				p.setPen(DtTock::Color(avgTock, 128));
				p.drawText(margin,row*16, QString("Avg Tock: %1ms").arg((float)avgTock/2.f));
				row++;
			}
		}
	}
	else
	{
		// Draw vertical background grid lines now that we know the spacing.
		p.setPen(QPen(QColor(20,20,20), 0.f));

		int gutter = 1;
		int barwid = 20;
		int gwid = barwid*2 + gutter;
		int drawWid = width();			// Allow a margin at the left and right.
		for(int value=0; value<=drawWid; value+=gwid)
		{
			int curX = value - 1;
			p.drawLine(curX,0, curX,height()-1);
		}
	}
	p.end();
	QWidget::paintEvent(pe);
	m_SystemId = systemId;
}

QSize AuStatusAtoms::sizeHint() const
{
	return QSize(500,204);
}

void AuStatusAtoms::SetTMinus(int ms)
{
	m_TMinus = ms;
}

void AuStatusAtoms::mouseReleaseEvent(QMouseEvent* event)
{
	QWidget::mouseReleaseEvent(event);
	if(event->button() != Qt::RightButton)
		return;
	if(m_SystemId.IsNull())
		return;
	emit SystemRightClicked(m_SystemId);
}
