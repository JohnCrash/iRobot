#include "stdheader.h"
#include "Controlet.h"
#include "InputFilter.h"

Controlet::Controlet():
	mEnable(true),
	mGlobal(false)
{
		InputFilter::getSingleton().addControlet( this );
}

Controlet::~Controlet(){
	InputFilter::getSingleton().removeControlet( this );
}

void Controlet::setEnable( bool b ){
	mEnable = b;
}

bool Controlet::isEnable(){
	return mEnable;
}

bool Controlet::isGlobal() const
{
	return mGlobal;
}

void Controlet::setGlobal( bool b )
{
	mGlobal = b;
}
