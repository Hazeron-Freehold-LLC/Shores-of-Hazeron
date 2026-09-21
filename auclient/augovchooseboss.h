//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovchooseboss.h,v 1.6 2022/02/19 17:56:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovChooseBoss_H
#define AuGovChooseBoss_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include "dgempire.h"

class AuOffice;
class AuOfficeDomainPlace;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

class AUCLIENT_DLL AuGovChooseBoss : public AuDialog
{
	Q_OBJECT;
	AuId								m_Boss;
	DgEmpire							m_Empire;

	// Widgets
	QTreeWidget*						m_GovernTree;
	QPushButton*						m_OkButton;

public:
	// 'governorId' is the governing official whose new boss is being selected.
	AuGovChooseBoss(const DgEmpire& empire, const AuId& governorId, QWidget* parent=0);
	virtual ~AuGovChooseBoss();

	const AuId& Boss() const
	{
		return m_Boss;
	}

public slots:
	void Accept();
	void SelectionChanged();
	virtual void setVisible(bool visible);

protected:
	void AddSubordinates(QTreeWidgetItem* boss, QSet<AuId>& alreadyListed);
	void FillGovernTree(QTreeWidget* tree, const AuId& governorId);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuGovChooseBoss_H
