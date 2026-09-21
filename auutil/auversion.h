//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auversion.h,v 1.275 2022/04/08 15:12:43 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// The rolling credits on the splash screen of Shores of Hazeron display these version numbers, along with the build date of the
// program. The information is displayed in the following format:
//   Version-LoginVersion-SceneVersion, Date

// This is the year that appears in copyright messages. e.g. Copyright 2022-? Hazeron Freehold LLC
const char* g_CopyrightYear = "2022";

// Version is informational only. Changing the application version has no functional effect on the program.
// NOTE: Nothing depends on this version number. It appears on the login screen. That is all.
// This version now tend to track the Universe number.
const char* g_Version = "7";

// LoginVersion must be changed if anything changes about the communication stream with the login servers.
// NOTE: If you changed something in DtLoginServer, this is probably the version number you need to change.
// At login, an error message appears on the client screen informing them when their software version is no longer compatible with
// the server, so they must go download the latest software update.
const char* g_LoginVersion = "b";

// SceneVersion must be changed if anything changes about the communication stream with the scene servers.
// NOTE: If you changed something in an atom's communication stream, this is probably the version number you need to change.
// At login, an error message appears on the client screen informing them when their software version is no longer compatible with
// the server, so they must go download the latest software update.
const char* g_SceneVersion = "2";
