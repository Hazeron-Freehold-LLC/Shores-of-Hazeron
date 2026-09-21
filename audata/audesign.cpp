//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesign.cpp,v 1.67 2022/02/19 18:20:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesign.h"
//
#include "aubddesign.h"
#include "aucrypt.h"
#include "augleffects.h"
#include "auglobal.h"
#include "auitdesign.h"
#include "auscdesign.h"
#include "auscparts.h"
#include "auscreport.h"
#include "autexturelib.h"
#include "dpbody.h"
#include "hashpack.h"

#define SOH_FILE_VER_UNCOMPRESSED		0x01
#define SOH_FILE_VER					0x02

#define DESIGN_SERIALIZE_VER			0x04					// At present, this must fit into a nibble.

static GLuint s_TexFace = 0;
static GLuint s_TexVertex = 0;

bool AuDesignPosition::IsValid() const
{
	if(!DpBody::CanRestorePositionPose(m_Pose))
		return false;
	if(m_HeadTilt > PIf/2 || m_HeadTilt < -PIf/2)
		return false;
	if(m_EnvM == AuMatrix4x4())
        return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuDesignTex::IsValid() const
{
	if(m_Image.isNull())
		return m_TexLib <= AuTextureLibItem::LibraryTextureCount();
	else
		return m_Image.width() <= MaxImageDimension() && m_Image.height() <= MaxImageDimension();
}

//static
int AuDesignTex::MaxImageDimension()
{
	return 256;
}

quint64 AuDesignTex::MemoryUsage() const
{
	quint64 mem = 0;
#if(QT_VERSION >= QT_VERSION_CHECK(5,10,0))
	mem += m_Image.sizeInBytes();
#else
	mem += m_Image.byteCount();
#endif
	mem += sizeof(m_ImageName);
	mem += sizeof(m_TexLib);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
AuDesign* AuDesign::Create(BLUEPRINT_TYPE designType)
{
	switch(designType)
	{
	case BLUEPRINT_Invalid:
		return 0;
	case BLUEPRINT_Building:
		return new AuBdDesign();
	case BLUEPRINT_Item:
		return new AuItDesign();
	case BLUEPRINT_Spacecraft:
		return new AuScDesign();
	case BLUEPRINT_Count:
		break;
	}
	qWarning() << "error: AuDesign::Create invalid type" << designType;
	return 0;
}

//static
QString AuDesign::StateName(STATE state)
{
	Q_ASSERT(state <= FinalAssembly);
	QString stateNames[] =
	{
		QObject::tr("Unfinished"),
		QObject::tr("Final"),
		QObject::tr("Deleted"),
		QObject::tr("Assembly"),
	};
	return stateNames[state];
}

AuDesign::AuDesign(unsigned int idDesign, const AuId& idArchitect, const QString& architect) : m_ArchId(idArchitect),
	m_ArchName(architect)
{
	Q_ASSERT(idDesign);
	m_Id = idDesign;
	m_State = Edit;
}

AuDesign::AuDesign()
{
	m_Id = 0;
	m_State = Edit;
}

AuDesign::~AuDesign()
{
}

AuDesign::AuDesign(const AuDesign &c) : m_GridPos(c.m_GridPos), m_ViewPos(c.m_ViewPos)
{
	m_ArchId = c.m_ArchId;
	m_ArchName = c.m_ArchName;
	m_Description = c.m_Description;
	m_Id = c.m_Id;
	m_Name = c.m_Name;
	m_State = c.m_State;
	for(int tp=0; tp < SCTEXI_Count; tp++)
		m_TexturePalette[tp] = c.m_TexturePalette[tp];
	m_Thumbnail = c.m_Thumbnail;
}

AuDesign& AuDesign::operator=(const AuDesign &c)
{
	m_ArchId = c.m_ArchId;
	m_ArchName = c.m_ArchName;
	m_Description = c.m_Description;
	m_GridPos = c.m_GridPos;
	m_Id = c.m_Id;
	m_Name = c.m_Name;
	m_State = c.m_State;
	for(int tp=0; tp < SCTEXI_Count; tp++)
		m_TexturePalette[tp] = c.m_TexturePalette[tp];
	m_Thumbnail = c.m_Thumbnail;
	m_ViewPos = c.m_ViewPos;
	return *this;
}

void AuDesign::DesignLoaded(const AuDesign& design)
{
	for(int tp=0; tp < SCTEXI_Count; tp++)
		m_TexturePalette[tp] = design.m_TexturePalette[tp];
	m_Thumbnail = design.m_Thumbnail;
}

void AuDesign::WriteTextures(QDataStream& out) const
{
	out << (quint8)SCTEXI_Count;
	for(int tp=0; tp < SCTEXI_Count; tp++)
		out << m_TexturePalette[tp];
}

void AuDesign::SetThumbnail(const QImage& im)
{
	m_Thumbnail = im;
}

bool AuDesign::ReadTextures(QDataStream& in)
{
	quint8 cpal;
	in >> cpal;
	if(cpal != 15)
		return false;
	for(int c=0; c<cpal; c++)
		in >> m_TexturePalette[c];
	return true;
}

void AuDesign::ReadTexturesOld(QDataStream& in)
{
	// Old palette had only one custom texture, at SCTEXI_Custom.
	QImage custom;
	in >> custom;
	if(!custom.isNull())
		m_TexturePalette[0].SetImage(QObject::tr("Custom"), custom);
	else
		m_TexturePalette[0].SetTexLibId(AuTextureLibItem::LibraryTextureHullMetal());

	quint8 cpal;
	in >> cpal;
	for(int c=0; c<cpal; c++)
	{
		quint16 texLibId;
		in >> texLibId;
		m_TexturePalette[c+2].SetTexLibId(texLibId);	// First of the old palette was SCTEXI_User1.
	}
}

void AuDesign::operator>>(QDataStream& out) const
{
	out << m_Id;
	out << m_Name;
	out << m_ArchId;
	out << m_ArchName;

	quint8 verState = (DESIGN_SERIALIZE_VER << 4) | m_State;
	out << (quint8)verState;

	WriteTextures(out);
	out << m_Thumbnail;
	out << m_Description;

	// Saved grid and view positions are not stored in final drawings.
	if(m_State != Final && m_State != Deleted)
	{
		out << (quint8)m_GridPos.size();
		for(QHash<quint8, AuMatrix4x4>::const_iterator gi = m_GridPos.cbegin(); gi != m_GridPos.cend(); gi++)
		{
			out << gi.key();
			out << *gi;
		}

		out << (quint8)m_ViewPos.size();
		for(QHash<quint8, AuDesignPosition>::const_iterator vi = m_ViewPos.cbegin(); vi != m_ViewPos.cend(); vi++)
		{
			out << vi.key();
			out << *vi;
		}
	}
}

void AuDesign::operator<<(QDataStream& in)
{
	in >> m_Id;
	in >> m_Name;
	in >> m_ArchId;
	in >> m_ArchName;

	quint8 verState;
	in >> verState;
	m_State = (STATE)(verState & 0x0f);
	quint8 ver = verState >> 4;

	if(ver >= 0x02)
		ReadTextures(in);
	else
		ReadTexturesOld(in);

	if(ver >= 0x01)
		in >> m_Thumbnail;
	else
		m_Thumbnail = QImage();

	if(ver >= 0x03)
		in >> m_Description;

	if(ver >= 0x04)
	{
		// Saved grid and view positions are not stored in final drawings.
		m_GridPos.clear();
		m_ViewPos.clear();
		if(m_State != Final && m_State != Deleted)
		{
			quint8 c;
			in >> c;
			while(c--)
			{
				quint8 pid;
				AuMatrix4x4 pos;
				in >> pid;
				in >> pos;
				m_GridPos.insert(pid, pos);
			}

			in >> c;
			while(c--)
			{
				quint8 pid;
				AuDesignPosition pos;
				in >> pid;
				in >> pos;
				m_ViewPos.insert(pid, pos);
			}
		}
	}
}

void AuDesign::Tx(QDataStream& out) const
{
	out << (quint8)m_State;
	out << m_Id;
	out << m_Name;
	out << m_ArchId;
	out << m_ArchName;
	WriteTextures(out);
	out << m_Thumbnail;
	out << m_Description;

	// Saved view positions are not stored in final drawings.
	if(m_State != Final && m_State != Deleted)
	{
		out << (quint8)m_GridPos.size();
		for(QHash<quint8, AuMatrix4x4>::const_iterator gi = m_GridPos.cbegin(); gi != m_GridPos.cend(); gi++)
		{
			out << gi.key();
			out << *gi;
		}

		out << (quint8)m_ViewPos.size();
		for(QHash<quint8, AuDesignPosition>::const_iterator vi = m_ViewPos.cbegin(); vi != m_ViewPos.cend(); vi++)
		{
			out << vi.key();
			out << *vi;
		}
	}
}

void AuDesign::Rx(QDataStream& in)
{
	quint8 state;
	in >> state;
	m_State = (STATE)state;
	in >> m_Id;
	in >> m_Name;
	in >> m_ArchId;
	in >> m_ArchName;
	ReadTextures(in);
	in >> m_Thumbnail;
	in >> m_Description;

	// Saved view positions are not stored in final drawings.
	m_GridPos.clear();
	m_ViewPos.clear();
	if(m_State != Final && m_State != Deleted)
	{
		quint8 c;
		in >> c;
		while(c--)
		{
			quint8 pid;
			AuMatrix4x4 pos;
			in >> pid;
			in >> pos;
			m_GridPos.insert(pid, pos);
		}

		in >> c;
		while(c--)
		{
			quint8 pid;
			AuDesignPosition pos;
			in >> pid;
			in >> pos;
			m_ViewPos.insert(pid, pos);
		}
	}
}

void AuDesign::SetTexturePalette(const AuDesignTex texturePalette[SCTEXI_Count])
{
	for(int tpi=0; tpi<SCTEXI_Count; tpi++)
		m_TexturePalette[tpi] = texturePalette[tpi];
}

bool AuDesign::Validate()
{
	bool isValid = true;

	if(!m_Name.isEmpty() && !IsNameValid(m_Name))
	{
		qDebug() << "AuDesign::Validate designId" << m_Id << "invalid name" << m_Name;
		// Reduce to simplified version.
		m_Name = m_Name.simplified();

		// Insure length is not too short and not too long.
		if(m_Name.isEmpty())
			m_Name = "Noname";
		else if(m_Name.length() > MaxNameLength())
			m_Name = m_Name.left(MaxNameLength());

		// Replace invalid characters.
		m_Name.replace('|', '.');

		isValid = false;
	}

	if(m_State > FinalAssembly)
	{
		//qDebug() << "AuDesign::Validate designId" << m_Id << "invalid state" << m_State;
		m_State = Edit;
		isValid = false;
	}

	return isValid;
}

QByteArray AuDesign::Pack() const
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)Type();
	out << *this;
	return ba;
}

//static
AuDesign* AuDesign::Unpack(const QByteArray& ba)
{
	if(ba.isEmpty())
		return 0;

	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_4_1);
	quint8 designType;
	in >> designType;
	AuDesign* design = Create((BLUEPRINT_TYPE)designType);
	if(!design)
		return 0;

	in >> *design;

	return design;
}

