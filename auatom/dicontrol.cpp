//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dicontrol.cpp,v 1.6 2022/02/19 17:44:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dicontrol.h"
//
#include <qgl.h>
#include <aumatrix4x4.h>

DiControl::DiControl(const AuPoint &fLoc)
{
	m_fLocation = fLoc;
}

DiControl::DiControl()
{
}

DiControl::~DiControl()
{

}

void DiControl::Transform(const AuMatrix4x4 &t)
{
	m_fLocation = t.Project(m_fLocation);
}

//////////////////////////////////////////////////////////////////////

void DiControlArray::DeleteContents()
{
	for(QVector<DiControl*>::iterator it = begin(); it != end(); it++)
		delete *it;
	clear();
}

void DiControlArray::Transform(const AuMatrix4x4 &t)
{
	for(QVector<DiControl*>::iterator it = begin(); it != end(); it++)
		(*it)->Transform(t);
}

/*
DiControl *DiControlArray::FindClosestControl(const AuPoint &fLoc, CRuntimeClass *pIsKindOf)
{
	DiControl *pclosest = NULL;
	double rclosest = 0.0;
	DiControl **pc = constData();
	for (int ic=count(); ic; ic--, pc++)
	{
		if (pIsKindOf && !(*pc)->IsKindOf(pIsKindOf))
			continue;
		double rdist = (*pc)->m_fLocation.Distance(fLoc);
		if (!pclosest || rdist < rclosest)
		{
			pclosest = *pc;
			rclosest = rdist;
		}
	}
	return pclosest;
}
*/
