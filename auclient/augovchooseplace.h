//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovchooseplace.h,v 1.6 2022/02/19 17:56:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovChoosePlace_H
#define AuGovChoosePlace_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"

class AuOffice;
class AuOfficeDomainPlace;
class DgEmpire;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class AUCLIENT_DLL AuGovChoosePlace : public AuDialog
{
	Q_OBJECT;
	AuOfficeDomainPlace*				m_Place;

	// Widgets
	QTreeWidget*						m_DomainTree;
	QPushButton*						m_OkButton;

public:
	AuGovChoosePlace(const DgEmpire& empire, const AuOffice& office, QWidget* parent=0);
	virtual ~AuGovChoosePlace();

	const AuOfficeDomainPlace* Place() const
	{
		return m_Place;
	}

public slots:
	void Accept();
	void SelectionChanged();
	virtual void setVisible(bool visible);

protected:
	void AddSubordinates(QTreeWidgetItem* domain, const DgEmpire& empire, const AuOffice& office, QSet<AuId>& alreadyListed,
			const QFont& domainFont, const QBrush& domainBrush, const QFont& noGovernorFont, const QBrush& noGovernorBrush);
	void FillTree(QTreeWidget* tree, const DgEmpire& empire, const AuOffice& office);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuGovChoosePlace_H