bool AuDesign::SaveToFile(const QString& fileName, quint8 fileType) const
{
	QFile file(fileName);
	if(!file.open(QFile::WriteOnly))
		return false;

	// Store my data into an encrypted byte array.
	AuCrypt crypt;
	QDataStream dsc(&crypt, QIODevice::WriteOnly);
	dsc.setVersion(QDataStream::Qt_5_0);
	dsc << qCompress(Pack());			// Store compressed byte array.

	// Encrypt the byte array.
	quint16 cryptKey = (fileType << 8) | SOH_FILE_VER;
	crypt.Crypt(cryptKey);

	// Store the encrypted byte array into the file.
	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_5_0);

	ds << (quint8)fileType;
	ds << (quint8)SOH_FILE_VER;
	ds << crypt;

	file.close();
	return true;
}

//static
AuDesign* AuDesign::LoadFromFile(const QString& fileName, quint8 expectedFileType)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
		return 0;

	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_5_0);
	return LoadFromDataStream(ds, expectedFileType);
}

//static
AuDesign* AuDesign::LoadFromByteArray(const QByteArray& ba, quint8 expectedFileType)
{
	QDataStream ds(ba);
	ds.setVersion(QDataStream::Qt_5_0);
	return LoadFromDataStream(ds, expectedFileType);
}

//static
AuDesign* AuDesign::LoadFromDataStream(QDataStream& ds, quint8 expectedFileType)
{
	quint8 fileType, fileVer;
	ds >> fileType;
	ds >> fileVer;
	if(fileType != expectedFileType)
		return 0;

	AuDesign* designLoaded = 0;
	if(fileVer == SOH_FILE_VER)
	{
		// Read data into an encrypted byte array.
		AuCrypt crypt;
		ds >> crypt;

		// Decrypt the byte array.
		quint16 cryptKey = (fileType << 8) | fileVer;
		crypt.Crypt(cryptKey);

		// Retrieve my data from the byte array.
		QByteArray bac;
		QDataStream dsc(&crypt, QIODevice::ReadOnly);
		dsc.setVersion(QDataStream::Qt_5_0);
		dsc >> bac;

		if(!bac.isEmpty())
		{
			// Uncompress the byte array.
			// Expect failure with old version files that did not qCompress.
			QByteArray ba = AuUncompress(bac, __FILE__, __LINE__,"AuDesign::LoadFromDataStream");

			// Unpack a design from the byte array.
			designLoaded = Unpack(ba);
		}
	}
	else if(fileVer == SOH_FILE_VER_UNCOMPRESSED)
	{
		// Read data into an encrypted byte array.
		AuCrypt crypt;
		ds >> crypt;

		// Decrypt the byte array.
		quint16 cryptKey = (fileType << 8) | fileVer;
		crypt.Crypt(cryptKey);

		// Retrieve my data from the byte array.
		QByteArray ba;
		QDataStream dsc(&crypt, QIODevice::ReadOnly);
		dsc.setVersion(QDataStream::Qt_5_0);
		dsc >> ba;

		designLoaded = Unpack(ba);
	}
	return designLoaded;
}

void AuDesign::SetProperties(const AuDesign& design)
{
	m_Name = design.m_Name;
	m_Description = design.m_Description;
}

