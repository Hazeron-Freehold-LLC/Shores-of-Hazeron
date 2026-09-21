//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aureportdialog.h,v 1.3 2022/02/19 18:40:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuReportDialog_H
#define AuReportDialog_H

#include <QDialog>

class AuReportDialog : public QDialog
{
	Q_OBJECT;
public:
	AuReportDialog(const QString& text, QWidget* parent=0);
	AuReportDialog(const QStringList& textLines, QWidget* parent=0);
	virtual ~AuReportDialog();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuReportDialog_H
