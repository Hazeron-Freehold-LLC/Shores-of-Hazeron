//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscconsolemoduledlg.cpp,v 1.3 2022/02/19 18:25:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscconsolemoduledlg.h"
//
#include "auglobal.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

AuScConsoleModuleDlg::AuScConsoleModuleDlg(SYSTEM system, bool isSc, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Console System Module"));
	setAuDlgIcon(QIcon(":data/UpgradeModule.png"));

	QLabel* lbmodules = new QLabel(tr("Choose a module for the %1 controlled by the console.")
			.arg(ScSystemName(system)), this);

	m_Modules = new QComboBox(this);

	if(isSc)
	{
		for(int cid=0; cid<CID_Count; cid++)
			if(DbCommodity::IsScModule(system, (COMMODITYID)cid))
				m_Modules->addItem(g_Au.CommodityName((COMMODITYID)cid), cid);
	}
	else
	{
		for(int cid=0; cid<CID_Count; cid++)
			if(DbCommodity::IsBdModule(system, (COMMODITYID)cid))
				m_Modules->addItem(g_Au.CommodityName((COMMODITYID)cid), cid);
	}

	QLabel* lbeffect;
	if(system == SYS_FTLDrive)
	{
		lbeffect = new QLabel(tr("Choose which surface effect will be animated when this FTL drive is active."), this);

		m_WhichEffect = new QComboBox(this);
		for(int s=0; s<4; s++)
			m_WhichEffect->addItem(tr("FTL Effect %1").arg(s+1), s);
	}
	else if(system == SYS_Transporter)
	{
		lbeffect = new QLabel(tr("Choose which surface effect will be animated when this transporter is active."), this);

		m_WhichEffect = new QComboBox(this);
		for(int s=0; s<10; s++)
			m_WhichEffect->addItem(tr("Transporter Effect %1").arg(s+1), s);
	}
	else if(system == SYS_Weapon)
	{
		lbeffect = new QLabel(tr("Choose which surface effect will be animated when this weapon is firing."), this);

		m_WhichEffect = new QComboBox(this);
		for(int s=0; s<10; s++)
			m_WhichEffect->addItem(tr("Weapon Effect %1").arg(s+1), s);
	}
	else
	{
		lbeffect = 0;
		m_WhichEffect = 0;
	}

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Accepts the module for the console system. "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbmodules);
	lay->addWidget(m_Modules);
	if(lbeffect && m_WhichEffect)
	{
		lay->addWidget(lbeffect);
		lay->addWidget(m_WhichEffect);
	}
	lay->addLayout(blay);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScConsoleModuleDlg::~AuScConsoleModuleDlg()
{
}

void AuScConsoleModuleDlg::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuScConsoleModuleDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/ConsoleModuleDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/ConsoleModuleDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/ConsoleModuleDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/ConsoleModuleDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScConsoleModuleDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/ConsoleModuleDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/ConsoleModuleDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/ConsoleModuleDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/ConsoleModuleDlg/SizeY",sz.height());
}

COMMODITYID AuScConsoleModuleDlg::Module() const
{
	return (COMMODITYID)m_Modules->itemData(m_Modules->currentIndex()).toInt();
}

int AuScConsoleModuleDlg::WhichEffect() const
{
	if(!m_WhichEffect)
		return 0;
	return m_WhichEffect->itemData(m_WhichEffect->currentIndex()).toInt();
}
