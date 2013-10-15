/*!
	@file
	@author		Albert Semenov
	@date		02/2010
	@module
*/
#ifndef __INPUT_CONVERTER_H__
#define __INPUT_CONVERTER_H__

#include <MyGUI.h>

//#define INPUT_KEY_NAME

#ifdef INPUT_KEY_NAME
#include <vector>
#include <string>
#endif

namespace base
{
	// Windows virtual keys
	enum VirtualKey
	{
		VLK_LBUTTON = 0x01,
		VLK_RBUTTON = 0x02,
		VLK_CANCEL = 0x03,
		VLK_MBUTTON = 0x04, /* NOT contiguous with L & RBUTTON */

		VLK_XBUTTON1 = 0x05, /* NOT contiguous with L & RBUTTON */
		VLK_XBUTTON2 = 0x06, /* NOT contiguous with L & RBUTTON */

		/*
			0x07 : unassigned
		*/

		VLK_BACK = 0x33,//Carbon kVK_Delete
		VLK_TAB = 0x30,

		/*
			0x0A - 0x0B : reserved
		*/

		VLK_RETURN = 0x24,

		VLK_SHIFT = 0x38,
		VLK_CONTROL = 0x3B,
		VLK_CAPITAL = 0x39, /* Capslock */

		VLK_ESCAPE = 0x35,

        /*
		VLK_CONVERT = 0x1C,
		VLK_NONCONVERT = 0x1D,
		VLK_ACCEPT = 0x1E,
		VLK_MODECHANGE = 0x1F,
         */
		VLK_SPACE = 0x31,

		/*
         Carbon Events.h
		*/

		VLK_0 = 0x1D,
		VLK_1 = 0x12,
		VLK_2 = 0x13,
		VLK_3 = 0x14,
		VLK_4 = 0x15,
		VLK_5 = 0x17,
		VLK_6 = 0x16,
		VLK_7 = 0x1A,
		VLK_8 = 0x1C,
		VLK_9 = 0x19,
		VLK_A = 0x00,
		VLK_B = 0x0B,
		VLK_C = 0x08,
		VLK_D = 0x02,
		VLK_E = 0x0E,
		VLK_F = 0x03,
		VLK_G = 0x05,
		VLK_H = 0x04,
		VLK_I = 0x22,
		VLK_J = 0x26,
		VLK_K = 0x28,
		VLK_L = 0x25,
		VLK_M = 0x2E,
		VLK_N = 0x2D,
		VLK_O = 0x1F,
		VLK_P = 0x23,
		VLK_Q = 0x0C,
		VLK_R = 0x0F,
		VLK_S = 0x01,
		VLK_T = 0x11,
		VLK_U = 0x20,
		VLK_V = 0x09,
		VLK_W = 0x0D,
		VLK_X = 0x07,
		VLK_Y = 0x10,
		VLK_Z = 0x06,

		VLK_NUMPAD0 = 0x52,
		VLK_NUMPAD1 = 0x53,
		VLK_NUMPAD2 = 0x54,
		VLK_NUMPAD3 = 0x55,
		VLK_NUMPAD4 = 0x56,
		VLK_NUMPAD5 = 0x57,
		VLK_NUMPAD6 = 0x58,
		VLK_NUMPAD7 = 0x59,
		VLK_NUMPAD8 = 0x5B,
		VLK_NUMPAD9 = 0x5C,
		VLK_MULTIPLY = 0x43,
		VLK_ADD = 0x45,
		VLK_DECIMAL = 0x41,
		VLK_DIVIDE = 0x4B,
		VLK_F1 = 0x7A,
		VLK_F2 = 0x78,
		VLK_F3 = 0x63,
		VLK_F4 = 0x76,
		VLK_F5 = 0x60,
		VLK_F6 = 0x61,
		VLK_F7 = 0x62,
		VLK_F8 = 0x64,
		VLK_F9 = 0x65,
		VLK_F10 = 0x6D,
		VLK_F11 = 0x67,
		VLK_F12 = 0x6F,
		VLK_F13 = 0x69,
		VLK_F14 = 0x6B,
		VLK_F15 = 0x71,
		VLK_F16 = 0x6A,
		VLK_F17 = 0x40,
		VLK_F18 = 0x4F,
		VLK_F19 = 0x50,
		VLK_F20 = 0x5A,


        VLK_OEM_MINUS = 0x1B, //Carbon kVK_ANSI_Minus
        VLK_OEM_3 = 0x32, //Carbon kVK_ANSI_Grave
        VLK_OEM_PLUS = 0x18, //Carbon kVK_ANSI_Equal
        VLK_OEM_5 = 0x2A, //Carbon kVK_ANSI_Backslash
        VLK_OEM_4 = 0x21, //kVK_ANSI_LeftBracket
        VLK_OEM_6 = 0x1E, //kVK_ANSI_RightBracket
        VLK_OEM_1 = 0x29, //kVK_ANSI_Semicolon
        VLK_OEM_COMMA = 0x2B, //kVK_ANSI_Comma
        VLK_OEM_PERIOD = 0x2F, //kVK_ANSI_Period
        VLK_OEM_2 = 0x2C, //kVK_ANSI_Slash
        
		/*
			0x97 - 0x9F : unassigned
		*/

		/*
			VLK_L* & VLK_R* - left and right Alt, Ctrl and Shift virtual keys.
			Used only as parameters to GetAsyncKeyState() and GetKeyState().
			No other API or message will distinguish left and right keys in this way.
		*/
		VLK_LSHIFT = 0x38,
		VLK_RSHIFT = 0x3C,
		VLK_LCONTROL = 0x3B,
		VLK_RCONTROL = 0x3E,
        VLK_COMMAND = 0x37,
        VLK_OPTION = 0x3A,
        VLK_ROPTION = 0x3D,
        VLK_FUNCTION = 0x3F,

