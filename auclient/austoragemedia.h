//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austoragemedia.h,v 1.6 2022/02/19 18:02:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStorageMedia_H
#define AuStorageMedia_H

#include "auclientlib.h"
#include "audialog.h"
#include "augear.h"
#include "auknownpatents.h"

class AuItem;
class QLabel;
class QPushButton;
class QSplitter;
class QTextBrowser;
class QTreeWidget;

class AUCLIENT_DLL AuStorageMedia : public AuDialog
{
	Q_OBJECT;
	AuGearPos							m_GearPos;

	// Widgets
	QTreeWidget*						m_ContentTree;
	QTextBrowser*						m_DataDesc;
	QSplitter*							m_DataSplitter;
	QPushButton*						m_DeleteButton;
	QLabel*								m_ItemDesc;

public:
	AuStorageMedia(const AuItem& mediaItem, const AuGearPos& gearPos, QWidget* parent=0);
	virtual ~AuStorageMedia();

public slots:
	virtual void setVisible(bool visible);
	void DeleteContent();
	void UpdateContent();

protected slots:
	void DataSplitterMoved(int pos, int index);
	void ShowData();

protected:
	void ListContent();
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuStorageMedia_H
