//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpostdlg.h,v 1.2 2022/02/19 18:22:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScPostDlg_H
#define AuScPostDlg_H

#include "audialog.h"

class QPushButton;
class QRadioButton;

class AuScPostDlg : public AuDialog
{
	Q_OBJECT;
private:
	//AuScPart::PART_TYPE
	quint8								m_PartType;				// AuScPart::TypeStatic.

	// Widgets
	QPushButton*						m_PickLocation;
	QRadioButton*						m_Sit;
	QRadioButton*						m_SitStationary;
	QRadioButton*						m_Stand;
	QRadioButton*						m_StandStationary;

public:
	AuScPostDlg(quint8 partType, QWidget* parent=0);
	virtual ~AuScPostDlg();

	//AuScPost::OCCUPANT_POSITION
	quint8 Position() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScPostDlg_H
