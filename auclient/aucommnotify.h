//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommnotify.h,v 1.6 2022/02/19 17:54:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommNotify_H
#define AuCommNotify_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"

class AuCommPacket;

class AUCLIENT_DLL AuCommNotify : public AuDialog
{
	Q_OBJECT;
public:
	AuCommNotify(const QString& avatarName, const AuCommPacket& msg, QWidget* parent=0);
	virtual ~AuCommNotify();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuCommNotify_H