		VLK_VOLUME_MUTE = 0x4A,
		VLK_VOLUME_DOWN = 0x49,
		VLK_VOLUME_UP = 0x48,
        
		VLK_DELETE = 0x75,//Carbon kVK_ForwardDelete
        VLK_HELP = 0x72,
        
		VLK_RIGHT = 0x7C,
		VLK_DOWN = 0x7D,
		VLK_PRIOR = 0x74, //Page up
		VLK_NEXT = 0x79, //Page down
		VLK_END = 0x77,
		VLK_HOME = 0x73,
		VLK_LEFT = 0x7B,
		VLK_UP = 0x7E,
        
		VLK_MAX
	};

	// Keyboard scan codes - OIS, DirectInput
	enum ScanCode
	{
		SC_UNASSIGNED = 0x00,
		SC_ESCAPE = 0x01,
		SC_1 = 0x02,
		SC_2 = 0x03,
		SC_3 = 0x04,
		SC_4 = 0x05,
		SC_5 = 0x06,
		SC_6 = 0x07,
		SC_7 = 0x08,
		SC_8 = 0x09,
		SC_9 = 0x0A,
		SC_0 = 0x0B,
		SC_MINUS = 0x0C,    // - on main keyboard
		SC_EQUALS = 0x0D,
		SC_BACK = 0x0E,    // backspace
		SC_TAB = 0x0F,
		SC_Q = 0x10,
		SC_W = 0x11,
		SC_E = 0x12,
		SC_R = 0x13,
		SC_T = 0x14,
		SC_Y = 0x15,
		SC_U = 0x16,
		SC_I = 0x17,
		SC_O = 0x18,
		SC_P = 0x19,
		SC_LBRACKET = 0x1A,
		SC_RBRACKET = 0x1B,
		SC_RETURN = 0x1C,    // Enter on main keyboard
		SC_LCONTROL = 0x1D,
		SC_A = 0x1E,
		SC_S = 0x1F,
		SC_D = 0x20,
		SC_F = 0x21,
		SC_G = 0x22,
		SC_H = 0x23,
		SC_J = 0x24,
		SC_K = 0x25,
		SC_L = 0x26,
		SC_SEMICOLON = 0x27,
		SC_APOSTROPHE = 0x28,
		SC_GRAVE = 0x29,    // accent
		SC_LSHIFT = 0x2A,
		SC_BACKSLASH = 0x2B,
		SC_Z = 0x2C,
		SC_X = 0x2D,
		SC_C = 0x2E,
		SC_V = 0x2F,
		SC_B = 0x30,
		SC_N = 0x31,
		SC_M = 0x32,
		SC_COMMA = 0x33,
		SC_PERIOD = 0x34,    // . on main keyboard
		SC_SLASH = 0x35,    // / on main keyboard
		SC_RSHIFT = 0x36,
		SC_MULTIPLY = 0x37,    // * on numeric keypad
		SC_LMENU = 0x38,    // left Alt
		SC_SPACE = 0x39,
		SC_CAPITAL = 0x3A,
		SC_F1 = 0x3B,
		SC_F2 = 0x3C,
		SC_F3 = 0x3D,
		SC_F4 = 0x3E,
		SC_F5 = 0x3F,
		SC_F6 = 0x40,
		SC_F7 = 0x41,
		SC_F8 = 0x42,
		SC_F9 = 0x43,
		SC_F10 = 0x44,
		SC_NUMLOCK = 0x45,
		SC_SCROLL = 0x46,    // Scroll Lock
		SC_NUMPAD7 = 0x47,
		SC_NUMPAD8 = 0x48,
		SC_NUMPAD9 = 0x49,
		SC_SUBTRACT = 0x4A,    // - on numeric keypad
		SC_NUMPAD4 = 0x4B,
		SC_NUMPAD5 = 0x4C,
		SC_NUMPAD6 = 0x4D,
		SC_ADD = 0x4E,    // + on numeric keypad
		SC_NUMPAD1 = 0x4F,
		SC_NUMPAD2 = 0x50,
		SC_NUMPAD3 = 0x51,
		SC_NUMPAD0 = 0x52,
		SC_DECIMAL = 0x53,    // . on numeric keypad
		SC_OEM_102 = 0x56,    // < > | on UK/Germany keyboards
		SC_F11 = 0x57,
		SC_F12 = 0x58,
		SC_F13 = 0x64,    //                     (NEC PC98)
		SC_F14 = 0x65,    //                     (NEC PC98)
		SC_F15 = 0x66,    //                     (NEC PC98)
        SC_COMMAND = 0x67, //Apple command
		SC_KANA = 0x70,    // (Japanese keyboard)
		SC_ABNT_C1 = 0x73,    // / ? on Portugese (Brazilian) keyboards
		SC_CONVERT = 0x79,    // (Japanese keyboard)
		SC_NOCONVERT = 0x7B,    // (Japanese keyboard)
		SC_YEN = 0x7D,    // (Japanese keyboard)
		SC_ABNT_C2 = 0x7E,    // Numpad . on Portugese (Brazilian) keyboards
		SC_NUMPADEQUALS = 0x8D,    // = on numeric keypad (NEC PC98)
		SC_PREVTRACK = 0x90,    // Previous Track (SC_CIRCUMFLEX on Japanese keyboard)
		SC_AT = 0x91,    //                     (NEC PC98)
		SC_COLON = 0x92,    //                     (NEC PC98)
		SC_UNDERLINE = 0x93,    //                     (NEC PC98)
		SC_KANJI = 0x94,    // (Japanese keyboard)
		SC_STOP = 0x95,    //                     (NEC PC98)
		SC_AX = 0x96,    //                     (Japan AX)
		SC_UNLABELED = 0x97,    //                        (J3100)
		SC_NEXTTRACK = 0x99,    // Next Track
		SC_NUMPADENTER = 0x9C,    // Enter on numeric keypad
		SC_RCONTROL = 0x9D,
		SC_MUTE = 0xA0,    // Mute
		SC_CALCULATOR = 0xA1,    // Calculator
		SC_PLAYPAUSE = 0xA2,    // Play / Pause
		SC_MEDIASTOP = 0xA4,    // Media Stop
		SC_VOLUMEDOWN = 0xAE,    // Volume -
		SC_VOLUMEUP = 0xB0,    // Volume +
		SC_WEBHOME = 0xB2,    // Web home
		SC_NUMPADCOMMA = 0xB3,    // , on numeric keypad (NEC PC98)
		SC_DIVIDE = 0xB5,    // / on numeric keypad
		SC_SYSRQ = 0xB7,
		SC_RMENU = 0xB8,    // right Alt
		SC_PAUSE = 0xC5,    // Pause
		SC_HOME = 0xC7,    // Home on arrow keypad
		SC_UP = 0xC8,    // UpArrow on arrow keypad
		SC_PGUP = 0xC9,    // PgUp on arrow keypad
		SC_LEFT = 0xCB,    // LeftArrow on arrow keypad
		SC_RIGHT = 0xCD,    // RightArrow on arrow keypad
		SC_END = 0xCF,    // End on arrow keypad
		SC_DOWN = 0xD0,    // DownArrow on arrow keypad
		SC_PGDOWN = 0xD1,    // PgDn on arrow keypad
		SC_INSERT = 0xD2,    // Insert on arrow keypad
		SC_DELETE = 0xD3,    // Delete on arrow keypad
		SC_LWIN = 0xDB,    // Left Windows key
		SC_RWIN = 0xDC,    // Right Windows key
		SC_APPS = 0xDD,    // AppMenu key
		SC_POWER = 0xDE,    // System Power
		SC_SLEEP = 0xDF,    // System Sleep
		SC_WAKE = 0xE3,    // System Wake
		SC_WEBSEARCH = 0xE5,    // Web Search
		SC_WEBFAVORITES = 0xE6,    // Web Favorites
		SC_WEBREFRESH = 0xE7,    // Web Refresh
		SC_WEBSTOP = 0xE8,    // Web Stop
		SC_WEBFORWARD = 0xE9,    // Web Forward
		SC_WEBBACK = 0xEA,    // Web Back
		SC_MYCOMPUTER = 0xEB,    // My Computer
		SC_MAIL = 0xEC,    // Mail
		SC_MEDIASELECT = 0xED,     // Media Select
		SC_MAX
	};
	
