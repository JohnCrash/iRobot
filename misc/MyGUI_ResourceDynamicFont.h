/*
	该文件主要用于中文等多字符语言，它不像ResourceTrueTypeFont一次性
	读入全部的字符图像，而是一种混合模式。它开始可以先一次性读入一部分
	字符图像，同时在用户在调用getGlyphInfo时没有发现字符。这时该类将
	即时加载这个字符图像。
	该类基本是从ResourceTrueTypeFont修改而来。并且保留了它的组织方式
	修改文件MyGUI_FontManager.cpp在函数void FontManager::initialise()中

	a.加入下列调用
	FactoryManager::getInstance().registerFactory<ResourceDynamicFont>(XML_TYPE_RESOURCE);

	b.在OgreTexture中加入函数(MyGUI_OgreTexture.cpp)

	//begin
	void* OgreTexture::lock(int l,int t,int r,int b,TextureUsage _access)
	{
		if (_access == TextureUsage::Write)
		{
			return mTexture->getBuffer()->lock(Ogre::Image::Box(l,t,r,b),Ogre::HardwareBuffer::HBL_DISCARD).data;
		}
		//仅仅用于动态字体的写入操作
		return nullptr;
	}
	
	bool OgreTexture::resize(int width,int height)
	{
		//删除老的材质
		Ogre::TextureManager::getSingleton().remove(mTexture->getName());

		//创建新的材质
		Ogre::TexturePtr newTex = Ogre::TextureManager::getSingleton().createManual(
			mName,
			mGroup,
			Ogre::TEX_TYPE_2D,
			width, 
			height,
			0,
			mPixelFormat,
			mUsage,
			this);
		newTex->load();
		//将老的材质blit到新的材质中
		Ogre::Image::Box box(0,0,std::min(width,getWidth()),std::min(height,getHeight()));
		newTex->getBuffer()->blit( mTexture->getBuffer(),box,box );
		
		mTexture = newTex;
		return true;
	}
	//end

	在IFont接口中加入
	virtual void* lock(int l,int t,int r,int b,TextureUsage _access) = 0;
	virtual bool resize(int width,int height) = 0;

	d.为了实现一种动态的字体材质增长模式，需要加入下列文件
		EditTextManager.h
		EditTextManager.cpp
		具体修改参考EditTextManager.h开头部分
*/

#ifndef __MYGUI_RESOURCE_DYNAMIC_FONT_H__
#define __MYGUI_RESOURCE_DYNAMIC_FONT_H__

#include "MyGUI_Prerequest.h"
#include "MyGUI_ITexture.h"
#include "MyGUI_IFont.h"

#ifdef MYGUI_USE_FREETYPE
	#include <ft2build.h>
	#include FT_FREETYPE_H
	#include FT_GLYPH_H
#endif // MYGUI_USE_FREETYPE

namespace MyGUI
{

	class MYGUI_EXPORT ResourceDynamicFont :
		public IFont
	{
		MYGUI_RTTI_DERIVED( ResourceDynamicFont )

	public:
		typedef std::vector<PairCodePoint> VectorPairCodePoint;
		typedef std::vector<RangeInfo> VectorRangeInfo;
		typedef std::vector<PairCodeCoord> VectorPairCodeCoord;

		ResourceDynamicFont();
		virtual ~ResourceDynamicFont();

		virtual void deserialization(xml::ElementPtr _node, Version _version);

		virtual GlyphInfo* getGlyphInfo(Char _id);

		virtual ITexture* getTextureFont();

		// получившаяся высота при генерации в пикселях
		virtual int getDefaultHeight();

	private:
		void addCodePointRange(Char _first, Char _second);
		void addHideCodePointRange(Char _first, Char _second);

		// проверяет, входит ли символ в зоны ненужных символов
		bool checkHidePointCode(Char _id);

		/** Clear the list of code point ranges. */
		void clearCodePointRanges();

		void initialise();

		void addGlyph(GlyphInfo* _info, Char _index,
			int _left, int _top, int _right, int _bottom, 
			int _finalw, int _finalh, float _aspect, int _addHeight = 0,
			float _advance = 0.0f,float _bearingX = 0.0f );

		uint8* writeData(uint8* _pDest, unsigned char _luminance, unsigned char _alpha, bool _rgba);

		//加入一个新的字型
		GlyphInfo* addGlyphInfo( Char _id );
		//产生一个新的
		bool newGlyphInfo( GlyphInfo* pgi,Char _id );
		//重新定位字体的纹理坐标
		void relocal( float sx,float sy );
		void reLocalGlyph( GlyphInfo& gi,float sx,float sy);
	private:
		// Source of the font
		std::string mSource;
		// Size of the truetype font, in points
		float mTtfSize;
		// Resolution (dpi) of truetype font
		uint mTtfResolution;

		bool mAntialiasColour;

		int mDistance;
		int mSpaceWidth;
		int mTabWidth;
		int mCursorWidth;
		int mSelectionWidth;
		int mOffsetHeight;
		int mHeightPix;

		// отдельная информация о символах
		GlyphInfo mSpaceGlyphInfo;
		GlyphInfo mTabGlyphInfo;
		GlyphInfo mSelectGlyphInfo;
		GlyphInfo mSelectDeactiveGlyphInfo;
		GlyphInfo mCursorGlyphInfo;

		// символы которые не нужно рисовать
		VectorPairCodePoint mVectorHideCodePoint;

		// вся информация о символах
		VectorRangeInfo mVectorRangeInfo;

		MyGUI::ITexture* mTexture;

		//字体缓冲
		FT_Library mftLibrary;
		FT_Face face;
		/*将initialise中的数据保持下来，在newGlyphInfo中使用
		*/
		struct Context
		{
			int length;
			int height; //绘制点位置
			int max_height;

			int finalHeight;
			int finalWidth;

			int max_bear;

			size_t pixel_bytes;
			FT_Int advance;

			float textureAspect;

			bool rgbaMode;
		};
		Context mContext;
		bool mIsFull; //空间已满
		//新增加的属性
		int mWidth;
		int mHeight;
	};

} // namespace MyGUI

#endif // __MYGUI_RESOURCE_TRUE_TYPE_FONT_H__
