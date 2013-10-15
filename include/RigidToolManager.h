#ifndef _RIGIDTOOLMANAGER_H_
#define _RIGIDTOOLMANAGER_H_
#include "RigidTool.h"

/*将操作Rigid的工具集中放在一起进行管理
	因为RigidTool的系列派生类，都使用实体等唯一的资源
	因此单独去使用将出现名字冲突。使用RigidToolManager
	来集中使用是个办法
*/
class RigidToolManager:
	public Ogre::Singleton<RigidToolManager>
{
public:
	RigidToolManager();
	virtual ~RigidToolManager();

	//停止全部控制界面
	void stopControl();
	//显示一个移动控制器，去围绕rigid。用来调整rigid的位置
	void moveControl( const VisualObjectPtr& rigid,
		const Ogre::Node::TransformSpace relativeTo );
	//旋转控制一个
	void rotateControl( const VisualObjectPtr& rigid );
	//打开一个调整刚体的界面
	void modifyControl( const VisualObjectPtr& rigid );
	//平移2d
	void move2dControl( const VisualObjectPtr& rigid );
protected:
	MoveToolPtr mMoveTool;
	RotateToolPtr mRotateTool;
	ModifyToolPtr mModifyTool;
	MoveTool2dPtr mMoveTool2;
};

#endif