void AuDesign::RenderEdges(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re, const AuScSelection& sel,
		bool objectMode, bool isDragging, const AuMatrix4x4& dragT, bool dragCopy, bool objectIsolate)
{
	const AuScParts& parts = Parts();
	AuMatrix4x4 pmv = projection * mv;
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(pmv);
	re.GLSL().LineShader().SetColor(g_Au.DesignerLineColor());

	if(sel.empty())
	{
		// Display all parts in unselected state.
		AuPointArray edges;
		for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
			if(!(*hi)->IsHidden())
				(*hi)->EdgeSegments(edges);

		if(!edges.empty())
		{
			// Make temporary VAO of the edge segments.
			AuGLVAO vao;
			vao.CreateForLines(re, edges, false, GL_STREAM_DRAW);

			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}
	else
	{
		if(objectMode)
		{
			// Individual edges can be selected.
			if(isDragging)
			{
				// Individual parts can be transformed.
				AuPointArray edges, seledges;
				for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
				{
					if((*hi)->IsHidden())
						continue;
					AuScSelection::const_iterator si = sel.find(hi.key());
					if(si == sel.cend() || (*hi)->IsLocked())
					{
						// Unselected or locked part.
						if(objectIsolate)
							continue;
						(*hi)->EdgeSegments(edges);
					}
					else
					{
						// Part is selected so some edges may be selected and others not.
						AuScPart* c = (*hi)->Clone();
						c->Transform(dragT, *si);
						AuPointArray tedges;
						c->EdgeSegments(tedges);
						delete c;

						if(isDragging && dragCopy)
						{
							// The drag is for the purpose of copying, so all parts are shown in their current positions.
							(*hi)->EdgeSegments(edges);	// All untransformed edges into unselected list.

							quint32 edgeIndex = 0;
							for(AuPointArray::const_iterator ei = tedges.cbegin(); ei != tedges.cend(); ei+=2, edgeIndex++)
							{
								if((*si).m_Edges.contains(edgeIndex))
								{
									// Selected edges are put into the selected list.
									seledges.append(*ei);
									seledges.append(*(ei+1));
								}
							}
						}
						else
						{
							// The drag is for the purpose of moving, so only unselected parts are shown at their current positions.
							quint32 edgeIndex = 0;
							for(AuPointArray::const_iterator ei = tedges.cbegin(); ei != tedges.cend(); ei+=2, edgeIndex++)
							{
								if((*si).m_Edges.contains(edgeIndex))
								{
									// Selected edges are put into the selected list.
									seledges.append(*ei);
									seledges.append(*(ei+1));
								}
								else
								{
									// Unselected edges are put into the unselected list.
									edges.append(*ei);
									edges.append(*(ei+1));
								}
							}
						}
					}
				}

				if(!edges.empty())
				{
					// Make temporary VAO of the edge segments.
					AuGLVAO vao;
					vao.CreateForLines(re, edges, false, GL_STREAM_DRAW);

					// Display the VAO once and clean it up.
					vao.SelectGL();
					vao.DrawGL(GL_LINES);
					vao.DeselectGL();
					vao.DeleteGL();
				}

				if(!seledges.empty())
				{
					re.GLSL().LineShader().SetColor(g_Au.DesignerSelColor());

					// Make temporary VAO of the edge segments.
					AuGLVAO vao;
					vao.CreateForLines(re, seledges, false, GL_STREAM_DRAW);

					// Display the VAO once and clean it up.
					vao.SelectGL();
					vao.DrawGL(GL_LINES);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
			else
			{
				AuPointArray edges, seledges;
				for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
				{
					if((*hi)->IsHidden())
						continue;
					AuScSelection::const_iterator si = sel.find(hi.key());
					if(si == sel.cend())
					{
						// Unselected part.
						if(objectIsolate)
							continue;
						(*hi)->EdgeSegments(edges);
					}
					else
					{
						// Part is selected so it is subject to object mode.
						if(isDragging && (dragCopy || (*hi)->IsLocked()))
						{
							// The drag is for the purpose of copying, so all parts are shown in their current positions.
							(*hi)->EdgeSegments(edges);	// Unselected part.
						}
						else
						{
							// The drag is for the purpose of moving, so only unselected parts are shown at their current positions.
							AuPointArray alledges;
							(*hi)->EdgeSegments(alledges);	// Unselected part.
							quint32 edgeIndex = 0;
							for(AuPointArray::const_iterator ei = alledges.cbegin(); ei != alledges.cend(); ei+=2, edgeIndex++)
							{
								if((*si).m_Edges.contains(edgeIndex))
								{
									seledges.append(*ei);
									seledges.append(*(ei+1));
								}
								else
								{
									edges.append(*ei);
									edges.append(*(ei+1));
								}
							}
						}
					}
				}

				if(!edges.empty())
				{
					// Make temporary VAO of the edge segments.
					AuGLVAO vao;
					vao.CreateForLines(re, edges, false, GL_STREAM_DRAW);

					// Display the VAO once and clean it up.
					vao.SelectGL();
					vao.DrawGL(GL_LINES);
					vao.DeselectGL();
					vao.DeleteGL();
				}

				if(!seledges.empty())
				{
					re.GLSL().LineShader().SetColor(g_Au.DesignerSelColor());

					// Make temporary VAO of the edge segments.
					AuGLVAO vao;
					vao.CreateForLines(re, seledges, false, GL_STREAM_DRAW);

					// Display the VAO once and clean it up.
					vao.SelectGL();
					vao.DrawGL(GL_LINES);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
		}
		else
		{
			// Display unselected parts.
			AuPointArray edges;
			if(isDragging)
			{
				if(dragCopy)
				{
					// The drag is for the purpose of copying, so all parts are shown in their current positions.
					for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
						if(!(*hi)->IsHidden())
							(*hi)->EdgeSegments(edges);
				}
				else
				{
					// The drag is for the purpose of moving, so unselected and locked parts are shown at their current positions.
					for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
						if(!(*hi)->IsHidden())
							if((*hi)->IsLocked() || !sel.contains(hi.key()))
								(*hi)->EdgeSegments(edges);	// Unselected or locked part.
				}
			}
			else
			{
				// Not dragging so only unselected parts are shown at their current positions.
				for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
					if(!(*hi)->IsHidden())
						if(!sel.contains(hi.key()))
							(*hi)->EdgeSegments(edges);	// Unselected part.
			}

			if(!edges.empty())
			{
				// Make temporary VAO of the edge segments.
				AuGLVAO vao;
				vao.CreateForLines(re, edges, false, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			edges.clear();
			if(isDragging)
			{
				// Selected parts are being dragged.
				// Drag operation affects the entire part.
				// Display selected parts at their proposed new position.
				AuMatrix4x4 pmvs = pmv * dragT;
				re.GLSL().LineShader().SetProjectionModelView(pmvs);

				for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
					if(!(*hi)->IsHidden())
						if(!(*hi)->IsLocked() && sel.contains(hi.key()))
							(*hi)->EdgeSegments(edges);	// Render selected unlocked part.
			}
			else
			{
				// Selected parts are not being dragged.
				// Display selected parts.
				for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
					if(!(*hi)->IsHidden())
						if(sel.contains(hi.key()))
							(*hi)->EdgeSegments(edges);	// Render selected part.
			}

			if(!edges.empty())
			{
				re.GLSL().LineShader().SetColor(g_Au.DesignerSelColor());

				// Make temporary VAO of the edge segments.
				AuGLVAO vao;
				vao.CreateForLines(re, edges, false, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_LINES);
				vao.DeselectGL();
				vao.DeleteGL();
			}
		}
	}

	re.GLSL().LineShader().Deselect();
}

void AuDesign::RenderFaceSprites(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		const AuScSelection& sel, const AuMatrix4x4& dragT)
{
	if(sel.empty())
		return;

	const AuScParts& parts = Parts();
	if(!s_TexFace)
		s_TexFace = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ScFace", ":data/DesignerFaceSprite.png");

	QList<AuPointAlpha> selFaces;
	QList<AuPointAlpha> unselFaces;
	for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
	{
		if((*hi)->IsHidden())
			continue;
		AuScSelection::const_iterator si = sel.find(hi.key());
		if(si != sel.cend())
		{
			AuScPart* c = (*hi)->Clone();
			c->Transform(dragT, *si);
			c->FaceSpriteLocs((*si).m_Faces, selFaces, unselFaces);
			delete c;
		}
	}

	AuMatrix4x4 pmv = projection * mv;

	re.GLSL().SpriteActualSizeShader().Select();
	re.GLSL().SpriteActualSizeShader().SetProjectionModelView(pmv);
	re.GLSL().SpriteActualSizeShader().SetTexture(s_TexFace);

	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	re.glEnable(GL_PROGRAM_POINT_SIZE); 

	if(!unselFaces.empty())
	{
		re.GLSL().SpriteActualSizeShader().SetColor(g_Au.DesignerLineColor());

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForSprites(re, unselFaces, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_POINTS);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	if(!selFaces.empty())
	{
		re.GLSL().SpriteActualSizeShader().SetColor(g_Au.DesignerSelColor());

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForSprites(re, selFaces, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_POINTS);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	re.glDisable(GL_PROGRAM_POINT_SIZE); 
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteActualSizeShader().Deselect();
}

void AuDesign::RenderVertexSprites(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		const AuScSelection& sel, const AuMatrix4x4& dragT)
{
	if(sel.empty())
		return;

	const AuScParts& parts = Parts();
	if(!s_TexVertex)
		s_TexVertex = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ScVertex", ":data/DesignerVertexSprite.png");

	QList<AuPointAlpha> selVerts;
	QList<AuPointAlpha> unselVerts;
	for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
	{
		if((*hi)->IsHidden())
			continue;
		AuScSelection::const_iterator si = sel.find(hi.key());
		if(si != sel.cend())
		{
			AuScPart* c = (*hi)->Clone();
			c->Transform(dragT, *si);
			c->VertexSpriteLocs((*si).m_Vertices, selVerts, unselVerts);
			delete c;
		}
	}

	AuMatrix4x4 pmv = projection * mv;

	re.GLSL().SpriteActualSizeShader().Select();
	re.GLSL().SpriteActualSizeShader().SetProjectionModelView(pmv);
	re.GLSL().SpriteActualSizeShader().SetTexture(s_TexVertex);

	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	re.glEnable(GL_PROGRAM_POINT_SIZE); 

	if(!unselVerts.empty())
	{
		re.GLSL().SpriteActualSizeShader().SetColor(g_Au.DesignerLineColor());

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForSprites(re, unselVerts, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_POINTS);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	if(!selVerts.empty())
	{
		re.GLSL().SpriteActualSizeShader().SetColor(g_Au.DesignerSelColor());

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForSprites(re, selVerts, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_POINTS);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	re.glDisable(GL_PROGRAM_POINT_SIZE); 
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteActualSizeShader().Deselect();
}

void AuDesign::RenderEdgesDragging(const AuMatrix4x4& projection, const AuMatrix4x4& mv, AuGLEffects& re,
		const AuScSelection& sel, bool objectMode, const AuMatrix4x4& dragT, bool showVertexSprites, bool showFaceSprites)
{
	if(sel.empty())
		return;

	const AuScParts& parts = Parts();
	AuMatrix4x4 pmv = projection * mv;
	re.GLSL().LineShader().Select();

	AuPointArray edges;
	if(objectMode)
	{
		// Drag operation affects selected vertices of the selected part.
		// Display selected parts at their current position, with selected edges at their new positions.
		re.GLSL().LineShader().SetProjectionModelView(pmv);
		for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
		{
			if((*hi)->IsHidden())
				continue;
			if((*hi)->IsLocked())
				continue;
			AuScSelection::const_iterator si = sel.find(hi.key());
			if(si != sel.cend())
			{
				AuScPart* c = (*hi)->Clone();
				c->Transform(dragT, *si);
				c->EdgeSegments(edges);
				delete c;
			}
		}
	}
	else
	{
		// Drag operation affects the entire part.
		// Display selected parts at their proposed new position.
		re.GLSL().LineShader().SetProjectionModelView(pmv * dragT);

		for(AuScParts::const_iterator hi = parts.cbegin(); hi != parts.cend(); hi++)
			if(!(*hi)->IsHidden())
				if(!(*hi)->IsLocked() && sel.contains(hi.key()))
					(*hi)->EdgeSegments(edges);	// Render selected part.
	}

	if(!edges.empty())
	{
		re.GLSL().LineShader().SetColor(g_Au.DesignerSelColor());

		// Make temporary VAO of the edge segments.
		AuGLVAO vao;
		vao.CreateForLines(re, edges, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		vao.SelectGL();
		vao.DrawGL(GL_LINES);
		vao.DeselectGL();
		vao.DeleteGL();
	}

	re.GLSL().LineShader().Deselect();

	// Display selected vertices in object mode.
	if(!sel.empty() && objectMode)
	{
		if(showFaceSprites)
			RenderFaceSprites(projection, mv, re, sel, dragT);
		if(showVertexSprites)
			RenderVertexSprites(projection, mv, re, sel, dragT);
	}
}

quint16 AuDesign::FindEdgeAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& sel, bool objectMode,
		bool objectIsolate, AuPoint* snapLoc) const
{
	if(objectMode && objectIsolate)
		return Parts().FindEdgeAtPixel(mvp, pixel, sel, snapLoc);
	else
		return Parts().FindEdgeAtPixel(mvp, pixel, snapLoc);
}

quint16 AuDesign::FindVertexAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& sel, bool objectMode,
		bool objectIsolate, AuPoint* snapLoc) const
{
	if(objectMode && objectIsolate)
		return Parts().FindVertexAtPixel(mvp, pixel, sel, snapLoc);
	else
		return Parts().FindVertexAtPixel(mvp, pixel, snapLoc);
}

quint16 AuDesign::FindPartObjectAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, bool findVertices, bool findEdges,
		bool findFaces) const
{
	return Parts().FindPartObjectAtPixel(mvp, pixel, findVertices, findEdges, findFaces);
}

quint16 AuDesign::FindPartObjectAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, const AuScSelection& partIds,
		bool findVertices, bool findEdges, bool findFaces) const
{
	return Parts().FindPartObjectAtPixel(mvp, pixel, partIds, findVertices, findEdges, findFaces);
}

void AuDesign::FindPartsInArea(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelection& partIds, bool findVertices,
		bool findEdges, bool findFaces) const
{
	Parts().FindPartsInArea(mvp, pixelRect, partIds, findVertices, findEdges, findFaces);
}

void AuDesign::FindPartObjectsInArea(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelection& partIds, bool findVertices,
		bool findEdges, bool findFaces) const
{
	Parts().FindPartObjectsInArea(mvp, pixelRect, partIds, findVertices, findEdges, findFaces);
}

quint16 AuDesign::FindPart(quint8 partType) const
{
	return Parts().FindPart(partType);
}

quint16 AuDesign::ClosestRemoteDoorInRoom(bool forVehicle, const AuPoint& loc, quint16 roomId,
		const AuDoorStateHash& doorStates) const
{
	const AuScParts& parts = Parts();
	quint16 doorId = 0;
	quint16 bestid = 0;
	double bestdist2 = -1;
	AuScParts::const_iterator ri = parts.find(roomId);
	if(ri != parts.cend() && (*ri)->IsRoomVoid())
	{
		// Found the room void. Locate a hull door that makes a connection into the room void.
		for(AuDoorStateHash::const_iterator ci = doorStates.cbegin(); ci != doorStates.cend(); ci++)
		{
			if(forVehicle)
			{
				if(!(*ci).IsRemoteVehicle())
					continue;
			}
			else
			{
				if(!(*ci).IsRemoteSpacecraft())
					continue;
			}
			// Locate a path segment associated with the open state of the hull door.
			bool doorPathIntoRoom = false;
			for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if(!(*pi)->IsPath())
					continue;
				if((*pi)->AssociatedPartId() != ci.key())
					continue;	// Path is not associated with the door.
				if((*pi)->AssociatedPartState() != AuScDoor::Open)
					continue;	// Path is not associated with the open state of the door.
				const AuScPathSeg& seg = (*pi)->PathSegment(0);
				if(!(*ri)->IsInsideVoid(seg.m_Seg.m_fLoc1) &&
						!(*ri)->IsInsideVoid(seg.m_Seg.m_fLoc2))
					continue;	// Neither end of the path is in the 'roomId' room.
				doorPathIntoRoom = true;
				break;
			}
			if(!doorPathIntoRoom)
				continue;
			double dist2 = loc.Distance2((*ci).OpLoc1());
			if(bestdist2 < 0 || dist2 < bestdist2)
			{
				bestdist2 = dist2;
				bestid = ci.key();
			}

			dist2 = loc.Distance2((*ci).OpLoc2());
			if(bestdist2 < 0 || dist2 < bestdist2)
			{
				bestdist2 = dist2;
				bestid = ci.key();
			}
		}
	}
	doorId = bestid;
	return doorId;
}

void AuDesign::PartIds(AuScSelection& partIds) const
{
	Parts().Ids(partIds);
}

void AuDesign::PartIdsNotOmitted(AuScSelection& partIds) const
{
	Parts().IdsNotOmitted(partIds);
}

void AuDesign::PartEdgeIds(AuScSelection& partIds) const
{
	const AuScParts& parts = Parts();
	for(AuScSelection::iterator si = partIds.begin(); si != partIds.end(); si++)
	{
		(*si).m_Edges.clear();

		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi != parts.cend())
			(*pi)->EdgeIds((*si).m_Edges);
	}
}

void AuDesign::PartFaceIds(AuScSelection& partIds) const
{
	const AuScParts& parts = Parts();
	for(AuScSelection::iterator si = partIds.begin(); si != partIds.end(); si++)
	{
		(*si).m_Faces.clear();

		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi != parts.cend())
			(*pi)->FaceIds((*si).m_Faces);
	}
}

void AuDesign::PartObjectIds(AuScSelection& partIds) const
{
	const AuScParts& parts = Parts();
	for(AuScSelection::iterator si = partIds.begin(); si != partIds.end(); si++)
	{
		(*si).m_Edges.clear();
		(*si).m_Faces.clear();
		(*si).m_Vertices.clear();

		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi != parts.cend())
		{
			(*pi)->EdgeIds((*si).m_Edges);
			(*pi)->FaceIds((*si).m_Faces);
			(*pi)->VertexIds((*si).m_Vertices);
		}
	}
}

void AuDesign::PartVertexIds(AuScSelection& partIds) const
{
	const AuScParts& parts = Parts();
	for(AuScSelection::iterator si = partIds.begin(); si != partIds.end(); si++)
	{
		(*si).m_Vertices.clear();

		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi != parts.cend())
			(*pi)->VertexIds((*si).m_Vertices);
	}
}

QMimeData* AuDesign::CreateMIME(const AuId& architectId, bool objectMode, const AuScSelection& partIds,
		const AuMatrix4x4& designPlane) const
{
	return Parts().CreateMIME(architectId, objectMode, partIds, designPlane);
}

QString AuDesign::PartSelectedDesc(quint16 partId) const
{
	return Parts().SelectedDesc(partId);
}

quint16 AuDesign::PartAdd(const AuScPart& part, AuScParts& partsAdded)
{
	if(!part.IsValid())
		return 0;

	// Some parts must match module with existing part of same system. i.e. shields and sensors.
	AuScParts& parts = Parts();
	if(part.IsShieldStation())
	{
		// Shield stations all have to share the same module. There is only one shield system.
		COMMODITYID mod = parts.ShieldModule();
		if(mod != CID_Invalid && mod != part.Module())
		{
			// The shield system module has already been established and it is different from the new part's module.
			// Set the module on the new part to match the existing system module.
			AuScPart* c = part.Clone();
			c->SetModule(mod);
			quint16 partId = parts.AddPart(*c, partsAdded);
			delete c;
			return partId;
		}
	}
	else if(part.IsSensorStation())
	{
		// Sensor stations all have to share the same module. There is only one sensor system.
		COMMODITYID mod = parts.SensorModule();
		if(mod != CID_Invalid && mod != part.Module())
		{
			// The sensor system module has already been established and it is different from the new part's module.
			// Set the module on the new part to match the existing system module.
			AuScPart* c = part.Clone();
			c->SetModule(mod);
			quint16 partId = parts.AddPart(*c, partsAdded);
			delete c;
			return partId;
		}
	}

	// We can attempt to add the part as is.
	quint16 partId = parts.AddPart(part, partsAdded);
	return partId;
}

bool AuDesign::PartObjectAdd(quint16 partId, const AuScPart& part, AuScParts& partsChanged)
{
	if(!part.IsValid())
		return false;

	AuScParts& parts = Parts();
	if(!parts.AddPartObjects(partId, part, partsChanged))
		return false;
	return true;
}

bool AuDesign::PartsCopy(const AuScSelection& partIds, AuScParts& partsAdded)
{
	AuScParts& parts = Parts();
	bool anyCopied = false;
	for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi != parts.cend())
		{
			if((*pi)->IsHidden())
				continue;

			if(PartAdd(**pi, partsAdded))
				anyCopied = true;
			else
				qWarning() << "error: AuDesign::PartsCopy part could not be copied" << (*pi)->PartName();
		}
		else
			qWarning() << "error: AuDesign::PartsCopy part not found id" << si.key();
	}
	if(!anyCopied)
		return false;
	return true;
}

bool AuDesign::PartsCopySymmetrical(const AuScSelection& partIds, const AuMatrix4x4& gridPlane, AuScParts& partsAdded)
{
	AuScParts& parts = Parts();
	bool anyCopied = false;
	for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi != parts.cend())
		{
			if((*pi)->IsHidden())
				continue;

			// Make a symmetrical clone across the X axis.
			AuScPart* pc = (*pi)->Clone();
			AuMatrix4x4 s(gridPlane);
			s.Scale(1,-1,1);
			s *= gridPlane.Inverse();
			pc->Transform(s);
			pc->ReverseFaces();

			if(PartAdd(*pc, partsAdded))
				anyCopied = true;
			else
				qWarning() << "error: AuDesign::PartsCopySymmetrical part could not be copied" << (*pi)->PartName();
			delete pc;
		}
		else
			qWarning() << "error: AuDesign::PartsCopySymmetrical part not found id" << si.key();
	}
	if(!anyCopied)
		return false;
	return true;
}

