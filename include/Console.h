#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#define STDOUT_BUFFER_MAX_SIZE (1024*8)
#define STDOUT_BUFFER_MAX_CAPS (STDOUT_BUFFER_MAX_SIZE+STDOUT_BUFFER_MAX_SIZE/2)

class Console{
public:
	Console();
	~Console();

	void openConsole();
	void closeConsole();
	//因为MyGUI不是多线程安全的
	//这里在主线程更新控制台内容
	void updateConsole();
	bool isOpen();
private:
	enum{
		READ=0,
		WRITE=1
	};

	void read_stdout();
	void _eventKeyButtonPressed(MyGUI::Widget* _sender, MyGUI::KeyCode _key, MyGUI::Char _char);

	std::string mOut;
	FILE* mStdOut;
	//读线程
	boost::thread* mThread;
	MyGUI::EditBox* mEdit;
	MyGUI::EditBox* mInput;

	int mPipe[2]; //一个管道
	FILE	mOldStdout;

	MyGUI::UString mAppendStr;
};

#endif