//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubuyblueprint.h,v 1.6 2022/02/19 17:52:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBuyBlueprint_H
#define AuBuyBlueprint_H

#include "auclientlib.h"
#include "audialog.h"
#include "augear.h"
#include "auknownpatents.h"

class QListWidget;
class QSpinBox;

class AUCLIENT_DLL AuBuyBlueprint : public AuDialog
{
	Q_OBJECT;
	AuGearPos							m_Media;				// Media selected in gear, if accepted.
	QString								m_MediaName;

	// Widgets
	QComboBox*							m_GearMedia;
	QPushButton*						m_Ok;

public:
	AuBuyBlueprint(quint32 designId, const QString& designName, const QString& terms, QWidget* parent=0);
	virtual ~AuBuyBlueprint();

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
};

#endif // AuBuyBlueprint_H
