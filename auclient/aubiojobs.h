//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubiojobs.h,v 1.7 2022/02/19 17:51:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBioJobs_H
#define AuBioJobs_H

#include "auclientlib.h"
#include "aujoboffer.h"

class AuOpenStoryView;
class AuPushButtonValue;
class AuIconButton;
class QLabel;
class QTextEdit;
class QTimer;
class QTreeView;

class AUCLIENT_DLL AuBioJobs : public QWidget
{
	Q_OBJECT;
private:
	QStandardItemModel					m_OpenStoryModel;
	AuJob*								m_SelectedJob;
	QTimer*								m_timer;

	// Widgets
	QTreeView*							m_BountyTree;
	QTextEdit*							m_JobDesc;
	AuOpenStoryView*					m_OpenJobs;
	AuIconButton*						m_StoryHint;

public:
	AuBioJobs(QWidget *parent=0);
	~AuBioJobs();

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void CloseStory();
	void OpenStorySelected(const QModelIndex& sel);
	void RequestWork();
	void ResetStory();
	void StopStory();
	void StoryHint();
	void SubmitStory();
	void UpdateContents();

signals:
	void CloseStoryRequested(AuJobOffer jobToClose);
	void StoryBlockRequested(quint32 storyInstance);
	void StoryHintRequested(quint32 storyInstance);
};

#endif // AuBioJobs_H
