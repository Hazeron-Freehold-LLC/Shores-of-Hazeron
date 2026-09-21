//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aujobsquerydialog.cpp,v 1.7 2022/04/08 16:41:10 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aujobsquerydialog.h"
//
#include "auapp.h"
#include <auglobal.h>
#include "AuNewJobView.h"
#include "autoolbutton.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSpinBox>
#include <QTimer>

#define c_MsgSpeed	100		// ms between dequeueing messages into the list

AuJobsQueryDialog::AuJobsQueryDialog(const AuId& personId, const AuPoint& sectorLoc, GALAXY galaxy, QWidget* parent) : AuDialog(parent)
{
	m_PersonId = personId;
	m_SectorLoc = sectorLoc;
	m_Galaxy = galaxy;

	m_MaxSectorsAway = g_Au.Settings().readNumEntry("Widget/Jobs/MaxSectorsAway", 2);
	if(m_MaxSectorsAway > MaxJobSectorRange())
		m_MaxSectorsAway = MaxJobSectorRange();

	setObjectName("Jobs");
	setAuDlgIcon(QIcon(":/data/Jobs.png"));
	setAuDlgTitle("<b>Job Offers</b>");

	m_JobsModel.insertColumns(0,4);
	m_JobsModel.setHeaderData(0, Qt::Horizontal, "");
	m_JobsModel.setHeaderData(1, Qt::Horizontal, "Job");
	m_JobsModel.setHeaderData(2, Qt::Horizontal, "Origin");
	m_JobsModel.setHeaderData(3, Qt::Horizontal, "Distance");
	m_JobsModel.setSortRole(Qt::UserRole+1);

	// Create a layout of the filter buttons.
	QHBoxLayout* filterLay = new QHBoxLayout;
	{
		// Create buttons for type filters.
		QSize filterIconSize(10,10);
		QMap<QString, AuJob::TYPE> jobTypes;					// Map of type names to sort them alphabetically.
		for(int itype=0; itype<AuJob::JobTypes; itype++)
		{
			AuJob::TYPE jobType = (AuJob::TYPE)itype;
			QString jobName = AuJob::TypeName(jobType);
			jobTypes.insert(jobName, jobType);

			QPixmap pic = QPixmap(AuJob::IconName(jobType))
				.scaled(filterIconSize.width(), filterIconSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			QIcon buttonIcon(pic);

			m_TypeFilter[itype] = g_Au.Settings().readNumEntry(QString("Widget/Jobs/%1/%2")
					.arg(m_PersonId.StringId())
					.arg(jobName), 0);

			m_TypeFilterButton[itype] = new AuToolButtonValue(buttonIcon, jobType);
			m_TypeFilterButton[itype]->setCheckable(true);
			m_TypeFilterButton[itype]->setIconSize(filterIconSize);
			m_TypeFilterButton[itype]->setToolTip(jobName);
			if(m_TypeFilter[itype] == 0)
				m_TypeFilterButton[itype]->setVisible(false);	// hide the filter buttons for types the user has never seen
			else
				m_TypeFilterButton[itype]->setChecked(m_TypeFilter[itype]-1);
			connect(m_TypeFilterButton[itype], SIGNAL(ClickedValue(int)), SLOT(TypeFilterClicked(int)));
		}

		// Add the buttons to the layout in alphabetical order.
		for(QMap<QString, AuJob::TYPE>::const_iterator jti = jobTypes.constBegin(); jti != jobTypes.constEnd(); jti++)
			filterLay->addWidget(m_TypeFilterButton[*jti]);
		filterLay->addStretch(1);
	}

	// Create a layout of the action buttons.
	QHBoxLayout* buttonLay = new QHBoxLayout;
	{
		m_JobCountLabel = new QLabel("Job Offers");

		QLabel* rangeLabel = new QLabel("Sector Range");
		rangeLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		QSpinBox* rangeBox = new QSpinBox;
		rangeBox->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		rangeBox->setMinimum(0);
		rangeBox->setMaximum(MaxJobSectorRange());				// Maximum range that jobs are sent by the server.
		rangeBox->setValue(m_MaxSectorsAway);
		connect(rangeBox, SIGNAL(valueChanged(int)), SLOT(RangeChanged(int)));

		AuIconButton* pbquery = new AuIconButton(QIcon(":/data/MsgSolicitSales.png"));
		pbquery->setToolTip(tr("<html>Query for a fresh list of job offers.</html>"));
		connect(pbquery, SIGNAL(clicked()), SLOT(QueryJobs()));

		AuIconButton* pbopen = new AuIconButton(QIcon(":/data/MsgAccept.png"));
		pbopen->setToolTip(tr("<html>Accept the job offer that is selected.</html>"));
		connect(pbopen, SIGNAL(clicked()), SLOT(AcceptJob()));

		buttonLay->addWidget(m_JobCountLabel);
		buttonLay->addStretch(1);
		buttonLay->addWidget(rangeLabel);
		buttonLay->addWidget(rangeBox);
		buttonLay->addStretch(1);
		buttonLay->addWidget(pbquery);
		buttonLay->addWidget(pbopen);
	}

	m_JobsView = new AuNewJobView;
	m_JobsView->setAlternatingRowColors(true);
	m_JobsView->setAllColumnsShowFocus(true);
	m_JobsView->setModel(&m_JobsModel);
	m_JobsView->setRootIsDecorated(false);
	m_JobsView->setSortingEnabled(true);
	m_JobsView->sortByColumn(3, Qt::AscendingOrder);			// Sort by range because this is the order the server sends them.
	m_JobsView->setToolTip(tr("<html>Job offers are listed that are ready to be accepted. "
				"Some job offers may only be available for a few moments.<br>"
				"Double-click a job offer to accept it.</html>"));
	m_JobsView->setColumnWidth(0,22);
	connect(m_JobsView, SIGNAL(ItemSelected(const QModelIndex&)), SLOT(NewJobSelected(const QModelIndex&)));
	connect(m_JobsView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(NewJobAccepted(const QModelIndex&)));
	//m_JobsView->header()->setSortIndicator(0, Qt::AscendingOrder);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addLayout(filterLay, row,0);
	row++;
	lay->addWidget(m_JobsView, row,0);
	row++;
	lay->addLayout(buttonLay, row,0);
	row++;
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	QueryJobs();

	m_Timer = new QTimer(this);
	connect(m_Timer, SIGNAL(timeout()), SLOT(DequeueJobOffer()));
}

AuJobsQueryDialog::~AuJobsQueryDialog()
{
}

QSize AuJobsQueryDialog::sizeHint() const
{
	return QSize(600,100);
}

void AuJobsQueryDialog::QueryJobs()
{
	m_JobsModel.removeRows(0, m_JobsModel.rowCount());
	m_JobOffers.clear();

	auApp->QueryJobs();

	m_JobCountLabel->setText(tr("%1 Job Offers").arg(m_JobsModel.rowCount()));
}

void AuJobsQueryDialog::NewJobAccepted(const QModelIndex& sel)
{
	if(sel.isValid())
	{
		QModelIndex col0 = sel.sibling(sel.row(), 0);

		// Get a copy of the job offer from the selected item.
		QByteArray bajob = col0.data(Qt::UserRole).toByteArray();
		QDataStream in(bajob);
		in.setVersion(QDataStream::Qt_4_1);
		in >> m_JobOffer;

		AcceptJob();
	}
	else
	{
		m_JobOffer = AuJobOffer();
	}
}

void AuJobsQueryDialog::NewJobSelected(const QModelIndex& sel)
{
	if(sel.isValid())
	{
		QModelIndex col0 = sel.sibling(sel.row(), 0);

		// Get a copy of the job offer from the selected item.
		QByteArray bajob = col0.data(Qt::UserRole).toByteArray();
		QDataStream in(bajob);
		in.setVersion(QDataStream::Qt_4_1);
		in >> m_JobOffer;
	}
	else
	{
		m_JobOffer = AuJobOffer();
	}
}

void AuJobsQueryDialog::JobAvailable(const AuJobOffer& jobOffer)
{
	m_JobsQueue.enqueue(jobOffer);								// queue the message so the timer can add it to the table
	if(!m_Timer->isActive() && isVisible())
		m_Timer->start(c_MsgSpeed);								// start the timer if this widget is visible
}

void AuJobsQueryDialog::DequeueJobOffer()
{
	if(!isVisible())
		m_Timer->stop();
	else
	{
		if(!m_JobsQueue.isEmpty())
		{
			m_JobOffers.append(m_JobsQueue.dequeue());			// dequeue the next message and add it to my list of jobs

			// The job is only added to the table if it is in the current filter.
			if(TestFilter(m_JobOffers.last()))
				AddJobOffer(m_JobOffers.last());				// dequeue the next message and add it to the table
		}
		if(m_JobsQueue.isEmpty())
			m_Timer->stop();									// stop the timer when the queue is empty
	}
}

// Returns true if it's ok to list the job.
bool AuJobsQueryDialog::TestFilter(const AuJobOffer& jobOffer)
{
	// Check type filter.
	if(m_TypeFilter[jobOffer.Job()->Type()] == 1)
		return false;											// The job type filter is turned off.

	// Check range.
	if(m_Galaxy != jobOffer.FromGalaxy())
		return false;											// The job offer is in a galaxy far far away.
	int range = (int)ceil(m_SectorLoc.Distance(jobOffer.FromSectorLoc()) / 10);	// result is in sectors
	if(range > m_MaxSectorsAway)
		return false;											// The job offer is too far away.

	// It's ok to list the job.
	return true;
}

void AuJobsQueryDialog::AddJobOffer(const AuJobOffer& jobOffer)
{
	AuJob::TYPE jobType = jobOffer.Job()->Type();
	QString jobName = AuJob::TypeName(jobType);
	Q_ASSERT(jobType != AuJob::JobTypes);

	// Pack the job offer into a byte array so it can be stored on the job item.
	QByteArray bajob;
	QDataStream out(&bajob, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << jobOffer;

	m_JobsModel.insertRow(0);

	QModelIndex itemIdx = m_JobsModel.index(0,0);
	m_JobsModel.setData(itemIdx, bajob, Qt::UserRole);			// Job offer stored on column 0.
	m_JobsModel.setData(itemIdx, jobName, Qt::UserRole+1);		// Sorting role
	m_JobsModel.item(0,0)->setIcon(jobOffer.Icon());
	m_JobsModel.item(0,0)->setToolTip(jobName);

	itemIdx = m_JobsModel.index(0,1);
	m_JobsModel.setData(itemIdx, jobOffer.Title());
	m_JobsModel.setData(itemIdx, jobOffer.Title(), Qt::UserRole+1);	// Sorting role
	m_JobsModel.item(0,1)->setToolTip(jobOffer.HtmlDesc());

	itemIdx = m_JobsModel.index(0,2);
	m_JobsModel.setData(itemIdx, jobOffer.FromName());
	m_JobsModel.setData(itemIdx, jobOffer.FromName(), Qt::UserRole+1);	// Sorting role
	m_JobsModel.item(0,2)->setToolTip(jobOffer.HtmlDesc());

	itemIdx = m_JobsModel.index(0,3);
	if(m_SectorLoc.FuzzyEquals(jobOffer.FromSectorLoc()))
	{
		m_JobsModel.setData(itemIdx, tr("Current Sector"));
		m_JobsModel.setData(itemIdx, 0, Qt::UserRole+1);	// Sorting role
	}
	else
	{
		int range = (int)ceil(m_SectorLoc.Distance(jobOffer.FromSectorLoc()) / 10);				// result is in sectors
		if(range > 1)
			m_JobsModel.setData(itemIdx, tr("%L1 Sectors Away").arg(range));
		else
			m_JobsModel.setData(itemIdx, tr("1 Sector Away"));
		m_JobsModel.setData(itemIdx, range, Qt::UserRole+1);	// Sorting role
	}
	m_JobsModel.item(0,3)->setToolTip(jobOffer.HtmlDesc());

	//m_JobsView->resizeColumnToContents(0);
	m_JobsView->resizeColumnToContents(1);
	m_JobsView->resizeColumnToContents(2);
	m_JobsView->sortByColumn(m_JobsView->header()->sortIndicatorSection(), Qt::AscendingOrder);

	m_JobCountLabel->setText(tr("%1 Job Offers").arg(m_JobsModel.rowCount()));

	if(m_TypeFilter[jobType] == 0)
	{
		// show the filter buttons when the users sees a type for the first time
		m_TypeFilter[jobType] = 2;	// on
		g_Au.Settings().writeEntry(QString("Widget/Jobs/%1/%2").arg(m_PersonId.StringId()).arg(jobName), m_TypeFilter[jobType]);

		m_TypeFilterButton[jobType]->setChecked(true);
		m_TypeFilterButton[jobType]->setVisible(true);
	}
}

void AuJobsQueryDialog::AcceptJob()
{
	if(m_JobOffer.Job())
	{
		auApp->AcceptJob(m_JobOffer);
		accept();
	}
}

void AuJobsQueryDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/Jobs/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/Jobs/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/Jobs/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/Jobs/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuJobsQueryDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/Jobs/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/Jobs/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/Jobs/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/Jobs/SizeY",sz.height());
}

void AuJobsQueryDialog::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();

		// Start the timer to dequeue job offers.
		if(!m_JobsQueue.isEmpty())
			m_Timer->start(c_MsgSpeed);
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);

		// Kill the timer. It will be restarted during the next setVisible.
		if(m_Timer->isActive())
			m_Timer->stop();
	}
}

