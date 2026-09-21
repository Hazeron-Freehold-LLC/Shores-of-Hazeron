//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientinterface.h,v 1.2 2022/02/19 17:53:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientInterface_H
#define AuClientInterface_H

#include "auatominterface.h"
#include "auid.h"

class AuClientInterface : public AuAtomInterface
{
	AuId								m_AvatarId;

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
};

#endif // AuClientInterface_H
