//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldarena.h,v 1.6 2022/02/19 17:51:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldArena_H
#define AuBldArena_H

#include "auarenaready.h"
#include "auclientlib.h"
#include "auid.h"
#include <QWidget>

class DaAtom;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTimer;

class AUCLIENT_DLL AuArenaDesignInfo
{
public:
	quint8								m_DesignQ;
	bool								m_IsScDesign;

public:
	inline AuArenaDesignInfo(quint8 designQ, bool isScDesign)
	{
		Q_ASSERT(designQ);
		m_DesignQ = designQ;
		m_IsScDesign = isScDesign;
	}

	inline AuArenaDesignInfo()
	{
		m_DesignQ = 1;
		m_IsScDesign = false;
	}
};

class AUCLIENT_DLL AuBldArena : public QWidget
{
	Q_OBJECT;
private:
	QHash<quint32, AuArenaDesignInfo>	m_ShipDesignInfo;	// key is ship design id, value is design info
	QTimer*		m_timer;

	// Widgets
	QTableWidget*						m_Arenas;
	QPushButton*						m_JoinArenaButton;
	QLabel*								m_MatchDesc;
	QComboBox*							m_MatchType;
	QLabel*								m_MatchTypeDesc;
	QCheckBox*							m_MaximizeShipQ;
	QComboBox*							m_MaxShipQ;
	QPushButton*						m_NewMatchButton;
	QLineEdit*							m_PasswordJoin;
	QLineEdit*							m_PasswordNew;
	QComboBox*							m_ShipDesignsJoin;
	QComboBox*							m_ShipDesignsNew;
	QCheckBox*							m_OpponentDesigns;
	QComboBox*							m_SpawnCrew;
	QComboBox*							m_SpawnOfficer;
	QComboBox*							m_SpawnPassengers;
	QComboBox*							m_SpawnTroops;
	QComboBox*							m_SpawnVehicles;
	QComboBox*							m_Teams;
	QComboBox*							m_TeamSize;

public:
	AuBldArena(QWidget* parent=0);
	virtual ~AuBldArena();

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void ArenaReady(AuArenaReady arena);

	void EnterArena();
	void JoinArena();

	void MatchChanged();

	void MatchTypeChanged(int index);

	void QueryArenas();

	void ShipDesignNewChanged(int index);

	void UpdateContents();
};

#endif // AuBldArena_H
