//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommtabscolumn.cpp,v 1.2 2022/02/19 18:10:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommtabscolumn.h"
//
#include "aucomm.h"
#include "aucommchannel.h"

AuCommTabsColumn::AuCommTabsColumn(AuComm* parent) : AuTabColumn(parent)
{
	SetDragTabsEnabled(true);

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), SLOT(Animate()));
	m_timer->start(600);
}

AuCommTabsColumn::~AuCommTabsColumn()
{
}

void AuCommTabsColumn::Animate()
{
	AuComm* comm = dynamic_cast<AuComm*>(parent());
	Q_ASSERT(comm);
	QList<AuCommChannel*> channelPages = comm->findChildren<AuCommChannel*>();
	for(QList<AuCommChannel*>::const_iterator cpi = channelPages.constBegin(); cpi != channelPages.constEnd(); cpi++)
	{
		QString channelName = (*cpi)->objectName();
		bool hasChatAbout = (*cpi)->HasMessageAboutAvatar();
		bool hasUnseenChat = (*cpi)->HasUnseenChat();
		bool isPageVisible = (*cpi)->isVisible();
		if(hasUnseenChat && !isPageVisible)
		{
			// Blink the color of the tab text.
			for(int tab=0; tab<Count(); tab++)
			{
				QString text = TabText(tab);
				if(text == channelName)
				{
					if(hasChatAbout)
					{
						if(TabTextColor(tab) != QColor(255,192,128))
							SetTabTextColor(tab, QColor(255,192,128));
						else
							SetTabTextColor(tab, QColor(128,96,64));
					}
					else
					{
						if(TabTextColor(tab) != QColor(128,255,128))
							SetTabTextColor(tab, QColor(128,255,128));
						else
							SetTabTextColor(tab, QColor(64,128,64));
					}
					break;
				}
			}
		}
		else
		{
			// Insure the tab text is the correct color.
			for(int tab=0; tab<Count(); tab++)
			{
				if(TabText(tab) == channelName)
				{
					if(hasChatAbout)
					{
						if(isPageVisible)
						{
							// Page for this tab is visible.
							if(TabTextColor(tab) != QColor(255,192,128))
								SetTabTextColor(tab, QColor(255,192,128));
						}
						else
						{
							// Page for this tab is not visible.
							if(TabTextColor(tab) != QColor(128,96,64))
								SetTabTextColor(tab, QColor(128,96,64));
						}
					}
					else
					{
						if(isPageVisible)
						{
							// Page for this tab is visible.
							if(TabTextColor(tab) != QColor(255,255,255))
								SetTabTextColor(tab, QColor(255,255,255));
						}
						else
						{
							// Page for this tab is not visible.
							if(TabTextColor(tab) != QColor(80,80,80))
								SetTabTextColor(tab, QColor(80,80,80));
						}
					}
					break;
				}
			}
		}
	}
}
