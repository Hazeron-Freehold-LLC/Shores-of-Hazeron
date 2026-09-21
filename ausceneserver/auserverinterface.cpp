//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auserverinterface.cpp,v 1.2 2022/02/19 18:39:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auserverinterface.h"
//
#include "auglobal.h"
#include "ausceneserverapp.h"
#include "daperson.h"
#include "dasolarsystem.h"

void AuServerInterface::SetAvatarId(const AuId& avatarId)
{
	m_AvatarId = avatarId;
}

void AuServerInterface::SetSystemId(const AuId& systemId)
{
	m_SystemId = systemId;
}

DaPerson* AuServerInterface::LockAvatar()
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = auSceneServerApp->AssignedSolarSystem(m_SystemId, ether);
	if(!system)
		return 0;
	return dynamic_cast<DaPerson*>(system->FindIdRecursive(m_AvatarId));
}

void AuServerInterface::UnlockAvatar()
{
	g_Au.UnlockEtheriumDelete();
}

