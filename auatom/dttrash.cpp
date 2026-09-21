//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dttrash.cpp,v 1.32 2022/02/19 17:46:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dttrash.h"
//
#include "auglobal.h"
#include "autime.h"
#include "daatom.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dauniverse.h"

DtTrash::DtTrash(QObject* parent) : QThread(parent), m_semAlive(1)
{
	m_semAlive.acquire(1);	// I live...
}

DtTrash::~DtTrash()
{
}

void DtTrash::run()
{
	while(Alive())
	{
		// Calculate a sleep time before deleting another atom, based on the number of atoms left to delet.
		size_t trashAtoms = g_Au.DeleteEtheriumTrashAtom();
		if(trashAtoms < 10000)
			msleep(10000/((unsigned long)trashAtoms+1));	// Distribute the deletions over a 10 second period.
		else
			msleep(1);										// Minimal yield when we have lots of work to do.
	}
}