bool AuDesign::PartsTransform(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged, bool objectMode)
{
	if(t.IsIdentity())
		return false;

	AuScParts& parts = Parts();
	bool anyChanged = false;
	if(objectMode)
	{
		for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
		{
			AuScParts::const_iterator pi = parts.find(si.key());
			if(pi == parts.cend())
				continue;

			if((*pi)->IsLocked() || (*pi)->IsHidden())
				continue;

			// Store part prior to change, for undo.
			partsChanged.insert(pi.key(), (*pi)->Clone());

			(*pi)->Transform(t, *si);
			anyChanged = true;
		}
	}
	else
	{
		for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
		{
			AuScParts::const_iterator pi = parts.find(si.key());
			if(pi == parts.cend())
				continue;

			if((*pi)->IsLocked() || (*pi)->IsHidden())
				continue;

			// Store part prior to change, for undo.
			partsChanged.insert(pi.key(), (*pi)->Clone());

			(*pi)->Transform(t);
			anyChanged = true;
		}
	}
	if(!anyChanged)
		return false;
	return true;
}

bool AuDesign::PartsTransformCopy(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged)
{
	if(t.IsIdentity())
		return false;

	AuScParts& parts = Parts();
	bool anyChanged = false;
	for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi == parts.cend())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->TransformCopy(t, *si);
		anyChanged = true;
	}
	if(!anyChanged)
		return false;
	return true;
}

bool AuDesign::PartsTransformLiftStops(const AuScSelection& partIds, const AuMatrix4x4& t, AuScParts& partsChanged)
{
	if(t.IsIdentity())
		return false;

	AuScParts& parts = Parts();
	bool anyChanged = false;
	for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi == parts.cend())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;
		if(!(*pi)->IsTurboLiftMesh())
			continue;

		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->TransformLiftStop(t);
		anyChanged = true;
	}
	if(!anyChanged)
		return false;
	return true;
}

bool AuDesign::PartDelete(quint16 partId, AuScParts& partsDeleted)
{
	if(!Parts().DeletePart(partId, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartObjectDelete(quint16 partId, const AuScSelectInfo& selInfo, AuScParts& partsChanged, AuScParts& partsDeleted)
{
	if(!Parts().DeletePartObjects(partId, selInfo, partsChanged, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartUpdate(quint16 partId, const AuScPart& part, AuScParts& partsUpdated)
{
	if(!Parts().UpdatePart(partId, part, partsUpdated))
		return false;
	return true;
}

void AuDesign::UndoRedoAdd(const AuScParts& parts)
{
	Parts().UndoRedoAdd(parts);
}

void AuDesign::UndoRedoDelete(const QList<quint16>& partIds)
{
	Parts().UndoRedoDelete(partIds);
}

void AuDesign::UndoRedoSwap(AuScParts& swapParts)
{
	Parts().UndoRedoSwap(swapParts);
}

bool AuDesign::SelectPartObjectsAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuScSelection& sel, bool findVertices,
		bool findEdges, bool findFaces) const
{
	return Parts().SelectPartObjectsAtPixel(mvp, pixel, sel, findVertices, findEdges, findFaces);
}

bool AuDesign::DeselectPartObjectsAtPixel(const AuMatrix4x4& mvp, const QPointF& pixel, AuScSelection& sel, bool findVertices,
		bool findEdges, bool findFaces) const
{
	return Parts().DeselectPartObjectsAtPixel(mvp, pixel, sel, findVertices, findEdges, findFaces);
}

bool AuDesign::SelectPartTextureVertices(quint16 partId, quint32 partVertex, AuScSelection& sel) const
{
	return Parts().SelectPartTextureVertices(partId, partVertex, sel);
}

bool AuDesign::DeselectPartTextureVertices(quint16 partId, quint32 partVertex, AuScSelection& sel) const
{
	return Parts().DeselectPartTextureVertices(partId, partVertex, sel);
}

void AuDesign::InvertPartObjectSelection(AuScSelection& sel) const
{
	AuScSelection allPartIds(sel);
	allPartIds.ClearObjectSelections(); 						// vertices, edges and faces
	PartObjectIds(allPartIds);									// Get all vertex ids of the selected parts.
	sel.InvertObjectSelection(allPartIds);
}

void AuDesign::TogglePartObjectSelectionInArea(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelection& sel) const
{
	AuScSelection toggle(sel);
	toggle.ClearObjectSelections();						    // vertices, edges and faces
	FindPartObjectsInArea(mvp, pixelRect, toggle, g_Au.DesignerFindVertices(), g_Au.DesignerFindEdges(), g_Au.DesignerFindFaces());
	sel.TogglePartObjectSelection(toggle);
}

bool AuDesign::PartFacesFracture(const AuScSelection& sel, AuScParts& partsChanged)
{
	return Parts().FractureFaces(sel, partsChanged);
}

bool AuDesign::PartFacesFuse(const AuScSelection& sel, AuScParts& partsChanged)
{
	return Parts().FuseFaces(sel, partsChanged);
}

bool AuDesign::PartFacesReverse(const AuScSelection& sel, AuScParts& partsChanged)
{
	return Parts().ReverseFaces(sel, partsChanged);
}

bool AuDesign::PartFacesDivide(const AuScSelection& sel, quint8 divisions, AuScParts& partsChanged)
{
	return Parts().DivideFaces(sel, divisions, partsChanged);
}

bool AuDesign::PartsJigCut(const AuId& archId, const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsChanged,
		AuScParts& partsDeleted)
{
	return Parts().JigCutParts(archId, sel, partsAdded, partsChanged, partsDeleted);
}

bool AuDesign::PartsJoin(quint16 targetPart, const AuScSelection& sel, AuScParts& partsChanged, AuScParts& partsDeleted)
{
	return Parts().JoinParts(targetPart, sel, partsChanged, partsDeleted);
}

bool AuDesign::PartsSplit(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsChanged)
{
	return Parts().SplitParts(sel, partsAdded, partsChanged);
}

bool AuDesign::PartVerticesFlatten(const AuScSelection& sel, AuScParts& partsChanged)
{
	return Parts().FlattenVertices(sel, partsChanged);
}

bool AuDesign::PartVerticesSmooth(const AuScSelection& sel, AuScParts& partsChanged)
{
	return Parts().SmoothVertices(sel, partsChanged);
}

bool AuDesign::PartSetLight(quint16 partId, const QColor& color, float kL, float kQ, float cosOuterCone, float cosInnerCone,
			AuScParts& partsChanged)
{
	return Parts().SetLight(partId, color, kL, kQ, cosOuterCone, cosInnerCone, partsChanged);
}

bool AuDesign::PartSetHidden(quint16 partId, bool hide, AuScParts& partsChanged)
{
	return Parts().SetHidden(partId, hide, partsChanged);
}

bool AuDesign::PartSetLocked(quint16 partId, bool lock, AuScParts& partsChanged)
{
	return Parts().SetLocked(partId, lock, partsChanged);
}

bool AuDesign::PartSetStopName(quint16 partId, const QString& name, AuScParts& partsChanged)
{
	return Parts().SetStopName(partId, name, partsChanged);
}

bool AuDesign::PartSetOpacity(quint16 partId, float opacity, AuScParts& partsChanged)
{
	return Parts().SetOpacity(partId, opacity, partsChanged);
}

bool AuDesign::PartSetState(quint16 partId, quint8 state, AuScParts& partsChanged)
{
	return Parts().SetState(partId, state, partsChanged);
}

bool AuDesign::PartAddState(quint16 partId, quint8 state, AuScParts& partsChanged)
{
	return Parts().AddState(partId, state, partsChanged);
}

void AuDesign::Faces(AuScFaceList& faces) const
{
	AuScSelection sel;
	PartIds(sel);
	PartFaceIds(sel);
	Faces(faces, sel);
}

void AuDesign::FacesNotOmitted(AuScFaceList& faces) const
{
	AuScSelection sel;
	PartIdsNotOmitted(sel);
	PartFaceIds(sel);
	Faces(faces, sel);
}

void AuDesign::Faces(AuScFaceList& faces, const AuScSelection& sel) const
{
	Parts().Faces(faces, sel);
}

void AuDesign::CutFaces(const AuScSelection& sel, const AuPointArray& cutLine, AuScParts& partsChanged)
{
	Parts().CutFaces(sel, cutLine, partsChanged);
}

void AuDesign::PunchFaces(const AuScSelection& sel, const AuPointArray& cutBnd, AuScParts& partsChanged)
{
	Parts().PunchFaces(sel, cutBnd, partsChanged);
}

void AuDesign::Loop(const AuScSelection& sel, const AuPoint& loc, AuPointArray& loop) const
{
	Parts().Loop(sel, loc, loop);
}

bool AuDesign::SetTurretBase(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis, AuScParts& partsChanged)
{
	return Parts().SetTurretBase(sel, basePivot, baseAxis, partsChanged);
}

bool AuDesign::SetTurretGun(const AuScSelection& sel, const AuPoint& gunPivot, const AuPoint& gunAxis, const AuPoint& gunEmitLoc,
		const AuPoint& gunEmitVec, const AuPoint& gunSightLoc, AuScParts& partsChanged)
{
	return Parts().SetTurretGun(sel, gunPivot, gunAxis, gunEmitLoc, gunEmitVec, gunSightLoc, partsChanged);
}

bool AuDesign::SetTurretStation(const AuScSelection& sel, quint8 gunnerPos, const AuPoint& gunnerLoc, const AuPoint& gunnerFacing,
		const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane, COMMODITYID module, AuScParts& partsChanged)
{
	return Parts().SetTurretStation(sel, gunnerPos, gunnerLoc, gunnerFacing, corner1, corner2, gridPlane, module, partsChanged);
}

bool AuDesign::PartAddStop(quint16 partId, const QString& stop, AuScParts& partsChanged)
{
	return Parts().AddStop(partId, stop, partsChanged);
}

bool AuDesign::PartDeleteStop(quint16 partId, const QString& stop, AuScParts& partsChanged)
{
	return Parts().DeleteStop(partId, stop, partsChanged);
}

bool AuDesign::PartSetStop(quint16 partId, const QString& stop, AuScParts& partsChanged)
{
	return Parts().SetStop(partId, stop, partsChanged);
}

QString AuDesign::PartStop(quint16 partId) const
{
	return Parts().TurboLiftAtStop(partId);
}

void AuDesign::PartStops(quint16 partId, QStringList& stations) const
{
	Parts().Stops(partId, stations);
}

bool AuDesign::PartVerticesEffectIntensity(const AuScSelection& sel, float intensity, AuScParts& partsChanged)
{
	return Parts().EffectIntensityVertices(sel, intensity, partsChanged);
}

bool AuDesign::PartVerticesGlow(const AuScSelection& sel, const QColor& glow, AuScParts& partsChanged)
{
	return Parts().GlowVertices(sel, glow, partsChanged);
}

bool AuDesign::PartFacesPaint(const AuScSelection& sel, const QColor& paint, AuScParts& partsChanged)
{
	return Parts().PaintFaces(sel, paint, partsChanged);
}

bool AuDesign::PartFacesPaint(const AuScSelection& sel, quint8 theme, quint8 opacity, AuScParts& partsChanged)
{
	return Parts().PaintFaces(sel, (AuScFace::COLOR_THEME)theme, opacity, partsChanged);
}

bool AuDesign::PartFacesPolish(const AuScSelection& sel, float shine, float reflectivity, AuScParts& partsChanged)
{
	return Parts().PolishFaces(sel, shine, reflectivity, partsChanged);
}

bool AuDesign::PartsSetFaceSelfIllumination(const AuScSelection& sel, float selfIllum, AuScParts& partsChanged)
{
	return Parts().SetFaceSelfIllumination(sel, selfIllum, partsChanged);
}

bool AuDesign::PartsSetFaceOpacity(const AuScSelection& sel, float opacity, AuScParts& partsChanged)
{
	return Parts().SetFaceOpacity(sel, opacity, partsChanged);
}

bool AuDesign::PartsSetRoomProperties(const AuScSelection& sel, bool isSunlit, INTERIOR_LIGHT initLighting,
		AuScParts& partsChanged)
{
	return Parts().SetRoomProperties(sel, isSunlit, initLighting, partsChanged);
}

bool AuDesign::PartsSetDoorProperties(const AuScSelection& sel, Qt::CheckState isAutomatic, Qt::CheckState initLocked,
		Qt::CheckState remoteSpacecraft, Qt::CheckState remoteVehicle, AuScParts& partsChanged)
{
	const AuScParts& parts = Parts();
	bool anyChanged = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi == parts.cend())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		if(!(*pi)->IsDoor())
			continue;

		bool hullDoor = LooksLikeHullDoor(pi.key());
		if(hullDoor == (*pi)->IsHullDoor() &&
				(isAutomatic == Qt::PartiallyChecked || (isAutomatic == Qt::Checked) == (*pi)->IsAutomaticDoor()) &&
				(initLocked == Qt::PartiallyChecked || (initLocked == Qt::Checked) == (*pi)->IsInitiallyLocked()) &&
				(remoteSpacecraft == Qt::PartiallyChecked || (remoteSpacecraft == Qt::Checked) ==(*pi)->IsRemoteSpacecraftDoor()) &&
				(remoteVehicle == Qt::PartiallyChecked || (remoteVehicle == Qt::Checked) == (*pi)->IsRemoteVehicleDoor()))
			continue;	// No change.

		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetHullDoor(hullDoor);							// Does nothing if this is not a change.
		(*pi)->SetDoorProperties(isAutomatic, initLocked, remoteSpacecraft, remoteVehicle);
		anyChanged = true;
	}
	return anyChanged;
}

bool AuDesign::PartsSetSiteProperties(const AuScSelection& sel, quint8 grade, quint8 grass, quint8 pavement, quint8 rocks,
		quint8 shrubs, quint8 terBlend, quint8 trees, AuScParts& partsChanged)
{
	const AuScParts& parts = Parts();
	bool anyChanged = false;
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi == parts.cend())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		if(!(*pi)->IsSite())
			continue;
		AuScSite* site = dynamic_cast<AuScSite*>(*pi);
		if(!site)
			continue;	// wut?

		if((grade == 255 || grade == site->Grade()) &&
				(grass == 255 || grass == site->Grass()) &&
				(pavement == 255 || pavement == site->Pavement()) &&
				(rocks == 255 || rocks == site->Rocks()) &&
				(shrubs == 255 || shrubs == site->Shrubs()) &&
				(terBlend == 255 || terBlend == site->TerrainBlend()) &&
				(trees == 255 || trees == site->Trees()))
			continue;	// No change.

		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetSiteProperties(grade, grass, pavement, rocks, shrubs, terBlend, trees);
		anyChanged = true;
	}
	return anyChanged;
}

bool AuDesign::PartsSetLightBulbProperties(const AuScSelection& sel, const QColor& color, float kL, float kQ, float cosOuterCone,
		float cosInnerCone, AuScParts& partsChanged)
{
	return Parts().SetLightBulbProperties(sel, color, kL, kQ, cosOuterCone, cosInnerCone, partsChanged);
}

bool AuDesign::PartsSetLightLensProperties(const AuScSelection& sel, const QColor& color, AuScParts& partsChanged)
{
	return Parts().SetLightLensProperties(sel, color, partsChanged);
}

bool AuDesign::PartsSetVoidProperties(const AuScSelection& sel, GRAVITY gravityType, const AuPoint& gravityUp,
		HOLD_ACCESS holdAccess, quint8 personnelAccess, AuScParts& partsChanged)
{
	return Parts().SetVoidProperties(sel, gravityType, gravityUp, holdAccess, personnelAccess, partsChanged);
}

bool AuDesign::PartsTextureUnwrap(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize,
		quint8 unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane,
		AuScParts& partsChanged)
{
	return Parts().TextureUnwrap(sel, texIndex, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane, partsChanged);
}

bool AuDesign::PartsTextureMove(const AuScSelection& sel, const QPointF& vec, AuScParts& partsChanged)
{
	return Parts().TextureMove(sel, vec, partsChanged);
}

bool AuDesign::PartsTextureRotate(const AuScSelection& sel, const QPointF& pivot, double angle, AuScParts& partsChanged)
{
	return Parts().TextureRotate(sel, pivot, angle, partsChanged);
}

bool AuDesign::PartsTextureStretch(const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY,
		AuScParts& partsChanged)
{
	return Parts().TextureStretch(sel, base, scaleX, scaleY, partsChanged);
}

bool AuDesign::PartsTextureFaces(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, AuScParts& partsChanged)
{
	return Parts().TextureFaces(sel, texIndex, partsChanged);
}

bool AuDesign::PartsTextureFacesCylindrical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	return Parts().TextureFacesCylindrical(sel, texIndex, texRepsAtEquator, texSize, gridPlane, partsChanged);
}

