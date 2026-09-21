//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetprogram.h,v 1.6 2022/02/19 17:47:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSetProgram_H
#define AuSetProgram_H

#include <QWidget>

class AuCheckBoxValue;
class QCheckBox;
class QLabel;
class QPushButton;

class AuSetProgram : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QPushButton*						m_CacheDirButton;
	AuCheckBoxValue*					m_Switches[32];

public:
	AuSetProgram(QWidget *parent=0);
	~AuSetProgram();

protected slots:
	void AboutSoH();
	void BitChanged(int state, int value);
	void ClearAllFiles();
	void ClearBuildingDesignFiles();
	void ClearBuildingExchangeFiles();
	void ClearDesignFiles();
	void ClearFloraFiles();
	void ClearGlobeFiles();
	void ClearLogs();
	void ClearNetCache();
	void ClearPathFiles();
	void ClearSpacecraftExchangeFiles();
	void ClearTerrainFiles();
	void SetCacheDir();
};

#endif // AuSetProgram_H
