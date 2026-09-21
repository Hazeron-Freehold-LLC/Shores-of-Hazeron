//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmaphighlightdialog.h,v 1.7 2022/02/19 18:01:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStarMapHighlightDialog_H
#define AuStarMapHighlightDialog_H

#include "auclientlib.h"
#include <audialog.h>

class QCheckBox;
class QRadioButton;
class AuCheckBoxValue;

class AUCLIENT_DLL AuStarMapHighlightDialog : public AuDialog
{
	Q_OBJECT;
private:
	QList<AuCheckBoxValue*>				m_EmpireChecks;			// Empire check boxes.
	QRadioButton*						m_EnemyPrecedent;
	QCheckBox*							m_ShowEmpire;
	QCheckBox*							m_ShowFriend;
	QCheckBox*							m_ShowNeutral;
	QCheckBox*							m_ShowEnemy;
	QCheckBox*							m_ShowUnknown;
	QCheckBox*							m_ShowExplored;
	QCheckBox*							m_ShowDetected;
	QRadioButton*						m_UnknownPrecedent;

public:
	AuStarMapHighlightDialog(quint32 viewerEmpireId,
			bool showEmpirePlaces,
			bool showFriendPlaces,
			bool showNeutralPlaces,
			bool showEnemyPlaces,
			bool showUnknownPlaces,
			bool showExploredPlaces,
			bool showDetectedPlaces,
			bool unknownPrecedesEnemy,
			const QList<quint32>& empiresIgnored,
			QWidget* parent=0);
	virtual ~AuStarMapHighlightDialog();

	bool EnemyPrecedesUnknown() const;

	QList<quint32> IgnoredEmpires() const;

	bool ShowDetected() const;

	bool ShowEmpire() const;

	bool ShowEnemy() const;

	bool ShowExplored() const;

	bool ShowFriend() const;

	bool ShowNeutral() const;

	bool ShowUnknown() const;

	bool UnknownPrecedesEnemy() const;

public slots:
	void ApplyFilter();
	void ClearFilter();
	void CheckAllEmpires();
	void InvertCheckedEmpires();
	virtual void setVisible(bool visible);
	void UncheckAllEmpires();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuStarMapHighlightDialog_H
