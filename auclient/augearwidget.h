//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augearwidget.h,v 1.17 2022/05/06 19:13:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGearWidget_H
#define AuGearWidget_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "augear.h"
#include "auglobal.h"
#include "dzspecies.h"
#include <QTreeView>
#include <QWidget>

class AuDNABase;
class AuHandCraftDialog;
class AuIconButton;
class AuStorageMedia;
class QLineEdit;
class QTimer;

class AUCLIENT_DLL AuGearTree : public QTreeView
{
	Q_OBJECT;
	int									m_ButtonsPressed;
	QPoint								m_PressPos;

public:
	AuGearTree(QWidget* parent=Q_NULLPTR);

	void ConfigureSort();

protected:
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);

signals:
	void MouseDoubleClicked();
	void MouseReleasedInside();
	void MouseReleasedOutside();
	void MouseRightClicked();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGearModel : public QStandardItemModel
{
	Q_OBJECT;
public:
	AuGearModel();

public:
	void AddGearPos(const AuGearPos& gpos, const AuGear& gear);

	void ClearMergeFlag(const AuGearSlot& gearSlot, const AuGearPos& nuGearPos, const QModelIndex& parentIdx = QModelIndex());

	void ListExpandedGearPos(const AuGearTree& tree, AuGearPosList& expanded, const QModelIndex& parentIdx=QModelIndex());

	void RemoveFlaggedItems(const QModelIndex& parentIdx = QModelIndex());

	// Returns true on success.
	bool RemoveGearPos(const AuGearPos& gpos, const QModelIndex& parentIdx=QModelIndex());

	void SetMergeFlags(const QModelIndex& parentIdx = QModelIndex());

	void Sort();

	// Returns true on success.
	bool UpdateGearPos(const AuGearPos& gpos, const AuGear& gear, const QModelIndex& parentIdx=QModelIndex());

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		return QAbstractItemModel::flags(index);
	}

protected:
	void AddGearPos(const AuGearPos& gpos, const AuGear& gear, QList<AuItemContentId>& path, QStandardItem* parentItem,
			const QModelIndex& parentIdx);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGearWidget : public QWidget
{
	Q_OBJECT;
private:	// Properties
	AuGear								m_Gear;					// My copy of the gear.
	AuId								m_GearId;				// Avatar id whose gear we have in the model and tree.
	AuGearModel							m_GearModel;
	AuHandCraftDialog*					m_HandCraftDlg;
	bool								m_HasBody;
	bool								m_HasBodyWear;
	bool								m_HasDigit;
	bool								m_HasEmptyBody;
	bool								m_HasEmptyBodyWear;
	bool								m_HasEmptyDigit;
	bool								m_HasEmptyFace;
	bool								m_HasEmptyHand;
	bool								m_HasEmptyHead;
	bool								m_HasEmptyLimb;
	bool								m_HasEmptyNeck;
	bool								m_HasEmptyWaist;
	bool								m_HasFace;
	bool								m_HasHand;
	bool								m_HasHead;
	bool								m_HasLimb;
	bool								m_HasNeck;
	bool								m_HasWaist;
	AuGearPos							m_PressPos;
	AuDNABase*							m_SceneAtomDNA;
	bool								m_SceneAtomHasAcid;
	bool								m_SceneAtomHasFire;
	AuGearPos							m_SelPos;
	AuStorageMedia*						m_StorageMediaDlg;
	QTimer*								m_Timer;				// UpdateContents timer

	// Widgets
	QLineEdit*							m_Account;
	AuIconButton*						m_Arm;
	QLineEdit*							m_Armor;
	QLineEdit*							m_Burden;
	AuIconButton*						m_Claim;
	AuIconButton*						m_CollectBounty;
	AuIconButton*						m_Decontaminate;
	AuIconButton*						m_Drink;
	AuIconButton*						m_Drop;
	AuIconButton*						m_Eat;
	AuIconButton*						m_Extinguish;
	AuIconButton*						m_Fuse;
	AuGearTree*							m_GearTree;
	AuIconButton*						m_HandCraft;
	AuIconButton*						m_Heal;
	AuIconButton*						m_Inscribe;
	AuIconButton*						m_Inspect;
	AuIconButton*						m_Light;
	AuIconButton*						m_NameSpecies;
	AuIconButton*						m_SetMessage;
	AuIconButton*						m_Settings;
	AuIconButton*						m_SlotBody;
	AuIconButton*						m_SlotBodyWear;
	AuIconButton*						m_SlotDigit;
	AuIconButton*						m_SlotFace;
	AuIconButton*						m_SlotHand;
	AuIconButton*						m_SlotHead;
	AuIconButton*						m_SlotLimb;
	AuIconButton*						m_SlotNeck;
	AuIconButton*						m_SlotWaist;
	AuIconButton*						m_Sound;
	AuIconButton*						m_StarmapLoc;
	AuIconButton*						m_StorageMedia;
	//AuIconButton*						m_Use;
	AuIconButton*						m_Visor;

public:
	AuGearWidget(QWidget* parent=0);
	virtual ~AuGearWidget();

public:
	inline const AuGearPos& GetSelectionPos() const
	{
		return m_SelPos;
	}

protected:
	// 'gearPos' will be invalid for items in containers inside of containers.
	//void BuildContainerTree(QStandardItem* parentItem, const AuGearPos& gearPos, const AuItemContent& content);
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);

