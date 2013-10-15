/*!
	@file
	@author		Albert Semenov
	@date		09/2009
*/

#ifndef __INPUT_MANAGER_H__
#define __INPUT_MANAGER_H__

#include "Input.h"

namespace base
{

	class InputManager :
		public Input
	{
	public:
		InputManager();
		virtual ~InputManager();

		void createInput(size_t _handle);
		void destroyInput();
		void captureInput();
		void setInputViewSize(int _width, int _height);

		virtual void onFileDrop(const std::wstring& _filename) { }
		virtual bool onWinodwClose(size_t _handle) { return true; }

		void setMousePosition(int _x, int _y);
		void updateCursorPosition();

		enum KeyState
		{
			DOWN = 1,
			UP = 0,
		};

		//取得具体按键的状态
		KeyState getKeyState(MyGUI::KeyCode _key);

		void frameEvent(float _time);
		void computeMouseMove();

		void mouseMove(int _absx, int _absy, int _absz);
		void mousePress(int _absx, int _absy, MyGUI::MouseButton _id);
		void mouseRelease(int _absx, int _absy, MyGUI::MouseButton _id);
        //内部Cocoa调用函数
        MyGUI::EditBox* _getCurrentEditWidget();
        void _setKeyState(int _key,bool bks );
	private:
		int mWidth;
		int mHeight;
		static bool msSkipMove;
		int mMouseX;
		int mMouseY;
		int mMouseZ;
		bool mMouseMove;
		static unsigned char mKeyStates[256];
	};

} // namespace input

#endif // __INPUT_MANAGER_H__
