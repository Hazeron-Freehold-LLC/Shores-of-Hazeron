//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austoragemedia.cpp,v 1.5 2022/02/19 18:17:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austoragemedia.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTreeWidget>
#include <QTextBrowser>

class AuStorageMediaTreeItem : public QTreeWidgetItem
{
public:
	enum STORAGE_MEDIA_TYPE
	{
		PatentsFolder = UserType+1,
		SpacecraftDesignsFolder,
		Patent,
		SpacecraftDesign,
		Mission,
		BdDesignsFolder,
		BdDesign,
		PapersFolder,
		Paper,
	};

public:
	AuStorageMediaTreeItem(STORAGE_MEDIA_TYPE type, QTreeWidgetItem* parent) : QTreeWidgetItem(parent, type)
	{
	}

	AuStorageMediaTreeItem(STORAGE_MEDIA_TYPE type, QTreeWidget* parent) : QTreeWidgetItem(parent, type)
	{
	}

public:
	// Sort column in QTreeWidget numerically. QTreeWidgetItem SortRole
	virtual bool operator<(const QTreeWidgetItem& other) const
	{
		switch(Type())
		{
		case BdDesignsFolder:
		case Mission:
		case Paper:
		case PapersFolder:
		case PatentsFolder:
		case SpacecraftDesignsFolder:
		default:
			break;
		case Patent:
			// Sort column 1 numerically, by patent Q.
			if(treeWidget()->sortColumn() == 1)
			{
				quint32 otherQ = other.data(0, Qt::UserRole+1).toUInt();
				quint32 myQ = data(0, Qt::UserRole+1).toUInt();
				return myQ < otherQ;
			}
			break;
		case BdDesign:
		case SpacecraftDesign:
			// Sort column 1 numerically, by design id.
			if(treeWidget()->sortColumn() == 1)
			{
				quint32 otherDesignId = other.data(0, Qt::UserRole).toUInt();
				quint32 myDesignId = data(0, Qt::UserRole).toUInt();
				return myDesignId < otherDesignId;
			}
			break;
		}
		// Default sort is alphabetical.
		return QTreeWidgetItem::operator<(other);
	}

public:
	inline STORAGE_MEDIA_TYPE Type() const
	{
		return (STORAGE_MEDIA_TYPE)type();
	}

};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuStorageMedia::AuStorageMedia(const AuItem& mediaItem, const AuGearPos& gearPos, QWidget* parent) : AuDialog(parent)
{
	COMMODITYID mediaCID = mediaItem.CommodityId();
	m_GearPos = gearPos;
	setAuDlgTitle(tr("Storage Media Contents"));
	setAuDlgIcon(QIcon(":data/c_ThumbDrive.png"));

	QLabel* lbico = new QLabel(this);
	lbico->setPixmap(QPixmap(g_Au.CommodityBitmapName(mediaCID)).scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	m_ItemDesc = new QLabel(mediaItem.ListDesc(), this);

	m_ContentTree = new QTreeWidget(this);
	m_ContentTree->setColumnCount(2);
	m_ContentTree->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_ContentTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	QStringList hdr;
	hdr << tr("Name");
	hdr << tr("Info");
	m_ContentTree->setHeaderLabels(hdr);
	m_ContentTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ContentTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ListContent();
	connect(m_ContentTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(ShowData()));

	m_DataDesc = new QTextBrowser(this);
	m_DataDesc->setReadOnly(true);

	m_DeleteButton = new QPushButton(tr("Delete..."), this);
	m_DeleteButton->setToolTip(tr("<html>"
				"Deletes the selected data items from the storage media."
				"A confirmation dialog is shown. "
				"</html>"));
	connect(m_DeleteButton, SIGNAL(clicked()), SLOT(DeleteContent()));

	m_DataSplitter = new QSplitter;
	m_DataSplitter->setOrientation(Qt::Horizontal);
	m_DataSplitter->addWidget(m_ContentTree);
	m_DataSplitter->addWidget(m_DataDesc);
	m_DataSplitter->setCollapsible(0, false);
	m_DataSplitter->setCollapsible(1, false);
	m_DataSplitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_DataSplitter, SIGNAL(splitterMoved(int, int)), SLOT(DataSplitterMoved(int, int)));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(lbico);
	blay->addWidget(m_ItemDesc);
	blay->addStretch();
	blay->addWidget(m_DeleteButton);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->addWidget(m_DataSplitter);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/StorageMedia/Splitter", splitterstate);
	m_DataSplitter->restoreState(splitterstate);
}

AuStorageMedia::~AuStorageMedia()
{
}

