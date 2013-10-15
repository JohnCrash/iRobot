/*!
	@file
	@author		Albert Semenov
	@date		09/2009
*/

#include "stdheader.h"
#include "mac/InputManager.h"
#include "mac/InputConverter.h"
#include <Cocoa/Cocoa.h>

static base::InputManager* sInputMger = 0;
    
@interface InputResponder : NSResponder <NSTextInputClient>
{
    NSTextInputContext *inputContext;
    CGFloat deltaZ;
    CGFloat mWidth;
    CGFloat mHeight;
    NSRange markedRange;
    NSRange selectedRange;
    bool bDoHandle;
    bool bKeydown;
    MyGUI::EditBox* mCurrentEdit;
}

@property(assign) NSWindow* Window;

@end

@implementation InputResponder

- (InputResponder *)initWidthWindow:(NSWindow *) _window
{
    deltaZ = 0;
    bDoHandle = false;
    bKeydown = false;
    [self setWindow:_window];
    //[window makeFirstResponder:gInputResponder];
    [_window setNextResponder:self];
    [_window setAcceptsMouseMovedEvents:YES];
    
    selectedRange = NSMakeRange(0, 0);
    markedRange = NSMakeRange(NSNotFound, 0);
    
    inputContext = [[NSTextInputContext alloc] initWithClient:self];
    [inputContext activate];
    return self;
}

- (void)setWidth:(CGFloat)w Height:(CGFloat)h
{
    mWidth = w;
    mHeight = h;
}

- (oneway void)release
{
    [inputContext deactivate];
    [inputContext release];
}

/*
    NSTextInputClient Protocol
 */
- (BOOL)hasMarkedText
{
    return (markedRange.location == NSNotFound ? NO : YES);
}

- (NSRange)markedRange
{
    return markedRange;
}

- (NSRange)selectedRange
{
    return selectedRange;
}

- (NSInteger)windowLevel {
    // This method is optional but easy to implement
    return [self.Window level];
}

- (CGFloat)fractionOfDistanceThroughGlyphForPoint:(NSPoint)aPoint {
}

- (CGFloat)baselineDeltaForCharacterAtIndex:(NSUInteger)anIndex {
}

- (void)unmarkText
{
    markedRange = NSMakeRange(NSNotFound, 0);
    [inputContext discardMarkedText];
}

- (NSArray *)validAttributesForMarkedText
{
    return [NSArray array];   
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
    if (actualRange) {
        *actualRange = aRange;
    }
    return nullptr;
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)newSelection replacementRange:(NSRange)replacementRange
{
    assert(mCurrentEdit);
    NSString* ns;
    
    if( [aString isKindOfClass:[NSAttributedString class]] )
    {
        ns = [aString string];
    }
    else
    {
        ns = (NSString*)aString;
    }
    
    if( markedRange.location == NSNotFound )
    {
        markedRange = NSMakeRange(mCurrentEdit->getTextCursor(), [aString length]);
        mCurrentEdit->insertText(MyGUI::UString([ns cStringUsingEncoding:NSUTF16StringEncoding]),markedRange.location);
    }
    else if( [aString length]==0 )
    {
        if( markedRange.location != NSNotFound )
            mCurrentEdit->eraseText(markedRange.location,markedRange.length);
        markedRange = NSMakeRange(NSNotFound, 0);
    }
    else
    {
        int length = [aString length];
        if( markedRange.length > length )
        {
            mCurrentEdit->eraseText(markedRange.location+length,markedRange.length-length);
        }
        else if( markedRange.length < length )
        {
            NSString* sub = [ns substringFromIndex:markedRange.length];
            mCurrentEdit->insertText(MyGUI::UString([sub cStringUsingEncoding:NSUTF8StringEncoding]),mCurrentEdit->getTextCursor());
        }
        markedRange.length = length;
    }
    
    selectedRange.location = markedRange.location + markedRange.length;
    selectedRange.length = newSelection.length;

    bDoHandle = true;
}

- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange
{
    assert(mCurrentEdit);
    
    if( [aString isKindOfClass:[NSString class]] )
    {
        if(markedRange.location!=NSNotFound)
        {
            mCurrentEdit->setTextSelection(markedRange.location,markedRange.location+markedRange.length);
            mCurrentEdit->deleteTextSelection();
        }
        for( int i = 0;i<[aString length];++i )
        {
            sInputMger->injectKeyPress(MyGUI::KeyCode::None,[aString characterAtIndex:i]);
        }
    }

    markedRange = NSMakeRange(NSNotFound, 0);

    [inputContext invalidateCharacterCoordinates];
    bDoHandle = true;
}

- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
    MyGUI::EditBox* edit = sInputMger->_getCurrentEditWidget();
    if( edit )
    {
        const MyGUI::IntCoord& ic = edit->getCursorCoord();
        NSRect rc = NSMakeRect(ic.left,ic.top+ic.height,ic.width,ic.height);
        NSRect fc = [[[self Window] contentView] frame];
        rc.origin.y = fc.size.height-rc.origin.y;
        rc.origin = [[self Window] convertBaseToScreen:rc.origin];
        return rc;
    }
}

- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint
{
}

- (void)doCommandBySelector:(SEL)aSelector
{
 //   [super doCommandBySelector:aSelector];
}

/*
 */
- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    NSPoint pos =[self mousePtToClient];
    deltaZ += [theEvent deltaY];
    
    [self updateKeyModifier:theEvent];
    sInputMger->mouseMove(pos.x,pos.y,deltaZ);
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
    NSPoint pos =[self mousePtToClient];
    
    [self updateKeyModifier:theEvent];
    sInputMger->mouseMove(pos.x,pos.y,deltaZ);
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    NSPoint pos =[self mousePtToClient];
    
    [self updateKeyModifier:theEvent];
    sInputMger->mouseMove(pos.x,pos.y,deltaZ);
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    NSPoint pos =[self mousePtToClient];
    
    [self updateKeyModifier:theEvent];
    sInputMger->mouseMove(pos.x,pos.y,deltaZ);
}

- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint pos =[self mousePtToClient];
    
    [self updateKeyModifier:theEvent];
    sInputMger->mousePress(pos.x,pos.y,MyGUI::MouseButton::Left);
}

- (void)mouseUp:(NSEvent *)theEvent
{
    NSPoint pos =[self mousePtToClient];
    
    [self updateKeyModifier:theEvent];
    sInputMger->mouseRelease(pos.x,pos.y,MyGUI::MouseButton::Left);
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
    NSPoint pos =[self mousePtToClient];
    
    [self updateKeyModifier:theEvent];
    sInputMger->mousePress(pos.x,pos.y,MyGUI::MouseButton::Right);
}

//将屏幕上的鼠标点转换到绘制客户空间
- (NSPoint)mousePtToClient
{
    NSPoint screenPos = [NSEvent mouseLocation];
    NSPoint pos = [[self Window] convertScreenToBase:screenPos];
    NSRect rect = [[[self Window] contentView] frame];
    CGFloat height = rect.size.height;
    pos.y = mHeight*(height-pos.y)/height;
    pos.x = mWidth*pos.x/rect.size.width;
    return pos;
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
    NSPoint pos =[self mousePtToClient];
    sInputMger->mouseRelease(pos.x,pos.y,MyGUI::MouseButton::Right);
}

- (void)updateKeyModifier:(NSEvent *)theEvent
{
    NSUInteger mf = [theEvent modifierFlags];
    sInputMger->_setKeyState(base::SC_LSHIFT,mf&NSShiftKeyMask);
    sInputMger->_setKeyState(base::SC_LCONTROL,mf&NSControlKeyMask);
    sInputMger->_setKeyState(base::SC_LMENU,mf&NSAlternateKeyMask);
    sInputMger->_setKeyState(base::SC_COMMAND,mf&NSCommandKeyMask);
}

//control,option,command,shift不产生keyDown事件
- (void)keyDown:(NSEvent *)theEvent
{
    mCurrentEdit = sInputMger->_getCurrentEditWidget();
    
    int keyCode = base::VirtualKeyToScanCode([theEvent keyCode]);
    sInputMger->_setKeyState(keyCode,true);
    [self updateKeyModifier:theEvent];
    
    if( mCurrentEdit )
    {
        bKeydown = true;
        bDoHandle = false;
        [inputContext handleEvent:theEvent];
        bKeydown = false;
    }
    else
        bDoHandle = false;

    if( !bDoHandle )
    {
        NSString* ch = [theEvent characters];
        if( [ch length]>0 )
        {
            for(int i = 0;i<[ch length];++i )
            {
                unichar c = [ch characterAtIndex:0];
                sInputMger->injectKeyPress(MyGUI::KeyCode::Enum(keyCode), c);
            }
        }
        else
        {
            sInputMger->injectKeyPress(MyGUI::KeyCode::Enum(keyCode),MyGUI::KeyCode::None);
        }        
    }
}

