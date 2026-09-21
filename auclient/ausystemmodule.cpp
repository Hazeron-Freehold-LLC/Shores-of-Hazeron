//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemmodule.cpp,v 1.3 2022/02/19 18:17:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausystemmodule.h"
//
#include "auglobal.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>

AuSystemModule::AuSystemModule(SYSTEM system, bool isBuilding, COMMODITYID currentModule, QWidget* parent) :
	AuDialog(parent)
{
	m_Module = currentModule;

	setAuDlgIcon(QIcon(":data/Design.png"));
	setAuDlgTitle(QString("%1 Module").arg(ScSystemName(system)));

	QLabel* lb = new QLabel(this);
	lb->setPixmap(QPixmap(SystemIcon(system)));

	m_Modules = new QComboBox(this);
	m_Modules->setToolTip(ScSystemName(system));
	ListModules(m_Modules, system, isBuilding, (COMMODITYID)m_Module);
	connect(m_Modules, SIGNAL(activated(int)), SLOT(ModuleChanged(int)));

	QPushButton* pbok = new QPushButton(tr("Place Equipment"));
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));
	pbok->setToolTip(tr("Place equipment using the selected system module."));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lb, row,0);
	lay->addWidget(m_Modules, row,1);
	row++;
	lay->addWidget(pbok, row,0, 1,2);
	setAuDlgLayout(lay);
}

AuSystemModule::~AuSystemModule()
{
}

void AuSystemModule::ListModules(QComboBox* cb, SYSTEM system, bool isBuilding, COMMODITYID currentModuleCID)
{
	for(DbCommodityArray::const_iterator ci = g_Au.m_CommodityArray.constBegin(); ci != g_Au.m_CommodityArray.constEnd(); ci++)
	{
		if(isBuilding)
		{
			if(DbCommodity::IsBdModule(system, (*ci).m_id))
				cb->addItem(QIcon((*ci).BitmapName()), (*ci).m_name, (*ci).m_id);
		}
		else
		{
			if(DbCommodity::IsScModule(system, (*ci).m_id))
				cb->addItem(QIcon((*ci).BitmapName()), (*ci).m_name, (*ci).m_id);
		}
	}

	int curIndex = cb->findData(currentModuleCID);
	if(curIndex >= 0)
		cb->setCurrentIndex(curIndex);
}

void AuSystemModule::ModuleChanged(int index)
{
	m_Module = m_Modules->itemData(index).toInt();
}
