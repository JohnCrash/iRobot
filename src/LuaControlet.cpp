#include "stdheader.h"
#include "LuaControlet.h"
#include "LuaManager.h"

void LuaControlet::injectMouseMove(int _absx, int _absy, int _absz)
{
	LuaManager::getSingleton().callEvent3Int("eventMouseMove",_absx,_absy,_absz);
}

void LuaControlet::injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id)
{
	LuaManager::getSingleton().callEvent3Int("eventMousePress",_absx,_absy,_id.getValue());
}

void LuaControlet::injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id)
{
	LuaManager::getSingleton().callEvent3Int("eventMouseRelease",_absx,_absy,_id.getValue());
}

void LuaControlet::injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text)
{
	LuaManager::getSingleton().callEvent2Int("eventKeyPress",_key.getValue(),(int)_text);
}

void LuaControlet::injectKeyRelease(MyGUI::KeyCode _key)
{
	LuaManager::getSingleton().callEventInt("eventKeyRelease",_key.getValue());
}

bool LuaControlet::frameStarted(const Ogre::FrameEvent& evt)
{
	LuaManager::getSingleton().callEventNumber("eventFrameStarted",evt.timeSinceLastFrame);
	return true;
}

bool LuaControlet::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	LuaManager::getSingleton().callEventNumber("eventFrameRenderingQueued",evt.timeSinceLastFrame);
	return true;
}

bool LuaControlet::frameEnded(const Ogre::FrameEvent& evt)
{
	LuaManager::getSingleton().callEventNumber("eventFrameEnded",evt.timeSinceLastFrame);
	return true;
}