//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucompileroutput.h,v 1.2 2022/02/19 17:55:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCompilerOutput_H
#define AuCompilerOutput_H

#include "audialog.h"

class AuScript;

class AuCompilerOutput : public AuDialog
{
	Q_OBJECT;
public:
	AuCompilerOutput(const QString& file, const AuScript& script, QWidget* parent=0);
	virtual ~AuCompilerOutput();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuCompilerOutput_H
