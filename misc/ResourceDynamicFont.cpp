#include "MyGUI_Precompiled.h"
#include "MyGUI_ResourceDynamicFont.h"
#include "MyGUI_DataManager.h"
#include "MyGUI_RenderManager.h"
#include "EditTextManager.h"
#include "MyGUI_EditText.h"

namespace MyGUI
{

	static const unsigned char FONT_MASK_SELECT = 0x88;
	static const unsigned char FONT_MASK_SELECT_DEACTIVE = 0x60;
	static const unsigned char FONT_MASK_SPACE = 0x00;
	static const unsigned char FONT_MASK_CHAR = 0xFF;
	static const int MIN_FONT_TEXTURE_WIDTH = 256;

	ResourceDynamicFont::ResourceDynamicFont() :
		mTtfSize(0),
		mTtfResolution(0),
		mAntialiasColour(false),
		mDistance(0),
		mSpaceWidth(0),
		mTabWidth(0),
		mCursorWidth(2),
		mSelectionWidth(6),
		mOffsetHeight(0),
		mHeightPix(0),
		mTexture(nullptr)
	{
		mIsFull = false;
		mWidth = 0;
		mHeight = 0;
	}

	ResourceDynamicFont::~ResourceDynamicFont()
	{
#ifdef MYGUI_USE_FREETYPE
		FT_Done_FreeType(mftLibrary);
#endif

		if (mTexture != nullptr)
		{
			RenderManager::getInstance().destroyTexture(mTexture);
			mTexture = nullptr;
		}
	}

	GlyphInfo* ResourceDynamicFont::getGlyphInfo(Char _id)
	{
		for (VectorRangeInfo::iterator iter = mVectorRangeInfo.begin(); iter != mVectorRangeInfo.end(); ++iter)
		{
			GlyphInfo* info = iter->getInfo(_id);
			if (info == nullptr) continue;
			if( info->codePoint == 0 )//表示没有装入
				break;
			return info;
		}
		if( _id != 0x20 ) //空格的话自己返回
		{
			//没有发现的编码尝试加入
			GlyphInfo* info = addGlyphInfo( _id );
			if( info != nullptr )return info;
		}
		//都没有发现返回一个空格
		return &mSpaceGlyphInfo;
	}

