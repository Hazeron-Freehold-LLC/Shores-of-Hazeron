//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audefaultspacecraft.cpp,v 1.9 2022/02/19 18:10:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audefaultspacecraft.h"
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

AuDefaultSpacecraft::AuDefaultSpacecraft(QWidget* parent) : AuDialog(parent)
{
	m_DesignId = 0;
	m_DesignQ = 1;

	setAuDlgTitle(tr("Default Spacecraft Factory Production"));
	setAuDlgIcon(QIcon(":data/bSpacecraftFactory.png"));

	QLabel* lbmake = new QLabel(tr("<html>"
				"<p>"
				"Spacecraft factories perform regular manufacturing like other factories. "
				"</p>"

				"<p>"
				"<b>Fleet</b> ships are spacecraft owned by the empire. "
				"Regular manufacturing produces fleet ships. "
				"Fleet ships receive fuel, repairs and ammunition free of charge at friendly ports. "
				"Fleet tax is levied on income earned using fleet ships. "
				"Fleet ships provide an easy way for new players to get started. "
				"</p>"

				"<p>"
				"<b>Stations</b> are launched into stationary synchronous orbit above the factory. "
				"Each empire can have one station, with no officer, per world. "
				"There is no limit to the number of stations with officers at a world. "
				"</p>"

				"<p>"
				"A new station will replace an existing station, if the new station is superior to the existing station. "
				"When a station is replaced, crew, cargo and the name of the existing station are transferred to the new station. "
				"</p>"

				"<p>"
				"<b>Small Ships</b> are built on the tarmac next to the factory. "
				"</p>"

				"<p>"
				"<b>Large Ships</b> are built in orbit next to a station. "
				"Fleet ships are limited to four per empire in orbit that are still <i>new</i>. "
				"The blueprint analysis highlights when it must be built in orbit due to its size. "
				"</p>"

				"<p>"
				"A new ship will replace an existing new ship, if the new ship is superior to the existing new ship. "
				"A new ship will wait for an empty space to launch that is free of spacecraft, vehicles, people and creatures. "
				"</p>"

				"<p>"
				"<b>Use once</b> blueprints cannot be used for regular manufacturing. "
				"Use the <i>Commission</i> or <i>Buy</i> options to build a use once blueprint. "
				"</p>"
				"</html>"), this);
	lbmake->setWordWrap(true);
	lbmake->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	m_MakeWhat = new QComboBox(this);
	m_MakeWhat->setToolTip(tr("<html>Selects the blueprint of spacecraft to manufacture. </html>"));

	QPushButton* pbmakeit = new QPushButton(tr("Make Regularly"), this);
	pbmakeit->setToolTip(tr("<html>"
				"Configures the spacecraft factory to make the chosen blueprint. "
				"The blueprint will be made regularly until the process is changed. "
				"</html>"));
	connect(pbmakeit, SIGNAL(clicked(bool)), SLOT(MakeOne()));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addStretch();
	blay->addWidget(pbmakeit);
	blay->setMargin(2);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbmake);
	lay->addWidget(m_MakeWhat);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuDefaultSpacecraft::~AuDefaultSpacecraft()
{
}

void AuDefaultSpacecraft::setVisible(bool visible)
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

void AuDefaultSpacecraft::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DefaultSpacecraft/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DefaultSpacecraft/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DefaultSpacecraft/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DefaultSpacecraft/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuDefaultSpacecraft::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/DefaultSpacecraft/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/DefaultSpacecraft/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/DefaultSpacecraft/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/DefaultSpacecraft/SizeY",sz.height());
}

void AuDefaultSpacecraft::MakeOne()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	// Design id is negative item data value.
	m_DesignId = m_MakeWhat->itemData(m_MakeWhat->currentIndex()).toUInt();
	m_DesignName = m_MakeWhat->itemData(m_MakeWhat->currentIndex(), Qt::UserRole+1).toString();
	m_DesignQ = m_MakeWhat->itemData(m_MakeWhat->currentIndex(), Qt::UserRole+3).toUInt();
	if(!m_DesignQ)
		m_DesignQ = 1;

	accept();
}

void AuDefaultSpacecraft::ListDesigns()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	m_MakeWhat->blockSignals(true);
	m_MakeWhat->clear();

	DaPerson* sceneAtom;
	view->LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		QSet<int> designsListed;	// So we list each one only once. All positive, rnd or otherwise.

		// Add "none" entry.
		m_MakeWhat->addItem(QIcon(":data/ScHide.png"), tr("None"), 0);
		designsListed.insert(0);

		// Spacecraft factories are restricted to the designs available.
		// Add designs in my possession.
		const AuGear* inv = sceneAtom->LockGearForRead();
		AuGearPosList allGearPos;
		inv->ListAll(allGearPos);
		for(AuGearPosList::const_iterator gi = allGearPos.cbegin(); gi != allGearPos.cend(); gi++)
		{
			const AuItem* gitem = inv->ItemAt(*gi);

			// We do not list use-once items for factory default production.
			const AuItemScDesign* scitem = dynamic_cast<const AuItemScDesign*>(gitem);
			if(scitem)
			{
				if(scitem->Exchange().CanBeManufactured() && !scitem->Exchange().UseOnce() &&
						scitem->Exchange().AccessibleTo(sceneAtom->Id(), sceneAtom->EmpireId()))
				{
					// Found a new AuScDesign disk that is not a use-once design.
					quint32 designId = scitem->Exchange().DesignId();
					if(!designsListed.contains(designId))
					{
						designsListed.insert(designId);
						QString text = tr("%1 %2 by %3")
							.arg(designId)
							.arg(scitem->Exchange().DesignName())
							.arg(scitem->Exchange().DesignArchitectName());
						m_MakeWhat->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, designId);
						int idx = m_MakeWhat->findData(designId);
						m_MakeWhat->setItemData(idx, scitem->Exchange().DesignName(), Qt::UserRole+1);
						m_MakeWhat->setItemData(idx, scitem->Exchange().DesignQuality(), Qt::UserRole+3);
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
					// Design must be manufacturable and must not be a use once design; those have to be commissioned individually.
					if(!(*di).CanBeManufactured() || (*di).UseOnce())
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
						m_MakeWhat->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, designId);
						int idx = m_MakeWhat->findData(designId);
						m_MakeWhat->setItemData(idx, (*di).DesignName(), Qt::UserRole+1);
						m_MakeWhat->setItemData(idx, (*di).DesignQuality(), Qt::UserRole+3);
					}
				}
				continue;
			}
		}
		sceneAtom->UnlockGear();
	}
	view->UnlockSceneAtom();

	m_MakeWhat->setCurrentIndex(-1);
	m_MakeWhat->blockSignals(false);
}
