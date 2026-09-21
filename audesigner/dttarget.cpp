//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dttarget.cpp,v 1.7 2022/04/15 16:39:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dttarget.h"
//
#include "audesignerview.h"
#include <QDebug>

DtTarget::DtTarget(AuDesignerView* parent) : QThread(parent), m_semAlive(1)
{
	m_semAlive.acquire(1);			// I live...
}

DtTarget::~DtTarget()
{
}

void DtTarget::run()
{
	//qDebug() << "DtTarget::run start";
	AuElapsedTimer elapseTimer;
	DaPerson* person;
	while(Alive())
	{
		//qDebug() << "DtTarget::run running";
		elapseTimer.start();
		AuDesignerView* view = (AuDesignerView*)parent();
		if(view)
		{
			view->LockSceneAtom(&person);
			if(person)
				view->UpdateTarget(person, false);					// false = m_TargetDesc and m_TargetDNA update not needed.
			view->UnlockSceneAtom();
		}

		// Wait for the time needed to try to even out the cycle time.
		int waitMs = 200 - elapseTimer.elapsed();
		if(waitMs > 0)
			msleep(waitMs);
	}
	//qDebug() << "DtTarget::run exit";
}
