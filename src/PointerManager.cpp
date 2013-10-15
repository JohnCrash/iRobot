/*!
	@file
	@author		Albert Semenov
	@date		1/2009
*/
#include "stdheader.h"
#include "PointerManager.h"

namespace base
{

	PointerManager::PointerManager()
	{
	}

	PointerManager::~PointerManager()
	{
	}

	void PointerManager::createPointerManager(size_t _handle)
	{
	}

	void PointerManager::destroyPointerManager()
	{
	}

	void PointerManager::setPointerVisible(bool _value)
	{
		MyGUI::PointerManager::getInstance().setVisible(_value);
	}

	void PointerManager::setPointerName(const std::string& _name)
	{
		MyGUI::PointerManager::getInstance().setPointer(_name);
	}

} // namespace input
