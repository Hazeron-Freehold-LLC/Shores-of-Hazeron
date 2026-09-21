//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignergearopen.cpp,v 1.5 2022/02/19 18:25:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesignergearopen.h"
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

AuDesignerGearOpen::AuDesignerGearOpen(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Open Blueprint from Gear"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Select a blueprint to open from those carried in your gear. "
				"</p>"
				"<p>"
				"Publish terms on the blueprint determine if the blueprint can be edited once it is opened. "
				"<br>"
				"<b>Copy</b> permission enables all forms of editing to the blueprint. "
				"Parts can be copied to the clipboard and the blueprint can be saved to a file or to the library. "
				"<br>"
				"<b>View</b> permission enables a blueprint to be opened for viewing only. "
				"Parts cannot be copied to the clipboard and the blueprint cannot be saved to a file or to the library. "
				"<br>"
				"<b>Manufacture</b> permission prohibits opening of a blueprint in the designer. "
				"Media with manufacture permission will not be shown among the choices. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_GearDesigns = new QComboBox(this);
	m_GearDesigns->setToolTip(tr("<html>Shows blueprints in your gear that can be opened. </html>"));

	QPushButton* pbo = new QPushButton(tr("Open Design"), this);
	pbo->setToolTip(tr("<html>Opens the selected blueprint.</html>"));
	connect(pbo, SIGNAL(clicked(bool)), SLOT(ChooseOne()));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addStretch();
	blay->addWidget(pbo);
	blay->setMargin(2);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_GearDesigns);
	lay->addLayout(blay);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuDesignerGearOpen::~AuDesignerGearOpen()
{
}

void AuDesignerGearOpen::setVisible(bool visible)
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

void AuDesignerGearOpen::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DesignerGearOpen/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DesignerGearOpen/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DesignerGearOpen/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DesignerGearOpen/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuDesignerGearOpen::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GearDesignOpen/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GearDesignOpen/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GearDesignOpen/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GearDesignOpen/SizeY",sz.height());
}

void AuDesignerGearOpen::ChooseOne()
{
	int curIdx = m_GearDesigns->currentIndex();
	if(curIdx < 0)
		return;

	m_DesignId = m_GearDesigns->itemData(curIdx).toUInt();
	m_Design = m_GearDesigns->itemData(curIdx, Qt::UserRole+1).value<AuGearPos>();
	accept();
}

void AuDesignerGearOpen::ListDesigns()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	m_GearDesigns->blockSignals(true);

	DaPerson* sceneAtom;
	view->LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		// Add designs in my possession.
		const AuGear* inv = sceneAtom->LockGearForRead();
		AuGearPosList allGearPos;
		inv->ListAll(allGearPos);
		for(AuGearPosList::const_iterator gi = allGearPos.cbegin(); gi != allGearPos.cend(); gi++)
		{
			const AuItem* gitem = inv->ItemAt(*gi);
			const AuItemBdDesign* bditem = dynamic_cast<const AuItemBdDesign*>(gitem);
			if(bditem)
			{
				if(bditem->Exchange().CanView() || bditem->Exchange().CanCopy())
				{
					if(!bditem->Exchange().AccessibleTo(sceneAtom->Id(), sceneAtom->EmpireId()))
						continue;

					QString perm;
					if(bditem->Exchange().CanCopy())
						perm = tr("Copy");
					else
						perm = tr("View");

					QString text = tr("%1 %2 by %3 (%4)")
						.arg(bditem->Exchange().DesignId())
						.arg(bditem->Exchange().DesignName())
						.arg(bditem->Exchange().DesignArchitectName())
						.arg(perm);
					m_GearDesigns->addItem(QIcon(g_Au.CommodityBitmapName(CID_BdDesignDisk)), text, bditem->Exchange().DesignId());
					int nuidx = m_GearDesigns->findData(bditem->Exchange().DesignId());
					if(nuidx >= 0)
						m_GearDesigns->setItemData(nuidx, QVariant::fromValue(*gi), Qt::UserRole+1);
				}
				continue;
			}

			const AuItemScDesign* scitem = dynamic_cast<const AuItemScDesign*>(gitem);
			if(scitem)
			{
				if(scitem->Exchange().CanView() || scitem->Exchange().CanCopy())
				{
					if(!scitem->Exchange().AccessibleTo(sceneAtom->Id(), sceneAtom->EmpireId()))
						continue;

					QString perm;
					if(scitem->Exchange().CanCopy())
						perm = tr("Copy");
					else
						perm = tr("View");

					QString text = tr("%1 %2 by %3 (%4)")
						.arg(scitem->Exchange().DesignId())
						.arg(scitem->Exchange().DesignName())
						.arg(scitem->Exchange().DesignArchitectName())
						.arg(perm);
					m_GearDesigns->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, scitem->Exchange().DesignId());
					int nuidx = m_GearDesigns->findData(scitem->Exchange().DesignId());
					if(nuidx >= 0)
						m_GearDesigns->setItemData(nuidx, QVariant::fromValue(*gi), Qt::UserRole+1);
				}
				continue;
			}

			const AuItemStorageMedia* media = dynamic_cast<const AuItemStorageMedia*>(gitem);
			if(media)
			{
				for(AuBdDesignExchange::const_iterator di = media->BlueprintsBd().cbegin(); di != media->BlueprintsBd().cend();
						di++)
				{
					if(!(*di).CanView() && !(*di).CanCopy())
						continue;
					// Found a new AuBdDesign disk.
					if(!(*di).AccessibleTo(sceneAtom->Id(), sceneAtom->EmpireId()))
						continue;

					QString perm;
					if((*di).CanCopy())
						perm = tr("Copy");
					else
						perm = tr("View");

					QString text = tr("%1 %2 by %3 (%4)")
						.arg((*di).DesignId())
						.arg((*di).DesignName())
						.arg((*di).DesignArchitectName())
						.arg(perm);
					m_GearDesigns->addItem(QIcon(g_Au.CommodityBitmapName(CID_BdDesignDisk)), text, (*di).DesignId());
					int nuidx = m_GearDesigns->findData((*di).DesignId());
					if(nuidx >= 0)
						m_GearDesigns->setItemData(nuidx, QVariant::fromValue(*gi), Qt::UserRole+1);
				}
				for(AuScDesignExchange::const_iterator di = media->BlueprintsSc().cbegin(); di != media->BlueprintsSc().cend();
						di++)
				{
					if(!(*di).CanView() && !(*di).CanCopy())
						continue;
					// Found a new AuScDesign disk.
					if(!(*di).AccessibleTo(sceneAtom->Id(), sceneAtom->EmpireId()))
						continue;

					QString perm;
					if((*di).CanCopy())
						perm = tr("Copy");
					else
						perm = tr("View");

					QString text = tr("%1 %2 by %3 (%4)")
						.arg((*di).DesignId())
						.arg((*di).DesignName())
						.arg((*di).DesignArchitectName())
						.arg(perm);
					m_GearDesigns->addItem(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDisk)), text, (*di).DesignId());
					int nuidx = m_GearDesigns->findData((*di).DesignId());
					if(nuidx >= 0)
						m_GearDesigns->setItemData(nuidx, QVariant::fromValue(*gi), Qt::UserRole+1);
				}
				continue;
			}
		}
		sceneAtom->UnlockGear();
	}
	view->UnlockSceneAtom();

	m_GearDesigns->setCurrentIndex(-1);
	m_GearDesigns->blockSignals(false);
}