bool AuDesign::PartsTextureFacesProject(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	return Parts().TextureFacesProject(sel, texIndex, texSize, gridPlane, partsChanged);
}

bool AuDesign::PartsTextureFacesSpherical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	return Parts().TextureFacesSpherical(sel, texIndex, texRepsAtEquator, gridPlane, partsChanged);
}

bool AuDesign::PartsEffectUnwrap(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize,
		quint8 unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane,
		AuScParts& partsChanged)
{
	return Parts().EffectUnwrap(sel, effect, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane, partsChanged);
}

bool AuDesign::PartsEffectMove(const AuScSelection& sel, const QPointF& vec, AuScParts& partsChanged)
{
	return Parts().EffectMove(sel, vec, partsChanged);
}

bool AuDesign::PartsEffectRotate(const AuScSelection& sel, const QPointF& pivot, double angle, AuScParts& partsChanged)
{
	return Parts().EffectRotate(sel, pivot, angle, partsChanged);
}

bool AuDesign::PartsEffectStretch(const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY,
		AuScParts& partsChanged)
{
	return Parts().EffectStretch(sel, base, scaleX, scaleY, partsChanged);
}

bool AuDesign::PartsEffectFaces(const AuScSelection& sel, SURFACE_EFFECT effect, AuScParts& partsChanged)
{
	return Parts().EffectFaces(sel, effect, partsChanged);
}

bool AuDesign::PartsEffectFacesCylindrical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	return Parts().EffectFacesCylindrical(sel, effect, texRepsAtEquator, texSize, gridPlane, partsChanged);
}

bool AuDesign::PartsEffectFacesProject(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	return Parts().EffectFacesProject(sel, effect, texSize, gridPlane, partsChanged);
}

bool AuDesign::PartsEffectFacesSpherical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane, AuScParts& partsChanged)
{
	return Parts().EffectFacesSpherical(sel, effect, texRepsAtEquator, gridPlane, partsChanged);
}

SURFACE_EFFECT AuDesign::Effect(const AuScSelection& sel) const
{
	return Parts().Effect(sel);
}

SC_TEXTURE_INDEX AuDesign::TextureIndex(const AuScSelection& sel) const
{
	return Parts().TextureIndex(sel);
}

bool AuDesign::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const
{
	return Parts().IsPathBlocked(fromLoc, toLoc, hitLoc, hitNormal);
}

bool AuDesign::PartsAssociate(const AuScSelection& partIds, AuScParts& partsChanged)
{
	// Look for the target of the association.
	AuScParts& parts = Parts();
	const AuScPart* target = 0;
	quint16 targetId = 0;
	const AuScPart* roomTarget = 0;
	quint16 roomTargetId = 0;
	const AuScPart* roomVoidTarget = 0;
	quint16 roomVoidTargetId = 0;
	bool lifeSupportStatus = false;
	bool genericMesh = false;
	const AuScPart* berthTarget = 0;
	quint16 berthTargetId = 0;
	bool berthName = false;
	for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi == parts.cend())
			continue;

		if((*pi)->IsHidden())
			continue;

		if((*pi)->IsRoomVoid())
		{
			roomVoidTarget = *pi;
			roomVoidTargetId = pi.key();
		}
		else if((*pi)->IsRoomMesh())
		{
			roomTarget = *pi;
			roomTargetId = pi.key();
		}
		else if((*pi)->IsBerth())
		{
			berthTarget = *pi;
			berthTargetId = pi.key();
		}
		else if((*pi)->Type() == AuScPart::Part_StatusLifeSupport)
			lifeSupportStatus = true;	// A life support status display is selected.
		else if((*pi)->Type() == AuScPart::Part_Mesh)
			genericMesh = true;			// A generic mesh is selected.
		else if((*pi)->Type() == AuScPart::Part_BerthName)
			berthName = true;			// A berth name decal is selected.

		// We are looking for a berth, door, landing gear, control station, turbo lift, or room void.
		if(!(*pi)->IsDoor() &&
				!(*pi)->IsLandingGear() &&
				!(*pi)->IsControlStation() &&
				!(*pi)->IsTurboLiftMesh())
			continue;

		// Don't do anything if we find more than one target.
		if(targetId)
			return false;
		target = *pi;
		targetId = pi.key();
	}

	// If no target was found, look for a room void to use as a target. It could have been the subject of the association.
	// Life support status displays are associated with room voids to show the air pressure there.
	// Generic meshes are associated with room voids to be lit according to in-room lighting.
	if(!target)
	{
		if(berthTarget && berthName)
		{
			target = berthTarget;
			targetId = berthTargetId;
		}
		else if(roomTarget && genericMesh)
		{
			target = roomTarget;
			targetId = roomTargetId;
		}
		else if(roomVoidTarget && lifeSupportStatus)
		{
			target = roomVoidTarget;
			targetId = roomVoidTargetId;
		}
	}

	// Can't continue if we did not find a target.
	if(!target)
		return false;

	// If the target cannot change states, this won't matter. Service panels do not care about the state of the associated station.
	quint8 targetState = target->State();

	// Associate all selected parts with the targetId.
	bool anyAssociated = false;
	for(AuScSelection::const_iterator si = partIds.begin(); si != partIds.cend(); si++)
	{
		if(si.key() == targetId)
			continue;	// Do not associate a part with itself.
		AuScParts::iterator pi = parts.find(si.key());
		if(pi == parts.end())
			continue;

		if((*pi)->IsHidden())
			continue;
		if((*pi)->IsLocked())
			continue;

		if((*pi)->AssociatedPartId() == targetId && (*pi)->AssociatedPartState() == targetState)
			continue;

		if(target == berthTarget && (*pi)->Type() != AuScPart::Part_BerthName)
			continue;	// Only berth name decals can be associated with berths.

		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetAssociatedPart(targetId, targetState);
		anyAssociated = true;
	}
	return anyAssociated;
}

