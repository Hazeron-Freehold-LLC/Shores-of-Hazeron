//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitemmanipulator.h,v 1.11 2022/05/05 19:29:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItemManipulator_H
#define AuItemManipulator_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include "aumatrix4x4.h"

class AuItem;
class DaItem;
class QComboBox;
class QLabel;
class QRadioButton;

class AUCLIENT_DLL AuItemManipulator : public AuDialog
{
	Q_OBJECT;
	AuItem*								m_Item;
	AuId								m_ItemId;
	QString								m_ItemInfo;
	QString								m_ItemStaticRoom;
	AuMatrix4x4							m_ItemTransform;

	// Widgets
	QComboBox*							m_MoveDist;
	QLabel*								m_OwnerLabel;
	QRadioButton*						m_RelEnv;
	QRadioButton*						m_RelItem;
	QComboBox*							m_SpinAngle;

public:
	AuItemManipulator(QWidget* parent=0);
	virtual ~AuItemManipulator();

	void SetItem(const DaItem& item);

	inline const AuId& ItemId() const
	{
		return m_ItemId;
	}

	virtual QSize sizeHint() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void Info();
	void MoveBackward();
	void MoveDown();
	void MoveForward();
	void MoveLeft();
	void MoveRight();
	void MoveUp();
	void Reset();
	void RotateXNeg();
	void RotateXPos();
	void RotateYNeg();
	void RotateYPos();
	void RotateZNeg();
	void RotateZPos();
	void Take();
};

#endif // AuItemManipulator_H
