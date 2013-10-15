#ifndef _FACTORY_H_
#define _FACTORY_H_

template<typename T> class Factory
{
public:
	static T* create(){ new T();}
};

#endif