public slots:
	void ReplaceSpecies(DzSpecies species);

protected slots:
	void ArmClicked();
	void CollectBountyClicked();
	void DecontaminateClicked();
	void DrinkClicked();
	void DropClicked();
	void EatClicked();
	void EnableControls();
	void ExtinguishClicked();
	void FuseClicked();
	void GearSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void GearTreeDoubleClicked();
	void GearTreeDrop();
	void GearTreeMove();
	void GearTreePressed(const QModelIndex& idx);
	void GearTreeRightClicked();
	void HandCraftClicked();
	void InscribeClicked();
	void InspectClicked();
	void LightClicked();
	void NameSpeciesClicked();
	void SetMessageClicked();
	void SettingsClicked();
	void SlotBodyClicked();
	void SlotBodyWearClicked();
	void SlotDigitClicked();
	void SlotFaceClicked();
	void SlotHandClicked();
	void SlotHeadClicked();
	void SlotLimbClicked();
	void SlotNeckClicked();
	void SlotWaistClicked();
	void SoundClicked();
	void StarmapLocClicked();
	void StorageMediaClicked();
	void UpdateContents();
	void UseClicked();
	void VisorClicked();

signals:
	void DropGearItem(AuGearPos pos);
	void InscribeGearItem(AuGearPos pos, QString inscription);
	void MoveGearItemToItem(AuGearPos fromPos, AuGearPos toPos);
	void MoveGearItemToSlot(AuGearPos fromPos, quint8);
	void RedeemGearItem(AuGearPos pos);
	void SetFlowerPot(AuGearPos pos, quint8 plantSize);
	void SetMessage(AuGearPos pos, QString message);
	void SetPictureFrame(AuGearPos pos, QString url, bool hideURL, bool rotate);
	void ShowGalaxyLocOnStarMap(QString galaxyName, AuPoint loc);
	void ToggleHandCraftUi();
	void UseGearItem(AuGearPos pos);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGearDock : public AuDockWidget
{
	Q_OBJECT;
public:
	AuGearWidget*						m_Gear;

public:
	AuGearDock(QWidget* parent) : AuDockWidget(":data/Inventory.png", tr("<b>Gear</b>"), tr("Gear"), parent)
	{
		SetWikiTopic("Gear");
		m_Gear = new AuGearWidget(this);
		setWidget(m_Gear);

		int dockSides = g_Au.Settings().readNumEntry("Dock/InventoryAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/InventoryAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuGearWidget_H
