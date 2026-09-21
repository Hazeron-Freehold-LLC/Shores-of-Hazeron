//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auserverinterface.h,v 1.2 2022/02/19 18:39:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuServerInterface_H
#define AuServerInterface_H

#include "auatominterface.h"
#include "auid.h"

class AuServerInterface : public AuAtomInterface
{
	AuId								m_AvatarId;
	AuId								m_SystemId;

public:	// Virtual Methods
	virtual DaPerson* LockAvatar();

	virtual void SetAvatarId(const AuId& avatarId);

	virtual void SetSystemId(const AuId& systemId);

	virtual void UnlockAvatar();

public:	// Methods
	inline const AuId& AvatarId() const
	{
		return m_AvatarId;
	}

	inline const AuId& SystemId() const
	{
		return m_SystemId;
	}
};

#endif // AuServerInterface_H
