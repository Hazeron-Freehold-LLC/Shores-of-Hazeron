//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audefaultspacecraft.h,v 1.7 2022/02/19 17:55:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDefaultSpacecraft_H
#define AuDefaultSpacecraft_H

#include "auclientlib.h"
#include "audialog.h"

class QComboBox;
class QLabel;

class AUCLIENT_DLL AuDefaultSpacecraft : public AuDialog
{
	Q_OBJECT;
private:
	quint32								m_DesignId;			// Design selected to buy, if accepted.
	QString								m_DesignName;		// Design selected to buy, if accepted.
	quint8								m_DesignQ;

	// Widgets
	QComboBox*							m_MakeWhat;

public:
	AuDefaultSpacecraft(QWidget* parent=0);
	virtual ~AuDefaultSpacecraft();

	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	inline const QString& DesignName() const
	{
		return m_DesignName;
	}

	inline quint8 DesignQ() const
	{
		return m_DesignQ;
	}

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListDesigns();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void MakeOne();
};

#endif // AuDefaultSpacecraft_H