	/*将字型绘制进入材质，并且设置正确的uv
	*/
	bool ResourceDynamicFont::newGlyphInfo( GlyphInfo* pgi,Char _id )
	{
		if( mIsFull )return false;

		if( checkHidePointCode(_id) )return false;

		FT_Error ftResult;
		ftResult = FT_Load_Char( face, _id, FT_LOAD_RENDER );

		if (ftResult)
		{
			// problem loading this glyph, continue
			MYGUI_LOG(Warning, "cannot load character " << _id << " in font " << getResourceName());
			return false;
		}

		//FT_Int glyph_advance = (face->glyph->advance.x >> 6 );
		FT_Int glyph_advance = (face->glyph->metrics.width >> 6 );
		unsigned char* buffer = face->glyph->bitmap.buffer;

		if (nullptr == buffer)
		{
			// Yuck, FT didn't detect this but generated a nullptr pointer!
			MYGUI_LOG(Warning, "Freetype returned nullptr for character " << _id << " in font " << getResourceName());
			return false;
		}

		int y_bearnig = mContext.max_bear - ( face->glyph->metrics.horiBearingY >> 6 );

		if ( mContext.finalWidth - 1 < (mContext.length + face->glyph->bitmap.width + mDistance) )
		{
			mContext.height += mContext.max_height + mDistance;
			mContext.length = mDistance;
		}

		/*仅仅锁定字符大小的矩形大小，完全的锁定导致图像被摧毁
		*/
		/*
		OgreTexture* pTexture = dynamic_cast<OgreTexture*>(mTexture);
		if( !pTexture )
		{
			MYGUI_LOG(Warning, "ResourceDynamicFont can't dynamic_cast mTexture to OgreTexture!" );
			return false;
		}
		*/
		//字体材质没有足够的空间容纳这个字符
		if( mContext.height + mContext.max_height + mDistance > mContext.finalHeight )
		{
		/*这里加入一个材质增长才操作，让他在高度上增长。
			MYGUI_LOG(Warning, "Freetype texture not enough space " << _id << " in font " << getResourceName() 
				<< " width:" << mContext.finalWidth << " height:"<< mContext.finalHeight );
			mIsFull = true; //防止反复提示
			return false;
			*/
			int oldHeight = mContext.finalHeight;
			mContext.finalHeight += mContext.finalHeight;
			mTexture->resize(mContext.finalWidth,mContext.finalHeight);
			//因为拉伸作用需要调整已有的字型的纹理坐标
			relocal( 1.0,
				(float)(oldHeight)/(float)mContext.finalHeight
				);
			//重新设置已有的全部文字的uv坐标
			EditTextManager::EnumeratorEditText e=EditTextManager::getInstance().getEnumerator();
			while( e.next() )
			{
				e->update();
			}
		}

		int bearingX = face->glyph->metrics.horiBearingX >> 6;

		//锁定同时会销毁以前的图像
		//uint8* imageData = (uint8*)mTexture->lock(TextureUsage::Write);
		uint8* imageData = (uint8*)mTexture->lock(
			mContext.length+bearingX,mContext.height+y_bearnig,
			mContext.length+bearingX+face->glyph->bitmap.width,mContext.height+y_bearnig+face->glyph->bitmap.rows,
			TextureUsage::Write
			);

		if (imageData != nullptr)
		{
			int height = mContext.height;
			int len = mContext.length;
			int pixel_bytes = mContext.pixel_bytes;
			bool rgbaMode = mContext.rgbaMode;

			size_t data_width = mContext.finalWidth * pixel_bytes;
			//size_t data_width = box.rowPitch * pixel_bytes;

			for (int j = 0; j < face->glyph->bitmap.rows; j++ )
			{
				//int row = j + height + y_bearnig;
				//uint8* pDest = &imageData[(row * data_width) + (len + ( face->glyph->metrics.horiBearingX >> 6 )) * pixel_bytes];
				uint8* pDest = &imageData[(j * data_width)];
				for (int k = 0; k < face->glyph->bitmap.width; k++ )
				{
					if (mAntialiasColour) 
						pDest = writeData(pDest, *buffer, *buffer, rgbaMode);
					else
						pDest = writeData(pDest, FONT_MASK_CHAR, *buffer, rgbaMode);
					buffer++;
				}
			}
		}
		else
		{
			MYGUI_LOG(Error, "ResourceDynamicFont, error lock texture, pointer is nullptr");
		}

		mTexture->unlock();

		addGlyph(pgi, _id, 
			mContext.length+bearingX, mContext.height,
			mContext.length +bearingX+ glyph_advance, mContext.height + mContext.max_height, 
			mContext.finalWidth, mContext.finalHeight, 
			mContext.textureAspect, mOffsetHeight,
			(face->glyph->advance.x/64.0f)-bearingX,
			(float)bearingX );

		mContext.length += (glyph_advance + mDistance);
		return true;
	}

	void ResourceDynamicFont::relocal( float sx,float sy )
	{
		for( VectorRangeInfo::iterator i = mVectorRangeInfo.begin();i!=mVectorRangeInfo.end();++i )
		{
			for( VectorGlyphInfo::iterator j = i->range.begin();j!=i->range.end();++j )
			{
				reLocalGlyph( *j ,sx,sy );
			}
		}
		reLocalGlyph(mSpaceGlyphInfo,sx,sy);
		reLocalGlyph(mTabGlyphInfo,sx,sy);
		reLocalGlyph(mSelectGlyphInfo,sx,sy);
		reLocalGlyph(mSelectDeactiveGlyphInfo,sx,sy);
		reLocalGlyph(mCursorGlyphInfo,sx,sy);
	}

	void ResourceDynamicFont::reLocalGlyph( GlyphInfo& gi,float sx,float sy)
	{
		gi.uvRect.left *= sx;
		gi.uvRect.top *= sy;
		gi.uvRect.right *= sx;
		gi.uvRect.bottom *= sy;
	}

