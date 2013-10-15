#include "stdheader.h"
#include "MainMenu.h"
#include "Game.h"

MainMenu::MainMenu(){
}

bool MainMenu::openStage(){
	if( mUI )return false;

	mUI.load("MainMenu.layout","Main");
	try{
		if( mUI ){
			mUI["Quit"]->eventMouseButtonClick += newDelegate(this, &MainMenu::notifyButtonClick);
			mUI["Builder"]->eventMouseButtonClick += newDelegate(this, &MainMenu::notifyButtonClick);
			mUI["Options"]->eventMouseButtonClick += newDelegate(this, &MainMenu::notifyButtonClick);
			mUI["Test"]->eventMouseButtonClick += newDelegate(this, &MainMenu::notifyButtonClick);
		}
	}catch(std::out_of_range& e){
	//	MYGUI_LOG(Warning, e.what());
	}
	return true;
}

void MainMenu::closeStage(){
	mUI.close();
}

void MainMenu::notifyButtonClick(MyGUI::Widget* _sender){
	if( mUI.IsWidget(_sender,"Quit") ){
		Game::getSingletonPtr()->quit();
	}else if( mUI.IsWidget(_sender,"Builder") ){
	}else if( mUI.IsWidget(_sender,"Options") ){
		Game::getSingletonPtr()->showOptionsDialog();
	}
}