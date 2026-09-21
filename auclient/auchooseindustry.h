//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auchooseindustry.h,v 1.7 2022/02/19 17:53:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuChooseIndustry_H
#define AuChooseIndustry_H

#include "auclientlib.h"
#include "audialog.h"
#include "dbindustry.h"

class QListWidget;
class QListWidgetItem;

class AUCLIENT_DLL AuChooseIndustry : public AuDialog
{
	Q_OBJECT;
private:
	INDUSTRYID							m_IndustryId;			// The id of the selected industry.

	// Widgets
	QListWidget*						m_Industries;

public:
	// 'withBuildings' is true to only offer industries that have buildings. This omits town square and road.
	// 'currentIndustry' will be selected in the list, if it's in the list. Use IID_None to select none.
	AuChooseIndustry(const QString& caption, INDUSTRYID currentIndustry, bool withBuildings, QWidget* parent=0);
	virtual ~AuChooseIndustry();

	inline INDUSTRYID IndustryId() const
	{
		return m_IndustryId;
	}

protected slots:
	void IndustryClicked(QListWidgetItem* item);
};

#endif // AuChooseIndustry_H
