//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommannounce.h,v 1.6 2022/02/19 17:54:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommAnnounce_H
#define AuCommAnnounce_H

#include "auclientlib.h"
#include <audialog.h>
#include "auid.h"

class QComboBox;
class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuCommAnnounce : public AuDialog
{
	Q_OBJECT;
private:
	AuId								m_SystemId;				// Id of solar system containing the scene atom.

	// Widgets
	QPushButton*						m_Announce;
	QLineEdit*							m_Comments;
	QComboBox*							m_Identifiers;
	QComboBox*							m_Loca;
	QComboBox*							m_Postures;
	QComboBox*							m_Repairs;

public:
	AuCommAnnounce(const QString& answerWho, QWidget* parent=0);
	virtual ~AuCommAnnounce();

	QString MessageText() const;

	const AuId& SystemId() const
	{
		return m_SystemId;
	}

public slots:
	virtual void setVisible(bool visible);

protected slots:

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuCommAnnounce_H
