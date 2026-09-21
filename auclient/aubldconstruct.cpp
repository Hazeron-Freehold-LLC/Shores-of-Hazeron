//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldconstruct.cpp,v 1.9 2022/02/26 00:06:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldconstruct.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include "deconstructionprocess.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>

AuBldConstruct::AuBldConstruct(QWidget* parent) : QWidget(parent)
{
	m_BdIndustry = IID_None;
	m_CapitolOpIndustry = IID_None;
	m_CitizenOk = false;
	m_DoWork = false;
	m_timer = 0;

	// Make the controls.
	m_Construct = new QLabel(this);
	m_Construct->setToolTip(tr("Name of the development that is under construction."));

	m_Labor = new QLabel(this);
	m_Labor->setToolTip(tr("Amount of labor needed by the construction."));
	m_Labor->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Materials = new QListWidget(this);
	m_Materials->setMinimumWidth(100);
	m_Materials->setToolTip(tr("Commodities consumed by the construction."));

	m_Modifiers = new QTableWidget(0,3,this);	// 0 rows, 2 columns
	m_Modifiers->setMinimumWidth(74);
	m_Modifiers->setMaximumWidth(74);
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
	m_Modifiers->setToolTip(tr("<html>Commodities that affect the construction without "
			"directly being consumed in the process. 'Required' "
			"commodities must be available, either in your ready inventory "
			"or the city's inventory, before the Work button can be "
			"pressed.</html>"));
	m_Modifiers->resizeColumnsToContents(); // This eliminates a horizontal scrollbar that was visible until the first update.


	m_Employ = new QCheckBox(tr("Employ Citizen"), this);
	m_Employ->setToolTip(tr("<html>Enables citizens of the city to Fetch materials and to "
			"Work this job. You must have governing authority over the city "
			"to change this setting.</html>"));
	connect(m_Employ, SIGNAL(stateChanged(int)), SLOT(CitizenEnableChanged(int)));

	m_Work = new QPushButton(tr("Work Construction Site"), this);
	m_Work->setToolTip(tr("<html>Works the construction site continuously until the job is done.<br>"
				"%1 Fetches commodities lying about in the area.<br>"
				"%1 Fetches commodities from containers lying in the area, such as crates and backpacks.<br>"
				"%1 Fetches commodities from city's inventory<br>"
				"%1 Work pauses if 'Required' commodities, such as tools, "
				"are not available in your ready inventory or the city's inventory.<br>"
				"%1 Work stops if you leave the construction site.<br>"
				"%1 Work stops if a wild animal enters the construction site.<br>"
				"%1 Work stops if an enemy unit enters the construction site.</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_Work, SIGNAL(clicked(bool)), SLOT(GoPushed()));

	QHBoxLayout* hlay = new QHBoxLayout;
	hlay->addWidget(m_Employ);
	hlay->addStretch(1);
	hlay->addWidget(m_Labor);

	// Build the layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_Construct, row,0, 1,2);	// Construct
	row++;
	lay->addWidget(m_Materials, row,0);			// Materials
	lay->addWidget(m_Modifiers, row,1);			// Modifiers
	row++;
	lay->addLayout(hlay, row,0, 1,2);
	row++;
	lay->addWidget(m_Work, row,0, 1,2);			// Work
	lay->setColumnStretch(0,1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuBldConstruct::~AuBldConstruct()
{
}

void AuBldConstruct::showEvent(QShowEvent* event)
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

	// Show all my widgets.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();
}

void AuBldConstruct::CitizenEnableChanged(int state)
{
	bool citizenOk = (state == Qt::Checked) ? true : false;
	if(citizenOk != m_CitizenOk)
	{
		AuFrame* pFrame = auApp->Frame();
		if(!pFrame)
			return;
		AuView* pView = pFrame->View();
		if(!pView)
			return;

		auApp->BdConstructionEnabled(citizenOk, m_CapitolOpId);
	}
}

void AuBldConstruct::FetchPushed()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;
	auApp->BdConstructionFetch(m_CapitolOpId);
}

void AuBldConstruct::GoPushed()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	// Toggle the auto-work state.
	m_DoWork = !m_DoWork;
	m_Work->setDown(m_DoWork);
	if(m_DoWork)
	{
		m_BdIndustry = IID_None;								// Init to get current building info.
		m_Work->setText(tr("Stop Working Construction Site"));
	}
	else
		m_Work->setText(tr("Work Construction Site"));
}

void AuBldConstruct::UpdateContents()
{
	if(!isVisible())
	{
		// Hide my widgets now or they force the parent AuBld dialog to be too big even though this page isn't visible.
		QGridLayout* lay = (QGridLayout*)layout();
		for(int iw=0; iw<lay->count(); iw++)
			if(lay->itemAt(iw)->widget())
				lay->itemAt(iw)->widget()->hide();

		// Turn off auto-work.
		m_DoWork = false;
		m_BdIndustry = IID_None;
		m_Labor->setText("");
		m_Work->setDown(false);
		m_Work->setText(tr("Work Construction Site"));

		// Kill the timer. It will be restarted during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	if(!pView)
		return;

	bool enableWork = m_Work->isEnabled();
	bool fetchNeeded = false;
	quint32 laborNeeded = 0;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() != IID_None && !person->IsBodyDyingOrDead())
	{
		if(!m_CapitolOpId.IsNull())
		{
			// Person is at a capitol building.
			if(m_BdIndustry == IID_None)
			{
				// First time through here; get current building info.
				m_BdIndustry = m_CapitolOpIndustry;
				m_BdId = m_CapitolOpId;
				m_BdLL = m_CapitolOpLL;
			}

			if(m_CapitolOpIndustry != m_BdIndustry ||
					m_CapitolOpId != m_BdId ||
					m_CapitolOpLL != m_BdLL)
			{
				// Person has selected a different construction site or started building something else here.
				m_Work->setDown(false);
				m_Work->setText(tr("Work Construction Site"));
				m_DoWork = false;
			}
		}
		else
		{
			// Person is not at a capitol building.
			if(m_BdIndustry == IID_None)
			{
				// First time through here; get current building info.
				m_BdIndustry = person->InIndustry();
				m_BdId = person->BdId();
				m_BdLL = person->InIndustryLL();
			}

			if(person->InIndustry() != m_BdIndustry ||
					person->BdId() != m_BdId ||
					person->InIndustryLL() != m_BdLL)
			{
				// Person has moved to a different construction site or started building something else here.
				m_BdIndustry = person->InIndustry();
				m_BdId = person->BdId();
				m_BdLL = person->InIndustryLL();
				m_Work->setDown(false);
				m_Work->setText(tr("Work Construction Site"));
				m_DoWork = false;
			}
		}

		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(person->FindParentPlanetInEnv());
		if(llworld)
		{
			const DaBd* bdOp = person->BdOp(m_CapitolOpId);
			if(bdOp)
			{
				AuGlobeLLF devLL;
				llworld->LLOfLoc(devLL, bdOp->Location());
				const DeConstructionProcess* cp = bdOp->LockConstructionProcessForRead();
				if(cp)
				{
					// Remember construction process settings.
					enableWork = true;
					m_CitizenOk = !cp->IsHalted();

					// Update controls using construction process info.
					if(m_Construct)
						m_Construct->setText(bdOp->Name());

					// Worker enabled checkbox.
					if(m_Employ)
					{
						m_Employ->blockSignals(true);
						m_Employ->setCheckState(m_CitizenOk ?  Qt::Checked : Qt::Unchecked);
						m_Employ->blockSignals(false);
					}

					// Materials and Modifiers
					// We'll do these simultaneously so we only have to loop
					// through the process components once.
					if(m_Materials && m_Modifiers)
					{
						m_Materials->clear();
						m_Modifiers->clearContents();
						while(m_Modifiers->rowCount())
							m_Modifiers->removeRow(0);
						for(DeProcessComponentArray::const_iterator pci = cp->Components().constBegin();
								pci != cp->Components().constEnd(); pci++)
						{
							QIcon icon((*pci).CommodityBitmapName());
							if((*pci).IsRequired() && (*pci).QtyConsumed() > 0)
							{
								// Construction "Materials" are those
								// components that are required AND that get
								// consumed in the process. Some required things
								// might not be consumed but could be required,
								// such as a tool.
								QString text;
								if((*pci).QtyApplied() == 0)
								{
									// No components applied.
									text = QString(tr("%1 needed")).arg((*pci).QtyConsumed());
								}
								else if((*pci).QtyApplied() == (*pci).QtyConsumed())
								{
									// All components applied.
									if((*pci).Quality() > 0)
										text = QString("%1 Q%2")
											.arg((*pci).QtyApplied())
											.arg((*pci).Quality());
									else
										text = QString("%1").arg((*pci).QtyApplied());
								}
								else
								{
									// Some components applied.
									if((*pci).Quality() > 0)
										text = QString("%1 of %2 Q%3")
											.arg((*pci).QtyApplied())
											.arg((*pci).QtyConsumed())
											.arg((*pci).Quality());
									else
										text = QString("%1 of %2")
											.arg((*pci).QtyApplied())
											.arg((*pci).QtyConsumed());
								}
								QListWidgetItem* lwi = new QListWidgetItem(icon, text, m_Materials);
								lwi->setToolTip(g_Au.CommodityName((*pci).CommodityId()));
							}
							else
							{
								// Construction "Modifiers" are those
								// components that are not required OR that do
								// not get consumed by the process.
								QString text;
								QString modicon;
								if((*pci).EffectBitmapName())
									modicon = QString((*pci).EffectBitmapName());
								switch((*pci).Effect())
								{
								case CE_None:
									text = tr("Does Nothing");
									break;
								case CE_DecreasesTime:
									text = tr("Decreases Time");
									break;
								case CE_DecreasesMaterials:
									text = tr("Decreases Materials");
									break;
								case CE_Required:
									text = tr("Required");
									break;
								case CE_RequiredQ:
									text = tr("Required and Limits Quality"
											"<br>"
											"<small>Output quality cannot exceed the quality of this patent.</small>");
									break;
								case CE_IncreasesQuantity:
									text = tr("Increases Output");
									break;
								}

								m_Modifiers->insertRow(0);

								int col=0;
								QTableWidgetItem* lwi = new QTableWidgetItem(icon, "");
								lwi->setSizeHint(QSize(1,16));
								lwi->setToolTip(g_Au.CommodityName((*pci).CommodityId()));
								m_Modifiers->setItem(0, col++, lwi);

								lwi = new QTableWidgetItem(QIcon(modicon), "");
								lwi->setSizeHint(QSize(1,16));
								lwi->setToolTip(tr("%1 %2").arg(g_Au.CommodityName((*pci).CommodityId())).arg(text));
								m_Modifiers->setItem(0, col++, lwi);

								if((*pci).QtyApplied())
								{
									// Component has been applied.
									QTableWidgetItem* lwi = new QTableWidgetItem(QIcon(":data/mpReady.png"), "");
									lwi->setSizeHint(QSize(1,16));
									lwi->setToolTip(tr("%1 Ready").arg(g_Au.CommodityName((*pci).CommodityId())));
									m_Modifiers->setItem(0, col++, lwi);
								}
								else if((*pci).IsRequired())
								{
									// Missing required component is holding up the process.
									QTableWidgetItem* lwi = new QTableWidgetItem(QIcon(":data/mpNotReady.png"), "");
									lwi->setSizeHint(QSize(1,16));
									lwi->setToolTip(tr("%1 Needed").arg(g_Au.CommodityName((*pci).CommodityId())));
									m_Modifiers->setItem(0, col++, lwi);
								}
							}
						}
						m_Modifiers->resizeColumnsToContents();
					}

					// Labor
					laborNeeded = cp->LaborNeeded();
					if(laborNeeded)
						m_Labor->setText(tr("%L1 Labor Needed").arg(laborNeeded));
					else
						m_Labor->setText(tr("No Labor Needed"));

					fetchNeeded = cp->WantsAnyComponent();
				}
				else
				{
					// Building is no longer under construction.
					m_DoWork = false;
					m_Labor->setText(tr("Construction Complete"));
					m_Materials->clear();
					m_Modifiers->clearContents();
					m_Work->setDown(false);
					m_Work->setText(tr("Work Construction Site"));
					enableWork = false;
				}
				bdOp->UnlockConstructionProcess();
			}
			else
			{
				// Am I at a geo road?
				DeConstructionProcess cp;
				if(person->IsAtGeo() && llworld->GeoUnderConstruction(person->InIndustryLL(), cp))
				{
					// Remember construction process settings.
					enableWork = true;
					m_CitizenOk = !cp.IsHalted();

					// Update controls using construction process info.
					if(m_Construct)
					{
						QString text;
						switch(cp.ConstructionProcessId())
						{
						case CPID_DirtRoad:
							text = tr("Dirt Road");
							break;
						case CPID_AsphaltRoad:
							text = tr("Asphalt Road");
							break;
						case CPID_ConcreteRoad:
							text = tr("Concrete Road");
							break;
						case CPID_RoadGrade:
							text = tr("Grade Area");
							break;
						case CPID_Irrigation:
							text = tr("Irrigated Area");
							break;
						case CPID_Defoliant:
							text = tr("Defoliated Area");
							break;
						case CPID_Clearing:
							text = tr("Clearing");
							break;
						default:
							text = tr("Road");
							break;
						}
						m_Construct->setText(text);
					}

					// Worker enabled checkbox.
					if(m_Employ)
					{
						m_Employ->blockSignals(true);
						m_Employ->setCheckState(m_CitizenOk ?  Qt::Checked : Qt::Unchecked);
						m_Employ->blockSignals(false);
					}

					// Materials and Modifiers
					// We'll do these simultaneously so we only have to loop
					// through the process components once.
					if(m_Materials && m_Modifiers)
					{
						m_Materials->clear();
						m_Modifiers->clearContents();
						while(m_Modifiers->rowCount())
							m_Modifiers->removeRow(0);
						for(DeProcessComponentArray::const_iterator pci = cp.Components().constBegin();
								pci != cp.Components().constEnd(); pci++)
						{
							QIcon icon((*pci).CommodityBitmapName());
							if((*pci).IsRequired() && (*pci).QtyConsumed() > 0)
							{
								// Construction "Materials" are those
								// components that are required AND that get
								// consumed in the process. Some required things
								// might not be consumed but could be required,
								// such as a tool.
								QString text;
								if((*pci).QtyApplied() == 0)
								{
									// No components applied.
									text = QString(tr("%1 needed")).arg((*pci).QtyConsumed());
								}
								else if((*pci).QtyApplied() == (*pci).QtyConsumed())
								{
									// All components applied.
									if((*pci).Quality() > 0)
										text = QString("%1 Q%2")
											.arg((*pci).QtyApplied())
											.arg((*pci).Quality());
									else
										text = QString("%1").arg((*pci).QtyApplied());
								}
								else
								{
									// Some components applied.
									if((*pci).Quality() > 0)
										text = QString("%1 of %2 Q%3")
											.arg((*pci).QtyApplied())
											.arg((*pci).QtyConsumed())
											.arg((*pci).Quality());
									else
										text = QString("%1 of %2")
											.arg((*pci).QtyApplied())
											.arg((*pci).QtyConsumed());
								}
								QListWidgetItem* lwi = new QListWidgetItem(icon, text, m_Materials);
								lwi->setToolTip(g_Au.CommodityName((*pci).CommodityId()));
							}
							else
							{
								// Construction "Modifiers" are those
								// components that are not required OR that do
								// not get consumed by the process.
								QString text;
								QString modicon;
								if((*pci).EffectBitmapName())
									modicon = QString((*pci).EffectBitmapName());
								switch((*pci).Effect())
								{
								case CE_None:
									text = tr("Does Nothing");
									break;
								case CE_DecreasesTime:
									text = tr("Decreases Time");
									break;
								case CE_DecreasesMaterials:
									text = tr("Decreases Materials");
									break;
								case CE_Required:
									text = tr("Required");
									break;
								case CE_RequiredQ:
									text = tr("Required and Limits Quality"
											"<br>"
											"<small>Output quality cannot exceed the quality of this patent.</small>");
									break;
								case CE_IncreasesQuantity:
									text = tr("Increases Output");
									break;
								}

								m_Modifiers->insertRow(0);

								int col=0;
								QTableWidgetItem* lwi = new QTableWidgetItem(icon, "");
								lwi->setSizeHint(QSize(1,16));
								lwi->setToolTip(g_Au.CommodityName((*pci).CommodityId()));
								m_Modifiers->setItem(0, col++, lwi);

								lwi = new QTableWidgetItem(QIcon(modicon), "");
								lwi->setSizeHint(QSize(1,16));
								lwi->setToolTip(tr("%1 %2").arg(g_Au.CommodityName((*pci).CommodityId())).arg(text));
								m_Modifiers->setItem(0, col++, lwi);

								if((*pci).QtyApplied())
								{
									// Component has been applied.
									QTableWidgetItem* lwi = new QTableWidgetItem(QIcon(":data/mpReady.png"), "");
									lwi->setSizeHint(QSize(1,16));
									lwi->setToolTip(tr("%1 Ready").arg(g_Au.CommodityName((*pci).CommodityId())));
									m_Modifiers->setItem(0, col++, lwi);
								}
								else if((*pci).IsRequired())
								{
									// Missing required component is holding up the process.
									QTableWidgetItem* lwi = new QTableWidgetItem(QIcon(":data/mpNotReady.png"), "");
									lwi->setSizeHint(QSize(1,16));
									lwi->setToolTip(tr("%1 Needed").arg(g_Au.CommodityName((*pci).CommodityId())));
									m_Modifiers->setItem(0, col++, lwi);
								}
							}
						}
						m_Modifiers->resizeColumnsToContents();
					}

					// Labor
					laborNeeded = cp.LaborNeeded();
					if(laborNeeded)
						m_Labor->setText(tr("%L1 Labor Needed").arg(laborNeeded));
					else
						m_Labor->setText(tr("No Labor Needed"));

					fetchNeeded = cp.WantsAnyComponent();
				}
				else
				{
					// My current building was not found.
					m_BdIndustry = IID_None;
					m_DoWork = false;
					m_Materials->clear();
					m_Modifiers->clearContents();
					m_Work->setDown(false);
					m_Work->setText(tr("Work Construction Site"));
					enableWork = false;
				}
			}
		}
		else
		{
			//qDebug() << "no" << person->InIndustry() << "construction process at" << person->InIndustryLL();
			enableWork = false;
		}
	}
	else
	{
		m_BdIndustry = IID_None;
		m_DoWork = false;
		m_Materials->clear();
		m_Modifiers->clearContents();
		m_Work->setDown(false);
		m_Work->setText(tr("Work Construction Site"));
		enableWork = false;
	}
	pView->UnlockSceneAtom();

	// Set enabled state of the work button.
	if(enableWork != m_Work->isEnabled())
		m_Work->setEnabled(enableWork);

	if(m_DoWork)
	{
		if(fetchNeeded)
			auApp->BdConstructionFetch(m_CapitolOpId);
		//if(laborNeeded)
			auApp->BdConstructionGo(m_CapitolOpId);
	}
}