bool AuDesign::PartsDissociate(const AuScSelection& partIds, AuScParts& partsChanged)
{
	// Associate all selected paths and voids with the targetId.
	AuScParts& parts = Parts();
	bool anyDissociated = false;
	for(AuScSelection::const_iterator si = partIds.begin(); si != partIds.cend(); si++)
	{
		AuScParts::iterator pi = parts.find(si.key());
		if(pi == parts.end())
			continue;

		if((*pi)->IsHidden())
			continue;
		if((*pi)->IsLocked())
			continue;

		if(!(*pi)->AssociatedPartId() && !(*pi)->AssociatedPartState())
			continue;

		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetAssociatedPart(0, 0);
		anyDissociated = true;
	}
	return anyDissociated;
}

COMMODITYID AuDesign::PartModule(quint16 partId) const
{
	return Parts().Module(partId);
}

QString AuDesign::PartReportDesc(quint16 partId) const
{
	return Parts().ReportDesc(partId);
}

quint16 AuDesign::GroundVehicleParkingSpots() const
{
	return Parts().GroundVehicleParkingSpots();
}

void AuDesign::GroundVehicleParkingSpots(QList<AuMatrix4x4>& spots) const
{
	Parts().GroundVehicleParkingSpots(spots);
}

void AuDesign::GroundVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const
{
	Parts().GroundVehicleParkingSpots(roomId, spots);
}

quint16 AuDesign::RocketParkingSpots() const
{
	return Parts().RocketParkingSpots();
}

void AuDesign::RocketParkingSpots(QList<AuMatrix4x4>& spots) const
{
	Parts().RocketParkingSpots(spots);
}

void AuDesign::RocketParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const
{
	Parts().RocketParkingSpots(roomId, spots);
}

quint16 AuDesign::SpacecraftParkingSpots() const
{
	return Parts().SpacecraftParkingSpots();
}

void AuDesign::SpacecraftParkingSpots(QList<AuMatrix4x4>& spots) const
{
	Parts().SpacecraftParkingSpots(spots);
}

quint16 AuDesign::SpaceVehicleParkingSpots() const
{
	return Parts().SpaceVehicleParkingSpots();
}

void AuDesign::SpaceVehicleParkingSpots(QList<AuMatrix4x4>& spots) const
{
	Parts().SpaceVehicleParkingSpots(spots);
}

void AuDesign::SpaceVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const
{
	Parts().SpaceVehicleParkingSpots(roomId, spots);
}

quint16 AuDesign::WaterVehicleParkingSpots() const
{
	return Parts().WaterVehicleParkingSpots();
}

void AuDesign::WaterVehicleParkingSpots(QList<AuMatrix4x4>& spots) const
{
	Parts().WaterVehicleParkingSpots(spots);
}

void AuDesign::WaterVehicleParkingSpots(quint16 roomId, QList<AuMatrix4x4>& spots) const
{
	Parts().WaterVehicleParkingSpots(roomId, spots);
}

void AuDesign::ServicePanels(AuScReportServicePanelHash& tables) const
{
	Parts().ServicePanels(tables);
}

void AuDesign::StatusDisplays(AuScReportStatusDisplayHash& tables) const
{
	Parts().StatusDisplays(tables);
}

void AuDesign::GameTables(AuScReportGameTableHash& tables) const
{
	Parts().GameTables(tables);
}

quint16 AuDesign::CaptBerths() const
{
	return Parts().CaptBerths();
}

quint16 AuDesign::Beds() const
{
	return Parts().Beds();
}

quint16 AuDesign::CrewBerths() const
{
	return Parts().CrewBerths();
}

quint16 AuDesign::OfficerBerths() const
{
	return Parts().OfficerBerths();
}

quint16 AuDesign::PassengerBerths() const
{
	return Parts().PassengerBerths();
}

quint16 AuDesign::TroopBerths() const
{
	return Parts().TroopBerths();
}

quint16 AuDesign::Turrets() const
{
	return Parts().Turrets();
}

void AuDesign::CommandStations(AuScPartSet& stations) const
{
	Parts().CommandStations(stations);
}

void AuDesign::ControlStations(AuScPartSet& stations) const
{
	Parts().ControlStations(stations);
}

void AuDesign::DesignerStations(AuScPartSet& stations) const
{
	Parts().DesignerStations(stations);
}

void AuDesign::Turrets(AuScPartSet& turrets) const
{
	Parts().Turrets(turrets);
}

void AuDesign::VehicleLaunchers(AuScPartSet& launchers) const
{
	Parts().VehicleLaunchers(launchers);
}

void AuDesign::VehicleLandingSystems(AuScPartSet& vls) const
{
	Parts().VehicleLandingSystems(vls);
}

GRAVITY AuDesign::QueryGravity(const AuPoint& loc, AuPoint& floorLoc, AuPoint& gravityN, AuPoint& terN) const
{
	return Parts().QueryGravity(loc, floorLoc, gravityN, terN);
}

COMMODITYID AuDesign::SensorModule() const
{
	return Parts().SensorModule();
}

COMMODITYID AuDesign::ShieldModule() const
{
	return Parts().ShieldModule();
}

COMMODITYID AuDesign::TurretModule(quint16 partId) const
{
	const AuScParts& parts = Parts();
	AuScParts::const_iterator pi = parts.find(partId);
	if(pi == parts.cend())
		return CID_Invalid;
	return (*pi)->Module();
}

COMMODITYID AuDesign::BayModule(quint16 partId) const
{
	const AuScParts& parts = Parts();
	AuScParts::const_iterator pi = parts.find(partId);
	if(pi == parts.cend())
		return CID_Invalid;
	return (*pi)->Module();
}

COMMODITYID AuDesign::FTLDriveModule(quint16 partId) const
{
	const AuScParts& parts = Parts();
	AuScParts::const_iterator pi = parts.find(partId);
	if(pi == parts.cend())
		return CID_Invalid;
	return (*pi)->Module();
}

bool AuDesign::HasHelm() const
{
	return Parts().HasHelm();
}

AuPoint AuDesign::BerthLocation(quint16 partId) const
{
	return Parts().BerthLocation(partId);
}

quint16 AuDesign::RoomVoidContainingLoc(const AuPoint& loc) const
{
	return Parts().RoomVoidContainingLoc(loc);
}

quint16 AuDesign::RoomVoidContainingLoc(const AuPoint& loc, double elevate) const
{
	return Parts().RoomVoidContainingLoc(loc, elevate);
}

bool AuDesign::IsInsideObstructionModel(const AuPoint& loc) const
{
	return Parts().IsInsideObstructionModel(loc);
}

void AuDesign::TransporterPadLocs(quint16 partId, AuPointArray& padLocs) const
{
	const AuScParts& parts = Parts();
	AuScParts::const_iterator pi = parts.find(partId);
	if(pi != parts.cend())
		(*pi)->PadLocs(padLocs);
}

AuPoint AuDesign::OperatorLocation(quint16 partId) const
{
	return Parts().OperatorLocation(partId);
}

bool AuDesign::SurgeryUnitInRoom(quint16 roomId, AuPoint& patientLoc) const
{
	return Parts().SurgeryUnitInRoom(roomId, patientLoc);
}

AuPoint AuDesign::WeaponEmitLocation(quint16 partId) const
{
	return Parts().WeaponEmitLoc(partId);
}

void AuDesign::CitizenPostLocs(QHash<quint16, AuPoint>& locs) const
{
	Parts().CitizenPostLocs(locs);
}

void AuDesign::CitizenPosts(AuScPartSet& posts) const
{
	Parts().CitizenPosts(posts);
}

void AuDesign::GuardPostLocs(QHash<quint16, AuPoint>& locs) const
{
	Parts().GuardPostLocs(locs);
}

void AuDesign::GuardPosts(AuScPartSet& posts) const
{
	Parts().GuardPosts(posts);
}

void AuDesign::LivestockPostLocs(QHash<quint16, AuPoint>& locs) const
{
	Parts().LivestockPostLocs(locs);
}

void AuDesign::LivestockPosts(AuScPartSet& posts) const
{
	Parts().LivestockPosts(posts);
}

AuPoint AuDesign::RoomVoidGravityUp(quint16 roomId, const AuLiftHash& liftStops) const
{
	return Parts().RoomVoidGravityUp(roomId, liftStops);
}

HOLD_ACCESS AuDesign::RoomVoidHoldAccess(quint16 roomId) const
{
	return Parts().RoomVoidHoldAccess(roomId);
}

bool AuDesign::RoomVoidIsSunlit(quint16 voidId) const
{
	return Parts().RoomVoidIsSunlit(voidId);
}

INTERIOR_LIGHT AuDesign::RoomLight(const QString& roomName) const
{
	return Parts().RoomLight(roomName);
}

INTERIOR_LIGHT AuDesign::RoomLight(quint16 roomId) const
{
	return Parts().RoomLight(roomId);
}

double AuDesign::RoomVoidHeightAboveFloor(quint16 roomId, const AuPoint& loc, bool getRoomUp, AuPoint& gravityN,
		const AuLiftHash& liftStops) const
{
	return Parts().RoomVoidHeightAboveFloor(roomId, loc, getRoomUp, gravityN, liftStops);
}

AuPoint AuDesign::HelmLocation() const
{
	return Parts().HelmLocation();
}

bool AuDesign::IsInsideHull(const AuPoint& loc) const
{
	return Parts().IsInsideHull(loc);
}

void AuDesign::ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestPowerPlant, AuPoint& closestSensor) const
{
	Parts().ClosestAudibleUnits(searchLoc, closestPowerPlant, closestSensor);
}

void AuDesign::ClosestAudibleUnits(const AuPoint& searchLoc, AuPoint& closestManeuverDrive, AuPoint& closestPowerPlant,
		AuPoint& closestSensor, AuPoint& closestFTLDrive, AuPoint& closestSiren) const
{
	Parts().ClosestAudibleUnits(searchLoc, closestManeuverDrive, closestPowerPlant, closestSensor, closestFTLDrive, closestSiren);
}

