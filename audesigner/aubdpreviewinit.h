//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdpreviewinit.h,v 1.3 2022/02/19 18:21:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdPreviewInit_H
#define AuBdPreviewInit_H

#include "audialog.h"

class QSpinBox;

class AuBdPreviewInit : public AuDialog
{
	Q_OBJECT;
	int									m_PreviewType;			// 0=none, 1=empty space, 2=sun only, 3=sun and world
																// 4=sun, world, moon, 5=inferno, 6=frigid, 7=gas giant

	// Widgets
	QSpinBox*							legen;

public:
	AuBdPreviewInit(QWidget* parent=0);
	virtual ~AuBdPreviewInit();

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
	void OkSea();
	void OkWorld();
	void SetCoast();
	void SetHills();
	void SetPlains();
	void SetRandom();
	void SetRiver();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuBdPreviewInit_H
