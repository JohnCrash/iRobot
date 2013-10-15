#include "MyGUI_Precompiled.h"
#include "EditTextManager.h"
#include "MyGUI_EditText.h"

namespace MyGUI
{
	template <> EditTextManager* Singleton<EditTextManager>::msInstance = nullptr;
	template <> const char* Singleton<EditTextManager>::mClassTypeName("EditTextManager");
	
	EditTextManager::EditTextManager()
	{
	}

	void EditTextManager::initialise()
	{
	}

	void EditTextManager::shutdown()
	{
		mEditTexts.clear();
	}

	void EditTextManager::add( EditText* pitem )
	{
		mEditTexts.push_back( pitem );
	}

	void EditTextManager::remove( EditText* pitem )
	{
		mEditTexts.remove(pitem);
	}

	EditTextManager::EnumeratorEditText EditTextManager::getEnumerator() const
	{
		return EnumeratorEditText(mEditTexts);
	}
}