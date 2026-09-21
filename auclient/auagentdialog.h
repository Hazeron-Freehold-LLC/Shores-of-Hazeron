//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentdialog.h,v 1.7 2022/02/19 17:49:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAgentDialog_H
#define AuAgentDialog_H

#include "auclientlib.h"
#include <audialog.h>

class AuAgentDesign;
class AuAgentEmpire;
class AuAgentEncounters;
class AuAgentEnvironment;
class AuAgentPlayers;
class AuAgentPower;
class AuAgentShip;
class AuAgentTeleport;
class AuId;
class AuTabColumn;
class QStackedLayout;

class AUCLIENT_DLL AuAgentDialog : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	AuAgentDesign*						m_PageDesign;
	AuAgentEmpire*						m_PageEmpire;
	AuAgentEncounters*					m_PageEncounters;
	AuAgentEnvironment*					m_PageEnvironment;
	AuAgentPlayers*						m_PagePlayers;
	AuAgentPower*						m_PagePower;
	AuAgentShip*						m_PageShip;
	QStackedLayout*						m_Pages;
	AuAgentTeleport*					m_PageTeleport;
	AuTabColumn*						m_Tabs;

public:
	AuAgentDialog(QWidget* parent=0);
	virtual ~AuAgentDialog();

	void EmpireDataComplete();

	// 'destType' is 0=character, 1=city, 2=unknown
	void SetGoToDestination(const AuId& id, const QString& name, int destType=2);

public slots:
	void SceneAtomChanged();
	virtual void setVisible(bool visible);

signals:
	void ControlsChanged();
	void MessageLimitChanged(int limit);

protected:
	virtual void keyPressEvent(QKeyEvent* event);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuAgentDialog_H
