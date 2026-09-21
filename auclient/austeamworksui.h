//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austeamworksui.h,v 1.3 2022/02/19 18:02:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSteamworksUI_H
#define AuSteamworksUI_H

#include "audialog.h"

class AuSteamworksUI : public AuDialog
{
	Q_OBJECT;
public:
	AuSteamworksUI(QWidget* parent=0);
	virtual ~AuSteamworksUI();
};

#endif // AuSteamworksUI_H
