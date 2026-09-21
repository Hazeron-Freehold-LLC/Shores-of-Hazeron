//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autablewidgetitem.h,v 1.3 2022/02/19 18:31:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuTableWidgetItem_H
#define AuTableWidgetItem_H

#include "augloballib.h"
#include <QTableWidgetItem>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A QTableWidgetItem that sorts numerically, instead of alphabetically.

class AUGLOBAL_DLL AuTableWidgetDistanceItem : public QTableWidgetItem
{
public:
	AuTableWidgetDistanceItem(double feet);

	bool operator<(const QTableWidgetItem& other) const
	{
		QString myText = text().left(text().length()-1);
		myText = myText.replace(",","");
		myText = myText.replace(".","");
		QString otherText = other.text().left(other.text().length()-1);
		otherText = otherText.replace(",","");
		otherText = otherText.replace(".","");

		double myDist = QVariant(myText).toDouble();
		double otherDist = QVariant(otherText).toDouble();
		return (myDist < otherDist) ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A QTableWidgetItem that sorts numerically, instead of alphabetically.

class AUGLOBAL_DLL AuTableWidgetIntegerItem : public QTableWidgetItem
{
public:
	AuTableWidgetIntegerItem(int value)
	{
		if(value)
			setText(QVariant(value).toString());
	}

	bool operator<(const QTableWidgetItem& other) const
	{
		int myInt = QVariant(text()).toInt();
		int otherInt = QVariant(other.text()).toInt();
		return (myInt < otherInt) ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A QTableWidgetItem that sorts numerically, instead of alphabetically.

class AUGLOBAL_DLL AuTableWidgetParsecItem : public QTableWidgetItem
{
public:
	AuTableWidgetParsecItem(double parsecs)
	{
		QString range;
		if(parsecs)
		{
			range = QString("%L1pc").arg(parsecs, 0, 'f', 2);
			setText(range);
		}
	}

	bool operator<(const QTableWidgetItem& other) const
	{
		double myRange = QVariant(text().left(text().length()-2)).toDouble();
		double otherRange = QVariant(other.text().left(other.text().length()-2)).toDouble();
		return (myRange < otherRange) ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A QTableWidgetItem that sorts numerically, instead of alphabetically.

class AUGLOBAL_DLL AuTableWidgetSortedItem : public QTableWidgetItem
{
public:
	AuTableWidgetSortedItem(double sortOrder, const QIcon & icon, const QString & text) : QTableWidgetItem(icon, text)
	{
		setData(Qt::UserRole+1, sortOrder);
	}

	bool operator<(const QTableWidgetItem& other) const
	{
		double mySortOrder = QVariant(data(Qt::UserRole+1)).toDouble();
		double otherSortOrder = QVariant(other.data(Qt::UserRole+1)).toDouble();
		return (mySortOrder < otherSortOrder) ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuTableWidgetItem_H
