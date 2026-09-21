//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupatentsget.h,v 1.6 2022/02/19 17:59:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPatentsGet_H
#define AuPatentsGet_H

#include "auclientlib.h"
#include "audialog.h"
#include "augear.h"
#include "auknownpatents.h"

class QListWidget;
class QSpinBox;

class AUCLIENT_DLL AuPatentsGet : public AuDialog
{
	Q_OBJECT;
	AuId								m_CapitolOpId;			// Id of building when operated from a capitol building.
	bool								m_EmpirePatents;		// True when accessing empire patents.
	AuGearPos							m_Media;				// Media selected in gear, if accepted.
	QString								m_MediaName;

	// Widgets
	QLabel*								m_CountLabel;
	QComboBox*							m_GearMedia;
	QPushButton*						m_Ok;
	QListWidget*						m_PatentList;
	QSpinBox*							m_QualitySpin;

public:
	AuPatentsGet(bool empirePatents, const AuId& opBd, QWidget* parent=0);
	virtual ~AuPatentsGet();

	inline const AuGearPos& MediaPos() const
	{
		return m_Media;
	}

	// Fills 'patents' with the selected patents.
	void Patents(QSet<COMMODITYID>& patents) const;

	quint8 QualityLimit() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListMedia();
	void ListPatents();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void EnableControls();
	void GearMediaChanged(int index);
	void PatentSelectionChanged();
};

#endif // AuPatentsGet_H