void AuJobsQueryDialog::TypeFilterClicked(int type)
{
	AuJob::TYPE jobType = (AuJob::TYPE)type;
	QString jobName = AuJob::TypeName(jobType);
	//qDebug() << "TypeFilterClicked" << type << jobName << m_TypeFilter[type];

	if(m_TypeFilter[type] == 2)
		m_TypeFilter[type] = 1;	// off
	else
		m_TypeFilter[type] = 2;	// on
	m_TypeFilterButton[type]->setChecked(m_TypeFilter[type]-1);
	g_Au.Settings().writeEntry(QString("Widget/Jobs/%1/%2").arg(m_PersonId.StringId()).arg(jobName), m_TypeFilter[type]);

	// Update the visible list based on the new filter setting.
	if(m_TypeFilter[type]-1)
	{
		// Add entries of the filter type because they have just become visible.
		for(QList<AuJobOffer>::const_iterator joi = m_JobOffers.constBegin(); joi != m_JobOffers.constEnd(); joi++)
			if((*joi).Job()->Type() == jobType && TestFilter(*joi))
				AddJobOffer(*joi);
	}
	else
	{
		// Remove entries of the filter type because they have just become invisible.
		int row=0;
		while(row < m_JobsModel.rowCount())
		{
			QModelIndex itemIdx = m_JobsModel.index(row, 0);					// column 0 holds the type name
			if(m_JobsModel.data(itemIdx, Qt::UserRole+1) == jobName)			// Job type name stored on column 0.
				m_JobsModel.removeRows(row, 1);
			else
				row++;
		}

		m_JobCountLabel->setText(tr("%1 Job Offers").arg(m_JobsModel.rowCount()));
	}
}

