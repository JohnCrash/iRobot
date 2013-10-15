#ifndef _FRAMEWORK_H_
#define _FRAMEWORK_H_
#include "Geometry.h"
#include "Rigid.h"

/*
	用来描述刚体和铰链组成的结构
*/
class Framework
{
public:
	Framework();
	virtual ~Framework();

	virtual void load( MyGUI::xml::Document doc );
	virtual void save( MyGUI::xml::Document doc );

	void addRigidNode( RigidPtr b );
	void addJointNode( JointPtr j );
protected:
	RigidPtr _getRoot();
	string mName;
	/* 该框架包括的全部Rigid和Joint
	*/
	typedef vector<RigidPtr> RigidMap;
	typedef vector<JointPtr> JointMap;
	RigidMap mRigids;
	JointMap mJoints;
};

#endif