/*
 *  Created by john on 11-10-31.
	该代码来自于Ogre例子程序
 */

#ifndef __DEMO_OSX_H__
#define __DEMO_OSX_H__

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE & __OBJC__

#import <Cocoa/Cocoa.h>

void _rander_init();
bool _rander_one_frame();

// All this does is suppress some messages in the run log.  NSApplication does not
// implement buttonPressed and apps without a NIB have no target for the action.
@implementation NSApplication (_suppressUnimplementedActionWarning)
- (void) buttonPressed:(id)sender
{
    /* Do nothing */
}
@end


#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
@interface AppDelegate : NSObject <NSApplicationDelegate>
#else
@interface AppDelegate : NSObject
#endif
{
    NSTimer *mTimer;
    NSDate *mDate;
    NSTimeInterval mLastFrameTime;
}

- (void)go;
- (void)renderOneFrame:(id)sender;

@property (retain) NSTimer *mTimer;
@property (nonatomic) NSTimeInterval mLastFrameTime;

@end

@implementation AppDelegate

@synthesize mTimer;
@dynamic mLastFrameTime;

- (NSTimeInterval)mLastFrameTime
{
    return mLastFrameTime;
}

- (void)setLastFrameTime:(NSTimeInterval)frameInterval
{
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. The display link will only fire 30 times a second when the
    // frame internal is two on a display that refreshes 60 times a second. The default
    // frame interval setting of one will fire 60 times a second when the display refreshes
    // at 60 times a second. A frame interval setting of less than one results in undefined
    // behavior.
    if (frameInterval >= 1)
    {
        mLastFrameTime = frameInterval;
    }
}

- (void)go {
    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    mLastFrameTime = 1;
    mTimer = nil;

	_rander_init();
	mTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)(1.0f / 60.0f) * mLastFrameTime
												  target:self
												selector:@selector(renderOneFrame:)
												userInfo:nil
												 repeats:YES];
	[pool release];
}
		
- (void)applicationDidFinishLaunching:(NSNotification *)application {
	mLastFrameTime = 1;
	mTimer = nil;
			
	[self go];
}
		
- (void)renderOneFrame:(id)sender{
	if( _rander_one_frame() )
	{
		return;
	}else{
		[mTimer invalidate];
		mTimer = nil;
		[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
	}
}
		
- (void)dealloc {
	if(mTimer){
		[mTimer invalidate];
		mTimer = nil;
	}

	[super dealloc];
}
		
@end
#endif

#endif