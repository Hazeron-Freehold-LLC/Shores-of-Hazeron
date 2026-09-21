//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augameclonepokui.h,v 1.8 2022/02/19 17:31:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGameClonepokUi_H
#define AuGameClonepokUi_H

#include "audialog.h"
#include "augame.h"
#include "augameclonepok.h"
#include "augear.h"
#include <QTreeView>

class AuGameClonepok;
class AuPushButtonValue;
class DaPerson;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QSlider;

class AuGameClonepokGearTree : public QTreeView
{
	Q_OBJECT;
	int									m_ButtonsPressed;
	QPoint								m_PressPos;

public:
	AuGameClonepokGearTree(QWidget* parent=Q_NULLPTR);

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

class AuGameClonepokGearModel : public QStandardItemModel
{
	Q_OBJECT;
public:
	AuGameClonepokGearModel();

public:
	void AddGearPos(const AuGearPos& gpos, const AuGear& gear);

	// Returns true on success.
	bool RemoveGearPos(const AuGearPos& gpos, const QModelIndex& parentIdx=QModelIndex());

	void Sort();

	virtual Qt::ItemFlags flags(const QModelIndex& index) const
	{
		return QAbstractItemModel::flags(index);
	}

protected:
	void AddGearPos(const AuGearPos& gpos, const AuGear& gear, QList<AuItemContentId>& path,
			QStandardItem* parentItem, const QModelIndex& parentIdx);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGameClonepokSetup : public AuDialog
{
	Q_OBJECT;
private:
	AuId								m_PlayerIds[MAX_GAME_PLAYERS];		// Id's of all players in the game.

	// Widgets
	QLineEdit*							m_Ante;
	QLabel*								m_MinCash;
	QPushButton*						m_Ok;
	AuPushButtonValue*					m_PlayerKick[MAX_GAME_PLAYERS];
	QLabel*								m_PlayerNames[MAX_GAME_PLAYERS];
	QComboBox*							m_Stack;

public:
	AuGameClonepokSetup(const DaPerson& person, const AuGameClonepok& game, QWidget* parent = 0);
	virtual ~AuGameClonepokSetup();

public slots:
	virtual void setVisible(bool visible);
	void StartGame();
	void KickPlayer(int playerIndex);
	void SettingsChanged();

protected:
	void RestoreGeometry();
	void SaveGeometry();

signals:
	void Accepted(quint8 gameType, QByteArray settings);	// All AuGame setup dialogs must emit this signal when completed.
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGameClonepokMap : public QWidget
{
	Q_OBJECT;
private:
	QImage								m_Map;
	QSet<quint8>						m_SelectedUnits;
	AuGameClonepokTileList				m_TilesInPlay;

public:
	AuGameClonepokMap(const DaPerson& person, const AuGameClonepok& game, QWidget* parent = 0);
	virtual ~AuGameClonepokMap();

	virtual QSize sizeHint() const;

signals:
	void HordeMoved(quint8 toSpace, QSet<quint8> selectedUnits);

protected:
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void paintEvent(QPaintEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGameClonepokTurn : public AuDialog
{
	Q_OBJECT;
private:
	quint32								m_CloneCost;
	AuGear								m_Gear;					// My copy of the gear.
	AuGameClonepokGearModel				m_GearModel;
	AuGameClonepok::TURNSTATE			m_TurnState;

	// Widgets
	QPushButton*						m_CreateClone;
	QComboBox*							m_DNASamples;
	AuGameClonepokGearTree* 			m_GearTree;
	AuGameClonepokMap*					m_Map;

public:
	AuGameClonepokTurn(const DaPerson& person, const AuGameClonepok& game, QWidget* parent = 0);
	virtual ~AuGameClonepokTurn();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void CreateClone();
	void MoveHorde(quint8 toSpace, QSet<quint8> unitIds);
	void QuitGame();

signals:
	void Accepted(quint8 gameType, QByteArray turn);	// All AuGame turn dialogs must emit this signal when completed.
	void Quit();										// Emit this signal when a player quits the game.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGameClonepokFinish : public AuDialog
{
	Q_OBJECT;
public:
	AuGameClonepokFinish(const QByteArray& gameResult, QWidget* parent = 0);
	virtual ~AuGameClonepokFinish();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuGameClonepokUi_H
