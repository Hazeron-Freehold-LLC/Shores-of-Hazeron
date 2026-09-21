//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: hashpack.h,v 1.9 2022/02/19 18:42:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include <QDataStream>
#include <QFile>
#include <QHash>

// Uncompresses a byte array and returns the result.
// 'bcomp' is the byte array to be uncompressed.
inline const QByteArray AuUncompress(const QByteArray& bcomp, const char* file, int line, const QString& hint)
{
	if(bcomp.isEmpty())
	{
		qWarning() << "warning: AuUncompress input data is empty" << file << "line" << line << "hint" << hint;
		return QByteArray();
	}

	QByteArray ba(qUncompress(bcomp));
	if(ba.isEmpty())
		qWarning() << "error: AuUncompress failed" << bcomp.size() << "bytes input data" << file << "line" << line
			<< "hint" << hint;
	return ba;
}

class HashpackTocEntry
{
private:
	unsigned int						m_CompressedFileSize;
	unsigned int						m_FileOffset;
	unsigned int						m_UncompressedFileSize;

public:
	HashpackTocEntry()
	{
		m_CompressedFileSize = 0;
		m_FileOffset = 0;
		m_UncompressedFileSize = 0;
	}

	HashpackTocEntry(unsigned int fileOffset, unsigned int fileSize, unsigned int compFileSize)
	{
		m_CompressedFileSize = compFileSize;
		m_FileOffset = fileOffset;
		m_UncompressedFileSize = fileSize;
	}

public:
	void operator>>(QDataStream& out) const
	{
		out << m_CompressedFileSize;
		out << m_FileOffset;
		out << m_UncompressedFileSize;
	}

	void operator<<(QDataStream& in)
	{
		in >> m_CompressedFileSize;
		in >> m_FileOffset;
		in >> m_UncompressedFileSize;
	}

public:
	inline unsigned int CompressedFileSize() const
	{
		return m_CompressedFileSize;
	}

	inline unsigned int FileOffset() const
	{
		return m_FileOffset;
	}

	inline unsigned int UncompressedFileSize() const
	{
		return m_UncompressedFileSize;
	}
};

inline QDataStream& operator<<(QDataStream& out, const HashpackTocEntry& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, HashpackTocEntry& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class Hashpack
{
public:
	// Reads one file 'fileName' from a hashpack archive. Returns the compressed file data in 'fileData'.
	// Returns
	// 0 = ok
	// 1 = failed to open 'harFileName'.
	// 2 = failed to fine 'fileName' in the har file.
	static inline int ReadFile(const QString& harFileName, const QString fileName, QByteArray& fileData)
	{
		QFile harFile(harFileName);
		if(!harFile.open(QIODevice::ReadOnly))
			return 1;

		QDataStream in(&harFile);
		in.setVersion(QDataStream::Qt_4_1);

		// Read the offset to the table of contents.
		unsigned int tocOffset;
		in >> tocOffset;

		// Read the compressed toc.
		in.device()->seek(tocOffset);
		QByteArray comptoc;
		in >> comptoc;
		QByteArray batoc;
		if(!comptoc.isEmpty())
			batoc = AuUncompress(comptoc, __FILE__, __LINE__, "toc");

		// Read the toc.
		QHash<QString, HashpackTocEntry> toc;   // Key is file name, value is offset in file.
		QDataStream stoc(batoc);
		stoc.setVersion(QDataStream::Qt_4_1);
		stoc >> toc;

		// Read the compressed files.
		QHash<QString, HashpackTocEntry>::const_iterator ti = toc.find(fileName);
		if(ti == toc.cend())
			return 2;

		in.device()->seek((*ti).FileOffset());
		in >> fileData;
		harFile.close();
		return 0;
	}

	// Reads all files from a hashpack archive. Returns the compressed file data in 'files'.
	// Returns
	// 0 = ok
	// 1 = failed to open 'harFileName'.
	static inline int ReadFiles(const QString& harFileName, QHash<QString, QByteArray>& files)
	{
		QFile harFile(harFileName);
		if(!harFile.open(QIODevice::ReadOnly))
			return 1;

		QDataStream in(&harFile);
		in.setVersion(QDataStream::Qt_4_1);

		// Read the offset to the table of contents.
		unsigned int tocOffset;
		in >> tocOffset;

		// Read the compressed toc.
		in.device()->seek(tocOffset);
		QByteArray comptoc;
		in >> comptoc;
		QByteArray batoc;
		if(!comptoc.isEmpty())
			batoc = AuUncompress(comptoc, __FILE__, __LINE__, "toc");

		// Read the toc.
		QHash<QString, HashpackTocEntry> toc;   // Key is file name, value is offset in file.
		QDataStream stoc(batoc);
		stoc.setVersion(QDataStream::Qt_4_1);
		stoc >> toc;

		// Read the compressed files.
		for(QHash<QString, HashpackTocEntry>::const_iterator ti = toc.cbegin(); ti != toc.cend(); ti++)
		{
			QByteArray v;
			in.device()->seek((*ti).FileOffset());
			in >> v;
			files.insert(ti.key(), v);
		}
		harFile.close();
		return 0;
	}
};

