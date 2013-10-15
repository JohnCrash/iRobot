#ifndef _INPUT_H_
#define _INPUT_H_

namespace base
{
	class Input
	{
	public:
		virtual void injectMouseMove(int _absx, int _absy, int _absz) =0;
		virtual void injectMousePress(int _absx, int _absy, MyGUI::MouseButton _id) =0;
		virtual void injectMouseRelease(int _absx, int _absy, MyGUI::MouseButton _id) =0;
		virtual void injectKeyPress(MyGUI::KeyCode _key, MyGUI::Char _text) =0;
		virtual void injectKeyRelease(MyGUI::KeyCode _key) =0;
	};
}
#endif