void AuStorageMedia::DataSplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/StorageMedia/Splitter", m_DataSplitter->saveState());
}

void AuStorageMedia::setVisible(bool visible)
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

void AuStorageMedia::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/StorageMedia/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/StorageMedia/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/StorageMedia/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/StorageMedia/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuStorageMedia::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/StorageMedia/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/StorageMedia/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/StorageMedia/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/StorageMedia/SizeY",sz.height());
}

void AuStorageMedia::ListContent()
{
	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	if(!pView)
		return;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const AuGear* gear = person->LockGearForRead();
		if(gear)
		{
			const AuItemStorageMedia* item = dynamic_cast<const AuItemStorageMedia*>(gear->ItemAt(m_GearPos));
			if(item)
			{
				// Found my storage media item.
				if(!item->Missions().empty())
				{
					AuStorageMediaTreeItem* folderItem = new AuStorageMediaTreeItem(AuStorageMediaTreeItem::SpacecraftDesignsFolder,
							m_ContentTree);
					folderItem->setIcon(0, QIcon(":data/Folder.png"));
					folderItem->setText(0, tr("Missions"));
					for(AuMissionList::const_iterator mi = item->Missions().cbegin(); mi != item->Missions().cend(); mi++)
					{
						AuStorageMediaTreeItem* item = new AuStorageMediaTreeItem(AuStorageMediaTreeItem::Mission, folderItem);
						item->setIcon(0, QIcon(":data/c_MissionDisk.png"));
						item->setText(0, mi.key());
						item->setData(0, Qt::UserRole, mi.key());
						item->setData(0, Qt::UserRole+1, (*mi).HtmlDesc());

						QString text = QString("%1 orders, %2")
							.arg((*mi).Count())
							.arg((*mi).Type() == AuMission::Single?QObject::tr("Do Once"):QObject::tr("Cycle"));
						item->setText(1, text);
					}
				}

				if(!item->Papers().empty())
				{
					AuStorageMediaTreeItem* folderItem = new AuStorageMediaTreeItem(AuStorageMediaTreeItem::PapersFolder,
							m_ContentTree);
					folderItem->setIcon(0, QIcon(":data/Folder.png"));
					folderItem->setText(0, tr("Papers"));
					for(QStringList::const_iterator pi = item->Papers().cbegin(); pi != item->Papers().cend(); pi++)
					{
						QString s(*pi);
						if(s.length() > 40)
							s = s.left(40) + "...";
						AuStorageMediaTreeItem* item = new AuStorageMediaTreeItem(AuStorageMediaTreeItem::Paper, folderItem);
						item->setIcon(0, QIcon(":data/c_Paper.png"));
						item->setText(0, s);
						item->setData(0, Qt::UserRole, *pi);
						item->setData(0, Qt::UserRole+1, *pi);
					}
				}

				if(!item->Patents().empty())
				{
					AuStorageMediaTreeItem* folderItem = new AuStorageMediaTreeItem(AuStorageMediaTreeItem::PatentsFolder,
							m_ContentTree);
					folderItem->setIcon(0, QIcon(":data/Folder.png"));
					folderItem->setText(0, tr("Patents"));
					for(QHash<COMMODITYID, quint8>::const_iterator pi = item->Patents().cbegin(); pi != item->Patents().cend();
							pi++)
					{
						AuStorageMediaTreeItem* item = new AuStorageMediaTreeItem(AuStorageMediaTreeItem::Patent, folderItem);
						item->setIcon(0, QIcon(":data/c_Patent.png"));
						item->setText(0, QString("%1 Q%2").arg(g_Au.CommodityName(pi.key())).arg(*pi));
						item->setData(0, Qt::UserRole, pi.key());
						item->setData(0, Qt::UserRole+1, *pi);
					}
				}

				if(!item->BlueprintsBd().empty())
				{
					AuStorageMediaTreeItem* folderItem = new AuStorageMediaTreeItem(AuStorageMediaTreeItem::BdDesignsFolder,
							m_ContentTree);
					folderItem->setIcon(0, QIcon(":data/Folder.png"));
					folderItem->setText(0, tr("Building Blueprints"));
					for(AuBdDesignExchange::const_iterator ei = item->BlueprintsBd().cbegin(); ei != item->BlueprintsBd().cend();
							ei++)
					{
						QString text;
						if((*ei).DesignState() == AuDesign::Final)
							text = QString("%1 Q%4 by %3")
								.arg((*ei).DesignName())
								.arg((*ei).DesignArchitectName())
								.arg((*ei).DesignQuality());
						else
							text = QString("%1 Assembly by %3")
								.arg((*ei).DesignName())
								.arg((*ei).DesignArchitectName());
						AuStorageMediaTreeItem* item = new AuStorageMediaTreeItem(AuStorageMediaTreeItem::BdDesign,
								folderItem);
						item->setIcon(0, QIcon(":data/c_BdDesignDrawing.png"));
						item->setText(0, text);
						item->setData(0, Qt::UserRole, ei.key());
						item->setData(0, Qt::UserRole+1, (*ei).ReportText());

						item->setText(1, QString("%1").arg(ei.key()));
					}
				}

				if(!item->BlueprintsSc().empty())
				{
					AuStorageMediaTreeItem* folderItem = new AuStorageMediaTreeItem(AuStorageMediaTreeItem::SpacecraftDesignsFolder,
							m_ContentTree);
					folderItem->setIcon(0, QIcon(":data/Folder.png"));
					folderItem->setText(0, tr("Spacecraft Blueprints"));
					for(AuScDesignExchange::const_iterator ei = item->BlueprintsSc().cbegin(); ei != item->BlueprintsSc().cend();
							ei++)
					{
						QString text;
						if((*ei).DesignState() == AuDesign::Final)
							text = QString("%1 Q%4 by %3")
								.arg((*ei).DesignName())
								.arg((*ei).DesignArchitectName())
								.arg((*ei).DesignQuality());
						else
							text = QString("%1 Assembly by %3")
								.arg((*ei).DesignName())
								.arg((*ei).DesignArchitectName());
						AuStorageMediaTreeItem* item = new AuStorageMediaTreeItem(AuStorageMediaTreeItem::SpacecraftDesign,
								folderItem);
						item->setIcon(0, QIcon(":data/c_ScDesignDrawing.png"));
						item->setText(0, text);
						item->setData(0, Qt::UserRole, ei.key());
						item->setData(0, Qt::UserRole+1, (*ei).ReportText());

						item->setText(1, QString("%1").arg(ei.key()));
					}
				}
			}
		}
		person->UnlockGear();
	}
	pView->UnlockSceneAtom();
	m_ContentTree->setSortingEnabled(true);
	m_ContentTree->sortItems(0, Qt::AscendingOrder);
}

