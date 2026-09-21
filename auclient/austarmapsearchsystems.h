//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapsearchsystems.h,v 1.12 2022/02/19 18:01:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStarMapSearchSystems_H
#define AuStarMapSearchSystems_H

#include "auclientlib.h"
#include "audialog.h"
#include "auether.h"
#include "auid.h"
#include "austarmapdata.h"
#include "daatomlist.h"

class AuCheckBoxValue;
class AuSystemScan;
class DaSolarSystem;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QRadioButton;
class QSpinBox;

class AUCLIENT_DLL AuStarMapSearchSystems : public AuDialog
{
	Q_OBJECT;
	AuEther								m_Ether;				// Holds our scanned systems atoms.
	bool								m_LocationSet;
	bool								m_ScanSaved;
	AuStarMapSystemSearch				m_Search;

	// Widgets
	QLineEdit*							m_Name;
	QCheckBox*							m_Case;
	QCheckBox*							m_RegEx;
	QCheckBox*							m_SystemCat;
	QCheckBox*							m_SystemName;

	QSpinBox*							m_Range;
	QComboBox*							m_Galaxy;
	QLineEdit*							m_SearchLoc;

	QList<AuCheckBoxValue*>				m_NaturalResources;
	QRadioButton*						m_ResAnd;
	QRadioButton*						m_ResOr;
	QSpinBox*							m_MinQuality;

	QSpinBox*							m_Stars;
	QCheckBox*							m_BlackHole;
	QCheckBox*							m_NeutronStar;
	QCheckBox*							m_StarTypeO;
	QCheckBox*							m_StarTypeB;
	QCheckBox*							m_StarTypeA;
	QCheckBox*							m_StarTypeF;
	QCheckBox*							m_StarTypeG;
	QCheckBox*							m_StarTypeK;
	QCheckBox*							m_StarTypeM;

	QSpinBox*							m_Worlds;
	QCheckBox*							m_GasGiantInferno;
	QCheckBox*							m_GasGiantInner;
	QCheckBox*							m_GasGiantHabitable;
	QCheckBox*							m_GasGiantOuter;
	QCheckBox*							m_GasGiantFrigid;
	QComboBox*							m_GlobeInferno;
	QComboBox*							m_GlobeInner;
	QComboBox*							m_GlobeHabitable;
	QComboBox*							m_GlobeOuter;
	QComboBox*							m_GlobeFrigid;
	QCheckBox*							m_RingInferno;
	QCheckBox*							m_RingInner;
	QCheckBox*							m_RingHabitable;
	QCheckBox*							m_RingOuter;
	QCheckBox*							m_RingFrigid;
	QSpinBox*							m_RingworldArcs;

	QPushButton*						m_ClearFilter;
	QPushButton*						m_LoadFilter;
	QListWidget*						m_Result;
	QPushButton*						m_SaveFilter;
	AuSystemScan*						m_Scan;
	QPushButton*						m_SearchButton;
	QLabel*								m_SearchResult;

public:
	AuStarMapSearchSystems(QWidget* parent=0);
	virtual ~AuStarMapSearchSystems();

	QList<int> CheckedNaturalResources() const;	// Returns a list of COMMODITYIDs.

	inline void ClearLocationReference()
	{
		m_LocationSet = false;
	}

	// Sets galaxy location controls, for range testing.
	void SetLocationReference(int galaxy, const AuPoint& galaxyLoc);

public slots:
	virtual void BlackHoleChanged(int state);
	virtual void CaseChanged(int state);
	virtual void CheckAllNaturalResources();
	virtual void ClearFilter();
	virtual void EnableControls();
	virtual void GalaxyChanged(int index);
	virtual void GasGiantFrigidChanged(int state);
	virtual void GasGiantHabitableChanged(int state);
	virtual void GasGiantOuterChanged(int state);
	virtual void GasGiantInfernoChanged(int state);
	virtual void GasGiantInnerChanged(int state);
	virtual void GlobeFrigidChanged(int index);
	virtual void GlobeHabitableChanged(int index);
	virtual void GlobeOuterChanged(int index);
	virtual void GlobeInfernoChanged(int index);
	virtual void GlobeInnerChanged(int index);
	virtual void InvertCheckedNaturalResources();
	virtual void LoadFilter();
	virtual void MinQualityChanged(int value);
	virtual void NameChanged(const QString& text);
	virtual void NaturalResourceStateChanged(int state, int value);
	virtual void NeutronStarChanged(int state);
	virtual void RangeChanged(int value);
	virtual void RegExChanged(int state);
	virtual void ResAndClicked();
	virtual void ResOrClicked();
	virtual void RingFrigidChanged(int state);
	virtual void RingHabitableChanged(int state);
	virtual void RingOuterChanged(int state);
	virtual void RingInfernoChanged(int state);
	virtual void RingInnerChanged(int state);
	virtual void RingworldArcsChanged(int value);
	virtual void SaveFilter();
	virtual void SearchLocChanged(const QString& text);
	virtual void SearchNow();
	virtual void SearchResultClicked(QListWidgetItem* item);
	virtual void setVisible(bool visible);
	virtual void StarsChanged(int value);
	virtual void StarTypeOChanged(int state);
	virtual void StarTypeBChanged(int state);
	virtual void StarTypeAChanged(int state);
	virtual void StarTypeFChanged(int state);
	virtual void StarTypeGChanged(int state);
	virtual void StarTypeKChanged(int state);
	virtual void StarTypeMChanged(int state);
	virtual void SystemCatChanged(int state);
	virtual void SystemNameChanged(int state);
	virtual void UncheckAllNaturalResources();
	virtual void WorldsChanged(int value);

protected:
	void PostFilter();			// Transfers current m_Search settings to controls.
	void RestoreGeometry();
	void RestoreScannedSystemsCache();
	void SaveGeometry();
	void SaveScannedSystemsCache();

signals:
	void SystemClicked(AuId placeId);
};

#endif // AuStarMapSearchSystems_H
