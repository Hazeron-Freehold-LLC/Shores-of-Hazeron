//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpreviewinit.h,v 1.2 2022/02/19 18:22:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScPreviewInit_H
#define AuScPreviewInit_H

#include "audialog.h"

class QSpinBox;

class AuScPreviewInit : public AuDialog
{
	Q_OBJECT;
	int									m_PreviewType;			// 0=none, 1=empty space, 2=sun only, 3=sun and world
																// 4=sun, world, moon, 5=inferno, 6=frigid, 7=gas giant

	// Widgets
	QSpinBox*							legen;

public:
	AuScPreviewInit(QWidget* parent=0);
	virtual ~AuScPreviewInit();

	quint32 GenSeed() const;

	inline int PreviewType() const
	{
		return m_PreviewType;
	}

public slots:
	virtual void setVisible(bool visible);
	void OkFrigid();
	void OkGasGiant();
	void OkInferno();
	void OkMoon();
	void OkRingworld();
	void OkSpace();
	void OkSun();
	void OkWorld();
	void SetRandom();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScPreviewInit_H
