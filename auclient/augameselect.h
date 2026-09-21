//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augameselect.h,v 1.6 2022/02/19 17:56:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGameSelect_H
#define AuGameSelect_H

#include "auclientlib.h"
#include "audialog.h"

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QTextBrowser;

class AUCLIENT_DLL AuGameSelect : public AuDialog
{
	Q_OBJECT;
private:
	quint8				m_GameType;		// AuGame::Type() of currently selected game.

	// Widget
	QTextBrowser*		m_DescBrowser;
	QListWidget*		m_GamesList;
	QPushButton*		m_Ok;

public:
	AuGameSelect(QWidget* parent=0);
	virtual ~AuGameSelect();

	inline quint8 GameType() const
	{
		return m_GameType;
	}

public slots:
	virtual void setVisible(bool visible);

protected slots:
	void GameSelected();
	void GameDoubleClicked(QListWidgetItem* item);
	void OkPressed();

protected:
	void GameSelected(QListWidgetItem* item);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuGameSelect_H
