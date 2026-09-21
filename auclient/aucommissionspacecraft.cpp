//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommissionspacecraft.cpp,v 1.7 2022/02/19 18:10:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommissionspacecraft.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

AuCommissionSpacecraft::AuCommissionSpacecraft(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Commission Fleet Spacecraft"));
	setAuDlgIcon(QIcon(":data/FleetShip.png"));
	m_DesignId = 0;

	QLabel* lbbuy = new QLabel(tr("<html>"
				"Spacecraft can be commissioned using your governmental authority. "
				"The design is added to the queue. "
				"Construction begins when the current running process is done. "
				"</p>"

				"<p>"
				"Quality of fleet spacecraft is not limited by the company quality restriction. "
				"</p>"

				"<p>"
				"Fleet ships receive free fuel, repairs and ammunition at friendly ports. "
				"Fleet tax is levied on income earned using fleet ships. "
				"</p>"
				"</html>"), this);
	lbbuy->setWordWrap(true);

	m_BuyWhat = new QComboBox(this);
	m_BuyWhat->setToolTip(tr("<html>Selects the design of the spacecraft to commission. </html>"));

	QPushButton* pbbuy = new QPushButton(tr("Commission Now"), this);
	pbbuy->setToolTip(tr("Commissions a spacecraft of the selected design to be built for the fleet. "));
	connect(pbbuy, SIGNAL(clicked(bool)), SLOT(BuyOne()));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addStretch();
	blay->addWidget(pbbuy);
	blay->setMargin(2);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbbuy);
	lay->addWidget(m_BuyWhat);
	lay->addLayout(blay);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuCommissionSpacecraft::~AuCommissionSpacecraft()
{
}

void AuCommissionSpacecraft::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		ListDesigns();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuCommissionSpacecraft::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/CommissionSpacecraft/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/CommissionSpacecraft/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/CommissionSpacecraft/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/CommissionSpacecraft/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuCommissionSpacecraft::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/CommissionSpacecraft/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/CommissionSpacecraft/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/CommissionSpacecraft/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/CommissionSpacecraft/SizeY",sz.height());
}

void AuCommissionSpacecraft::BuyOne()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	// Design id is negative item data value.
	m_DesignId = m_BuyWhat->itemData(m_BuyWhat->currentIndex(), Qt::UserRole).toUInt();
	m_BuyDesign = m_BuyWhat->itemData(m_BuyWhat->currentIndex(), Qt::UserRole+1).value<AuGearPos>();
	//qDebug() << "AuCommissionSpacecraft::BuyOne designId" << m_DesignId;
	accept();
}

void AuCommissionSpacecraft::ListDesigns()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	m_BuyWhat->blockSignals(true);

	DaPerson* sceneAtom;
	view->LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		// Add designs in my possession.
		QSet<quint32> designsListed;
		const AuGear* inv = sceneAtom->LockGearForRead();
		AuGearPosList allGearPos;
		inv->ListAll(allGearPos);
		for(AuGearPosList::const_iterator gi = allGearPos.cbegin(); gi != allGearPos.cend(); gi++)
		{
			const AuItem* gitem = inv->ItemAt(*gi);

			const AuItemScDesign* scitem = dynamic_cast<const AuItemScDesign*>(gitem);
			if(scitem)
			{
				if(scitem->Exchange().CanBeManufactured() &&
						scitem->Exchange().AccessibleTo(sceneAtom->Id(), sceneAtom->EmpireId()))
				{
					// Found a new AuScDesign disk.
					quint32 designId = scitem->Exchange().DesignId();
					if(!designsListed.contains(designId))
					{
						designsListed.insert(designId);
						QString text = tr("%1 %2 by %3")
							.arg(scitem->Exchange().DesignId())
							.arg(scitem->Exchange().DesignName())
							.arg(scitem->Exchange().DesignArchitectName());
						m_BuyWhat->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, designId);
						int nuidx = m_BuyWhat->findData(designId);
						if(nuidx >= 0)
							m_BuyWhat->setItemData(nuidx, QVariant::fromValue(*gi), Qt::UserRole+1);
					}
				}
				continue;
			}

			const AuItemStorageMedia* media = dynamic_cast<const AuItemStorageMedia*>(gitem);
			if(media)
			{
				for(AuScDesignExchange::const_iterator di = media->BlueprintsSc().cbegin(); di != media->BlueprintsSc().cend();
						di++)
				{
					// Design must be manufacturable.
					if(!(*di).CanBeManufactured())
						continue;
					// Found a new AuScDesign disk.
					if(!(*di).AccessibleTo(sceneAtom->Id(), sceneAtom->EmpireId()))
						continue;

					quint32 designId = (*di).DesignId();
					if(!designsListed.contains(designId))
					{
						designsListed.insert(designId);
						QString text = tr("%1 %2 by %3")
							.arg(designId)
							.arg((*di).DesignName())
							.arg((*di).DesignArchitectName());
						m_BuyWhat->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDrawing)), text, designId);
						int nuidx = m_BuyWhat->findData(designId);
						if(nuidx >= 0)
							m_BuyWhat->setItemData(nuidx, QVariant::fromValue(*gi), Qt::UserRole+1);
					}
				}
				continue;
			}
		}
		sceneAtom->UnlockGear();
	}
	view->UnlockSceneAtom();

	m_BuyWhat->setCurrentIndex(-1);
	m_BuyWhat->blockSignals(false);
}
