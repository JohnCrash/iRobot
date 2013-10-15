/*
 *  Created by john on 11-11-2.
 */
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE & __OBJC__

#include "Main.h"

static id mAppDelegate;

int main(int argc, char *argv[])
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    mAppDelegate = [[AppDelegate alloc] init];
    [[NSApplication sharedApplication] setDelegate:mAppDelegate];
	int retVal = NSApplicationMain(argc, (const char **) argv);
	
	[pool release];
	
	return retVal;
}

#else
#include "stdheader.h"
#include "Game.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT argc)
{
	tGame app;
	try
	{
		app.run();
	}
	catch(MyGUI::Exception& e)
	{
		std::cerr << "An exception has occured" << " : " << e.getFullDescription().c_str();
	}
}

#endif