AuPoint AuDesign::TargetLocation() const
{
	return Parts().TargetLoc();
}

bool AuDesign::HasSealedParts(const AuId& archId) const
{
	return Parts().HasSealedParts(archId);
}

bool AuDesign::HasSealedParts(const AuId& archId, const AuScSelection& sel) const
{
	return Parts().HasSealedParts(archId, sel);
}

void AuDesign::SetArchitectAfterFileLoad(const AuId& id, const QString& name)
{
	SetArchitectId(id);
	SetArchitectName(name);
	Parts().SetArchitectAfterFileLoad(id, name);
}

void AuDesign::ClearArchitectForFileSave(const AuId& id)
{
	SetArchitectId(AuId(0));
	SetArchitectName("");
	Parts().ClearArchitectForFileSave(id);
}

bool AuDesign::PartsUpdateHullDoors(const AuScSelection& partIds, AuScParts& partsChanged)
{
	const AuScParts& parts = Parts();
	bool anyChanged = false;
	for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi == parts.cend())
			continue;

		if((*pi)->IsLocked() || (*pi)->IsHidden())
			continue;

		if(!(*pi)->IsDoor())
			continue;

		bool hullDoor = LooksLikeHullDoor(pi.key());
		if(hullDoor == (*pi)->IsHullDoor())
			continue;	// No change.

		// Store part prior to change, for undo.
		partsChanged.insert(pi.key(), (*pi)->Clone());

		(*pi)->SetHullDoor(hullDoor);
		anyChanged = true;
	}
	return anyChanged;
}

bool AuDesign::LooksLikeHullDoor(quint16 partId) const
{
	// Check voids associated with the part.
	const AuScParts& parts = Parts();
	bool hullVoid = false;	// True if hull void associated with the door.
	bool roomVoid = false;	// True if room void associated with the door.
	for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
	{
		if((*pi)->AssociatedPartId() != partId)
			continue;
		if((*pi)->AssociatedPartState() != AuScDoor::Open)
			continue;
		if((*pi)->IsHullVoid())
		{
			if(roomVoid)
				return true;
			hullVoid = true;
		}
		else if((*pi)->IsRoomVoid())
		{
			if(hullVoid)
				return true;
			roomVoid = true;
		}
	}
	return false;
}

void AuDesign::DoorStates(AuDoorStateHash& doors) const
{
	Parts().DoorStates(doors);
}

void AuDesign::LandingGear(AuScPartSet& landingGear) const
{
	Parts().LandingGear(landingGear);
}

void AuDesign::RoomVoidNames(QStringList& rooms) const
{
	Parts().RoomVoidNames(rooms);
}

QString AuDesign::RoomVoidName(quint16 roomId) const
{
	const AuScParts& parts = Parts();
	AuScParts::const_iterator pi = parts.find(roomId);
	if(pi == parts.cend())
		return "";
	if(!(*pi)->IsRoomVoid())
		return "";
	return (*pi)->GroupName();
}

void AuDesign::TurboLifts(AuScReportLiftHash& lifts) const
{
	Parts().TurboLifts(lifts);
}

void AuDesign::TurboLifts(AuLiftHash& lifts) const
{
	Parts().TurboLifts(lifts);
}

void AuDesign::TurboLiftStopMatrix(quint16 liftId, const QString& stopName, AuMatrix4x4& stopM) const
{
	Parts().TurboLiftStopMatrix(liftId, stopName, stopM);
}

void AuDesign::ContrailEmitters(QMap<AuPoint, float>& contrails) const
{
	Q_UNUSED(contrails);
}

AuSysId AuDesign::SightedSystem(quint16 partId) const
{
	return Parts().SightedSystem(partId);
}

bool AuDesign::PartsMoveDoorSwitches(const AuScSelection& sel, const AuPoint& switch1, const AuPoint& switch2,
		AuScParts& partsChanged)
{
	return Parts().MoveDoorSwitches(sel, switch1, switch2, partsChanged);
}

bool AuDesign::SurgeryPatientPos(quint16 partId, AuPoint& patientLoc, quint8& patientPos) const
{
	return Parts().SurgeryPatientPos(partId, patientLoc, patientPos);
}

bool AuDesign::SetTransporterPadLocs(const AuScSelection& sel, const AuPointArray& padLocs, AuScParts& partsChanged)
{
	return Parts().SetTransporterPadLocs(sel, padLocs, partsChanged);
}

bool AuDesign::SetWeaponEmitLoc(const AuScSelection& sel, const AuPoint& emitLoc, AuScParts& partsChanged)
{
	return Parts().SetWeaponEmitLoc(sel, emitLoc, partsChanged);
}

bool AuDesign::IsOpenDoorVoid(quint16 roomId, quint16 doorId) const
{
	return Parts().IsOpenDoorVoid(roomId, doorId);
}

void AuDesign::LaunchPosition(quint16 launcherId, AuMatrix4x4& launchPadM, AuPoint& launchExit) const
{
	Parts().LaunchPosition(launcherId, launchPadM, launchExit);
}

void AuDesign::LandingSystemPosition(quint16 landerId, AuMatrix4x4& landerPosM) const
{
	Parts().LandingSystemPosition(landerId, landerPosM);
}

void AuDesign::ListRoomNames(QStringList& roomNames) const
{
	Parts().ListRoomNames(roomNames);
}

AuPoint AuDesign::TurretBaseAxis(quint16 turretId) const
{
	const AuScParts& parts = Parts();
	AuScParts::const_iterator ti = parts.find(turretId);
	if(ti == parts.cend())
		return AuPoint(0,0);
	return (*ti)->BaseAxis();
}

void AuDesign::TurretBaseMatrix(quint16 turretId, float baseRot, AuMatrix4x4& baseM) const
{
	const AuScParts& parts = Parts();
	AuScParts::const_iterator ti = parts.find(turretId);
	if(ti == parts.cend())
		return;
	(*ti)->TurretBaseMatrix(baseRot, baseM);
}

void AuDesign::TurretGunMatrix(quint16 turretId, float baseRot, float gunRot, AuMatrix4x4& gunM) const
{
	const AuScParts& parts = Parts();
	AuScParts::const_iterator ti = parts.find(turretId);
	if(ti == parts.cend())
		return;
	(*ti)->TurretGunMatrix(baseRot, gunRot, gunM);
}

quint64 AuDesign::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_ArchId);
	mem += StringMemoryUsage(m_ArchName);
	mem += StringMemoryUsage(m_Description);
	mem += sizeof(m_Id);
	mem += StringMemoryUsage(m_Name);
	mem += sizeof(m_State);
	mem += sizeof(m_TexturePalette);
	for(int tp=0; tp < SCTEXI_Count; tp++)
		mem += m_TexturePalette[tp].MemoryUsage();
#if(QT_VERSION >= QT_VERSION_CHECK(5,10,0))
	mem += m_Thumbnail.sizeInBytes();
#else
	mem += m_Thumbnail.byteCount();
#endif
	return mem;
}

//static
double AuDesign::SensorRangeDeepSpace(double sensorRangeFt)
{
	if(sensorRangeFt <= 0)
		return 0;
	return cbrt(sensorRangeFt / FeetPerMeter()) / 10.;
}

void AuDesign::VisibleFaces(AuScFaceList& faces) const
{
	Parts().VisibleFaces(faces);
}

void AuDesign::VisibleFaces(AuScFaceList& faces, const QString& groupName) const
{
	Parts().VisibleFaces(faces, groupName);
}

void AuDesign::PartDoorProperties(const AuScSelection& partIds, Qt::CheckState& isAutomatic, Qt::CheckState& initLocked,
		Qt::CheckState& remoteSpacecraft, Qt::CheckState& remoteVehicle)
{
	bool first = true;
	const AuScParts& parts = Parts();
	for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi == parts.cend())
			continue;
		if(!(*pi)->IsDoor())
			continue;

		if(first)
		{
			isAutomatic = (*pi)->IsAutomaticDoor() ? Qt::Checked : Qt::Unchecked;
			initLocked = (*pi)->IsInitiallyLocked() ? Qt::Checked : Qt::Unchecked;
			remoteSpacecraft = (*pi)->IsRemoteSpacecraftDoor() ? Qt::Checked : Qt::Unchecked;
			remoteVehicle = (*pi)->IsRemoteVehicleDoor() ? Qt::Checked : Qt::Unchecked;
			first = false;
		}
		else
		{
			Qt::CheckState nisAutomatic = (*pi)->IsAutomaticDoor() ? Qt::Checked : Qt::Unchecked;
			Qt::CheckState ninitLocked = (*pi)->IsInitiallyLocked() ? Qt::Checked : Qt::Unchecked;
			Qt::CheckState nremoteSpacecraft = (*pi)->IsRemoteSpacecraftDoor() ? Qt::Checked : Qt::Unchecked;
			Qt::CheckState nremoteVehicle = (*pi)->IsRemoteVehicleDoor() ? Qt::Checked : Qt::Unchecked;

			if(isAutomatic != Qt::PartiallyChecked && isAutomatic != nisAutomatic)
				isAutomatic = Qt::PartiallyChecked;
			if(initLocked != Qt::PartiallyChecked && initLocked != ninitLocked)
				initLocked = Qt::PartiallyChecked;
			if(remoteSpacecraft != Qt::PartiallyChecked && remoteSpacecraft != nremoteSpacecraft)
				remoteSpacecraft = Qt::PartiallyChecked;
			if(remoteVehicle != Qt::PartiallyChecked && remoteVehicle != nremoteVehicle)
				remoteVehicle = Qt::PartiallyChecked;
		}
	}

	if(first)
	{
		// No door was selected.
		isAutomatic = Qt::PartiallyChecked;
		initLocked = Qt::PartiallyChecked;
		remoteSpacecraft = Qt::PartiallyChecked;
		remoteVehicle = Qt::PartiallyChecked;
	}
}

void AuDesign::PartSiteProperties(const AuScSelection& partIds, quint8& grade, quint8& grass, quint8& pavement, quint8& rocks,
		quint8& shrubs, quint8& terBlend, quint8& trees)
{
	bool first = true;
	const AuScParts& parts = Parts();
	for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi == parts.cend())
			continue;
		if(!(*pi)->IsSite())
			continue;
		AuScSite* site = dynamic_cast<AuScSite*>(*pi);
		if(!site)
			continue;	// wut?

		if(first)
		{
			grade = site->Grade();
			grass = site->Grass();
			pavement = site->Pavement();
			rocks = site->Rocks();
			shrubs = site->Shrubs();
			terBlend = site->TerrainBlend();
			trees = site->Trees();
			first = false;
		}
		else
		{
			quint8 ngrade = site->Grade();
			quint8 ngrass = site->Grass();
			quint8 npavement = site->Pavement();
			quint8 nrocks = site->Rocks();
			quint8 nshrubs = site->Shrubs();
			quint8 nterBlend = site->TerrainBlend();
			quint8 ntrees = site->Trees();

			if(grade != 255 && grade != ngrade)
				grade = 255;
			if(grass != 255 && grass != ngrass)
				grass = 255;
			if(pavement != 255 && pavement != npavement)
				pavement = 255;
			if(rocks != 255 && rocks != nrocks)
				rocks = 255;
			if(shrubs != 255 && shrubs != nshrubs)
				shrubs = 255;
			if(terBlend != 255 && terBlend != nterBlend)
				terBlend = 255;
			if(trees != 255 && trees != ntrees)
				trees = 255;
		}
	}

	if(first)
	{
		// No door was selected.
		grade = 255;
		grass = 255;
		pavement = 255;
		rocks = 255;
		shrubs = 255;
		terBlend = 255;
		trees = 255;
	}
}