	//加入一个新的字型
	GlyphInfo* ResourceDynamicFont::addGlyphInfo( Char _id )
	{
		GlyphInfo gi;

		if( !newGlyphInfo( &gi,_id ) )return nullptr;

		//首先查找看看有没有合适的RangeInfo，如果有就放入其中
		for( VectorRangeInfo::iterator iter = mVectorRangeInfo.begin(); iter != mVectorRangeInfo.end(); ++iter)
		{
			if( iter->isExist( _id ) )
			{
				iter->setInfo( _id,&gi );
				return iter->getInfo( _id );
			}
		}
		/*没有发现合适的RangeInfo则产生一个新的
			将unicode16分成每256(0-FF)为一个区Rang，然后在和固有的范围做一个排除运算
		*/
		Char begin,end;
		begin = _id & 0xff00;
		end = _id | 0xff;
		for( VectorRangeInfo::iterator iter = mVectorRangeInfo.begin(); iter != mVectorRangeInfo.end(); ++iter)
		{
			if( iter->first >= begin && iter->first <= end &&
				iter->last >= begin && iter->last <= end ) //两都落在begin,end中，将begin,end分成3个部分
			{
				if( _id < iter->first )
					end = iter->first-1;
				else
					begin = iter->last+1;
			}
			else
			{
				if( iter->first >= begin && iter->first <= end  )
					end = iter->first-1;
				if( iter->last >= begin && iter->last <= end )
					begin = iter->last+1;
			}
		}
		mVectorRangeInfo.push_back( RangeInfo(begin,end) );
		mVectorRangeInfo.back().setInfo( _id,&gi );
		return mVectorRangeInfo.back().getInfo( _id );
	}

	void ResourceDynamicFont::addGlyph(GlyphInfo* _info, Char _index,
		int _left, int _top, int _right, int _bottom, 
		int _finalw, int _finalh, float _aspect, int _addHeight,
		float _advance,float _bearingX )
	{
		_info->codePoint = _index;
		_info->uvRect.left = (float)_left / (float)_finalw;  // u1
		_info->uvRect.top = (float)(_top + _addHeight) / (float)_finalh;  // v1
		_info->uvRect.right = (float)( _right ) / (float)_finalw; // u2
		_info->uvRect.bottom = ( _bottom + _addHeight ) / (float)_finalh; // v2

		_info->width = (float)(_right - _left);
		_info->height = (float)(_bottom - _top);
		if( _advance == 0.0f )
			_info->advance = _info->width;
		else
			_info->advance = _advance;
		_info->bearingX = _bearingX;
	}

	uint8* ResourceDynamicFont::writeData(uint8* _pDest, unsigned char _luminance, unsigned char _alpha, bool _rgba)
	{
		if (_rgba)
		{
			*_pDest++ = _luminance; // luminance
			*_pDest++ = _luminance; // luminance
			*_pDest++ = _luminance; // luminance
			*_pDest++ = _alpha; // alpha
		}
		else
		{
			*_pDest++ = _luminance; // luminance
			*_pDest++ = _alpha; // alpha
		}
		return _pDest;
	}

