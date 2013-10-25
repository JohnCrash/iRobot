#ifndef _FRAMEWORK_H_
#define _FRAMEWORK_H_
#include "Geometry.h"
#include "Rigid.h"

/*
	用来描述刚体和铰链组成的结构
*/
class Framework:public Object
{
public:
	Framework();
	virtual ~Framework();

	virtual void load( MyGUI::xml::ElementPtr doc );
	virtual void save( MyGUI::xml::ElementPtr doc );

	void addJoint( JointPtr j );
	void removeJoint( JointPtr j );
protected:
    void idAll(); //标记全部部件
    void reConstruct(); //重新构造框架,在load后调用

	string mName;
	/* 该框架包括的全部的Joint
	*/
	typedef vector<JointPtr> JointMap;
	JointMap mJoints;
};

#endif
