//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auline.cpp,v 1.2 2022/02/19 18:38:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auline.h"
//
#include "auplane.h"

// calculate implicit values from explicit values
void AuLine::ExplicitToImplicit()
{
	AuPlane fpl(AuPoint(0.0,0.0,0.0),V);
	fpl.LineIntersection(*this,N);
	N.Unit();
	c = N.Dot(U);	
}
