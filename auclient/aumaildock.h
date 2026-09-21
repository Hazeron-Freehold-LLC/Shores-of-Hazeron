//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumaildock.h,v 1.8 2022/02/19 17:58:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMailDock_H
#define AuMailDock_H

#include "auclientlib.h"
#include <audockwidget.h>
#include <auglobal.h>
#include <auid.h>

class AuId;
class AuMail;

class AUCLIENT_DLL AuMailDock : public AuDockWidget
{
	Q_OBJECT;
public:
	AuMail*								m_Mail;

public:
	AuMailDock(const AuId& selId, QWidget* parent);

	// Called when local list of avatars is updated.
	void AvatarsUpdated();

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/MailAreas", (int)allowedAreas());
	}

	void SetRecipient(const AuId& personId);

	void StopThreads();

	virtual void UpdateTitleShortcut();

signals:
	void AlertMessageReceived(AuMailMsg msg);
	void GoDirectToCoord(AuPoint galaxyLoc);
	void GoDirectToSystem(AuId systemId, QString systemName, AuPoint galaxyLoc);
	void GoLandAtCity(AuId cityId, QString cityName);
	void GoOrbit(AuId worldId, QString worldName);
	void GoOrbitCity(AuId cityId, QString cityName);
	void GoViaWormholes(AuId systemId, QString systemName);
	void UpdateSquelch(AuId idContact, bool bDelete);
	void UpdateSquelches(AuIdList idContacts, bool bDelete);
};

#endif // AuMailDock_H
