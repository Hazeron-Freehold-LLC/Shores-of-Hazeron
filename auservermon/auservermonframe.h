//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservermonframe.h,v 1.21 2022/02/19 18:40:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuServerMonFrame_H
#define AuServerMonFrame_H

#include <QMainWindow>

class AuPlayersTable;
class AuReportWidget;
class AuServerCon;
class QSplitter;

class AuServerMonFrame : public QMainWindow
{
	Q_OBJECT;
private:
	// Widgets
	AuPlayersTable*			m_Players;
	AuReportWidget*			m_Report;
	AuServerCon*			m_ServerCon;
	QSplitter*				m_Splitter;

public:
	AuServerMonFrame(QWidget* parent=0);
	virtual ~AuServerMonFrame();

public slots:
	void RestorePlayerColumns();
	void SavePlayerColumns();
	void ServerSelected(int serverMode);
	void SplitterMoved(int pos, int index);

protected:
	virtual void closeEvent(QCloseEvent*);
	void CreateMenuBar();
};

#endif // AuServerMonFrame_H
