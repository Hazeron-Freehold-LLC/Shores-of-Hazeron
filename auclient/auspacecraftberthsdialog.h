//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftberthsdialog.h,v 1.6 2022/02/19 18:01:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSpacecraftBerthsDialog_H
#define AuSpacecraftBerthsDialog_H

#include "auclientlib.h"
#include <audialog.h>

class AuSpacecraftBerths;

class AUCLIENT_DLL AuSpacecraftBerthsDialog : public AuDialog
{
	Q_OBJECT;
	AuSpacecraftBerths*					m_Berths;

public:
	AuSpacecraftBerthsDialog(QWidget* parent=0);
	virtual ~AuSpacecraftBerthsDialog();

public slots:
	virtual void setVisible(bool visible);
	void SplitterMoved(int pos, int index);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuSpacecraftBerthsDialog_H
