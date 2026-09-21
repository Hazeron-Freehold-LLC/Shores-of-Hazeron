//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupatentsget.cpp,v 1.7 2022/02/19 18:16:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aupatentsget.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "dgdata.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>

AuPatentsGet::AuPatentsGet(bool empirePatents, const AuId& opBd, QWidget* parent) : AuDialog(parent)
{
	m_CapitolOpId = opBd;
	m_EmpirePatents = empirePatents;
	setAuDlgTitle(tr("Request Patents"));
	setAuDlgIcon(QIcon(":data/c_Patent.png"));

	m_PatentList = new QListWidget(this);
	m_PatentList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_PatentList->setSpacing(2);
	connect(m_PatentList, SIGNAL(itemSelectionChanged()), SLOT(PatentSelectionChanged()));

	QLabel* lbq = new QLabel(tr("Limit Patent Quality"), this);
	lbq->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_QualitySpin = new QSpinBox(this);
	m_QualitySpin->setRange(1,255);
	m_QualitySpin->setValue(255);
	m_QualitySpin->setToolTip(tr("<html>Sets the maximum quality of patents to be copied. "
				"Enables you to request patents at a lower quality level than those available. "
				"</html>"));

	m_CountLabel = new QLabel(tr("No patents selected."), this);

	QLabel* lbmedia = new QLabel(tr("<html>Choose media in your gear. "
				"Paper and blank disks will only store one patent.</html>"), this);
	lbmedia->setWordWrap(true);

	m_GearMedia = new QComboBox(this);
	m_GearMedia->setToolTip(tr("<html>Shows media in your gear that can be used to store patents. "
				"If paper or a blank disk is selected, only one patent will be stored.</html>"));
	connect(m_GearMedia, SIGNAL(currentIndexChanged(int)), SLOT(GearMediaChanged(int)));

	m_Ok = new QPushButton(tr("Ok"), this);
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	m_Ok->setEnabled(false);
	m_Ok->setToolTip(tr("<html>"
				"Request copies of the selected patents, to be recorded on my media."
				"</html>"));
	connect(m_Ok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* qlay = new QHBoxLayout;
	qlay->addWidget(lbq, 1);
	qlay->addWidget(m_QualitySpin);
	qlay->setMargin(0);
	qlay->setSpacing(2);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_GearMedia, 1);
	blay->addWidget(m_Ok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_PatentList);
	lay->addLayout(qlay);
	lay->addWidget(m_CountLabel);
	lay->addWidget(lbmedia);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuPatentsGet::~AuPatentsGet()
{
}

void AuPatentsGet::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		ListPatents();
		ListMedia();
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuPatentsGet::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GetPatents/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GetPatents/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GetPatents/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GetPatents/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuPatentsGet::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GetPatents/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GetPatents/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GetPatents/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GetPatents/SizeY",sz.height());
}

void AuPatentsGet::Patents(QSet<COMMODITYID>& patents) const
{
	QItemSelectionModel* sel = m_PatentList->selectionModel();
	if(sel && sel->hasSelection())
	{
		QModelIndexList selRowIdxs = sel->selectedRows();
		for(QModelIndexList::const_iterator si = selRowIdxs.cbegin(); si != selRowIdxs.cend(); si++)
		{
			COMMODITYID patentCID  = (COMMODITYID)(*si).data(Qt::UserRole).toInt();
			patents.insert(patentCID);
		}
	}
}

void AuPatentsGet::ListPatents()
{
	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	if(!pView)
		return;
	quint32 empireId = pView->SceneAtomEmpire();
	if(!empireId)
		return;
	// Lock the root and find the scene atom.
	AuKnownPatents patents;	// Patents available.
	if(m_EmpirePatents)
	{
		DgEmpire emp;
		emp.m_id = empireId;
		const DgData& gd = g_Au.LockDataForRead();
		bool hasEmp = gd.Empire(emp);
		g_Au.UnlockData();
		if(!hasEmp)
			return;
		emp.ListPatents(patents);
	}
	else
	{
		DaPerson* person;
		pView->LockSceneAtom(&person);
		if(person && person->InIndustry() != IID_None)
		{
			const DaBd* bd = person->BdOp(m_CapitolOpId);
			if(bd)
				bd->ListPatents(patents, bd->IndustryId());
		}
		pView->UnlockSceneAtom();
	}

	m_PatentList->setSortingEnabled(false);
	for(QHash<COMMODITYID, quint8>::const_iterator pi = patents.cbegin(); pi != patents.cend(); pi++)
	{
		// Consolidate the inventory into a list that stacks items instead of listing them one-by-one.
		// Lists items in top-level containers.
		// Does not list containers that are not empty.
		if(!DbCommodity::IsPatent(pi.key()))
			continue;
		QString name = QString("%1 Q%2").arg(g_Au.CommodityName(pi.key())).arg(*pi);
		QListWidgetItem* lwi = new QListWidgetItem(QIcon(g_Au.CommodityBitmapName(pi.key())), name, m_PatentList);
		lwi->setData(Qt::UserRole, pi.key());
		lwi->setData(Qt::UserRole+1, *pi);
	}
	m_PatentList->setSortingEnabled(true);
	m_PatentList->sortItems();
}

void AuPatentsGet::ListMedia()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	m_GearMedia->blockSignals(true);

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
			if(gitem->CommodityId() == CID_BlankDisk)
			{
				m_GearMedia->addItem(QIcon(gitem->BitmapName()), gitem->ListDesc(), QVariant::fromValue(*gi));
				continue;
			}
			else if(gitem->CommodityId() == CID_Paper && gitem->Inscription().isEmpty())
			{
				m_GearMedia->addItem(QIcon(gitem->BitmapName()), gitem->ListDesc(), QVariant::fromValue(*gi));
				continue;
			}
			const AuItemStorageMedia* media = dynamic_cast<const AuItemStorageMedia*>(gitem);
			if(media)
			{
				if(media->StorageSpaceAvailable())
					m_GearMedia->addItem(QIcon(gitem->BitmapName()), gitem->ListDesc(), QVariant::fromValue(*gi));
				continue;
			}
		}
		sceneAtom->UnlockGear();
	}
	view->UnlockSceneAtom();

	m_GearMedia->setCurrentIndex(-1);
	m_GearMedia->blockSignals(false);
}

void AuPatentsGet::GearMediaChanged(int index)
{
	m_Media = m_GearMedia->itemData(index).value<AuGearPos>();
	m_MediaName = m_GearMedia->itemText(index);
	EnableControls();
}

void AuPatentsGet::EnableControls()
{
	bool enableOk = true;

	QItemSelectionModel* sel = m_PatentList->selectionModel();
	if(!sel || !sel->hasSelection())
		enableOk = false;

	if(!m_Media.IsValid())
		enableOk = false;

	m_Ok->setEnabled(enableOk);
}

quint8 AuPatentsGet::QualityLimit() const
{
	return m_QualitySpin->value();
}

void AuPatentsGet::PatentSelectionChanged()
{
	int countSel = 0;
	QItemSelectionModel* sel = m_PatentList->selectionModel();
	if(sel)
		countSel = sel->selectedIndexes().count();
	else
		countSel = 0;
	if(countSel == 0)
		m_CountLabel->setText(tr("No patents selected."));
	else if(countSel == 1)
		m_CountLabel->setText(tr("One patent selected."));
	else
		m_CountLabel->setText(tr("%1 patents selected.").arg(countSel));
	EnableControls();
}
