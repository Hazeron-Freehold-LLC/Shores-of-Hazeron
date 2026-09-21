//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auutil.cpp,v 1.7 2022/02/19 18:42:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auutil.h"
//
#include "auversion.h"
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QTcpSocket>

AuUtil::AuUtil()
{
	//qDebug() << "AuUtil::AuUtil Init";
	Q_INIT_RESOURCE(AuUtil);
	//qDebug() << "AuUtil::AuUtil Ready";
}

//static
const char* AuUtil::LoginVersion()
{
	return g_LoginVersion;
}

//static
const char* AuUtil::SceneVersion()
{
	return g_SceneVersion;
}

//static
const char* AuUtil::Version()
{
	return g_Version;
}

//static
QString AuUtil::VersionFull()
{
	return QString("Version %1%2%3, %4").arg(Version()).arg(LoginVersion()).arg(SceneVersion()).arg(__DATE__);
}

//static
const char* AuUtil::CopyrightYear()
{
	return g_CopyrightYear;
}

//static
quint16 AuUtil::NetCryptKey(quint16 keyBase)
{
	quint16 cryptKey;
	if(keyBase)
	{
		// A non-zero base value was passed. This is used to muddle the version-based key.
		cryptKey = keyBase + (quint16)((*g_LoginVersion << 8) | *g_Version);
		if(!cryptKey)
			cryptKey = keyBase;
	}
	else
	{
		// No base value was passed. Use the pure version-based key.
		cryptKey = (quint16)((*g_LoginVersion << 8) | *g_Version);
		if(!cryptKey)
			cryptKey = (quint16)(('a' << 8) | 'u');
	}
	return cryptKey;
}

//static
int AuUtil::WritePacketSize(QDataStream& ds, quint32 pktSize)
{
	switch(PacketSizeBytes(pktSize))
	{
	case 1:
		ds << (quint8)pktSize;
		return 1;
	case 2:
		{
			quint8 B1 = (quint8)((pktSize >> 8) | 0x40);		// 1 extra size byte.
			quint8 B2 = (quint8)(pktSize & 0xff);
			ds << B1;
			ds << B2;
		}
		return 2;
	case 3:
		{
			quint8 B1 = (quint8)((pktSize >> 16) | 0x80);		// 2 extra size bytes.
			quint8 B2 = (quint8)((pktSize >> 8) & 0xff);
			quint8 B3 = (quint8)(pktSize & 0xff);
			ds << B1;
			ds << B2;
			ds << B3;
		}
		return 3;
	case 4:
	default:
		{
			quint8 B1 = (quint8)((pktSize >> 24) | 0xc0);		// 3 extra size bytes.
			quint8 B2 = (quint8)((pktSize >> 16) & 0xff);
			quint8 B3 = (quint8)((pktSize >> 8) & 0xff);
			quint8 B4 = (quint8)(pktSize & 0xff);
			ds << B1;
			ds << B2;
			ds << B3;
			ds << B4;
		}
		return 4;
	}
}

//static
int AuUtil::WritePacketSize(QByteArray& buffer, quint32 pktSize)
{
	QDataStream ds(&buffer, QIODevice::ReadWrite);
	ds.setVersion(QDataStream::Qt_4_1);
	ds.skipRawData(buffer.size());								// Position to the end of any data already in the buffer.
	return WritePacketSize(ds, pktSize);
}

//static
int AuUtil::WritePacketSize(QTcpSocket& socket, quint32 pktSize)
{
	QByteArray ba;
	WritePacketSize(ba, pktSize);
	return socket.write(ba);
}

//static
int AuUtil::ReadPacketSize(QDataStream& in, quint32& pktSize, int& pktSizeBytesLeft)
{
	if(pktSizeBytesLeft)
	{
		// We are waiting for the remaining bytes of a multibyte block size to arrive. We have already read the first byte.
		// All of the remaining bytes of the packet size value are ready to be read.
		if(pktSizeBytesLeft == 1)
		{
			// Read the second byte.
			quint8 B2;
			in >> B2;
			pktSize = (pktSize << 8) | B2;
			pktSizeBytesLeft = 0;
			return 2;				// We read 2 total bytes.
		}
		else if(pktSizeBytesLeft == 2)
		{
			// Read the second and third bytes.
			quint8 B2, B3;
			in >> B2;
			in >> B3;
			pktSize = (pktSize << 16) | ((quint32)B2 << 8) | B3;
			pktSizeBytesLeft = 0;
			return 3;				// We read 3 total bytes.
		}
		else
		{
			// Read the second, third and fourth bytes.
			quint8 B2, B3, B4;
			in >> B2;
			in >> B3;
			in >> B4;
			pktSize = (pktSize << 24) | ((quint32)B2 << 16) | ((quint32)B3 << 8) | B4;
			pktSizeBytesLeft = 0;
			return 4;				// We read 4 total bytes.
		}
	}
	else
	{
		// Read the first byte of the packet size. There can be 1 to 4 packet size bytes.
		quint8 B1;
		in >> B1;
		pktSize = B1 & 0x3f;			// Mask off top two bits to get the first packet size byte; there could be up to three more.
		pktSizeBytesLeft = B1 >> 6;		// Top two bits tell us how many additional packet size bytes remain to be read.
		return 1;						// We read one byte.
	}
}

//static
int AuUtil::ReadPacketSize(QTcpSocket& socket, quint32& pktSize, int& pktSizeBytesLeft)
{
	//qDebug() << "AuUtil::ReadPacketSize" << pktSize << pktSizeBytesLeft << "avail" << socket.bytesAvailable();
	QDataStream in(&socket);
	in.setVersion(QDataStream::Qt_4_1);
	if(pktSizeBytesLeft)
	{
		// We are waiting for the remaining bytes of a multibyte block size to arrive. We have already read the first byte.
		// Insure the bytes available are enough to read the remaining bytes of the block size.
		if(socket.bytesAvailable() < pktSizeBytesLeft)
			return 1;					// We have still only read one byte so far.
		else
		{
			// All of the remaining bytes of the packet size value are ready to be read.
			if(pktSizeBytesLeft == 1)
			{
				// Read the second byte.
				quint8 B2;
				in >> B2;
				pktSize = (pktSize << 8) | B2;
				pktSizeBytesLeft = 0;
				return 2;				// We read 2 total bytes.
			}
			else if(pktSizeBytesLeft == 2)
			{
				// Read the second and third bytes.
				quint8 B2, B3;
				in >> B2;
				in >> B3;
				pktSize = (pktSize << 16) | ((quint32)B2 << 8) | B3;
				pktSizeBytesLeft = 0;
				return 3;				// We read 3 total bytes.
			}
			else
			{
				// Read the second, third and fourth bytes.
				quint8 B2, B3, B4;
				in >> B2;
				in >> B3;
				in >> B4;
				pktSize = (pktSize << 24) | ((quint32)B2 << 16) | ((quint32)B3 << 8) | B4;
				pktSizeBytesLeft = 0;
				return 4;				// We read 4 total bytes.
			}
		}
	}
	else
	{
		// Read the first byte of the packet size. There can be 1 to 4 packet size bytes.
		quint8 B1;
		in >> B1;
		pktSize = B1 & 0x3f;			// Mask off top two bits to get the first packet size byte; there could be up to three more.
		pktSizeBytesLeft = B1 >> 6;		// Top two bits tell us how many additional packet size bytes remain to be read.
		//qDebug() << "AuUtil::ReadPacketSize B1" << B1 << pktSize << pktSizeBytesLeft << "bytes" << socket.bytesAvailable();
		return 1;						// We read one byte.
	}
}