void AuJobsQueryDialog::RangeChanged(int sectorRange)
{
	if(sectorRange < 0)
		return;
	else if(sectorRange > MaxJobSectorRange())
		return;

	int oldRange = m_MaxSectorsAway;
	m_MaxSectorsAway = sectorRange;
	g_Au.Settings().writeEntry("Widget/Jobs/MaxSectorsAway", m_MaxSectorsAway);

	// Update the visible list based on the new range setting.
	if(m_MaxSectorsAway > oldRange)
	{
		// Add entries that are now within range.
		for(QList<AuJobOffer>::const_iterator joi = m_JobOffers.constBegin(); joi != m_JobOffers.constEnd(); joi++)
			if(TestFilter(*joi))
			{
				// Found a job offer that matches the current filter.
				int range = (int)ceil(m_SectorLoc.Distance((*joi).FromSectorLoc()) / 10);	// result is in sectors
				if(range <= oldRange)
					continue;									// The job offer was already in the list at the previous range.

				// Found a job offer that was not previously in range but now it is in range.
				AddJobOffer(*joi);
			}
	}
	else if(m_MaxSectorsAway < oldRange)
	{
		// Remove entries that are no longer in range.
		int row=0;
		while(row < m_JobsModel.rowCount())
		{
			QModelIndex itemIdx = m_JobsModel.index(row, 3);	// column 3 holds the sector range
			if(m_JobsModel.data(itemIdx, Qt::UserRole+1).toInt() > m_MaxSectorsAway)
				m_JobsModel.removeRows(row, 1);
			else
				row++;
		}

		m_JobCountLabel->setText(tr("%1 Job Offers").arg(m_JobsModel.rowCount()));
	}
}
