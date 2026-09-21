//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubuyspacecraft.cpp,v 1.7 2022/02/19 18:10:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubuyspacecraft.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "demanufacturingprocess.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

AuBuySpacecraft::AuBuySpacecraft(quint8 maxQ, QWidget* parent) : AuDialog(parent)
{
	Q_ASSERT(maxQ);
	m_DesignId = 0;
	m_MaxQ = maxQ;

	setAuDlgTitle(tr("Buy Company Spacecraft"));
	setAuDlgIcon(QIcon(":data/c_Spacecraft.png"));

	QLabel* lbbuy = new QLabel(tr("<html>"
				"Spacecraft can be purchased using your personal funds. "
				"The design is added to the queue. "
				"Construction begins when the current running process is done. "
				"When completed, mail is sent to the owner. "
				"</p>"

				"<p>"
				"Your avatar will be the owner of the ship. "
				"Spacecraft owned by an avatar are considered to be <i>company</i> ships. "
				"An avatar may own any number of ships in their company. "
				"</p>"

				"<p>"
				"Company ships must purchase their own fuel, repairs and ammunition. "
				"Fleet tax is not levied on income earned using company ships. "
				"</p>"

				"<p>"
				"The new ship flies the flag of the buyer's empire. "
				"Manufacture of company ships is contingent upon the factory empire's commerce restrictions. "
				"They may restrict the sale of company ships to the buyer, based upon the buyer's empire. "
				"</p>"
				"</html>"), this);
	lbbuy->setWordWrap(true);

	m_BuyWhat = new QComboBox(this);
	m_BuyWhat->setToolTip(tr("<html>Selects the design of the spacecraft to purchase. </html>"));
	connect(m_BuyWhat, SIGNAL(activated(int)), SLOT(BuySelectionChanged(int)));

	m_Price = new QLabel(this);
	m_Price->setToolTip(tr("<html>"
				"<p>"
				"Old style designs must be available in the cache to estimate the cost of building them. "
				"If the cost estimate says <i>Unknown</i>, try loading the design in a design studio. "
				"This will fetch the data needed to estimate the cost. "
				"</p>"
				"</html>"));
	m_Price->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QPushButton* pbbuy = new QPushButton(tr("Buy Now"), this);
	pbbuy->setToolTip(tr("Buys a spacecraft of the selected design using your personal funds. "));
	connect(pbbuy, SIGNAL(clicked(bool)), SLOT(BuyOne()));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addStretch();
	blay->addWidget(m_Price);
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

AuBuySpacecraft::~AuBuySpacecraft()
{
}

void AuBuySpacecraft::setVisible(bool visible)
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

void AuBuySpacecraft::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BuySpacecraft/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BuySpacecraft/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BuySpacecraft/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BuySpacecraft/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBuySpacecraft::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BuySpacecraft/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BuySpacecraft/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BuySpacecraft/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BuySpacecraft/SizeY",sz.height());
}

void AuBuySpacecraft::BuySelectionChanged(int index)
{
	if(index < 0)
		return;

	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	m_DesignId = m_BuyWhat->itemData(index).toUInt();
	if(!m_DesignId)
	{
		m_Price->setText("");
		return;
	}
	quint8 costEstQ = m_BuyWhat->itemData(index, Qt::UserRole+3).toUInt();
	double estCost = m_BuyWhat->itemData(index, Qt::UserRole+4).toDouble();
	if(estCost == 0)
	{
		// Make a manufacturing process for the design so we can estimate the cost of manufacturing the design.
		// This requires the design to be available locally, should only be an issue for old style designs.
		DeManufacturingProcess mfgProc;
		mfgProc.InitSc(m_DesignId, costEstQ);

		// Estimate the cost at the design quality.
		estCost = mfgProc.EstimatedCost(costEstQ);
	}

	QString text;
	if(estCost > 0)
		text = tr("Est. %L1%2 Using Q%4")
			.arg(estCost,0,'f',0)
			.arg(g_Au.CurrencySymbol())
			.arg(costEstQ);
	else
		text = tr("Est. Unknown");
	m_Price->setText(text);
}

void AuBuySpacecraft::BuyOne()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	// Design id is negative item data value.
	m_BuyDesign = m_BuyWhat->itemData(m_BuyWhat->currentIndex(), Qt::UserRole+5).value<AuGearPos>();

	accept();
}

void AuBuySpacecraft::ListDesigns()
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
					if(scitem->Exchange().DesignQuality() <= m_MaxQ)
					{
						quint32 designId = scitem->Exchange().DesignId();
						if(!designsListed.contains(designId))
						{
							designsListed.insert(designId);
							QString text = tr("%1 %2 by %3")
								.arg(scitem->Exchange().DesignId())
								.arg(scitem->Exchange().DesignName())
								.arg(scitem->Exchange().DesignArchitectName());
							m_BuyWhat->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, designId);
							int idx = m_BuyWhat->findData(designId);
							m_BuyWhat->setItemData(idx, scitem->Exchange().DesignName(), Qt::UserRole+1);
							m_BuyWhat->setItemData(idx, scitem->Exchange().DesignQuality(), Qt::UserRole+3);
							m_BuyWhat->setItemData(idx, scitem->Exchange().EstimatedCost(), Qt::UserRole+4);
							m_BuyWhat->setItemData(idx, QVariant::fromValue(*gi), Qt::UserRole+5);
						}
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

					if((*di).DesignQuality() <= m_MaxQ)
					{
						quint32 designId = (*di).DesignId();
						if(!designsListed.contains(designId))
						{
							designsListed.insert(designId);
							QString text = tr("%1 %2 by %3")
								.arg((*di).DesignId())
								.arg((*di).DesignName())
								.arg((*di).DesignArchitectName());
							m_BuyWhat->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, designId);
							int idx = m_BuyWhat->findData(designId);
							m_BuyWhat->setItemData(idx, (*di).DesignName(), Qt::UserRole+1);
							m_BuyWhat->setItemData(idx, (*di).DesignQuality(), Qt::UserRole+3);
							m_BuyWhat->setItemData(idx, (*di).EstimatedCost(), Qt::UserRole+4);
							m_BuyWhat->setItemData(idx, QVariant::fromValue(*gi), Qt::UserRole+5);
						}
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

	m_Price->setText("");
}
