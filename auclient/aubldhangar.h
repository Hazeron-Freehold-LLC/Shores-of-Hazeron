//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldhangar.h,v 1.6 2022/02/19 17:51:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldHangar_H
#define AuBldHangar_H

#include "auclientlib.h"
#include <QWidget>

class DaPerson;
class DpBldSpacecraftFactory;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuBldHangar : public QWidget
{
	Q_OBJECT;
private:
	QPushButton*						m_Launch;
	QPushButton*						m_Mothball;
	QComboBox*							m_NearbyShips;
	QLineEdit*							m_StoredShipInfo;
	QLineEdit*							m_StoredShipsInfo;
	QCheckBox*							m_TakeCommand;

public:
	AuBldHangar(QWidget* parent=0);
	virtual ~AuBldHangar();

protected:
	virtual void hideEvent(QHideEvent* event);
	void ListNearbyShips();
	virtual void showEvent(QShowEvent* event);

public slots:
	void QueryAnswered(QString shipDesc, quint32 shipDesign, quint32 shipsInHangar);

protected slots:
	void Query();
	void Launch();
	void Mothball();
};

#endif // AuBldHangar_H
