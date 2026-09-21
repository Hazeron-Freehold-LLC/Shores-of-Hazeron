//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausettings.h,v 1.13 2022/02/19 18:31:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUSETTINGS_H
#define AUSETTINGS_H

#include "augloballib.h"
#include <QColor>
#include <QFont>
#include <QSettings>
#include <QStringList>

class AUGLOBAL_DLL AuSettings : public QSettings
{
public:
	AuSettings(QSettings::Format format, const QString& organization, const QString& application=QString(), QObject* parent=0);
	~AuSettings();

	bool readBoolEntry(const QString& key, bool def) const;
	QColor readColorEntry(const QString& key, const QColor& def) const;
	QString readEntry(const QString& key, const QString& def=QString()) const;
	void readEntry(const QString& key, QByteArray& ba);
	QFont readFontEntry(const QString& key, const QFont& def) const;
	QList<int> readIntListEntry(const QString& key);
	int readNumEntry(const QString& key, int def=0) const;
	unsigned int readUInt(const QString& key, unsigned int def=0) const;
	double readNumEntry(const QString& key, double def) const;
	QStringList readStringListEntry(const QString& key);

	void writeEntry(const QString& key, const QByteArray& ba);
	void writeEntry(const QString& key, const QColor& color);
	void writeEntry(const QString& key, const QFont& font);
	void writeEntry(const QString& key, bool value);
	void writeEntry(const QString& key, double value);
	void writeEntry(const QString& key, int value);
	void writeEntry(const QString& key, unsigned int value);
	void writeEntry(const QString& key, const QString& value);
	void writeEntry(const QString& key, const QList<int>& value);
	void writeEntry(const QString& key, const QStringList& value);
};

#endif // AUSETTINGS_H
