//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddbdpreview.h,v 1.11 2022/04/15 16:42:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdBdPreview_H
#define DdBdPreview_H

#include "aubdreport.h"
#include "auelapsedtimer.h"
#include "auglobell.h"
#include "auknownpatents.h"
#include "ddcommand.h"
#include "dbmanufacturingprocess.h"
#include "deconstructionprocess.h"
#include <QMouseEvent>
#include <QKeyEvent>

class AuGLEffects;
class DaBd;
class DaWorldLL;

class DdBdPreview : public DdCommand  
{
private:
	float								m_AlignRot;				// Radians. 99 = invalid. Used for rotating to existing development.
	AuGLVAO								m_BdVAO;
	float								m_NorthZ;				// Angle to orient building facing to the north.
	double								m_PlatformAltMSL;		// MSL altitude for aerial platforms.
	bool								m_RotAligned;			// True if the rotation was calculated by aligning to existing dev.
	float								m_RotZ;					// Building model rotation on its Z axis.
	AuBdReport							m_Rpt;
	bool								m_RptOk;

	QString								m_sReason;
	AuElapsedTimer						m_LastTime;

public:
	DdBdPreview(AuDesignerView* view);
	virtual ~DdBdPreview();

	// Keyboard monitoring functions.
	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	// Mouse monitoring functions.
	virtual void OnButtonDown(QMouseEvent* e);
	virtual void OnMouseWheel(QWheelEvent* e);

	// Cursor rendering functions.
	virtual void CreateDeviceObjects(AuGLEffects& re);
	virtual void DeleteDeviceObjects(AuGLEffects& re);
	virtual void RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom, const QPoint& pxCursor,
			AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape);

	// Set appropriate device cursor. Default sets Cross cursor if
	// pSceneAtom != 0, otherwise sets Arrow cursor.
	virtual void SetCursor(const AuMatrix4x4& tCamera, DaPerson* pSceneAtom, const QPoint& pxCursor);

protected:
	void AlignToExistingDevelopment();

	void ConstructBuilding(const QPoint &pxCursor);

	int TestBuildLocation(DaPerson* sceneAtom, const QPoint& pxCursor, const DaWorldLL* &pTerrain, AuPoint& fBuildLoc,
			DaBd*& exBd) const;

	inline float ZRotation() const
	{
		if(m_RotAligned)
			return m_RotZ;				// Rotation calculated by alignment with existing development.
		else
			return m_RotZ - m_NorthZ;	// Rotation calculated based on north alignment of building.
	}
};

#endif // DdBdPreview_H
