#include "stdheader.h"
#include "Geometry.h"
#include "Rigid.h"
#include "RigidToolManager.h"
#include "InputFilter.h"

SINGLETON(RigidToolManager)

RigidToolManager::RigidToolManager(){
	mMoveTool.reset( new MoveTool() );
	mRotateTool.reset( new RotateTool() );
	mModifyTool.reset( new ModifyTool() );
	mMoveTool2.reset( new MoveTool2d() );
}

RigidToolManager::~RigidToolManager(){
}

void RigidToolManager::stopControl(){
	VisualObjectPtr nullRigid;

	//隐藏
	mMoveTool->controlRigid( nullRigid );
	mRotateTool->controlRigid( nullRigid );
	mModifyTool->controlRigid( nullRigid );
	mMoveTool2->controlRigid( nullRigid,Ogre::Plane() );
}

void RigidToolManager::moveControl( const VisualObjectPtr& rigid,
								   const Ogre::Node::TransformSpace relativeTo )
{
	stopControl();
	mMoveTool->controlRigid( rigid,relativeTo );
}

void RigidToolManager::rotateControl( const VisualObjectPtr& rigid ){
	stopControl();
	mRotateTool->controlRigid( rigid );
}

void RigidToolManager::modifyControl( const VisualObjectPtr& rigid ){
	stopControl();
	mModifyTool->controlRigid( rigid );
}

void RigidToolManager::move2dControl( const VisualObjectPtr& rigid )
{
	stopControl();

	if( rigid )
		mMoveTool2->controlRigid( rigid,
		Ogre::Plane(Ogre::Vector3::UNIT_Y,rigid->getPosition())
		);
}