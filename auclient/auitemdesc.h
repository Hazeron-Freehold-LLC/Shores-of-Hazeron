//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitemdesc.h,v 1.8 2022/02/19 17:57:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItemDesc_H
#define AuItemDesc_H

#include "auclientlib.h"
#include "audef.h"
#include "audialog.h"

class AuItem;
class QSplitter;

class AUCLIENT_DLL AuItemDesc : public AuDialog
{
	Q_OBJECT;
private:	// Properties
	// Widgets
	QSplitter*							m_Splitter;

public:
	AuItemDesc(const AuItem& item, QWidget* parent=0);
	virtual ~AuItemDesc();

public slots:
	virtual void setVisible(bool visible);
	void SplitterMoved(int pos, int index);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuItemDesc_H
