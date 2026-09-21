//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dicontrol.h,v 1.5 2022/02/19 17:35:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// DiControl
// DiControlArray
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

#ifndef DICONTROL_H
#define DICONTROL_H

#include <QVector>
#include <aupoint.h>

class AuMatrix4x4;

// This class represents the physical location of a control device on an atom.
// A control device enables interaction with the atom by the user by providing
// a selectable location that is considered in the context of feeding input to
// the atom. This distinguishes between multiple input locations on an atom such
// as when interacting with a ship hull from the outside on which there are
// several doors. Each door is a DiControl location that the user can select
// before issuing the command to open the door.

class DiControl
{
public:
	AuPoint		m_fLocation;	// Relative to the atom.

public:
	DiControl(const AuPoint &fLoc);
	DiControl();
	virtual ~DiControl();

	virtual double ActivationDistance() const
	{
		// Most controls require the person to be within some distance to actuate it.
		return 15.0;
	}
	void Transform(const AuMatrix4x4 &t);
};

class DiControlArray : public QVector<DiControl*>
{
public:
	void DeleteContents();
	void Transform(const AuMatrix4x4 &t);
	//DiControl *FindClosestControl(const AuPoint &fLoc, CRuntimeClass *pIsKindOf=NULL);
};

#endif // DICONTROL_H
