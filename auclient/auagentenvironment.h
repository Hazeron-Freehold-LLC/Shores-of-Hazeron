//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentenvironment.h,v 1.8 2022/02/19 17:49:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAgentEnvironment_H
#define AuAgentEnvironment_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;

class AUCLIENT_DLL AuAgentEnvironment : public QWidget
{
	Q_OBJECT;
public:
	AuAgentEnvironment(QWidget* parent=0);
	virtual ~AuAgentEnvironment();

protected slots:
	void EnvChgCitizenDNA();
	void EnvChgGroundCover();
	void EnvChgIndigenousDNA();
	void EnvChgShrubbery();
	void EnvChgTrees();
	void EnvClearIndigenousDNA();
	void RequestCityHistoryReport();
	void RequestCityLastReport();
	void ShowSceneAtomicStructure();
	void ShowTargetAtomicStructure();

protected:
	int CurrentFloraZone() const;			// 0=polar, 1=temperate, 2=tropical, negative is an error code
	void EnvChgPlantDNA(quint8 plantType);	// 'plantType' is 0=ground cover, 1=shrubbery, 2=trees
};

#endif // AuAgentEnvironment_H
