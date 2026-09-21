//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aushiptitle.h,v 1.2 2022/02/19 18:00:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuShipTitle_H
#define AuShipTitle_H

#include "auclientlib.h"
#include "audialog.h"
#include "augear.h"

class QComboBox;
class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuShipTitle : public AuDialog
{
	Q_OBJECT;
private:
	AuGearPos							m_Media;				// Media selected in gear, if accepted.

	// Widgets
	QComboBox*							m_GearMedia;
	QPushButton*						m_Ok;

public:
	AuShipTitle(QWidget* parent=0);
	~AuShipTitle();

	inline const AuGearPos& MediaPos() const
	{
		return m_Media;
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
	void OkPressed();
};

#endif // AuShipTitle_H
