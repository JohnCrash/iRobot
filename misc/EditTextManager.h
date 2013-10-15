/* 为了对动态字体的材质实现动态的扩展，需要在字体材质扩展后。调整已经存在的文字uv坐标
	MyGUI的文字处理被集中在EditText类中。因此在字体材质扩展时需要通知全部的EditText实例
	调整uv坐标除了在编译中加入EditTextManager.h与EditTextManager.cpp外还需要对MyGUI其他
	部分进行修改，修改步骤如下：
	a.修改MyGUI_Gui.h文件
		加入在类Gui中加入
		EditTextManager* mEditTextManager;
		同时这牵扯到在文件MyGUI_Prerequest.h中加入类声明
		class EditTextManager;
	b.修改MyGUI_Gui.cpp在对应位置加入
		mEditTextManager = new EditTextManager();

		mEditTextManager->initialise();
		与
		mEditTextManager->shutdown();

		delete mEditTextManager;
	c.修改MyGUI_EditText.h和MyGUI_EditText.cpp
		在类EditText的构造和析构函数中分别加入
		EditTextManager::getInstance().add( this );
		和
		EditTextManager::getInstance().remove( this );
		同时为EditText加入新的方法void update();和变量bool mNeedUpdate;\

		void EditText::update()
		{
			if (nullptr != mNode) mNode->outOfDate(mRenderItem);

			mNeedUpdate = true;
		}

		修改函数doRender的开头部分，修改后如下
		if (nullptr == mFont) return;

		if (!mVisible || mEmptyView) 
		{
			if( mNeedUpdate )
				updateRawData();
			mNeedUpdate = false;
			return;
		}

		bool _update = mRenderItem->getCurrentUpdate();
		if (_update || mNeedUpdate ) mTextOutDate = true;

		mNeedUpdate = false;

		if (mTextOutDate) updateRawData();

		Vertex* _vertex = mRenderItem->getCurrentVertexBuffer();
*/
#ifndef __MYGUI_EDITTEXT_MANAGER_H__
#define __MYGUI_EDITTEXT_MANAGER_H__

#include "MyGUI_Prerequest.h"
#include "MyGUI_Singleton.h"
#include "MyGUI_RenderItem.h"
#include "MyGUI_Enumerator.h"

namespace MyGUI
{
	class EditText;

	class MYGUI_EXPORT EditTextManager :
		public Singleton<EditTextManager>
	{
	public:
		typedef std::list<EditText*> EditTextList;
		typedef Enumerator<EditTextList> EnumeratorEditText;
	public:
		EditTextManager();

		void initialise();
		void shutdown();

		void add( EditText* pitem );
		void remove( EditText* pitem );

		EnumeratorEditText getEnumerator() const;
	private:
		EditTextList mEditTexts;
	};
}

#endif