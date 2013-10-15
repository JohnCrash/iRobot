#ifndef _JOINT_GROUP_H_
#define _JOINT_GROUP_H_

#include "Joint.h"
#include "Rigid.h"

class JointGroup
{
public:
	JointGroup();
	JointGroup(const string& n);
	virtual ~JointGroup();
	/*	
		将两个对象用球形关节连接起来
	*/
	JointPtr linkJointBall( const RigidPtr& r1,const RigidPtr& r2);

protected:
	string mName;
	dJointGroupID mGroupID;
	vector<JointPtr> mJoints; 
};

typedef boost::shared_ptr<JointGroup> JointGroupPtr;

#endif