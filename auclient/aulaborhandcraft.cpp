//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulaborhandcraft.cpp,v 1.20 2022/05/04 22:01:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulaborhandcraft.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include "demanufacturingprocess.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>

AuHandCraftDock::AuHandCraftDock(QWidget* parent) :
	AuDockWidget(":data/it_HandCraft.png", tr("<b>Handcraft</b>"), tr("Handcraft"), parent)
{
	SetWikiTopic("Handcraft");
	m_HandCraft = new AuLaborHandCraft(this);
	setWidget(m_HandCraft);

	int dockSides = g_Au.Settings().readNumEntry("Dock/HandcraftAreas", (int)allowedAreas());
	setAllowedAreas((Qt::DockWidgetAreas)dockSides);
}

void AuHandCraftDock::SetAllowedAreas(Qt::DockWidgetAreas areas)
{
	setAllowedAreas(areas);
	g_Au.Settings().writeEntry("Dock/HandcraftAreas", (int)allowedAreas());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuHandCraftPanel::AuHandCraftPanel(QWidget* parent) : QWidget(parent)
{
	m_MPID = 0;

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"<i>Handcrafting</i> creates a new item from items in your gear. "
				"If the new item does not fit into your gear, it is dropped at your location. "
				"</p>"
				"<p>"
				"Manufacturing buildings enable specialized handcrafting choices, due to their workshop facilities. "
				"For example, to handcraft metal from ore you possess, go to a smelter. "
				"</p>"
				"<p>"
				"When a patent is needed, your gear or your current building must have a copy of the patent. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	m_Note = new QLabel(this);
	m_Note->setWordWrap(true);
	m_Note->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	// Make the controls.
	m_Status = new QLabel(tr("Nothing<br>Not Ready"), this);
	m_Status->setToolTip(tr("Shows current status of the handcrafting process."));

	m_StatusIcon = new QLabel(this);
	m_StatusIcon->setToolTip(tr("Shows current status of the handcrafting process."));

	m_MakeWhat = new QComboBox(this);
	m_MakeWhat->setToolTip(tr("Selects a handcrafting process to run."));
	connect(m_MakeWhat, SIGNAL(activated(int)), SLOT(ProcessSelectionChanged(int)));

	m_Components = new QListWidget(this);
	m_Components->setToolTip(tr("Commodities consumed by the handcrafting process. All commodities must be available in your "
				"gear before the <b>Make</b> button can be pressed."));
	m_Components->setMinimumHeight(10);

	m_Modifiers = new QTableWidget(0,3,this);	// 0 rows, 3 columns
	m_Modifiers->setMinimumWidth(70);
	m_Modifiers->setMaximumWidth(110);
	m_Modifiers->setMinimumHeight(10);
	m_Modifiers->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_Modifiers->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Modifiers->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Modifiers->setShowGrid(false);
	m_Modifiers->horizontalHeader()->hide();
	m_Modifiers->horizontalHeader()->setDefaultSectionSize(24);
	m_Modifiers->horizontalHeader()->setMinimumSectionSize(24);
	m_Modifiers->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
	m_Modifiers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	m_Modifiers->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	m_Modifiers->verticalHeader()->hide();
	m_Modifiers->verticalHeader()->setDefaultSectionSize(18);
	m_Modifiers->setToolTip(tr("<html>Commodities that affect the handcrafting process without being consumed. "
				"Though not directly consumed, these commodities may wear out when used by the process.<br>"
				"<i>Required</i> commodities must be available in your gear before the <b>Make</b> button can be pressed."
				"</html>"));

	m_Make = new QToolButton(this);
	m_Make->setIcon(QIcon(":data/it_HandCraft.png"));
	m_Make->setIconSize(QSize(40,40));
	m_Make->setToolTip(tr("<html><b>Make</b><br>"
				"Handcrafts a new item using items in your gear. "
				"All commodities consumed by the process must be in your gear prior to pressing <b>Make</b>. "
				"The item can not be made if <i>required</i> commodities, such as tools, are not available in your gear."));
	connect(m_Make, SIGNAL(clicked(bool)), SLOT(GoPushed()));

	// Build the layout.
	QHBoxLayout* slay = new QHBoxLayout;
	slay->addWidget(m_StatusIcon);
	slay->addWidget(m_Status);
	slay->addStretch();

	QVBoxLayout* rlay = new QVBoxLayout;
	rlay->addWidget(m_Note);
	rlay->addLayout(slay);
	rlay->addStretch();

	QHBoxLayout* mlay = new QHBoxLayout;
	mlay->addLayout(rlay);
	mlay->addWidget(m_Make);

	QVBoxLayout* vlay = new QVBoxLayout;
	vlay->addWidget(lbinfo);
	vlay->addWidget(m_MakeWhat);
	vlay->addLayout(mlay);
	vlay->addStretch();

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addLayout(vlay);
	lay->addWidget(m_Components, 10);
	lay->addWidget(m_Modifiers, 10);
	lay->setMargin(0);
	setLayout(lay);
}

AuHandCraftPanel::~AuHandCraftPanel()
{
}

void AuHandCraftPanel::UpdateContents(DaPerson* person)
{
	// Don't do anything if the combo box has its list open, otherwise it's disruptive.
	if(m_MakeWhat->view() && m_MakeWhat->view()->isVisible())
		return;

	// Must not allow handcraft at a building under construction when the building is required.
	bool inBd = false;			// true if in a bd that is not under construction and not destroyed.
	bool inCity = false;		// true if in a city building that is not under construction and not destroyed.
	bool hasPower = false;
	AuKnownPatents patents;
	DaWorldLL* llworld = person->FindParentWorldInEnv();
	AuId bldCity;
	if(llworld)
	{
		if(!person->BdId().IsNull())
		{
			DaBd* bd = llworld->FindBd(person->BdId());
			if(bd && !bd->IsUnderConstruction() && !bd->IsDestroyed())
			{
				inBd = true;
				hasPower = bd->HasPower();
				bd->ListPatents(patents, bd->IndustryId());
			}
		}
	}

	// Add patents in the person's possession.
	person->ListPatents(patents);

	// Manufacturing processes available and current process.
	INDUSTRYID iid = person->InIndustry();
	bool iidreq = false;					// true if any processes are added that require the person's industry to be run
	{
		// List manufacturing processes available for handcrafting.
		m_MakeWhat->blockSignals(true);
		m_MakeWhat->clear();
		const DgData& gd = g_Au.LockDataForRead();
		for(DbManufacturingProcessArray::const_iterator mpi = gd.m_bManufacturingProcessArray.cbegin();
				mpi < gd.m_bManufacturingProcessArray.cend(); mpi++)
		{
			// Do not list patented processes unless the patent is available.
			if(!gd.m_bManufacturingComponentArray.HasNecessaryPatents((*mpi).m_id, patents))
				continue;
			// Skip processes that can never be handcrafted.
			if((*mpi).m_craft == 0)
				continue;

			// Skip processes that depend on the scene atom's current industry.
			bool atAppropriateIndustry = (inBd || inCity) && (*mpi).m_idi == iid;
			if((*mpi).m_craft == 2 && !atAppropriateIndustry)
				continue;

			// Add an item for this manufacturing process.
			// The icon is the commodity produced.
			// The user data is the manufacturing process id.
			COMMODITYID cid = (COMMODITYID)(*mpi).m_idc;
			//QString cname = g_Au.CommodityName(cid) + ", " + (*mpi).m_name;
			QString cname = (*mpi).m_name;
			if((*mpi).m_craft == 2)
			{
				cname += " *";
				iidreq = true;
			}

			bool inserted = false;
			for(int idx=0; idx<m_MakeWhat->count(); idx++)
				if(cname.localeAwareCompare(m_MakeWhat->itemText(idx)) < 0)
				{
					m_MakeWhat->insertItem(idx, QIcon(g_Au.CommodityBitmapName(cid)), cname, (*mpi).m_id);
					inserted = true;
					break;
				}
			if(!inserted)
				m_MakeWhat->addItem(QIcon(g_Au.CommodityBitmapName(cid)), cname, (*mpi).m_id);
		}
		g_Au.UnlockData();

		// Reselect my current manufacturing process, if it's still available.
		int curIdx = m_MakeWhat->findData(m_MPID);
		if(curIdx < 0)
		{
			// The current process is not in the list? This happens the first time through when m_MPID is still 0.
			if(m_MakeWhat->count() > 0)
			{
				// This is the normal path. Unusual circumstances would result in there being no processes in the combo box.
				curIdx = 0;
				m_MPID = m_MakeWhat->itemData(0).toInt();
			}
			else
				m_MPID = 0;
		}
		if(curIdx >= 0)
			m_MakeWhat->setCurrentIndex(curIdx);
		m_MakeWhat->blockSignals(false);
	}

	if(iidreq)
	{
		const DgData& gd = g_Au.LockDataForRead();
		QString iname = gd.IndustryName(iid);
		g_Au.UnlockData();
		m_Note->setText(tr("<html>"
					"<small>"
					"* %1"
					"</small>"
					"</html>")
				.arg(iname));
	}
	else
		m_Note->setText("");

	// Set up a manufacturing process as if all possible materials were fetched from the person's gear.
	DeManufacturingProcess mp;
	{
		// Init the manufacturing process.
		mp.SetHandCraftingProcessId(m_MPID);

		// Electricity may be available.
		if(hasPower)
			mp.SupplyAllPower(0);

		// Try to satisfy the needs of the process using the person's gear.
		const AuGear* inven = person->LockGearForRead();
		for(DeProcessComponentArray::iterator pci = mp.Components().begin(); pci != mp.Components().end(); pci++)
		{
			if(!(*pci).IsRequired())
				continue;	// Handcrafting processes only care about required modifiers.
			if((*pci).IsPower())
				continue;	// Electricity is not taken from the person's gear.

			// Determine how many are needed.
			int needToHave = (*pci).QtyConsumed();
			if(!needToHave)
				needToHave = 1;	// This required component doesn't consume the item so the consumed amount is 0; it is still needed.

			if(DbCommodity::IsPatent((*pci).CommodityId()))
			{
				// We already insure the patents were present or this process would not be listed.
				(*pci).SetAppliedMaterial(1, patents.PatentQuality((*pci).CommodityId()));
				continue;
			}

			// Determine how many are in the person's gear.
			int hasItems = 0;
			int hasQuality = 0;
			for(AuGear::const_iterator ii = inven->begin(); ii != inven->end() && hasItems < needToHave; ii++)
			{
				if((*ii).Item().CommodityId() == (*pci).CommodityId())
				{
					hasItems += (*ii).Item().Count();
					if(hasItems > needToHave)
						hasItems = needToHave;
					if(!hasQuality || hasQuality > (*ii).Item().Quality())	// Result is the lowest quality applied.
						hasQuality = (*ii).Item().Quality();
				}

				// Look inside of containers.
				if((*ii).Item().Content())
				{
					const AuItemContent* contents = (*ii).Item().Content();
					for(AuItemContent::const_iterator ci = contents->begin(); ci != contents->end() && hasItems < needToHave; ci++)
					{
						if((*ci).second->CommodityId() == (*pci).CommodityId())
						{
							hasItems += (*ci).second->Count();
							if(hasItems > needToHave)
								hasItems = needToHave;
							if(!hasQuality || hasQuality > (*ci).second->Quality())	// Result is the lowest quality applied.
								hasQuality = (*ci).second->Quality();
						}
					}
				}
			}

			// Set the component as if the materials had been applied.
			if(hasItems)
				(*pci).SetAppliedMaterial(hasItems, hasQuality);
		}
		person->UnlockGear();
	}

	// Components and Modifiers
	// We'll do these simultaneously so we only have to loop through the process components once.
	m_Components->clear();
	m_Modifiers->clear();
	while(m_Modifiers->rowCount())
		m_Modifiers->removeRow(0);
	if(m_MPID)
	{
		for(DeProcessComponentArray::const_iterator pci = mp.Components().cbegin(); pci != mp.Components().cend(); pci++)
		{
			QIcon icon((*pci).CommodityBitmapName());
			if((*pci).IsRequired() && (*pci).QtyConsumed() > 0)
			{
				// Construction "Materials" are those components that are
				// required AND that get consumed in the process. Some
				// required things might not be consumed but could be
				// required, such as a tool.
				QString text;
				if((*pci).QtyApplied() == 0)
				{
					// No components applied.
					text = tr("%1 needed").arg((*pci).QtyConsumed());
				}
				else if((*pci).QtyApplied() == (*pci).QtyConsumed())
				{
					// All components applied.
					if((*pci).Quality() > 0)
						text = tr("%1 Q%2")
							.arg((*pci).QtyApplied())
							.arg((*pci).Quality());
					else
						text = tr("%1").arg((*pci).QtyApplied());
				}
				else
				{
					// Some components applied.
					if((*pci).Quality() > 0)
						text = tr("%1 of %2 Q%3").arg((*pci).QtyApplied())
								.arg((*pci).QtyConsumed()).arg((*pci).Quality());
					else
						text = tr("%1 of %2").arg((*pci).QtyApplied())
								.arg((*pci).QtyConsumed());
						//text.sprintf("%d of %d", (*pci).QtyApplied(),
						//		(*pci).QtyConsumed());
				}
				QListWidgetItem* lwi = new QListWidgetItem(icon, text, m_Components);
				lwi->setToolTip(g_Au.CommodityName((*pci).CommodityId()));
			}
			else
			{
				// "Modifiers" are those components that are not required OR that do not get consumed by the process.
				if(!(*pci).IsRequired())
					continue;	// Handcrafting processes only care about required modifiers.

				QString text = tr("Required");
				QString modicon;
				if((*pci).EffectBitmapName())
					modicon = QString((*pci).EffectBitmapName());

				m_Modifiers->insertRow(0);

				int col=0;
				QTableWidgetItem* lwi = new QTableWidgetItem(icon, "");
				lwi->setToolTip(g_Au.CommodityName((*pci).CommodityId()));
				m_Modifiers->setItem(0, col++, lwi);

				lwi = new QTableWidgetItem(QIcon(modicon), "");
				lwi->setToolTip(tr("%1 %2").arg(g_Au.CommodityName((*pci).CommodityId())).arg(text));
				m_Modifiers->setItem(0, col++, lwi);

				if((*pci).QtyApplied())
				{
					// Component has been applied.
					QTableWidgetItem* lwi = new QTableWidgetItem(QIcon(":data/mpReady.png"), "");
					lwi->setToolTip(tr("%1 Ready").arg(g_Au.CommodityName((*pci).CommodityId())));
					m_Modifiers->setItem(0, col++, lwi);
				}
				else if((*pci).IsRequired())
				{
					// Missing required component is holding up the process.
					QTableWidgetItem* lwi = new QTableWidgetItem(QIcon(":data/mpNotReady.png"), "");
					lwi->setToolTip(tr("%1 Needed").arg(g_Au.CommodityName((*pci).CommodityId())));
					m_Modifiers->setItem(0, col++, lwi);
				}
			}
		}
		m_Modifiers->resizeColumnsToContents();
		//m_Modifiers->resizeRowsToContents();
	}

	// Status
	if(m_MPID)
	{
		// Set the name of the commodity being produced.
		QString makeWhat = mp.CommodityName();

		QString status;
		if(mp.CanRun())
		{
			status = QString("Ready");
			m_StatusIcon->setPixmap(QPixmap(":/data/mpReady.png"));
			m_Make->setEnabled(true);
		}
		else
		{
			status = QString("Not Ready");
			m_StatusIcon->setPixmap(QPixmap(":/data/mpNotReady.png"));
			m_Make->setEnabled(false);
		}

		if(!makeWhat.isEmpty())
			status = makeWhat + '\n' + status;
		m_Status->setText(status);
	}
	else
	{
		QString status = QString("Select a Handcrafting Process\nNot Ready");
		m_StatusIcon->setPixmap(QPixmap(":/data/mpNotReady.png"));
		m_Make->setEnabled(false);
		m_Status->setText(status);
	}
}

void AuHandCraftPanel::GoPushed()
{
	if(!m_MPID)
		return;

	if(m_MPID == MPID_Flag)
	{
		AuFrame* pFrame = auApp->Frame();
		if(!pFrame)
			return;
		AuView* pView = pFrame->View();
		if(!pView)
			return;
		if(!pView->SceneAtomEmpire())
		{
			auApp->CreateEmpireUI();
			return;
		}
	}
	emit Go(m_MPID);
}

void AuHandCraftPanel::ProcessSelectionChanged(int index)
{
	if(index < 0)
		return;
	int mpid = m_MakeWhat->itemData(index).toInt();
	if(mpid != m_MPID)
		m_MPID = mpid;	// Helps to suppress double hit on this slot.
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

AuLaborHandCraft::AuLaborHandCraft(QWidget* parent) : QWidget(parent)
{
	m_timer = 0;

	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Make the handcrafting controls.
	m_Panel = new AuHandCraftPanel(this);
	connect(m_Panel, SIGNAL(Go(int)), auApp, SLOT(HandCraftGo(int)));

	// Build the layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_Panel, row++,0);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuLaborHandCraft::~AuLaborHandCraft()
{
}

void AuLaborHandCraft::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(1000);
}

void AuLaborHandCraft::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
		m_Panel->UpdateContents(person);
	pView->UnlockSceneAtom();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuHandCraftDialog::AuHandCraftDialog(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Handcraft"));
	setAuDlgIcon(QIcon(":data/it_HandCraft.png"));
	SetWikiTopic("Handcraft");

	m_HandCraft = new AuLaborHandCraft(this);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_HandCraft);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuHandCraftDialog::~AuHandCraftDialog()
{
}

void AuHandCraftDialog::setVisible(bool visible)
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

void AuHandCraftDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/HandCraft/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/HandCraft/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/HandCraft/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/HandCraft/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuHandCraftDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/HandCraft/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/HandCraft/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/HandCraft/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/HandCraft/SizeY",sz.height());
}

QSize AuHandCraftDialog::sizeHint() const
{
	return QSize(700,400);
}