- (void)insertText:(id)aString
{
}

- (void)keyUp:(NSEvent *)theEvent
{
    int keyCode = base::VirtualKeyToScanCode([theEvent keyCode]);
    sInputMger->_setKeyState(keyCode,false);
    [self updateKeyModifier:theEvent];

    if( !bDoHandle )
    {
        NSUInteger mf = [theEvent modifierFlags];

        sInputMger->injectKeyRelease(MyGUI::KeyCode::Enum(keyCode));
    }
}

@end

namespace base
{
	unsigned char InputManager::mKeyStates[256];
    InputResponder * gInputResponder;

	InputManager::KeyState InputManager::getKeyState(MyGUI::KeyCode _key)
	{
		assert(_key.getValue()>=0&&_key.getValue()<256);
		return mKeyStates[_key.getValue()]?DOWN:UP;
	}

	InputManager::InputManager() :
		mWidth(0),
		mHeight(0),
		mMouseX(0),
		mMouseY(0),
		mMouseZ(0),
		mMouseMove(false)
	{
		assert(!sInputMger);

		//初始化键态
		for( int i = 0;i < 256;i++ )
			mKeyStates[i] = UP;

		sInputMger = this;
	}

	InputManager::~InputManager()
	{
		assert(sInputMger);
		sInputMger = 0;
	}

    void InputManager::_setKeyState(int _key,bool bks )
    {
        if( _key >=0 && _key < 256 )
        {
            if( bks )
            {
                mKeyStates[_key] = DOWN;
            }
            else
                mKeyStates[_key] = UP;
        }
    }
    
    MyGUI::EditBox* InputManager::_getCurrentEditWidget()
    {
        MyGUI::Widget* widget = MyGUI::InputManager::getInstance().getKeyFocusWidget();
        if( widget )
        {
            MyGUI::EditBox* pedit = widget->castType<MyGUI::EditBox>(false);
            if( pedit )
            {
                if( pedit->getEditReadOnly() )
                    return nullptr;
                else
                    return pedit;
            }
        }
        return nullptr;
    }
    
	void InputManager::createInput(size_t _handle)
	{
		NSWindow* window = (NSWindow*)_handle;
        gInputResponder = [[InputResponder alloc] initWidthWindow:window];
		MyGUI::Gui::getInstance().eventFrameStart += MyGUI::newDelegate(this, &InputManager::frameEvent);
        MyGUI::InputManager::getInstance().setKeyboardStates(mKeyStates);
	}

	void InputManager::destroyInput()
	{
		MyGUI::Gui::getInstance().eventFrameStart -= MyGUI::newDelegate(this, &InputManager::frameEvent);

		// если мы подменили процедуру, то вернем на место
        [gInputResponder release];
	}

	void InputManager::captureInput()
	{
	}

	void InputManager::setInputViewSize(int _width, int _height)
	{
		mWidth = _width;
		mHeight = _height;
        [gInputResponder setWidth:_width Height:_height];
	}

	void InputManager::setMousePosition(int _x, int _y)
    {
	}

	void InputManager::updateCursorPosition()
	{
	}

	void InputManager::frameEvent(float _time)
	{
		computeMouseMove();
	}

	void InputManager::computeMouseMove()
	{
		if (mMouseMove)
		{
			injectMouseMove(mMouseX, mMouseY, mMouseZ);
			mMouseMove = false;
		}
	}

	void InputManager::mouseMove(int _absx, int _absy, int _absz)
	{
		mMouseX = _absx;
		mMouseY = _absy;
		mMouseZ = _absz;
		mMouseMove = true;
	}

	void InputManager::mousePress(int _absx, int _absy, MyGUI::MouseButton _id)
	{
		computeMouseMove();
		injectMousePress(_absx, _absy, _id);
	}

	void InputManager::mouseRelease(int _absx, int _absy, MyGUI::MouseButton _id)
	{
		computeMouseMove();
		injectMouseRelease(_absx, _absy, _id);
	}

} // namespace input