	void ResourceDynamicFont::initialise()
	{

#ifndef MYGUI_USE_FREETYPE

		MYGUI_LOG(Error, "ResourceDynamicFont '" << getResourceName() << "' - Ttf font disabled. Define MYGUI_USE_FREETYE if you need ttf fonts.");

#else // MYGUI_USE_FREETYPE

		/* 以前的加载方式是直接读出全部的字体文件，这对字母文字可以快速完成。
			但是对于中文就有效率问题
		*/
//		IDataStream* datastream = DataManager::getInstance().getData(mSource);
//		if (!datastream)
//		{
//			MYGUI_LOG(Error, "Could not open font face!");
//			return;
//		}

		mTexture = RenderManager::getInstance().createTexture(MyGUI::utility::toString((size_t)this, "_TrueTypeFont"));

		// ManualResourceLoader implementation - load the texture
		// Init freetype
		if ( FT_Init_FreeType( &mftLibrary ) ) MYGUI_EXCEPT("Could not init FreeType library!");

		// Load font
		//FT_Face face;

//		size_t datasize = datastream->size();
//		uint8* data = new uint8[datasize];
//		datastream->read(data, datasize);
//		delete datastream;
		std::string fontFile = DataManager::getInstance().getDataPath(mSource);
		
		/*这里使用文件模式，如果使用内存模式不得不一直保存data数据。
		*/
		//if ( FT_New_Memory_Face( mftLibrary, data, (FT_Long)datasize, 0, &face ) )
		if( FT_New_Face( mftLibrary,fontFile.c_str(),0,&face) )
		{
			MYGUI_EXCEPT("Could not open font face!");
		}
		// Convert our point size to freetype 26.6 fixed point format
		FT_F26Dot6 ftSize = (FT_F26Dot6)(mTtfSize * (1 << 6));
		if ( FT_Set_Char_Size( face, ftSize, 0, mTtfResolution, mTtfResolution ) )
			MYGUI_EXCEPT("Could not set char size!");

		int max_height = 0, max_bear = 0;

		int spec_len = mCursorWidth + mSelectionWidth + mSelectionWidth + mSpaceWidth + mTabWidth + (mDistance * 5);
		int len = mDistance + spec_len;
		int height = 0; // здесь используется как колличество строк

		int finalWidth = MIN_FONT_TEXTURE_WIDTH;
		// trying to guess necessary width for texture
		while (mTtfSize * mTtfResolution > finalWidth * 6) finalWidth *= 2;

		for (VectorRangeInfo::iterator iter = mVectorRangeInfo.begin(); iter != mVectorRangeInfo.end(); ++iter)
		{
			for (Char index = iter->first; index <= iter->last; ++index)
			{

				// символ рисовать ненужно
				if (checkHidePointCode(index)) continue;

				if (FT_Load_Char( face, index, FT_LOAD_RENDER )) continue;
				if (nullptr == face->glyph->bitmap.buffer) continue;
				FT_Int advance = (face->glyph->advance.x >> 6 ) + ( face->glyph->metrics.horiBearingX >> 6 );

				if ( ( 2 * ( face->glyph->bitmap.rows << 6 ) - face->glyph->metrics.horiBearingY ) > max_height )
					max_height = ( 2 * ( face->glyph->bitmap.rows << 6 ) - face->glyph->metrics.horiBearingY );

				if ( face->glyph->metrics.horiBearingY > max_bear )
					max_bear = face->glyph->metrics.horiBearingY;

				len += (advance + mDistance);
				if ( finalWidth - 1 < (len + advance + mDistance) )
				{
					height ++;
					len = mDistance;
				}

			}
		}

		max_height >>= 6;
		max_bear >>= 6;

		int finalHeight = (height + 1) * (max_height + mDistance) + mDistance;

		//make it more squared
		while (finalHeight > finalWidth)
		{
			finalHeight /= 2;
			finalWidth *= 2;
		}

		// вычисляем ближайшую кратную 2
		int needHeight = 1;
		while (needHeight < finalHeight) needHeight <<= 1;
		finalHeight = needHeight;

		/*手动设定宽高，这个设置依赖与字体的xml文件
			同时这个尺寸也不是固定不变的，如果增加的字符使材质不能容纳。材质将会被加长。
			这有做可以保证已经存在的字符被拷贝到新的材质。
			被建议的材质尺寸的初值是512x512，在空间不够的时候他会变成512x1024。
		*/
		if( mWidth != 0 && mHeight != 0 )
		{
			if( finalWidth < mWidth )
				finalWidth = mWidth;
			if( finalHeight < mHeight )
				finalHeight = mHeight;
		}

		float textureAspect = (float)finalWidth / (float)finalHeight;

		// if L8A8 (2 bytes per pixel) not supported use 4 bytes per pixel R8G8B8A8
		// where R == G == B == L
		bool rgbaMode = ! MyGUI::RenderManager::getInstance().isFormatSupported(PixelFormat::L8A8, TextureUsage::Static | TextureUsage::Write);

		const size_t pixel_bytes = rgbaMode ? 4 : 2;
		size_t data_width = finalWidth * pixel_bytes;
		size_t data_size = finalWidth * finalHeight * pixel_bytes;

		MYGUI_LOG(Info, "ResourceDynamicFont '" << getResourceName() << "' using texture size " << finalWidth << " x " << finalHeight);
		MYGUI_LOG(Info, "ResourceDynamicFont '" << getResourceName() << "' using real height " << max_height << " pixels");
		mHeightPix = max_height;

		uint8* imageData = new uint8[data_size];

		uint8* dest = imageData;
		// Reset content (White, transparent)
		for (size_t i = 0; i < data_size; i += pixel_bytes)
		{
			dest = writeData(dest, 0xFF, 0x00, rgbaMode);
		}

		// текущее положение в текстуре
		len = mDistance;
		height = mDistance; // здесь используется как текущее положение высоты
		FT_Int advance = 0;

		//------------------------------------------------------------------
		// создаем символ пробела
		//------------------------------------------------------------------
		advance = mSpaceWidth;

		// перевод на новую строку
		if ( finalWidth - 1 < (len + advance + mDistance) )
		{
			height += max_height + mDistance;
			len = mDistance;
		}

		for (int j = 0; j < max_height; j++ )
		{
			int row = j + (int)height;
			uint8* pDest = &imageData[(row * data_width) + len * pixel_bytes];
			for (int k = 0; k < advance; k++ )
			{
				pDest = writeData(pDest, FONT_MASK_CHAR, FONT_MASK_SPACE, rgbaMode);
			}
		}

		addGlyph(&mSpaceGlyphInfo, FontCodeType::Space, 
			len, height, len + advance, height + max_height, finalWidth, finalHeight, textureAspect, mOffsetHeight);
		len += (advance + mDistance);

		//------------------------------------------------------------------
		// создаем табуляцию
		//------------------------------------------------------------------
		advance = mTabWidth;

		// перевод на новую строку
		if ( finalWidth - 1 < (len + advance + mDistance) )
		{
			height += max_height + mDistance;
			len = mDistance;
		}

		for (int j = 0; j < max_height; j++ )
		{
			int row = j + (int)height;
			uint8* pDest = &imageData[(row * data_width) + len * pixel_bytes];
			for (int k = 0; k < advance; k++ )
			{
				pDest = writeData(pDest, FONT_MASK_CHAR, FONT_MASK_SPACE, rgbaMode);
			}
		}

		addGlyph(&mTabGlyphInfo, FontCodeType::Tab, len, height, len + advance, height + max_height, finalWidth, finalHeight, textureAspect, mOffsetHeight);
		len += (advance + mDistance);

		//------------------------------------------------------------------
		// создаем выделение
		//------------------------------------------------------------------
		advance = mSelectionWidth+2*mDistance;
		for (int j = 0; j < max_height; j++ )
		{
			int row = j + (int)height;
			uint8* pDest = &imageData[(row * data_width) + len * pixel_bytes];
			for (int k = 0; k < advance; k++ )
			{
				pDest = writeData(pDest, FONT_MASK_CHAR, FONT_MASK_SELECT, rgbaMode);
			}
		}

		// перевод на новую строку
		if ( finalWidth - 1 < (len + advance + mDistance) )
		{
			height += max_height + mDistance;
			len = mDistance;
		}

		addGlyph(&mSelectGlyphInfo, FontCodeType::Selected, len, height, len + advance, height + max_height, finalWidth, finalHeight, textureAspect, mOffsetHeight);
		len += (advance + mDistance);

		//------------------------------------------------------------------
		// создаем неактивное выделение
		//------------------------------------------------------------------
		advance = mSelectionWidth+2*mDistance;

		// перевод на новую строку
		if ( finalWidth - 1 < (len + advance + mDistance) )
		{
			height += max_height + mDistance;
			len = mDistance;
		}

		for (int j = 0; j < max_height; j++ )
		{
			int row = j + (int)height;
			uint8* pDest = &imageData[(row * data_width) + len * pixel_bytes];
			for (int k = 0; k < advance; k++ )
			{
				pDest = writeData(pDest, FONT_MASK_CHAR, FONT_MASK_SELECT_DEACTIVE, rgbaMode);
			}
		}

		addGlyph(&mSelectDeactiveGlyphInfo, FontCodeType::SelectedBack, len, height, len + advance, height + max_height, finalWidth, finalHeight, textureAspect, mOffsetHeight);
		len += (advance + mDistance);

		//------------------------------------------------------------------
		// создаем курсор
		//------------------------------------------------------------------
		advance = mCursorWidth;

		// перевод на новую строку
		if ( finalWidth - 1 < (len + advance + mDistance) )
		{
			height += max_height + mDistance;
			len = mDistance;
		}

		for (int j = 0; j < max_height; j++ )
		{
			int row = j + (int)height;
			uint8* pDest = &imageData[(row * data_width) + len * pixel_bytes];
			for (int k = 0; k < advance; k++ )
			{
				pDest = writeData(pDest, (k & 1) ? 0 : 0xFF, FONT_MASK_CHAR, rgbaMode);
			}
		}

		addGlyph(&mCursorGlyphInfo, FontCodeType::Cursor, len, height, len + advance, height + max_height, finalWidth, finalHeight, textureAspect, mOffsetHeight);
		len += (advance + mDistance);

		//------------------------------------------------------------------
		// создаем все остальные символы
		//------------------------------------------------------------------
		FT_Error ftResult;
		for (VectorRangeInfo::iterator iter = mVectorRangeInfo.begin(); iter != mVectorRangeInfo.end(); ++iter)
		{
			size_t pos = 0;
			for (Char index = iter->first; index <= iter->last; ++index, ++pos)
			{
				// сомвол рисовать не нада
				if (checkHidePointCode(index)) continue;

				GlyphInfo& info = iter->range.at(pos);

				ftResult = FT_Load_Char( face, index, FT_LOAD_RENDER );
				if (ftResult)
				{
					// problem loading this glyph, continue
					MYGUI_LOG(Warning, "cannot load character " << index << " in font " << getResourceName());
					continue;
				}

				FT_Int glyph_advance = (face->glyph->metrics.width >> 6 );
				unsigned char* buffer = face->glyph->bitmap.buffer;

				if (nullptr == buffer)
				{
					// Yuck, FT didn't detect this but generated a nullptr pointer!
					MYGUI_LOG(Warning, "Freetype returned nullptr for character " << index << " in font " << getResourceName());
					continue;
				}

				int y_bearnig = max_bear - ( face->glyph->metrics.horiBearingY >> 6 );

				// перевод на новую строку
				if ( finalWidth - 1 < (len + face->glyph->bitmap.width + mDistance) )
				{
					height += max_height + mDistance;
					len = mDistance;
				}

				int bearingX = face->glyph->metrics.horiBearingX >> 6;
				for (int j = 0; j < face->glyph->bitmap.rows; j++ )
				{
					int row = j + (int)height + y_bearnig;
					uint8* pDest = &imageData[(row * data_width) + (len + bearingX) * pixel_bytes];
					for (int k = 0; k < face->glyph->bitmap.width; k++ )
					{
						if (mAntialiasColour) pDest = writeData(pDest, *buffer, *buffer, rgbaMode);
						else pDest = writeData(pDest, FONT_MASK_CHAR, *buffer, rgbaMode);
						buffer++;
					}
				}

				addGlyph(&info, index, 
					len+bearingX, height, 
					len +bearingX + glyph_advance, height + max_height, 
					finalWidth, finalHeight, textureAspect, mOffsetHeight,
					(face->glyph->advance.x/64.0f)-bearingX,
					(float)bearingX
					);
				len += (glyph_advance + mDistance);

			}
		}

		{ //收集绘制上下文，在添加新的字型时需要这些信息
			mContext.length = len;
			mContext.height = height;
			mContext.max_height = max_height;
			mContext.finalHeight = finalHeight;
			mContext.finalWidth = finalWidth;
			mContext.max_bear = max_bear;
			mContext.pixel_bytes = pixel_bytes;
			mContext.advance = advance;
			mContext.textureAspect = textureAspect;
			mContext.rgbaMode = rgbaMode;
		}

		// Добавляем спец символы в основной список
		// пробел можно не добавлять, его вернет по ошибке
		RangeInfo info(FontCodeType::Selected, FontCodeType::Tab);
		info.setInfo(FontCodeType::Selected, &mSelectGlyphInfo);
		info.setInfo(FontCodeType::SelectedBack, &mSelectDeactiveGlyphInfo);
		info.setInfo(FontCodeType::Cursor, &mCursorGlyphInfo);
		info.setInfo(FontCodeType::Tab, &mTabGlyphInfo);

		mVectorRangeInfo.push_back(info);


		mTexture->createManual(finalWidth, finalHeight, TextureUsage::Static | TextureUsage::Write, rgbaMode ? PixelFormat::R8G8B8A8 : PixelFormat::L8A8);

		void* buffer_ptr = mTexture->lock(TextureUsage::Write);
		if (buffer_ptr != nullptr)
		{
			memcpy(buffer_ptr, imageData, data_size);
		}
		else
		{
			MYGUI_LOG(Error, "ResourceDynamicFont, error lock texture, pointer is nullptr");
		}
		mTexture->unlock();

		delete [] imageData;
//		delete [] data;

#endif // MYGUI_USE_FREETYPE

	}

