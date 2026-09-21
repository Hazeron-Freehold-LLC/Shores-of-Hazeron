//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientinterface.cpp,v 1.3 2022/02/19 18:10:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auclientinterface.h"
//
#include "auglobal.h"
#include "daperson.h"

void AuClientInterface::SetAvatarId(const AuId& avatarId)
{
	m_AvatarId = avatarId;
}

void AuClientInterface::SetSystemId(const AuId& systemId)
{
	Q_UNUSED(systemId);	// Client doesn't care.
}

DaPerson* AuClientInterface::LockAvatar()
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	if(m_AvatarId.IsNull())
		return 0;
	return dynamic_cast<DaPerson*>(ether.FindNormalAtom(m_AvatarId));
}

void AuClientInterface::UnlockAvatar()
{
	g_Au.UnlockEtheriumDelete();
}

