//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: linuxPlatform.h,v 1.8 2022/12/01 16:35:26 build Exp daveh $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// This file normally resides in the parent folder of the project folder. That way it can be configured on different Linux build
// machines and it isn't clobbered when you get all new source.

// Linux version number identifies the bundle of the current build machine. This should be configured on each Linux build computer.
//  0 = Unknown
//  1 = Debian 9
//  2 = Manjaro 18
//  3 = Mint 19
//  4 = Ubuntu 18
//  5 = Debian 10
//  6 = Manjaro 20
//  7 = Ubuntu 20
//  8 = Manjaro 21
//  9 = Fedora 35
//  10 = Debian 11
//  11 = Ubuntu 22
#define SEiLINUXPLATFORM 11				// This value must fit into an unsigned 16 bit number.
