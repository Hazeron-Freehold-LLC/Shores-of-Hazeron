//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomchat.cpp,v 1.6 2022/02/19 18:19:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auatomchat.h"
//

#if defined(WIN32)
unsigned int qHash(const AuAtomChat& key)
{
	return key.TimeStamp().MSecsSinceEpoch();	// Probably good enough for Windows.
}
#endif

void AuAtomChatList::Decay()
{
	iterator it = begin();
	while(it != end())
	{
		if((*it).IsTooOld())
			it = erase(it);
		else
			it++;
	}
}
