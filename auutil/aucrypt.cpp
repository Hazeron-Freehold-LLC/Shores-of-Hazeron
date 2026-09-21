//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucrypt.cpp,v 1.8 2022/04/08 19:14:59 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucrypt.h"
//
#include <QHostInfo>

//const int HOST_NAME_MAX = 255;

// Encrypts/decrypts this array of bytes.
// The number of bytes in the array will not change.
// 'wKey' is used to encrypt the data; it must not be zero.
// Calling this function a second time on data that is already encrypted
// will decrypt the data, if the same wKey is used.
// The data can be encrypted in multiple passes, as long as it is decrypted
// in the same way.
void AuCrypt::Crypt(short int wKey)
{
	Q_ASSERT(wKey != 0);			// The encryption key must not be zero.
	if (wKey == 0)
		return;					// Encrypting with a zero key does not change the data.

	unsigned char bSeed = (unsigned char)((wKey >> 8) & 0x0f);
	unsigned char bInc = (unsigned char)(wKey & 0x0f);
	for(Iterator it = begin(); it != end(); it++)
	{
		(*it) ^= bSeed;	// XOR with the current seed value.
		bSeed += bInc;	// Increment the seed value for the next byte to be encrypted.
	}
}

// Stores the encrypted contents of a string into this byte array. 
// The contents of this byte array are replaced by the encrypted contents of the string.
void AuCrypt::EncryptString(const QString &sEncrypt, short int wKey)
{
	unsigned int uiChars = sEncrypt.length();
	resize(uiChars);
	Iterator it = begin();
	for (unsigned int uiChar=0; uiChar<uiChars; uiChar++)
	{
		(*it) = sEncrypt[uiChar].toLatin1();
		it++;
	}
	Crypt(wKey);
}

// Returns the decrypted contents of this byte array as a string. 
// The contents of this byte array are not changed.
QString AuCrypt::DecryptString(short int wKey) const
{
	AuCrypt bDecrypt(*this);
	bDecrypt.Crypt(wKey);	// Decrypt the data.
	QString sDecrypt;
	for(ConstIterator it = bDecrypt.constBegin(); it != bDecrypt.constEnd(); it++)
		sDecrypt += (*it);
	return(sDecrypt);
}

// Hostname included in algorithm.
// Includes the computer name so the encrypted string can't be easily copied to another computer.
void AuCrypt::EncryptLocalString(const QString &sEncrypt, short int wKey)
{
	// Build the local version of the string to encrypt by including our hostname.
	QString localstr;
	QString host = QHostInfo::localHostName();
	if(!host.isEmpty())
		localstr = host + sEncrypt;	// Insert hostname at front of encrypted string.
	else
		localstr = QString("\\") + sEncrypt;	// Failed to get hostname so sub with bogus hostname.

	// Encrypt the local version of the string.
	EncryptString(localstr, wKey);
}

// Returns QString() if hostname incorrect.
QString AuCrypt::DecryptLocalString(short int wKey) const
{
	// Decrypt the string.
	QString localstr = DecryptString(wKey);

	// The computer name was included so the encrypted string can't be easily copied to another computer.
	QString host = QHostInfo::localHostName();
	if(host.isEmpty())
		host = "\\";	// Failed to get hostname so sub with bogus hostname.

	// The hostname was inserted at the front of the encrypted string so it's easy to
	// compare and strip now.
	if(localstr.length() >= host.length() && localstr.left(host.length()) == host)
		return localstr.mid(host.length());			// Strip the hostname and return the rest. Blank is valid.
	else
		return QString();						// The hostname was incorrect so return blank.
}

void AuCrypt::EncryptByteArray(const QByteArray& ba, short int wKey)
{
	clear();
	append(ba);
	Crypt(wKey);
}

// Stores the encrypted contents of a pointer into this byte array. 
// The contents of this byte array are replaced by the encrypted contents of the pointer.
void AuCrypt::EncryptBytes(const unsigned char *pByte, int lBytes, short int wKey)
{
	resize(lBytes);
	Iterator it = begin();
	memcpy(it, pByte, lBytes);
	Crypt(wKey);
}

// Returns the decrypted contents of this byte array into a pointer. 
// The contents of this byte array are not changed.
void AuCrypt::DecryptBytes(unsigned char *pByte, int lBytes, short int wKey) const
{
	int lSafeBytes = size();
	if(lSafeBytes > lBytes)
		lSafeBytes = lBytes;
	AuCrypt bDecrypt(*this);
	bDecrypt.Crypt(wKey);	// Decrypt the data.
	ConstIterator it = begin();
	memcpy(pByte, it, lSafeBytes);
}

// Returns the unsigned long checksum of adding the bytes in the array.
unsigned int AuCrypt::ByteSum() const
{
	unsigned int uiCheckSum = 0;
	for(ConstIterator it = begin(); it != end(); it++)
		uiCheckSum += (*it);	// Accumulate the sum of the bytes.
	return(uiCheckSum);
}

/*
// Very inefficient.
// Each byte is formatted as a hex pair and appended to a string.
QString AuCrypt::ToASCII() const
{
	QString str;
	QString sch;
	for(ConstIterator it = begin(); it != end(); it++)
	{
		unsigned char ch = (*it);
		sch.sprintf("%02x", ch);
		str += sch;
	}

	return str;
}

// Converts ToASCII formatted string s back into a byte array.
void AuCrypt::FromASCII(const QString& s)
{
	int isize = s.length()/2;
	Q_ASSERT(isize*2 == s.length());	// Odd length string is a problem.
	if(isize*2 != s.length())	// Odd length string is a problem.
		return;

	// Resize this array to receive the decoded data.
	resize(isize);

	// Stuff the array by decoding the string one byte at a time.
	Iterator it = begin();
	int ch;
	QString sch;
	for(int ich=0; ich<s.length(); ich+=2)
	{
		sch = s.mid(ich, 2);
#if defined(Q_OS_WIN)
		sscanf_s((const char*)sch.toLatin1(), "%02x", &ch);
#else
		sscanf((const char*)sch.toLatin1(), "%02x", &ch);
#endif
		(*it) = (unsigned char)ch;
		it++;
	}
}
*/

