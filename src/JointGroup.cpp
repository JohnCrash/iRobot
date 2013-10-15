#include "stdheader.h"
#include "Geometry.h"
#include "JointGroup.h"
#include "RigidManager.h"

JointGroup::JointGroup()
{
}

JointGroup::JointGroup( const string& n )
:mName(n)
{
	mGroupID = dJointGroupCreate(0);
}

JointGroup::~JointGroup()
{
	//删除组和组里面的全部关节
	dJointGroupDestroy(mGroupID);
}

JointPtr JointGroup::linkJointBall(  const RigidPtr& o1,const RigidPtr& o2 )
{
	/*
	if( o1 && o2 )
	{
		JointPtr joint = JointPtr(new JointBall(mGroupID,o1->getBodyID(),o2->getBodyID()));
		mJoints.push_back( joint );
		return joint;
	}
	else
	{
		ERROR_LOG("linkJointBall invalid argument");
	}*/
	return JointPtr();
}
