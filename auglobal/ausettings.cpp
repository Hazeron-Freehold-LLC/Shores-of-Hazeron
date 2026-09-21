//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausettings.cpp,v 1.14 2022/02/19 18:33:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausettings.h"
//

//static bool trace = false;	// Set this to 'true' to get debugs of all reads and writes.

AuSettings::AuSettings(QSettings::Format format, const QString& organization, const QString& application,
		QObject* parent) : QSettings(format, QSettings::UserScope, organization, application, parent)
{
}

AuSettings::~AuSettings()
{
}

// Returns empty array if not found.
QList<int> AuSettings::readIntListEntry(const QString& key)
{
	QVariant var = QSettings::value(key);
	QList<QVariant> varList = var.toList();

	QList<int> intList;
	for(QList<QVariant>::const_iterator vli = varList.constBegin(); vli != varList.constEnd(); vli++)
		intList.append((*vli).toInt());

	return intList;
}

// Returns empty array if not found.
QStringList AuSettings::readStringListEntry(const QString& key)
{
	QVariant var = QSettings::value(key);
	return var.toStringList();
}

QString AuSettings::readEntry(const QString& key, const QString& def/*=QString()*/) const
{
	QVariant var = QSettings::value(key, QVariant(def));
	//if(trace)
		//qDebug("AuSettings::readEntry %s=%s", (const char*)key.toAscii(), (const char*)var.toString().toAscii());
	return var.toString();
}

QColor AuSettings::readColorEntry(const QString& key, const QColor& def) const
{
	QVariant var = QSettings::value(key, QVariant(def));
	//if(trace)
		//qDebug("AuSettings::readColorEntry %s=%s", (const char*)key.toAscii(), (const char*)var.toString().toAscii());
	return var.value<QColor>();
}

QFont AuSettings::readFontEntry(const QString& key, const QFont& def) const
{
	QVariant var = QSettings::value(key, QVariant(def));
	//if(trace)
		//qDebug("AuSettings::readFontEntry %s=%s", (const char*)key.toAscii(), (const char*)var.toString().toAscii());
	return var.value<QFont>();
}

void AuSettings::readEntry(const QString& key, QByteArray& ba)
{
	QVariant var = QSettings::value(key);
	ba = var.toByteArray();
}

bool AuSettings::readBoolEntry(const QString& key, bool def) const
{
	QVariant var = QSettings::value(key, QVariant(def));
	//if(trace)
		//qDebug("AuSettings::readBoolEntry %s=%c",
				//(const char*)key.toAscii(), var.toBool()?'t':'f');
	return var.toBool();
}

unsigned int AuSettings::readUInt(const QString& key, unsigned int def/*=0*/) const
{
	QVariant var = QSettings::value(key, QVariant(def));
	//if(trace)
		//qDebug("AuSettings::readNumEntry %s=%d", (const char*)key.toAscii(), var.toUInt());
	return var.toUInt();
}

int AuSettings::readNumEntry(const QString& key, int def/*=0*/) const
{
	QVariant var = QSettings::value(key, QVariant(def));
	//if(trace)
		//qDebug("AuSettings::readNumEntry %s=%d",
				//(const char*)key.toAscii(), var.toInt());
	return var.toInt();
}

double AuSettings::readNumEntry(const QString& key, double def) const
{
	QVariant var = QSettings::value(key, QVariant(def));
	//if(trace)
		//qDebug("AuSettings::readNumEntry %s=%g",
				//(const char*)key.toAscii(), var.toDouble());
	return var.toDouble();
}

void AuSettings::writeEntry(const QString& key, bool value)
{
	//if(trace)
		//qDebug("AuSettings::writeEntry %s=%c",
				//(const char*)key.toAscii(), value?'t':'f');
	QVariant var(value);
	QSettings::setValue(key, var);
}

void AuSettings::writeEntry(const QString& key, double value)
{
	//if(trace)
		//qDebug("AuSettings::writeEntry %s=%g", (const char*)key.toAscii(), value);
	QVariant var(value);
	QSettings::setValue(key, var);
}

void AuSettings::writeEntry(const QString& key, unsigned int value)
{
	//if(trace)
		//qDebug("AuSettings::writeEntry %s=%d", (const char*)key.toAscii(), value);
	QVariant var(value);
	QSettings::setValue(key, var);
}

void AuSettings::writeEntry(const QString& key, int value)
{
	//if(trace)
		//qDebug("AuSettings::writeEntry %s=%d", (const char*)key.toAscii(), value);
	QVariant var(value);
	QSettings::setValue(key, var);
}

void AuSettings::writeEntry(const QString& key, const QString& value)
{
	//if(trace)
		//qDebug("AuSettings::writeEntry %s=%s", (const char*)key.toAscii(), (const char*)value.toAscii());
	QVariant var(value);
	QSettings::setValue(key, var);
}

void AuSettings::writeEntry(const QString& key, const QList<int>& value)
{
	//if(trace)
	//{
		//qDebug("AuSettings::writeEntry %s=(", (const char*)key.toAscii());
		//for(QList<int>::const_iterator it=value.constBegin(); it != value.constEnd(); it++)
			//qDebug("AuSettings::writeEntry %d", *it);
		//qDebug("AuSettings::writeEntry )");
	//}
	QList<QVariant> var;
	for(QList<int>::const_iterator vi = value.constBegin(); vi != value.constEnd(); vi++)
		var.append(QVariant(*vi));
	QSettings::setValue(key, var);
}

void AuSettings::writeEntry(const QString& key, const QStringList& value)
{
	//if(trace)
	//{
		//qDebug("AuSettings::writeEntry %s=(", (const char*)key.toAscii());
		//for(QStringList::const_iterator it=value.constBegin(); it != value.constEnd(); it++)
			//qDebug("AuSettings::writeEntry %s", (const char*)(*it).toAscii());
		//qDebug("AuSettings::writeEntry )");
	//}
	QVariant var(value);
	QSettings::setValue(key, var);
}

void AuSettings::writeEntry(const QString& key, const QByteArray& ba)
{
	QVariant var(ba);
	QSettings::setValue(key, var);
}

void AuSettings::writeEntry(const QString& key, const QColor& color)
{
	QVariant var(color);
	QSettings::setValue(key, var);
}

void AuSettings::writeEntry(const QString& key, const QFont& font)
{
	QVariant var(font);
	QSettings::setValue(key, var);
}