	class Table
	{
	public:
		Table()
		{
#ifdef INPUT_KEY_NAME
			mVirtualKeyToName.resize(VLK_MAX);

			#define DECLARE_VIRTUAL_KEY(_key) mVirtualKeyToName[_key] = #_key;

			DECLARE_VIRTUAL_KEY(VLK_LBUTTON)
			DECLARE_VIRTUAL_KEY(VLK_RBUTTON)
			DECLARE_VIRTUAL_KEY(VLK_CANCEL)
			DECLARE_VIRTUAL_KEY(VLK_MBUTTON)

			DECLARE_VIRTUAL_KEY(VLK_XBUTTON1)
			DECLARE_VIRTUAL_KEY(VLK_XBUTTON2)

			DECLARE_VIRTUAL_KEY(VLK_BACK)
			DECLARE_VIRTUAL_KEY(VLK_TAB)

			DECLARE_VIRTUAL_KEY(VLK_CLEAR)
			DECLARE_VIRTUAL_KEY(VLK_RETURN)

			DECLARE_VIRTUAL_KEY(VLK_SHIFT)
			DECLARE_VIRTUAL_KEY(VLK_CONTROL)
			DECLARE_VIRTUAL_KEY(VLK_MENU)
			DECLARE_VIRTUAL_KEY(VLK_PAUSE)
			DECLARE_VIRTUAL_KEY(VLK_CAPITAL)

			DECLARE_VIRTUAL_KEY(VLK_KANA)
			DECLARE_VIRTUAL_KEY(VLK_HANGEUL)
			DECLARE_VIRTUAL_KEY(VLK_HANGUL)
			DECLARE_VIRTUAL_KEY(VLK_JUNJA)
			DECLARE_VIRTUAL_KEY(VLK_FINAL)
			DECLARE_VIRTUAL_KEY(VLK_HANJA)
			DECLARE_VIRTUAL_KEY(VLK_KANJI)

			DECLARE_VIRTUAL_KEY(VLK_ESCAPE)

			DECLARE_VIRTUAL_KEY(VLK_CONVERT)
			DECLARE_VIRTUAL_KEY(VLK_NONCONVERT)
			DECLARE_VIRTUAL_KEY(VLK_ACCEPT)
			DECLARE_VIRTUAL_KEY(VLK_MODECHANGE)

			DECLARE_VIRTUAL_KEY(VLK_SPACE)
			DECLARE_VIRTUAL_KEY(VLK_PRIOR)
			DECLARE_VIRTUAL_KEY(VLK_NEXT)
			DECLARE_VIRTUAL_KEY(VLK_END)
			DECLARE_VIRTUAL_KEY(VLK_HOME)
			DECLARE_VIRTUAL_KEY(VLK_LEFT)
			DECLARE_VIRTUAL_KEY(VLK_UP)
			DECLARE_VIRTUAL_KEY(VLK_RIGHT)
			DECLARE_VIRTUAL_KEY(VLK_DOWN)
			DECLARE_VIRTUAL_KEY(VLK_SELECT)
			DECLARE_VIRTUAL_KEY(VLK_PRINT)
			DECLARE_VIRTUAL_KEY(VLK_EXECUTE)
			DECLARE_VIRTUAL_KEY(VLK_SNAPSHOT)
			DECLARE_VIRTUAL_KEY(VLK_INSERT)
			DECLARE_VIRTUAL_KEY(VLK_DELETE)
			DECLARE_VIRTUAL_KEY(VLK_HELP)

			DECLARE_VIRTUAL_KEY(VLK_0)
			DECLARE_VIRTUAL_KEY(VLK_1)
			DECLARE_VIRTUAL_KEY(VLK_2)
			DECLARE_VIRTUAL_KEY(VLK_3)
			DECLARE_VIRTUAL_KEY(VLK_4)
			DECLARE_VIRTUAL_KEY(VLK_5)
			DECLARE_VIRTUAL_KEY(VLK_6)
			DECLARE_VIRTUAL_KEY(VLK_7)
			DECLARE_VIRTUAL_KEY(VLK_8)
			DECLARE_VIRTUAL_KEY(VLK_9)
			DECLARE_VIRTUAL_KEY(VLK_A)
			DECLARE_VIRTUAL_KEY(VLK_B)
			DECLARE_VIRTUAL_KEY(VLK_C)
			DECLARE_VIRTUAL_KEY(VLK_D)
			DECLARE_VIRTUAL_KEY(VLK_E)
			DECLARE_VIRTUAL_KEY(VLK_F)
			DECLARE_VIRTUAL_KEY(VLK_G)
			DECLARE_VIRTUAL_KEY(VLK_H)
			DECLARE_VIRTUAL_KEY(VLK_I)
			DECLARE_VIRTUAL_KEY(VLK_J)
			DECLARE_VIRTUAL_KEY(VLK_K)
			DECLARE_VIRTUAL_KEY(VLK_L)
			DECLARE_VIRTUAL_KEY(VLK_M)
			DECLARE_VIRTUAL_KEY(VLK_N)
			DECLARE_VIRTUAL_KEY(VLK_O)
			DECLARE_VIRTUAL_KEY(VLK_P)
			DECLARE_VIRTUAL_KEY(VLK_Q)
			DECLARE_VIRTUAL_KEY(VLK_R)
			DECLARE_VIRTUAL_KEY(VLK_S)
			DECLARE_VIRTUAL_KEY(VLK_T)
			DECLARE_VIRTUAL_KEY(VLK_U)
			DECLARE_VIRTUAL_KEY(VLK_V)
			DECLARE_VIRTUAL_KEY(VLK_W)
			DECLARE_VIRTUAL_KEY(VLK_X)
			DECLARE_VIRTUAL_KEY(VLK_Y)
			DECLARE_VIRTUAL_KEY(VLK_Z)

			DECLARE_VIRTUAL_KEY(VLK_LWIN)
			DECLARE_VIRTUAL_KEY(VLK_RWIN)
			DECLARE_VIRTUAL_KEY(VLK_APPS)

			DECLARE_VIRTUAL_KEY(VLK_SLEEP)

			DECLARE_VIRTUAL_KEY(VLK_NUMPAD0)
			DECLARE_VIRTUAL_KEY(VLK_NUMPAD1)
			DECLARE_VIRTUAL_KEY(VLK_NUMPAD2)
			DECLARE_VIRTUAL_KEY(VLK_NUMPAD3)
			DECLARE_VIRTUAL_KEY(VLK_NUMPAD4)
			DECLARE_VIRTUAL_KEY(VLK_NUMPAD5)
			DECLARE_VIRTUAL_KEY(VLK_NUMPAD6)
			DECLARE_VIRTUAL_KEY(VLK_NUMPAD7)
			DECLARE_VIRTUAL_KEY(VLK_NUMPAD8)
			DECLARE_VIRTUAL_KEY(VLK_NUMPAD9)
			DECLARE_VIRTUAL_KEY(VLK_MULTIPLY)
			DECLARE_VIRTUAL_KEY(VLK_ADD)
			DECLARE_VIRTUAL_KEY(VLK_SEPARATOR)
			DECLARE_VIRTUAL_KEY(VLK_SUBTRACT)
			DECLARE_VIRTUAL_KEY(VLK_DECIMAL)
			DECLARE_VIRTUAL_KEY(VLK_DIVIDE)
			DECLARE_VIRTUAL_KEY(VLK_F1)
			DECLARE_VIRTUAL_KEY(VLK_F2)
			DECLARE_VIRTUAL_KEY(VLK_F3)
			DECLARE_VIRTUAL_KEY(VLK_F4)
			DECLARE_VIRTUAL_KEY(VLK_F5)
			DECLARE_VIRTUAL_KEY(VLK_F6)
			DECLARE_VIRTUAL_KEY(VLK_F7)
			DECLARE_VIRTUAL_KEY(VLK_F8)
			DECLARE_VIRTUAL_KEY(VLK_F9)
			DECLARE_VIRTUAL_KEY(VLK_F10)
			DECLARE_VIRTUAL_KEY(VLK_F11)
			DECLARE_VIRTUAL_KEY(VLK_F12)
			DECLARE_VIRTUAL_KEY(VLK_F13)
			DECLARE_VIRTUAL_KEY(VLK_F14)
			DECLARE_VIRTUAL_KEY(VLK_F15)
			DECLARE_VIRTUAL_KEY(VLK_F16)
			DECLARE_VIRTUAL_KEY(VLK_F17)
			DECLARE_VIRTUAL_KEY(VLK_F18)
			DECLARE_VIRTUAL_KEY(VLK_F19)
			DECLARE_VIRTUAL_KEY(VLK_F20)
			DECLARE_VIRTUAL_KEY(VLK_F21)
			DECLARE_VIRTUAL_KEY(VLK_F22)
			DECLARE_VIRTUAL_KEY(VLK_F23)
			DECLARE_VIRTUAL_KEY(VLK_F24)

			DECLARE_VIRTUAL_KEY(VLK_NUMLOCK)
			DECLARE_VIRTUAL_KEY(VLK_SCROLL)

			DECLARE_VIRTUAL_KEY(VLK_OEM_NEC_EQUAL)

			DECLARE_VIRTUAL_KEY(VLK_OEM_FJ_JISHO)
			DECLARE_VIRTUAL_KEY(VLK_OEM_FJ_MASSHOU)
			DECLARE_VIRTUAL_KEY(VLK_OEM_FJ_TOUROKU)
			DECLARE_VIRTUAL_KEY(VLK_OEM_FJ_LOYA)
			DECLARE_VIRTUAL_KEY(VLK_OEM_FJ_ROYA)

			DECLARE_VIRTUAL_KEY(VLK_LSHIFT)
			DECLARE_VIRTUAL_KEY(VLK_RSHIFT)
			DECLARE_VIRTUAL_KEY(VLK_LCONTROL)
			DECLARE_VIRTUAL_KEY(VLK_RCONTROL)
			DECLARE_VIRTUAL_KEY(VLK_LMENU)
			DECLARE_VIRTUAL_KEY(VLK_RMENU)

			DECLARE_VIRTUAL_KEY(VLK_BROWSER_BACK)
			DECLARE_VIRTUAL_KEY(VLK_BROWSER_FORWARD)
			DECLARE_VIRTUAL_KEY(VLK_BROWSER_REFRESH)
			DECLARE_VIRTUAL_KEY(VLK_BROWSER_STOP)
			DECLARE_VIRTUAL_KEY(VLK_BROWSER_SEARCH)
			DECLARE_VIRTUAL_KEY(VLK_BROWSER_FAVORITES)
			DECLARE_VIRTUAL_KEY(VLK_BROWSER_HOME)

			DECLARE_VIRTUAL_KEY(VLK_VOLUME_MUTE)
			DECLARE_VIRTUAL_KEY(VLK_VOLUME_DOWN)
			DECLARE_VIRTUAL_KEY(VLK_VOLUME_UP)
			DECLARE_VIRTUAL_KEY(VLK_MEDIA_NEXT_TRACK)
			DECLARE_VIRTUAL_KEY(VLK_MEDIA_PREV_TRACK)
			DECLARE_VIRTUAL_KEY(VLK_MEDIA_STOP)
			DECLARE_VIRTUAL_KEY(VLK_MEDIA_PLAY_PAUSE)
			DECLARE_VIRTUAL_KEY(VLK_LAUNCH_MAIL)
			DECLARE_VIRTUAL_KEY(VLK_LAUNCH_MEDIA_SELECT)
			DECLARE_VIRTUAL_KEY(VLK_LAUNCH_APP1)
			DECLARE_VIRTUAL_KEY(VLK_LAUNCH_APP2)

			DECLARE_VIRTUAL_KEY(VLK_OEM_1)
			DECLARE_VIRTUAL_KEY(VLK_OEM_PLUS)
			DECLARE_VIRTUAL_KEY(VLK_OEM_COMMA)
			DECLARE_VIRTUAL_KEY(VLK_OEM_MINUS)
			DECLARE_VIRTUAL_KEY(VLK_OEM_PERIOD)
			DECLARE_VIRTUAL_KEY(VLK_OEM_2)
			DECLARE_VIRTUAL_KEY(VLK_OEM_3)

			DECLARE_VIRTUAL_KEY(VLK_OEM_4)
			DECLARE_VIRTUAL_KEY(VLK_OEM_5)
			DECLARE_VIRTUAL_KEY(VLK_OEM_6)
			DECLARE_VIRTUAL_KEY(VLK_OEM_7)
			DECLARE_VIRTUAL_KEY(VLK_OEM_8)

			DECLARE_VIRTUAL_KEY(VLK_OEM_AX)
			DECLARE_VIRTUAL_KEY(VLK_OEM_102)
			DECLARE_VIRTUAL_KEY(VLK_ICO_HELP)
			DECLARE_VIRTUAL_KEY(VLK_ICO_00)

			DECLARE_VIRTUAL_KEY(VLK_PROCESSKEY)

			DECLARE_VIRTUAL_KEY(VLK_ICO_CLEAR)

			DECLARE_VIRTUAL_KEY(VLK_PACKET)

			DECLARE_VIRTUAL_KEY(VLK_OEM_RESET)
			DECLARE_VIRTUAL_KEY(VLK_OEM_JUMP)
			DECLARE_VIRTUAL_KEY(VLK_OEM_PA1)
			DECLARE_VIRTUAL_KEY(VLK_OEM_PA2)
			DECLARE_VIRTUAL_KEY(VLK_OEM_PA3)
			DECLARE_VIRTUAL_KEY(VLK_OEM_WSCTRL)
			DECLARE_VIRTUAL_KEY(VLK_OEM_CUSEL)
			DECLARE_VIRTUAL_KEY(VLK_OEM_ATTN)
			DECLARE_VIRTUAL_KEY(VLK_OEM_FINISH)
			DECLARE_VIRTUAL_KEY(VLK_OEM_COPY)
			DECLARE_VIRTUAL_KEY(VLK_OEM_AUTO)
			DECLARE_VIRTUAL_KEY(VLK_OEM_ENLW)
			DECLARE_VIRTUAL_KEY(VLK_OEM_BACKTAB)

			DECLARE_VIRTUAL_KEY(VLK_ATTN)
			DECLARE_VIRTUAL_KEY(VLK_CRSEL)
			DECLARE_VIRTUAL_KEY(VLK_EXSEL)
			DECLARE_VIRTUAL_KEY(VLK_EREOF)
			DECLARE_VIRTUAL_KEY(VLK_PLAY)
			DECLARE_VIRTUAL_KEY(VLK_ZOOM)
			DECLARE_VIRTUAL_KEY(VLK_NONAME)
			DECLARE_VIRTUAL_KEY(VLK_PA1)
			DECLARE_VIRTUAL_KEY(VLK_OEM_CLEAR)

			#undef DECLARE_VIRTUAL_KEY


			mScanCodeToName.resize(SC_MAX);
			#define DECLARE_SCAN_CODE(_code) mScanCodeToName[_code] = #_code;

			DECLARE_SCAN_CODE(SC_ESCAPE)
			DECLARE_SCAN_CODE(SC_1)
			DECLARE_SCAN_CODE(SC_2)
			DECLARE_SCAN_CODE(SC_3)
			DECLARE_SCAN_CODE(SC_4)
			DECLARE_SCAN_CODE(SC_5)
			DECLARE_SCAN_CODE(SC_6)
			DECLARE_SCAN_CODE(SC_7)
			DECLARE_SCAN_CODE(SC_8)
			DECLARE_SCAN_CODE(SC_9)
			DECLARE_SCAN_CODE(SC_0)
			DECLARE_SCAN_CODE(SC_MINUS)
			DECLARE_SCAN_CODE(SC_EQUALS)
			DECLARE_SCAN_CODE(SC_BACK)
			DECLARE_SCAN_CODE(SC_TAB)
			DECLARE_SCAN_CODE(SC_Q)
			DECLARE_SCAN_CODE(SC_W)
			DECLARE_SCAN_CODE(SC_E)
			DECLARE_SCAN_CODE(SC_R)
			DECLARE_SCAN_CODE(SC_T)
			DECLARE_SCAN_CODE(SC_Y)
			DECLARE_SCAN_CODE(SC_U)
			DECLARE_SCAN_CODE(SC_I)
			DECLARE_SCAN_CODE(SC_O)
			DECLARE_SCAN_CODE(SC_P)
			DECLARE_SCAN_CODE(SC_LBRACKET)
			DECLARE_SCAN_CODE(SC_RBRACKET)
			DECLARE_SCAN_CODE(SC_RETURN)
			DECLARE_SCAN_CODE(SC_LCONTROL)
			DECLARE_SCAN_CODE(SC_A)
			DECLARE_SCAN_CODE(SC_S)
			DECLARE_SCAN_CODE(SC_D)
			DECLARE_SCAN_CODE(SC_F)
			DECLARE_SCAN_CODE(SC_G)
			DECLARE_SCAN_CODE(SC_H)
			DECLARE_SCAN_CODE(SC_J)
			DECLARE_SCAN_CODE(SC_K)
			DECLARE_SCAN_CODE(SC_L)
			DECLARE_SCAN_CODE(SC_SEMICOLON)
			DECLARE_SCAN_CODE(SC_APOSTROPHE)
			DECLARE_SCAN_CODE(SC_GRAVE)
			DECLARE_SCAN_CODE(SC_LSHIFT)
			DECLARE_SCAN_CODE(SC_BACKSLASH)
			DECLARE_SCAN_CODE(SC_Z)
			DECLARE_SCAN_CODE(SC_X)
			DECLARE_SCAN_CODE(SC_C)
			DECLARE_SCAN_CODE(SC_V)
			DECLARE_SCAN_CODE(SC_B)
			DECLARE_SCAN_CODE(SC_N)
			DECLARE_SCAN_CODE(SC_M)
			DECLARE_SCAN_CODE(SC_COMMA)
			DECLARE_SCAN_CODE(SC_PERIOD)
			DECLARE_SCAN_CODE(SC_SLASH)
			DECLARE_SCAN_CODE(SC_RSHIFT)
			DECLARE_SCAN_CODE(SC_MULTIPLY)
			DECLARE_SCAN_CODE(SC_LMENU)
			DECLARE_SCAN_CODE(SC_SPACE)
			DECLARE_SCAN_CODE(SC_CAPITAL)
			DECLARE_SCAN_CODE(SC_F1)
			DECLARE_SCAN_CODE(SC_F2)
			DECLARE_SCAN_CODE(SC_F3)
			DECLARE_SCAN_CODE(SC_F4)
			DECLARE_SCAN_CODE(SC_F5)
			DECLARE_SCAN_CODE(SC_F6)
			DECLARE_SCAN_CODE(SC_F7)
			DECLARE_SCAN_CODE(SC_F8)
			DECLARE_SCAN_CODE(SC_F9)
			DECLARE_SCAN_CODE(SC_F10)
			DECLARE_SCAN_CODE(SC_NUMLOCK)
			DECLARE_SCAN_CODE(SC_SCROLL)
			DECLARE_SCAN_CODE(SC_NUMPAD7)
			DECLARE_SCAN_CODE(SC_NUMPAD8)
			DECLARE_SCAN_CODE(SC_NUMPAD9)
			DECLARE_SCAN_CODE(SC_SUBTRACT)
			DECLARE_SCAN_CODE(SC_NUMPAD4)
			DECLARE_SCAN_CODE(SC_NUMPAD5)
			DECLARE_SCAN_CODE(SC_NUMPAD6)
			DECLARE_SCAN_CODE(SC_ADD)
			DECLARE_SCAN_CODE(SC_NUMPAD1)
			DECLARE_SCAN_CODE(SC_NUMPAD2)
			DECLARE_SCAN_CODE(SC_NUMPAD3)
			DECLARE_SCAN_CODE(SC_NUMPAD0)
			DECLARE_SCAN_CODE(SC_DECIMAL)
			DECLARE_SCAN_CODE(SC_OEM_102)
			DECLARE_SCAN_CODE(SC_F11)
			DECLARE_SCAN_CODE(SC_F12)
			DECLARE_SCAN_CODE(SC_F13)
			DECLARE_SCAN_CODE(SC_F14)
			DECLARE_SCAN_CODE(SC_F15)
			DECLARE_SCAN_CODE(SC_KANA)
			DECLARE_SCAN_CODE(SC_ABNT_C1)
			DECLARE_SCAN_CODE(SC_CONVERT)
			DECLARE_SCAN_CODE(SC_NOCONVERT)
			DECLARE_SCAN_CODE(SC_YEN)
			DECLARE_SCAN_CODE(SC_ABNT_C2)
			DECLARE_SCAN_CODE(SC_NUMPADEQUALS)
			DECLARE_SCAN_CODE(SC_PREVTRACK)
			DECLARE_SCAN_CODE(SC_AT)
			DECLARE_SCAN_CODE(SC_COLON)
			DECLARE_SCAN_CODE(SC_UNDERLINE)
			DECLARE_SCAN_CODE(SC_KANJI)
			DECLARE_SCAN_CODE(SC_STOP)
			DECLARE_SCAN_CODE(SC_AX)
			DECLARE_SCAN_CODE(SC_UNLABELED)
			DECLARE_SCAN_CODE(SC_NEXTTRACK)
			DECLARE_SCAN_CODE(SC_NUMPADENTER)
			DECLARE_SCAN_CODE(SC_RCONTROL)
			DECLARE_SCAN_CODE(SC_MUTE)
			DECLARE_SCAN_CODE(SC_CALCULATOR)
			DECLARE_SCAN_CODE(SC_PLAYPAUSE)
			DECLARE_SCAN_CODE(SC_MEDIASTOP)
			DECLARE_SCAN_CODE(SC_VOLUMEDOWN)
			DECLARE_SCAN_CODE(SC_VOLUMEUP)
			DECLARE_SCAN_CODE(SC_WEBHOME)
			DECLARE_SCAN_CODE(SC_NUMPADCOMMA)
			DECLARE_SCAN_CODE(SC_DIVIDE)
			DECLARE_SCAN_CODE(SC_SYSRQ)
			DECLARE_SCAN_CODE(SC_RMENU)
			DECLARE_SCAN_CODE(SC_PAUSE)
			DECLARE_SCAN_CODE(SC_HOME)
			DECLARE_SCAN_CODE(SC_UP)
			DECLARE_SCAN_CODE(SC_PGUP)
			DECLARE_SCAN_CODE(SC_LEFT)
			DECLARE_SCAN_CODE(SC_RIGHT)
			DECLARE_SCAN_CODE(SC_END)
			DECLARE_SCAN_CODE(SC_DOWN)
			DECLARE_SCAN_CODE(SC_PGDOWN)
			DECLARE_SCAN_CODE(SC_INSERT)
			DECLARE_SCAN_CODE(SC_DELETE)
			DECLARE_SCAN_CODE(SC_LWIN)
			DECLARE_SCAN_CODE(SC_RWIN)
			DECLARE_SCAN_CODE(SC_APPS)
			DECLARE_SCAN_CODE(SC_POWER)
			DECLARE_SCAN_CODE(SC_SLEEP)
			DECLARE_SCAN_CODE(SC_WAKE)
			DECLARE_SCAN_CODE(SC_WEBSEARCH)
			DECLARE_SCAN_CODE(SC_WEBFAVORITES)
			DECLARE_SCAN_CODE(SC_WEBREFRESH)
			DECLARE_SCAN_CODE(SC_WEBSTOP)
			DECLARE_SCAN_CODE(SC_WEBFORWARD)
			DECLARE_SCAN_CODE(SC_WEBBACK)
			DECLARE_SCAN_CODE(SC_MYCOMPUTER)
			DECLARE_SCAN_CODE(SC_MAIL)
			DECLARE_SCAN_CODE(SC_MEDIASELECT)

			#undef DECLARE_SCAN_CODE
#endif

			memset(mVirtualKeyToScanCode, 0, VLK_MAX);
			memset(mScanCodeToVirtualKey, 0, SC_MAX);

			#define ADD_MAP(_keyName) \
				mVirtualKeyToScanCode[VLK_##_keyName] = SC_##_keyName; \
				mScanCodeToVirtualKey[SC_##_keyName] = VLK_##_keyName;
			#define ADD_MAP2(_virtualKey, _scanCode) \
				mVirtualKeyToScanCode[VLK_##_virtualKey] = SC_##_scanCode; \
				mScanCodeToVirtualKey[SC_##_scanCode] = VLK_##_virtualKey;

			ADD_MAP(0)
			ADD_MAP(1)
			ADD_MAP(2)
			ADD_MAP(3)
			ADD_MAP(4)
			ADD_MAP(5)
			ADD_MAP(6)
			ADD_MAP(7)
			ADD_MAP(8)
			ADD_MAP(9)

			ADD_MAP(A)
			ADD_MAP(B)
			ADD_MAP(C)
			ADD_MAP(D)
			ADD_MAP(E)
			ADD_MAP(F)
			ADD_MAP(G)
			ADD_MAP(H)
			ADD_MAP(I)
			ADD_MAP(J)
			ADD_MAP(K)
			ADD_MAP(L)
			ADD_MAP(M)
			ADD_MAP(N)
			ADD_MAP(O)
			ADD_MAP(P)
			ADD_MAP(Q)
			ADD_MAP(R)
			ADD_MAP(S)
			ADD_MAP(T)
			ADD_MAP(U)
			ADD_MAP(V)
			ADD_MAP(W)
			ADD_MAP(X)
			ADD_MAP(Y)
			ADD_MAP(Z)

			ADD_MAP(F1)
			ADD_MAP(F2)
			ADD_MAP(F3)
			ADD_MAP(F4)
			ADD_MAP(F5)
			ADD_MAP(F6)
			ADD_MAP(F7)
			ADD_MAP(F8)
			ADD_MAP(F9)
			ADD_MAP(F10)
			ADD_MAP(F11)
			ADD_MAP(F12)
			ADD_MAP(F13)
			ADD_MAP(F14)
			ADD_MAP(F15)

			ADD_MAP(NUMPAD0)
			ADD_MAP(NUMPAD1)
			ADD_MAP(NUMPAD2)
			ADD_MAP(NUMPAD3)
			ADD_MAP(NUMPAD4)
			ADD_MAP(NUMPAD5)
			ADD_MAP(NUMPAD6)
			ADD_MAP(NUMPAD7)
			ADD_MAP(NUMPAD8)
			ADD_MAP(NUMPAD9)

			ADD_MAP(ESCAPE)
			ADD_MAP(TAB)
			ADD_MAP(RETURN)
			ADD_MAP(SPACE)
			ADD_MAP(BACK)
            
            ADD_MAP(COMMAND) //Apple command
            
			ADD_MAP2(OEM_3, GRAVE)
			ADD_MAP2(OEM_MINUS, MINUS)
			ADD_MAP2(OEM_PLUS, EQUALS)
			ADD_MAP2(OEM_5, BACKSLASH)
			ADD_MAP2(OEM_4, LBRACKET)
			ADD_MAP2(OEM_6, RBRACKET)
			ADD_MAP2(CAPITAL, CAPITAL)
			ADD_MAP2(OEM_1, SEMICOLON)
			ADD_MAP2(SHIFT, LSHIFT)
			ADD_MAP2(OEM_COMMA, COMMA)
			ADD_MAP2(OEM_PERIOD, PERIOD)
			ADD_MAP2(OEM_2, SLASH)
			ADD_MAP2(CONTROL, LCONTROL)
            ADD_MAP2(OPTION,LMENU)
            
			ADD_MAP2(LEFT, LEFT)
			ADD_MAP2(RIGHT, RIGHT)
			ADD_MAP2(UP, UP)
			ADD_MAP2(DOWN, DOWN)
			ADD_MAP2(DELETE, DELETE)
			ADD_MAP2(HOME, HOME)
			ADD_MAP2(END, END)
			ADD_MAP2(PRIOR, PGUP)
			ADD_MAP2(NEXT, PGDOWN)

			ADD_MAP2(DIVIDE, DIVIDE)
			ADD_MAP2(MULTIPLY, MULTIPLY)
			ADD_MAP2(ADD, ADD)
			ADD_MAP2(DECIMAL, DECIMAL)

			ADD_MAP(NUMPAD0)
			ADD_MAP(NUMPAD1)
			ADD_MAP(NUMPAD2)
			ADD_MAP(NUMPAD3)
			ADD_MAP(NUMPAD4)
			ADD_MAP(NUMPAD5)
			ADD_MAP(NUMPAD6)
			ADD_MAP(NUMPAD7)
			ADD_MAP(NUMPAD8)
			ADD_MAP(NUMPAD9)


			#undef ADD_MAP
			#undef ADD_MAP2
		}

		int VirtualKeyToScanCode(unsigned short _virtualKey) const
		{
			if (_virtualKey < VLK_MAX)
				return (int)mVirtualKeyToScanCode[_virtualKey];
			return 0;
		}

		int ScanCodeToVirtualKey(int _scanCode) const
		{
			if (_scanCode < SC_MAX)
				return (int)mScanCodeToVirtualKey[_scanCode];
			return 0;
		}

#ifdef INPUT_KEY_NAME
		const std::string VirtualKeyToName(WPARAM _virtualKey) const
		{
			if (_virtualKey < VLK_MAX)
				return mVirtualKeyToName[_virtualKey];
			return "";
		}

		const std::string ScanCodeToName(int _scanCode) const
		{
			if (_scanCode < SC_MAX)
				return mScanCodeToName[_scanCode];
			return "";
		}
#endif


	private:
		unsigned char mVirtualKeyToScanCode[VLK_MAX];
		unsigned char mScanCodeToVirtualKey[SC_MAX];

#ifdef INPUT_KEY_NAME
		std::vector<std::string> mVirtualKeyToName;
		std::vector<std::string> mScanCodeToName;
#endif
	};

	const Table& getTable()
	{
		static Table table;
		return table;
	}

	int VirtualKeyToScanCode(unsigned short _virtualKey)
	{
		const Table& table = getTable();
		return table.VirtualKeyToScanCode(_virtualKey);
	}

	int ScanCodeToVirtualKey(int _scanCode)
	{
		const Table& table = getTable();
		return table.ScanCodeToVirtualKey(_scanCode);
	}

#ifdef INPUT_KEY_NAME

	std::string VirtualKeyToName(WPARAM _virtualKey)
	{
		const Table& table = getTable();
		return table.VirtualKeyToName(_virtualKey);
	}

	std::string ScanCodeToName(int _scanCode)
	{
		const Table& table = getTable();
		return table.ScanCodeToName(_scanCode);
	}

#endif

}

#endif // __INPUT_CONVERTER_H__
