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
    
    //检测框架是否有循环连接,如果有返回true,否则返回false
    bool checkCycle();
    //找出框架中被连接最多的刚体部
    RigidPtr getBodyRigid();
protected:
    void saveRigid(MyGUI::xml::ElementPtr node,RigidPtr rgd);
    void saveJoint(MyGUI::xml::ElementPtr node,JointPtr joint,RigidPtr other);

	string mName;
	/* 该框架包括的全部的Joint
	*/
	JointMap mJoints;
};

#endif
