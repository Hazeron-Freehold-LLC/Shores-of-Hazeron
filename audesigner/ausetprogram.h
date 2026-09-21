//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetprogram.h,v 1.12 2022/02/19 18:23:04 daveh Exp $
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
	int									m_IconFrame;
	QTimer*								m_timer;				// For the animated icon.

	// Widgets
	QLabel*								m_AtomManagersLabel;
	QPushButton*						m_CacheDirButton;
	QPushButton*						m_GameSaveDirButton;
	QPushButton*						m_PictureDirButton;
	AuCheckBoxValue*					m_Switches[32];
	QLabel*								m_WorkerPoolLabel;
	QLabel*								m_WorkerThreadsIcon;
	QLabel*								m_WorkerThreadsLabel;

public:
	AuSetProgram(QWidget *parent=0);
	~AuSetProgram();

protected:
	virtual void showEvent(QShowEvent* event);

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
	void ClearSavedGames();
	void ClearSpacecraftExchangeFiles();
	void ClearTerrainFiles();
	void SetCacheDir();
	void SetGameSaveDir();
	void SetPictureDir();
	void SetWorkerThreads(int value);
	void UpdateContents();
};

#endif // AuSetProgram_H
