//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscsphereuniformdlg.h,v 1.2 2022/02/19 18:22:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScSphereUniformDlg_H
#define AuScSphereUniformDlg_H

#include "audialog.h"

class QSpinBox;

class AuScSphereUniformDlg : public AuDialog
{
	Q_OBJECT;
private:
	//AuScPart::PART_TYPE
	quint8								m_PartType;				// AuScPart::TypeStatic.

	// Widgets
	QSpinBox*							m_EqStepsSpin;

public:
	AuScSphereUniformDlg(quint8 partType, QWidget* parent=0);
	virtual ~AuScSphereUniformDlg();

	quint16 EquatorSteps() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void OkPressed();
};

#endif // AuScSphereUniformDlg_H
