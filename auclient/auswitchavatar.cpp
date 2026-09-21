//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auswitchavatar.cpp,v 1.4 2022/02/19 18:17:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auswitchavatar.h"
//
#include "auapp.h"
#include "auframe.h"
#include "aupushbutton.h"
#include "auview.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QTimer>
#include <QVBoxLayout>

AuSwitchAvatar::AuSwitchAvatar(QWidget* parent) : AuDialog(parent)
{
	m_Timer = 0;
	setAuDlgIcon(QIcon(":data/Contacts.png"));
	setAuDlgTitle("<b>Switch to Avatar</b>");
	m_MailAlso = g_Au.Settings().readBoolEntry("Widget/SwitchAvatar/MailAlso", true);

	// Get the known ids and names of their characters.
	QStringList names;
	AuIdList ids;
	const DgData& gd = g_Au.LockDataForRead();
	AuId currentId = gd.ClientUnitId();
	for(int ichr=0; ichr<6; ichr++)
	{
		AuId unitId = gd.ClientUnitId(ichr);
		if(!unitId.IsNull())
		{
			//qDebug() << unitId << gd.ClientUnitName(unitId);
			ids.append(unitId);
			names.append(gd.ClientUnitName(unitId));
		}
	}
	g_Au.UnlockData();

	QVBoxLayout* lay = new QVBoxLayout;

	// Create widgets.
	for(int ichr=0; ichr<6; ichr++)
	{
		m_Avatars[ichr] = new AuPushButtonValue("", ichr, this);
		connect(m_Avatars[ichr], SIGNAL(ClickedValue(int)), SLOT(SwitchToAvatar(int)));
		m_Avatars[ichr]->hide();
		lay->addWidget(m_Avatars[ichr]);
	}

	QCheckBox* ckMailAlso = new QCheckBox(tr("Switch Mail Also"), this);
	ckMailAlso->setCheckState(m_MailAlso ? Qt::Checked : Qt::Unchecked);
	ckMailAlso->setToolTip("<html>When checked, the current recipient on the <b>Mail</b> window will "
			"change when you switch to a different avatar.</html>");
	connect(ckMailAlso, SIGNAL(stateChanged(int)), SLOT(SwitchMailAlso(int)));
	lay->addWidget(ckMailAlso);

	setAuDlgLayout(lay);

	auApp->RequestAvatars();
}

AuSwitchAvatar::~AuSwitchAvatar()
{
}

void AuSwitchAvatar::SwitchToAvatar(int index)
{
	const DgData& gd = g_Au.LockDataForRead();
	m_SwitchTo = gd.ClientUnitId(index);
	g_Au.UnlockData();

	accept();
}

void AuSwitchAvatar::SwitchMailAlso(int checkBoxState)
{
	m_MailAlso = checkBoxState == Qt::Checked;
	g_Au.Settings().writeEntry("Widget/SwitchAvatar/MailAlso", m_MailAlso);
}

void AuSwitchAvatar::showEvent(QShowEvent* event)
{
	AuDialog::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(1000);

	UpdateContents();
}

void AuSwitchAvatar::hideEvent(QHideEvent* event)
{
	AuDialog::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_Timer && m_Timer->isActive())
		m_Timer->stop();
}

void AuSwitchAvatar::UpdateContents()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}

	// Get the known ids and names of their characters.
	const DgData& gd = g_Au.LockDataForRead();
	for(int ichr=0; ichr<6; ichr++)
	{
		AuId unitId = gd.ClientUnitId(ichr);
		if(!unitId.IsNull())
		{
			m_Avatars[ichr]->show();
			m_Avatars[ichr]->setText(gd.ClientUnitName(unitId));
			m_Avatars[ichr]->setEnabled(unitId != gd.ClientUnitId());	// Disable current dude; they can't switch to it.
		}
		else
			m_Avatars[ichr]->hide();
	}
	g_Au.UnlockData();
}
