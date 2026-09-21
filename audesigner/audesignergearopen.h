//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignergearopen.h,v 1.2 2022/02/19 18:21:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDesignerGearOpen_H
#define AuDesignerGearOpen_H

#include "audialog.h"
#include "augear.h"

class QComboBox;
class QLabel;

class AuDesignerGearOpen : public AuDialog
{
	Q_OBJECT;
private:
	AuGearPos							m_Design;				// Design selected in gear, if accepted.
	quint32								m_DesignId;

	// Widgets
	QComboBox*							m_GearDesigns;

public:
	AuDesignerGearOpen(QWidget* parent=0);
	virtual ~AuDesignerGearOpen();

	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	inline AuGearPos GearPos() const
	{
		return m_Design;
	}

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListDesigns();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void ChooseOne();
};

#endif // AuDesignerGearOpen_H
