//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentplayers.h,v 1.8 2022/02/19 17:49:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAgentPlayers_H
#define AuAgentPlayers_H

#include "auclientlib.h"
#include "auid.h"
#include <QWidget>

class AuPlayersTable;
class QLabel;
class QTableWidgetItem;

class AUCLIENT_DLL AuAgentPlayers : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	AuPlayersTable*						m_Players;
	QLabel*								m_PlayersLabel;

public:
	AuAgentPlayers(QWidget* parent=0);
	virtual ~AuAgentPlayers();

signals:
	void AgentActionComplete();
	void AvatarSelected(AuId charId, QString charName);

protected slots:
	void GoToAvatar();
	void PlayerSelectionChanged();
	void PlayerRightClicked(QTableWidgetItem* item);
	void RefreshPlayersOnline();
	void RestoreSettings();
	void SaveSettings();
};

#endif // AuAgentPlayers_H
