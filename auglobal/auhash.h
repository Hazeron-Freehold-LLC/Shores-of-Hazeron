//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auhash.h,v 1.4 2022/02/19 18:30:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef auhash_H
#define auhash_H

#include "augloballib.h"
#include "auid.h"

class AUGLOBAL_DLL AuDesignIdHash
{
public:
	inline size_t operator()(const quint32& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AUGLOBAL_DLL AuEmpireIdHash
{
public:
	inline size_t operator()(const quint32& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AUGLOBAL_DLL AuKey_Hash
{
public:
	inline size_t operator()(const Qt::Key& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AUGLOBAL_DLL AuStringHash
{
public:
	inline size_t operator()(const QString& o) const
	{
		return hash<unsigned int>()(qHash(o));
	}
};

#endif // auhash_H