void AuDesign::PartLightBulbProperties(const AuScSelection& partIds, QColor& color, float& kL, float& kQ, float& cosOuterCone,
		float& cosInnerCone) const
{
	const AuScParts& parts = Parts();
	for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi == parts.cend())
			continue;
		if(!(*pi)->CanChangeLight())
			continue;
		color = (*pi)->LightColor();
		kL = (*pi)->LightkL();
		kQ = (*pi)->LightkQ();
		cosOuterCone = (*pi)->LightCosOuterCone();
		cosInnerCone = (*pi)->LightCosInnerCone();
	}
}

void AuDesign::PartLightLensProperties(const AuScSelection& partIds, QColor& color) const
{
	const AuScParts& parts = Parts();
	for(AuScSelection::const_iterator si = partIds.cbegin(); si != partIds.cend(); si++)
	{
		AuScParts::const_iterator pi = parts.find(si.key());
		if(pi == parts.cend())
			continue;
		if(!(*pi)->IsLightLens())
			continue;
		color = (*pi)->LightColor();
	}
}

QString AuDesign::PartName(quint16 partId) const
{
	const AuScParts& parts = Parts();
	AuScParts::const_iterator pi = parts.find(partId);
	if(pi == parts.cend())
		return QString();
	return (*pi)->PartName();
}

void AuDesign::PaintInUse(QList<QColor>& paintInUse) const
{
	Parts().PaintInUse(paintInUse);
}

AuPoint AuDesign::RemoteCameraLocation(quint16 currentCamera) const
{
	return Parts().RemoteCameraLocation(currentCamera);
}

quint16 AuDesign::RemoteCameraNext(quint16 currentCamera) const
{
	return Parts().RemoteCameraNext(currentCamera);
}

quint16 AuDesign::RemoteCameraPrevious(quint16 currentCamera) const
{
	return Parts().RemoteCameraPrevious(currentCamera);
}

bool AuDesign::RemoteCameraVerify(quint16 cameraId) const
{
	return Parts().RemoteCameraVerify(cameraId);
}

bool AuDesign::PartRenameStop(quint16 partId, const QString& oldStopName, const QString& nuStopName, AuScParts& partsChanged)
{
	return Parts().RenameStop(partId, oldStopName, nuStopName, partsChanged);
}

int AuDesign::ViewPositionSave(quint8 posId, const AuDesignPosition& pos)
{
	// Validate args.
	if(posId < 1 || posId > 9)
		return 2;	// Pos id is not valid.
	if(!pos.IsValid())
		return 3;	// Pos data is not valid.

	// Look for existing entry to replace.
	QHash<quint8, AuDesignPosition>::iterator vi = m_ViewPos.find(posId);
	if(vi != m_ViewPos.end())
	{
		// Replace existing entry.
		*vi = pos;
		return 1;
	}

	// Add new entry.
	m_ViewPos.insert(posId, pos);
	return 0;		// Ok
}

int AuDesign::ViewPositionLoad(quint8 posId, AuDesignPosition& pos) const
{
	// Validate args.
	if(posId < 1 || posId > 9)
		return 1;	// Pos id is not valid.

	// Locate requested pos.
	QHash<quint8, AuDesignPosition>::const_iterator vi = m_ViewPos.find(posId);
	if(vi == m_ViewPos.cend())
		return 2;	// Pos not found to restore.
	if(!(*vi).IsValid())
		return 3;	// Pos data not valid.
	pos = *vi;
	return 0;		// Ok
}

QSet<quint8> AuDesign::ViewPositions() const
{
	QSet<quint8> posIds;
	for(QHash<quint8, AuDesignPosition>::const_iterator vi = m_ViewPos.cbegin(); vi != m_ViewPos.cend(); vi++)
		posIds.insert(vi.key());
	return posIds;
}

int AuDesign::GridPositionSave(quint8 posId, const AuMatrix4x4& pos)
{
	// Validate args.
	if(posId < 1 || posId > 9)
		return 2;	// Pos id is not valid.
	if(pos.IsZero())
		return 3;	// Pos data is not valid.

	// Look for existing entry to replace.
	QHash<quint8, AuMatrix4x4>::iterator gi = m_GridPos.find(posId);
	if(gi != m_GridPos.end())
	{
		// Replace existing entry.
		*gi = pos;
		return 1;
	}

	// Add new entry.
	m_GridPos.insert(posId, pos);
	return 0;		// Ok
}

int AuDesign::GridPositionLoad(quint8 posId, AuMatrix4x4& pos) const
{
	// Validate args.
	if(posId < 1 || posId > 9)
		return 1;	// Pos id is not valid.

	// Locate requested pos.
	QHash<quint8, AuMatrix4x4>::const_iterator gi = m_GridPos.find(posId);
	if(gi == m_GridPos.cend())
		return 2;	// Pos not found to restore.
	if((*gi).IsZero())
		return 3;	// Pos data not valid.
	pos = *gi;
	return 0;		// Ok
}

QSet<quint8> AuDesign::GridPositions() const
{
	QSet<quint8> posIds;
	for(QHash<quint8, AuMatrix4x4>::const_iterator gi = m_GridPos.cbegin(); gi != m_GridPos.cend(); gi++)
		posIds.insert(gi.key());
	return posIds;
}

bool AuDesign::SelectContiguous(double angleTol, AuScSelection& sel) const
{
	return Parts().SelectContiguous(angleTol, sel);
}

//static
void AuDesign::FenceTexturePalette(AuGLEffects& re, AuTexturePalette& textures)
{
	textures[SCTEXI_User0] = AuTextureLibItem::LibraryTexture(re, TEXLIB_BrickLight);
	textures[SCTEXI_Material] = AuTextureLibItem::LibraryTexture(re, AuTextureLibItem::LibraryTextureHullStone());
	textures[SCTEXI_User1] = AuTextureLibItem::LibraryTexture(re, TEXLIB_BrickWall);
	textures[SCTEXI_User2] = AuTextureLibItem::LibraryTexture(re, TEXLIB_CobblestoneLight);
	textures[SCTEXI_User3] = AuTextureLibItem::LibraryTexture(re, TEXLIB_Concrete);
	textures[SCTEXI_User4] = AuTextureLibItem::LibraryTexture(re, TEXLIB_Decals);
	textures[SCTEXI_User5] = AuTextureLibItem::LibraryTexture(re, TEXLIB_SandBags);
	textures[SCTEXI_User6] = AuTextureLibItem::LibraryTexture(re, TEXLIB_Logs);
	textures[SCTEXI_User7] = AuTextureLibItem::LibraryTexture(re, TEXLIB_MetalBrushed);
	textures[SCTEXI_User8] = AuTextureLibItem::LibraryTexture(re, TEXLIB_MetalScratchRust);
	textures[SCTEXI_User9] = AuTextureLibItem::LibraryTexture(re, TEXLIB_TerraCottaRoof);
	textures[SCTEXI_User10] = AuTextureLibItem::LibraryTexture(re, TEXLIB_StuccoWallPlaster);
	textures[SCTEXI_User11] = AuTextureLibItem::LibraryTexture(re, TEXLIB_ConcretePrestressed);
	textures[SCTEXI_User12] = AuTextureLibItem::LibraryTexture(re, TEXLIB_WoodBare);
	textures[SCTEXI_User13] = AuTextureLibItem::LibraryTexture(re, TEXLIB_ChainLink);
}

void AuDesign::PartGroupNames(QStringList& groupNames) const
{
	Parts().GroupNames(groupNames);
}

bool AuDesign::PartSetGroup(quint16 partId, const QString& groupName, AuScParts& partsChanged)
{
	return Parts().SetGroupName(partId, groupName, partsChanged);
}

void AuDesign::BarrierFaces(AuScFaceList& barrierFaces, const QString& groupName) const
{
	Parts().BarrierFaces(barrierFaces, groupName);
}

void AuDesign::ExternalReferences(QSet<quint32>& xrefs) const
{
	Parts().ExternalReferences(xrefs);
}

bool AuDesign::PartFillFace(const AuScSelection& sel, const AuPointArray& boundary, const AuMatrix4x4& gridPlane,
		AuScParts& partsChanged)
{
	if(!Parts().FillFace(sel, boundary, gridPlane, partsChanged))
		return false;
	return true;
}

bool AuDesign::PartFillQuad(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3, const AuPoint& v4,
		AuScParts& partsChanged)
{
	if(!Parts().FillQuad(sel, v1, v2, v3, v4, partsChanged))
		return false;
	return true;
}

bool AuDesign::PartFillTriangle(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3,
		AuScParts& partsChanged)
{
	if(!Parts().FillTriangle(sel, v1, v2, v3, partsChanged))
		return false;
	return true;
}

bool AuDesign::PartSetOmitted(quint16 partId, bool omit, AuScParts& partsChanged)
{
	if(!Parts().SetOmitted(partId, omit, partsChanged))
		return false;
	return true;
}

bool AuDesign::PartSetSealed(quint16 partId, bool seal, AuScParts& partsChanged)
{
	if(!Parts().SetSealed(partId, seal, partsChanged))
		return false;
	return true;
}

bool AuDesign::PartDeleteState(quint16 partId, quint8 state, AuScParts& partsChanged)
{
	if(!Parts().DeleteState(partId, state, partsChanged))
		return false;
	return true;
}

bool AuDesign::PartAddFaces(quint16 partId, const AuScFaceList& faces, AuScParts& partsChanged)
{
	if(!Parts().AddFaces(partId, faces, partsChanged))
		return false;
	return true;
}

bool AuDesign::PartsMakeNavLightLens(const AuScSelection& sel, const QColor& color, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!Parts().MakeNavLightLens(sel, color, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeDecal(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!Parts().MakeDecal(sel, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeDoor(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!Parts().MakeDoor(sel, partsAdded, partsDeleted))
		return false;
	return true;
}

/*
bool AuDesign::PartsMakeHeadlightLens(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!Parts().MakeHeadlightLens(sel, partsAdded, partsDeleted))
		return false;
	return true;
}
*/

bool AuDesign::PartsMakeLightLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!Parts().MakeLightLens(sel, lightColor, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeRoom(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!Parts().MakeRoom(sel, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeHull(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!Parts().MakeHull(sel, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeMesh(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!Parts().MakeMesh(sel, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeLandingGear(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!Parts().MakeLandingGear(sel, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeRotatingBeaconLens(const AuScSelection& sel, const QColor& lightColor, const AuPoint& rotationAxis,
		AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!Parts().MakeRotatingBeaconLens(sel, lightColor, rotationAxis, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeStrobeLens(const AuScSelection& sel, const QColor& lightColor, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!Parts().MakeStrobeLens(sel, lightColor, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeTurboLift(const AuScSelection& sel, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!Parts().MakeTurboLift(sel, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeTurret(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis, AuScParts& partsAdded,
		AuScParts& partsDeleted)
{
	if(!Parts().MakeTurret(sel, basePivot, baseAxis, partsAdded, partsDeleted))
		return false;
	return true;
}

bool AuDesign::PartsMakeWindow(const AuScSelection& sel, float opacity, AuScParts& partsAdded, AuScParts& partsDeleted)
{
	if(!Parts().MakeWindow(sel, opacity, partsAdded, partsDeleted))
		return false;
	return true;
}

void AuDesign::FixPartAssociations(const QHash<quint16, quint16>& nuPartIds)
{
	Parts().FixPartAssociations(nuPartIds);
}
