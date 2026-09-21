//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovassignoffice.h,v 1.10 2022/02/19 17:56:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovAssignOffice_H
#define AuGovAssignOffice_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include "auoffice.h"
#include "dgempire.h"

class QPushButton;
class QListWidget;

class AUCLIENT_DLL AuGovAssignOffice : public AuDialog
{
	Q_OBJECT;
private:
	AuId								m_Assignee;
	DgEmpire							m_Empire;
	AuOffice							m_Office;				// Office settings we will assign.
	AuId								m_PlaceId;
	QString								m_PlaceName;

	// Widgets
	QPushButton*						m_AddButton;
	QPushButton*						m_OkButton;
	QListWidget*						m_DomainList;
	QPushButton*						m_RemoveButton;

public:
	AuGovAssignOffice(quint32 empireId, const AuId& assigner, const AuId& assignee, QWidget* parent=0);
	virtual ~AuGovAssignOffice();

	const AuOffice& Office() const
	{
		return m_Office;
	}

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void AddDomain();
	void AssignOffice();
	void DomainSelectionChanged();
	void RemoveDomain();
	void TitleOfOfficeChanged(const QString& title);
	void ToggleCanAcceptCitySurrender();
	void ToggleCanAccessGovtAccount();
	void ToggleCanBuild();
	void ToggleCanDelegatePermissions();
	void ToggleCanInvite();
	void ToggleCanKick();
	void ToggleCanLoadOfficers();
	void ToggleCanManageDossier();
	void ToggleCanManageTrade();
	void ToggleCanSendEmpireMail();
	void ToggleCanSetAnnouncements();
	void ToggleCanSetCapital();
	void ToggleCanSetColorTheme();
	void ToggleCanSetCommerce();
	void ToggleCanSetContrailColor();
	void ToggleCanSetEmpireFlag();
	void ToggleCanSetJoinPassword();
	void ToggleCanSetGalaxyCapital();
	void ToggleCanSetMfg();
	void ToggleCanSetMfgSpacecraft();
	void ToggleCanSetPlaceNames();
	void ToggleCanSetPoliticalStance();
	void ToggleCanSetResearch();
	void ToggleCanSetSectorCapital();
	void ToggleCanSetTax();
	void ToggleCanSetTitle();
	void ToggleCanSetWarPolicy();
	void ToggleCanUseDiplomacyChannel();
	void ToggleRxFleetMessages();
	void ToggleRxSensorScans();
};

#endif // AuGovAssignOffice_H