	void ResourceDynamicFont::addCodePointRange(Char _first, Char _second)
	{
		mVectorRangeInfo.push_back(RangeInfo(_first, _second));
	}

	void ResourceDynamicFont::addHideCodePointRange(Char _first, Char _second)
	{
		mVectorHideCodePoint.push_back(PairCodePoint(_first, _second));
	}

	// проверяет, входит ли символ в зоны ненужных символов
	bool ResourceDynamicFont::checkHidePointCode(Char _id)
	{
		for (VectorPairCodePoint::iterator iter = mVectorHideCodePoint.begin(); iter != mVectorHideCodePoint.end(); ++iter)
		{
			if (iter->isExist(_id)) return true;
		}
		return false;
	}

	void ResourceDynamicFont::clearCodePointRanges()
	{
		mVectorRangeInfo.clear();
		mVectorHideCodePoint.clear();
	}

	void ResourceDynamicFont::deserialization(xml::ElementPtr _node, Version _version)
	{
		Base::deserialization(_node, _version);

		xml::ElementEnumerator node = _node->getElementEnumerator();
		while (node.next())
		{
			if (node->getName() == "Property")
			{
				const std::string& key = node->findAttribute("key");
				const std::string& value = node->findAttribute("value");
				if (key == "Source") mSource = value;
				else if (key == "Size") mTtfSize = utility::parseFloat(value);
				else if (key == "Resolution") mTtfResolution = utility::parseUInt(value);
				else if (key == "Antialias") mAntialiasColour = utility::parseBool(value);
				else if (key == "SpaceWidth") mSpaceWidth = utility::parseInt(value);
				else if (key == "TabWidth") mTabWidth = utility::parseInt(value);
				//else if (key == "CursorWidth") mCursorWidth = utility::parseInt(value);
				else if (key == "Distance") mDistance = utility::parseInt(value);
				else if (key == "OffsetHeight") mOffsetHeight = utility::parseInt(value);
				else if(key == "Width") mWidth = utility::parseInt(value);
				else if(key == "Height") mHeight = utility::parseInt(value);
			}
			else if (node->getName() == "Codes")
			{
				xml::ElementEnumerator range = node->getElementEnumerator();
				while (range.next("Code"))
				{
					std::string range_value;
					std::vector<std::string> parse_range;
					// диапазон включений
					if (range->findAttribute("range", range_value))
					{
						parse_range = utility::split(range_value);
						if (!parse_range.empty())
						{
							int first = utility::parseInt(parse_range[0]);
							int last = parse_range.size() > 1 ? utility::parseInt(parse_range[1]) : first;
							addCodePointRange(first, last);
						}
					}
					// диапазон исключений
					else if (range->findAttribute("hide", range_value))
					{
						parse_range = utility::split(range_value);
						if (!parse_range.empty())
						{
							int first = utility::parseInt(parse_range[0]);
							int last = parse_range.size() > 1 ? utility::parseInt(parse_range[1]) : first;
							addHideCodePointRange(first, last);
						}
					}
				}
			}
		}

		// инициализируем
		initialise();
	}

	ITexture* ResourceDynamicFont::getTextureFont()
	{
		return mTexture;
	}

	// получившаяся высота при генерации в пикселях
	int ResourceDynamicFont::getDefaultHeight()
	{
		return mHeightPix;
	}

} // namespace MyGUI
