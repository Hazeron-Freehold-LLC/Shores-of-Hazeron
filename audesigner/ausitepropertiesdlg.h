//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausitepropertiesdlg.h,v 1.2 2022/02/19 18:23:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSitePropertiesDlg_H
#define AuSitePropertiesDlg_H

#include "audialog.h"

class QComboBox;

class AuSitePropertiesDlg : public AuDialog
{
	Q_OBJECT;

	// Widgets
	QComboBox*							m_Blend;
	QComboBox*							m_Grade;
	QComboBox*							m_Grass;
	QComboBox*							m_Rocks;
	QComboBox*							m_Shrubs;
	QComboBox*							m_Trees;
	QComboBox*							m_Pave;

public:
	AuSitePropertiesDlg(quint8 grade, quint8 grass, quint8 pavement, quint8 rocks, quint8 shrubs, quint8 terBlend, quint8 trees,
			QWidget* parent=0);
	virtual ~AuSitePropertiesDlg();

	int Blend() const;

	int Grade() const;

	int Grass() const;

	int Pavement() const;

	int Rocks() const;

	int Shrubs() const;

	int Trees() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListBlends(quint8 terBlend);
	void ListGrades(quint8 grade);
	void ListGrass(quint8 grass);
	void ListPavement(quint8 pavement);
	void ListRocks(quint8 rocks);
	void ListShrubs(quint8 shrubs);
	void ListTrees(quint8 trees);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuSitePropertiesDlg_H
