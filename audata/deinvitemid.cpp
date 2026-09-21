//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: deinvitemid.cpp,v 1.4 2022/02/19 18:20:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "deinvitemid.h"
//
#include "audef.h"

bool DeInvItemId::IsValid() const
{
	if(m_CID <= CID_Invalid || m_CID >= CID_Count)
		return false;
	if(!m_Quality)
		return false;
	return true;
}
