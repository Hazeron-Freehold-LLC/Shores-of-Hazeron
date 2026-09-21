//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aujobsquerydialog.h,v 1.6 2022/02/19 17:57:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuJobsQueryDialog_H
#define AuJobsQueryDialog_H

#include "auclientlib.h"
#include <audialog.h>
#include <auid.h>
#include <aujoboffer.h>
#include <QQueue>
#include <QStandardItemModel>

class AuJobOffer;
class AuNewJobView;
class AuToolButtonValue;

class AUCLIENT_DLL AuJobsQueryDialog : public AuDialog
{
	Q_OBJECT;
private:
	GALAXY								m_Galaxy;				// Galaxy of scene atom. For range filtering.
	AuJobOffer							m_JobOffer;				// Selected job offer.
	QList<AuJobOffer>					m_JobOffers;			// Job offers in my list, so I can filter them on/off.
	QStandardItemModel					m_JobsModel;			// Available jobs.
	QQueue<AuJobOffer>					m_JobsQueue;			// Queue of jobs received so we don't lock up adding them to UI.
	quint16								m_MaxSectorsAway;
	AuId								m_PersonId;				// Id of person for filter segregation.
	AuPoint								m_SectorLoc;			// Sector location of scene atom. For range filtering.
	QTimer*								m_Timer;
	int									m_TypeFilter[AuJob::JobTypes];	// 0=user has never seen this type, 1=off, 2=on

	// Widgets
	QLabel*								m_JobCountLabel;
	AuNewJobView*						m_JobsView;
	AuToolButtonValue*					m_TypeFilterButton[AuJob::JobTypes];

public:
	AuJobsQueryDialog(const AuId& personId, const AuPoint& sectorLoc, GALAXY galaxy, QWidget* parent);
	virtual ~AuJobsQueryDialog();

	void JobAvailable(const AuJobOffer& job);

public slots:
	virtual void setVisible(bool visible);

protected:
	void AddJobOffer(const AuJobOffer& jobOffer);
	void RestoreGeometry();
	void SaveGeometry();
	virtual QSize sizeHint() const;

	// Returns true if it's ok to list the job.
	bool TestFilter(const AuJobOffer& jobOffer);

protected slots:
	void AcceptJob();
	void DequeueJobOffer();
	void NewJobAccepted(const QModelIndex& sel);
	void NewJobSelected(const QModelIndex& sel);
	void QueryJobs();
	void RangeChanged(int sectorRange);
	void TypeFilterClicked(int type);
};

#endif // AuJobsQueryDialog_H
