//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemaccesscodenew.h,v 1.6 2022/02/19 18:02:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSystemAccessCodeNew_H
#define AuSystemAccessCodeNew_H

#include "auclientlib.h"
#include "audialog.h"

class QSpinBox;

class AUCLIENT_DLL AuSystemAccessCodeNew : public AuDialog
{
	Q_OBJECT;
	bool								m_CodeDisabled;

	// Widgets
	QSpinBox*							m_DigitHundreds;
	QSpinBox*							m_DigitOnes;
	QSpinBox*							m_DigitTens;

public:
	AuSystemAccessCodeNew(QWidget* parent=0);
	virtual ~AuSystemAccessCodeNew();

	quint16 AccessCode() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DisableCode();
	void Randomize();
};

#endif // AuSystemAccessCodeNew_H
