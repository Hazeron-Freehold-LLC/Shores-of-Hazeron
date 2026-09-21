//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdfencedlg.cpp,v 1.9 2022/02/19 18:25:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubdfencedlg.h"
//
#include "aubdexchangefilter.h"
#include "auclientglobal.h"
#include "audesignerapp.h"
#include "audesignerframe.h"
#include "augear.h"
#include "auscparts.h"
#include "auview.h"
#include "daperson.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuBdFenceDlg::AuBdFenceDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Fence Style"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Choose the kind of fence. "
				"Built in fence styles are listed. "
				"</p>"
				"<p>"
				"When on line, fence blueprints in your gear and in the exchange are listed. "
				"</p>"
				"<p>"
				"Fences made using blueprints can only be previewed while on line. "
				"When off line, those fences will appear as a row of stakes connected with string, "
				"like a fence under construction. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_StyleChoices = new QComboBox(this);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setToolTip(tr("<html>"
				"Accepts the chosen fence style. "
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(DoOk()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_StyleChoices);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBdFenceDlg::~AuBdFenceDlg()
{
}

void AuBdFenceDlg::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		ListStyles();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuBdFenceDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BuildingFenceDlg2/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BuildingFenceDlg2/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BuildingFenceDlg2/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BuildingFenceDlg2/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBdFenceDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BuildingFenceDlg2/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BuildingFenceDlg2/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BuildingFenceDlg2/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BuildingFenceDlg2/SizeY",sz.height());
}

void AuBdFenceDlg::DoOk()
{
	accept();
}

quint32 AuBdFenceDlg::DesignId() const
{
	quint32 designId = m_StyleChoices->itemData(m_StyleChoices->currentIndex()).toUInt();
	if(designId <= AuScFence::STYLE_Count)
		return 0;
	return designId;
}

int AuBdFenceDlg::Style() const
{
	quint32 style = m_StyleChoices->itemData(m_StyleChoices->currentIndex()).toUInt();
	if(style <= AuScFence::STYLE_Count)
		return style;
	return AuScFence::Custom;
}

void AuBdFenceDlg::ListStyles()
{
	for(int s=AuScFence::Posts; s < AuScFence::Custom; s++)
	{
		AuScFence::STYLE style = (AuScFence::STYLE)s;
		m_StyleChoices->addItem(QIcon(AuScFence::StyleIcon(style)), AuScFence::StyleName(style), style);
	}

	AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
	AuView* view = frame->View();

	// Filter
	QSet<AuId> excludePub;
	bool terLand = true;
	bool terSeaBottom = false;
	bool intendedIndustry = true;
	INDUSTRYID industryId = IID_Fence;
	ORBITZONE ozone = OZONE_Habitable;
	bool gasGiant = false;
	bool breathableAtmo = true;
	bool enclosurePresent = false;
	bool mustBeRated = false;		// Only applies to exchange.
	int minRating = 0;				// Only applies to exchange.

	// Check gear for fence designs.
	AuId sceneAtomId;
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		sceneAtomId = person->Id();

		// List the building design disks for 'industryId' the person is carrying.
		const AuGear* inv = person->LockGearForRead();
		if(inv)
		{
			AuGearPosList allGearPos;
			inv->ListAll(allGearPos);
			for(AuGearPosList::const_iterator gi = allGearPos.cbegin(); gi != allGearPos.cend(); gi++)
			{
				const AuItemBdDesign* item = dynamic_cast<const AuItemBdDesign*>(inv->ItemAt(*gi));
				if(item)
				{
					if(item->Exchange().IndustryId() != IID_Fence)
						continue;
					if(!AuBdExchangeFilter::TestFilter(excludePub, terLand, terSeaBottom, intendedIndustry, industryId, ozone,
								gasGiant, breathableAtmo, enclosurePresent, false, 0, item->Exchange().Report(), 0))
						continue;

					QString design = tr("%1 %2 by %3")
						.arg(item->Exchange().DesignId())
						.arg(item->Exchange().DesignName())
						.arg(item->Exchange().DesignArchitectName());
					m_StyleChoices->addItem(QIcon(AuScFence::StyleIcon(AuScFence::Custom)), design, item->Exchange().DesignId());
					continue;
				}

				const AuItemStorageMedia* storage = dynamic_cast<const AuItemStorageMedia*>(inv->ItemAt(*gi));
				if(storage)
				{
					for(AuBdDesignExchange::const_iterator bi = storage->BlueprintsBd().cbegin();
							bi != storage->BlueprintsBd().cend(); bi++)
					{
						if((*bi).IndustryId() != IID_Fence)
							continue;
						if(!AuBdExchangeFilter::TestFilter(excludePub, terLand, terSeaBottom, intendedIndustry, industryId, ozone,
									gasGiant, breathableAtmo, enclosurePresent, false, 0, (*bi).Report(), 0))
							continue;

						QString design = tr("%1 %2 by %3")
							.arg((*bi).DesignId())
							.arg((*bi).DesignName())
							.arg((*bi).DesignArchitectName());
						m_StyleChoices->addItem(QIcon(AuScFence::StyleIcon(AuScFence::Custom)), design, (*bi).DesignId());
					}
					continue;
				}
			}
		}
		person->UnlockGear();
	}
	view->UnlockSceneAtom();

	// Check exchange for fence designs.
	QHash<quint32, AuBdDesignExchangeEntryClient>& designs = g_AuClient.LockBdDesignExchange();
	for(QHash<quint32, AuBdDesignExchangeEntryClient>::const_iterator di = designs.cbegin(); di != designs.cend(); di++)
	{
		if((*di).DesignState() != AuDesign::Final)
			continue;
		if((*di).Price() != 0 && (*di).DesignArchitectId() != sceneAtomId)
			continue;
		if(!AuBdExchangeFilter::TestFilter(excludePub, terLand, terSeaBottom, intendedIndustry, industryId, ozone, gasGiant,
					breathableAtmo, enclosurePresent, mustBeRated, minRating, (*di).Report(), (*di).Rating()))
			continue;

		QString design = tr("%1 %2 by %3")
			.arg((*di).DesignId())
			.arg((*di).DesignName())
			.arg((*di).DesignArchitectName());
		m_StyleChoices->addItem(QIcon(AuScFence::StyleIcon(AuScFence::Custom)), design, (*di).DesignId());
	}
	g_AuClient.UnlockBdDesignExchange();
}
