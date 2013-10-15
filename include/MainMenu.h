#ifndef _MAINMENU_H_
#define _MAINMENU_H_
#include "Stage.h"
#include "SimpleUI.h"

class MainMenu:public Stage
{
public:
	MainMenu();
	virtual bool openStage();
	virtual void closeStage();

protected:
	void notifyButtonClick(MyGUI::Widget* _sender);

	SimpleUI mUI;
};

#endif