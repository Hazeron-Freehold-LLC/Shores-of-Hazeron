//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitfinalizedlg.h,v 1.3 2022/05/05 16:34:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItFinalizeDlg_H
#define AuItFinalizeDlg_H

#include "audialog.h"

class AuDesign;
class AuId;
class QPushButton;
class QTextBrowser;

class AuItFinalizeDlg : public AuDialog
{
	Q_OBJECT;
private:
	bool								m_Assembly;				// On accept, true if assembly, false if design.
	AuDesign*							m_Design;				// Clone of the one passed in the constructor.
	quint32								m_DesignId;				// When specified by an agent.

	// Widgets
	QPushButton*						m_DoAssembly;
	QPushButton*						m_DoDesign;
	QTextBrowser*						m_Verification;

public:
	AuItFinalizeDlg(const AuDesign& design, const QImage& thumb, const AuId& archId, const QString& archName, QWidget* parent=0);
	virtual ~AuItFinalizeDlg();

	inline AuDesign* Design()
	{
		return m_Design;
	}

	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	inline bool IsAssembly() const
	{
		return m_Assembly;
	}

	QString Report() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoAssembly();
	void DoItem();
};

#endif // AuItFinalizeDlg_H
