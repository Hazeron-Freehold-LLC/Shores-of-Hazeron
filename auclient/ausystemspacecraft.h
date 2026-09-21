//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemspacecraft.h,v 1.6 2022/02/19 18:02:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSystemSpacecraft_H
#define AuSystemSpacecraft_H

#include "auclientlib.h"
#include "audialog.h"

class AuId;

class AUCLIENT_DLL AuSystemSpacecraft : public AuDialog
{
	Q_OBJECT;
public:
	AuSystemSpacecraft(quint32 empireId, const AuId& sectorId, const AuId& systemId, const QString& systemName, QWidget* parent=0);
	virtual ~AuSystemSpacecraft();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
	virtual QSize sizeHint() const;
};

#endif // AuSystemSpacecraft_H
