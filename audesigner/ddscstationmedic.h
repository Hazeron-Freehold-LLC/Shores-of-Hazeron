//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscstationmedic.h,v 1.6 2022/02/19 18:24:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScStationMedic_H
#define DdScStationMedic_H

#include "dbcommodity.h"
#include "ddcommand.h"

class DdScStationMedic : public DdCommand  
{
private:
	AuPoint								m_Corner1;				// First corner.
	AuPoint								m_Corner2;				// Second corner.
	int									m_GetPoint;				// 0=corner, 1=opposite corner
	AuPoint								m_MedicFacing;
	AuPoint								m_MedicLoc;
	//AuScStation::OPERATOR_POSITION
	quint8								m_MedicPos;
	COMMODITYID							m_Module;
	//AuScStationMedic::PATIENT_POSITION
	AuPoint								m_PatientLoc;
	quint8								m_PatientPos;
	AuMatrix4x4							m_ScreenPlane;

public:
	DdScStationMedic(AuDesignerView* view);

	// Returns final result, which could be different from 'loc'. That is where the design plane will move if set to 'follow work'.
	virtual AuPoint CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods);

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	virtual QString WikiTopic() const;

protected:
	bool MakeIt(const AuPoint& patientFacing);
	virtual void UpdatePrompt() const;
};

#endif // DdScStationMedic_H