void AuStorageMedia::UpdateContent()
{
	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	if(!pView)
		return;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const AuGear* gear = person->LockGearForRead();
		if(gear)
		{
			const AuItemStorageMedia* item = dynamic_cast<const AuItemStorageMedia*>(gear->ItemAt(m_GearPos));
			if(item)
			{
				// Found my storage media item.
				m_ItemDesc->setText(item->ListDesc());

				// Look for existing missions folder.
				QList<QTreeWidgetItem*> missionFolders = m_ContentTree->findItems(tr("Missions"),
						Qt::MatchFixedString|Qt::MatchCaseSensitive);
				if(!missionFolders.empty())
				{
					// Found existing folder. Did the count change?
					AuStorageMediaTreeItem* mitem = (AuStorageMediaTreeItem*)missionFolders.first();
					if(mitem->childCount() > item->Missions().size())
					{
						// One or more missions were deleted.
						QList<AuStorageMediaTreeItem*> removeChildren;	// Children to be removed.
						for(int ci=0; ci < mitem->childCount(); ci++)
						{
							AuStorageMediaTreeItem* ch = (AuStorageMediaTreeItem*)mitem->child(ci);
							QString ct = ch->text(0);
							if(!item->Missions().contains(ct))
								removeChildren.append(ch);
						}
						// Remove the children.
						while(!removeChildren.empty())
						{
							AuStorageMediaTreeItem* ch = removeChildren.takeFirst();
							mitem->removeChild(ch);
							delete ch;
						}
					}

					if(item->Missions().empty())
					{
						// Remove the folder.
						int ti = m_ContentTree->indexOfTopLevelItem(mitem);
						if(ti >= 0)
							delete m_ContentTree->takeTopLevelItem(ti);
					}
				}

				// Look for existing papers folder.
				QList<QTreeWidgetItem*> paperFolders = m_ContentTree->findItems(tr("Papers"),
						Qt::MatchFixedString|Qt::MatchCaseSensitive);
				if(!paperFolders.empty())
				{
					// Found existing folder. Did the count change?
					AuStorageMediaTreeItem* pitem = (AuStorageMediaTreeItem*)paperFolders.first();
					if(pitem->childCount() > item->Papers().size())
					{
						// One or more papers were deleted.
						QList<AuStorageMediaTreeItem*> removeChildren;	// Children to be removed.
						for(int ci=0; ci < pitem->childCount(); ci++)
						{
							AuStorageMediaTreeItem* ch = (AuStorageMediaTreeItem*)pitem->child(ci);
							QString ct = ch->text(0);
							bool removeIt = true;	// Start pessimistic, until we find the corresponding paper.
							for(QStringList::const_iterator pi = item->Papers().cbegin(); pi != item->Papers().cend(); pi++)
							{
								QString s(*pi);
								if(s.length() > 40)
									s = s.left(40) + "...";
								if(s == ct)
								{
									removeIt = false;	// Found the source paper. Do not remove the item.
									break;
								}
							}
							if(removeIt)
								removeChildren.append(ch);
						}
						// Remove the children.
						while(!removeChildren.empty())
						{
							AuStorageMediaTreeItem* ch = removeChildren.takeFirst();
							pitem->removeChild(ch);
							delete ch;
						}
					}

					if(item->Papers().empty())
					{
						// Remove the folder.
						int ti = m_ContentTree->indexOfTopLevelItem(pitem);
						if(ti >= 0)
							delete m_ContentTree->takeTopLevelItem(ti);
					}
				}

				// Look for existing patents folder.
				QList<QTreeWidgetItem*> patentFolders = m_ContentTree->findItems(tr("Patents"),
						Qt::MatchFixedString|Qt::MatchCaseSensitive);
				if(!patentFolders.empty())
				{
					// Found existing folder. Did the count change?
					AuStorageMediaTreeItem* pitem = (AuStorageMediaTreeItem*)patentFolders.first();
					if(pitem->childCount() > item->Patents().size())
					{
						// One or more patents were deleted.
						QList<AuStorageMediaTreeItem*> removeChildren;	// Children to be removed.
						for(int ci=0; ci < pitem->childCount(); ci++)
						{
							AuStorageMediaTreeItem* ch = (AuStorageMediaTreeItem*)pitem->child(ci);
							COMMODITYID cid = (COMMODITYID)ch->data(0, Qt::UserRole).toInt();
							quint8 q = ch->data(0, Qt::UserRole+1).toUInt();
							if(!item->Patents().HasPatent(cid, q))
								removeChildren.append(ch);
						}
						// Remove the children.
						while(!removeChildren.empty())
						{
							AuStorageMediaTreeItem* ch = removeChildren.takeFirst();
							pitem->removeChild(ch);
							delete ch;
						}
					}

					if(item->Patents().empty())
					{
						// Remove the folder.
						int ti = m_ContentTree->indexOfTopLevelItem(pitem);
						if(ti >= 0)
							delete m_ContentTree->takeTopLevelItem(ti);
					}
				}

				// Look for existing bd blueprints folder.
				QList<QTreeWidgetItem*> bdFolders = m_ContentTree->findItems(tr("Building Blueprints"),
						Qt::MatchFixedString|Qt::MatchCaseSensitive);
				if(!bdFolders.empty())
				{
					// Found existing folder. Did the count change?
					AuStorageMediaTreeItem* bitem = (AuStorageMediaTreeItem*)bdFolders.first();
					if(bitem->childCount() > item->BlueprintsBd().size())
					{
						// One or more blueprints were deleted.
						QList<AuStorageMediaTreeItem*> removeChildren;	// Children to be removed.
						for(int ci=0; ci < bitem->childCount(); ci++)
						{
							AuStorageMediaTreeItem* ch = (AuStorageMediaTreeItem*)bitem->child(ci);
							QString ct = ch->text(0);
							quint32 di = ch->data(0, Qt::UserRole).toUInt();	// Design id
							if(!item->BlueprintsBd().contains(di))
								removeChildren.append(ch);
						}
						// Remove the children.
						while(!removeChildren.empty())
						{
							AuStorageMediaTreeItem* ch = removeChildren.takeFirst();
							bitem->removeChild(ch);
							delete ch;
						}
					}

					if(item->BlueprintsBd().empty())
					{
						// Remove the folder.
						int ti = m_ContentTree->indexOfTopLevelItem(bitem);
						if(ti >= 0)
							delete m_ContentTree->takeTopLevelItem(ti);
					}
				}

				// Look for existing bd blueprints folder.
				QList<QTreeWidgetItem*> scFolders = m_ContentTree->findItems(tr("Spacecraft Blueprints"),
						Qt::MatchFixedString|Qt::MatchCaseSensitive);
				if(!scFolders.empty())
				{
					// Found existing folder. Did the count change?
					AuStorageMediaTreeItem* bitem = (AuStorageMediaTreeItem*)scFolders.first();
					if(bitem->childCount() > item->BlueprintsSc().size())
					{
						// One or more blueprints were deleted.
						QList<AuStorageMediaTreeItem*> removeChildren;	// Children to be removed.
						for(int ci=0; ci < bitem->childCount(); ci++)
						{
							AuStorageMediaTreeItem* ch = (AuStorageMediaTreeItem*)bitem->child(ci);
							QString ct = ch->text(0);
							quint32 di = ch->data(0, Qt::UserRole).toUInt();	// Design id
							if(!item->BlueprintsSc().contains(di))
								removeChildren.append(ch);
						}
						// Remove the children.
						while(!removeChildren.empty())
						{
							AuStorageMediaTreeItem* ch = removeChildren.takeFirst();
							bitem->removeChild(ch);
							delete ch;
						}
					}

					if(item->BlueprintsSc().empty())
					{
						// Remove the folder.
						int ti = m_ContentTree->indexOfTopLevelItem(bitem);
						if(ti >= 0)
							delete m_ContentTree->takeTopLevelItem(ti);
					}
				}
			}
		}
		person->UnlockGear();
	}
	pView->UnlockSceneAtom();
}

