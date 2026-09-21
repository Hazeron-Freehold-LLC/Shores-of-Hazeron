//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdsitedlg.h,v 1.2 2022/02/19 18:21:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdSiteDlg_H
#define AuBdSiteDlg_H

#include "audialog.h"

class QComboBox;

class AuBdSiteDlg : public AuDialog
{
	Q_OBJECT;
	int									m_Draw;					// 0=draw, 1=agricultural, 2=commercial, 3=industrial,
																// 4=residential, 5=shore, 6=mine

	// Widgets
	QComboBox*							m_Blend;
	QComboBox*							m_Grade;
	QComboBox*							m_Grass;
	QComboBox*							m_Rocks;
	QComboBox*							m_Shrubs;
	QComboBox*							m_Trees;
	QComboBox*							m_Pave;

public:
	AuBdSiteDlg(QWidget* parent=0);
	virtual ~AuBdSiteDlg();

	int Blend() const;

	inline int Draw() const
	{
		return m_Draw;
	}

	int Grade() const;

	int Grass() const;

	int Pavement() const;

	int Rocks() const;

	int Shrubs() const;

	int Trees() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListBlends();
	void ListGrades();
	void ListGrass();
	void ListPavement();
	void ListRocks();
	void ListShrubs();
	void ListTrees();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoDraw();
	void DoLotAgricultural();
	void DoLotCommercial();
	void DoLotIndustrial();
	void DoLotMine();
	void DoLotResidential();
	void DoLotShore();
};

#endif // AuBdSiteDlg_H
