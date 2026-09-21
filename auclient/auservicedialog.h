//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservicedialog.h,v 1.6 2022/02/19 18:00:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuServiceDialog_H
#define AuServiceDialog_H

#include "auclientlib.h"
#include <audialog.h>

class AuCommChannel;
class QStackedLayout;
class QTabBar;

class AUCLIENT_DLL AuServiceDialog : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QStackedLayout* m_Pages;
	QTabBar*		m_Tabs;

public:
	AuServiceDialog(AuCommChannel* parent=0);
	virtual ~AuServiceDialog();

	AuCommChannel* Channel();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuServiceDialog_H