void AuStorageMedia::DeleteContent()
{
	QList<QTreeWidgetItem *> selItems = m_ContentTree->selectedItems();
	if(selItems.empty())
		return;

	QString smsg = tr("<html>"
			"Confirm delete of selected items."
			"</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	QSet<COMMODITYID> patentCIDs;
	QSet<quint32> bdDesigns;
	QSet<quint32> scDesigns;
	QSet<QString> missions;
	QSet<QString> papers;
	for(QList<QTreeWidgetItem*>::const_iterator si = selItems.cbegin(); si != selItems.cend(); si++)
	{
		AuStorageMediaTreeItem::STORAGE_MEDIA_TYPE type = (AuStorageMediaTreeItem::STORAGE_MEDIA_TYPE)(*si)->type();
		switch(type)
		{
		case AuStorageMediaTreeItem::PapersFolder:
		case AuStorageMediaTreeItem::PatentsFolder:
		case AuStorageMediaTreeItem::SpacecraftDesignsFolder:
		case AuStorageMediaTreeItem::BdDesignsFolder:
			continue;
		case AuStorageMediaTreeItem::Mission:
			{
				QString missionName = (*si)->data(0, Qt::UserRole).toString();
				missions.insert(missionName);
			}
			break;
		case AuStorageMediaTreeItem::BdDesign:
			{
				quint32 designId = (*si)->data(0, Qt::UserRole).toUInt();
				bdDesigns.insert(designId);
			}
			break;
		case AuStorageMediaTreeItem::Paper:
			{
				QString text = (*si)->data(0, Qt::UserRole).toString();
				papers.insert(text);
			}
			break;
		case AuStorageMediaTreeItem::Patent:
			{
				COMMODITYID patentCID = (COMMODITYID)(*si)->data(0, Qt::UserRole).toInt();
				patentCIDs.insert(patentCID);
			}
			break;
		case AuStorageMediaTreeItem::SpacecraftDesign:
			{
				quint32 designId = (*si)->data(0, Qt::UserRole).toUInt();
				scDesigns.insert(designId);
			}
			break;
		}
	}
	auApp->GearDeleteContents(m_GearPos, patentCIDs, scDesigns, bdDesigns, missions, papers);
	//accept();
}

void AuStorageMedia::ShowData()
{
	const AuStorageMediaTreeItem* item = (const AuStorageMediaTreeItem*)m_ContentTree->currentItem();
	if(!item)
	{
		m_DataDesc->setHtml("");
		return;
	}

	QString info;
	switch(item->Type())
	{
	case AuStorageMediaTreeItem::BdDesignsFolder:
	case AuStorageMediaTreeItem::PapersFolder:
	case AuStorageMediaTreeItem::PatentsFolder:
	case AuStorageMediaTreeItem::SpacecraftDesignsFolder:
		break;
	case AuStorageMediaTreeItem::Paper:
		info = item->data(0, Qt::UserRole+1).toString();
		break;
	case AuStorageMediaTreeItem::Patent:
		break;
	case AuStorageMediaTreeItem::BdDesign:
	case AuStorageMediaTreeItem::Mission:
	case AuStorageMediaTreeItem::SpacecraftDesign:
		info = item->data(0, Qt::UserRole+1).toString();
		break;
	}
	m_DataDesc->setHtml(info);
}
