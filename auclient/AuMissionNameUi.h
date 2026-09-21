//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuMissionNameUi.h,v 1.6 2022/02/19 17:58:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMissionNameUi_H
#define AuMissionNameUi_H

#include "auclientlib.h"
#include "audialog.h"
#include "augear.h"

class QComboBox;
class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuMissionNameUi : public AuDialog
{
	Q_OBJECT;
private:
	AuGearPos							m_Media;				// Media selected in gear, if accepted.
	QString								m_Name;

	// Widgets
	QComboBox*							m_GearMedia;
	QLineEdit*							m_NameEdit;
	QPushButton*						m_Ok;

public:
	AuMissionNameUi(QWidget* parent=0);
	~AuMissionNameUi();

	inline const AuGearPos& MediaPos() const
	{
		return m_Media;
	}

	inline const QString& Name() const
	{
		return m_Name;
	}

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListMedia();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void EnableControls();
	void GearMediaChanged(int index);
	void NameChanged(const QString& name);
	void OkPressed();
};

#endif // AuMissionNameUi_H
