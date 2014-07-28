local print = print
local geo = require "geo"
local gui = require "gui"
local game = require "game"
local hotkey = require "hotkey"
local utility = require "utility"
local options = require "options"
local tool_gettext = require "tool_gettext"
local mat = require "mat"
local registry = require "registry"
local xml = require "xml"

local ipairs = ipairs

_ENV = module(...)

--当前使用的文件
local currentFile

--为给窗户设置命令处理
function SetCommandHandler(parent,cmd)
	for i,v in ipairs(parent:childs()) do
		if v:getTypeName()=="MenuItem" and v:getName()~="" then
			v:setScript("eventMouseButtonClick",cmd)
		end
		SetCommandHandler(v,cmd)
	end
end

function currentControl( obj )
	if not obj then
		return
	end
	if gCurrentTool == "_move" then
		geo.moveControl(obj)
	elseif gCurrentTool == "_move2d" then
		geo.move2dControl(obj)
	elseif gCurrentTool == "_rotate" then
		geo.rotateControl(obj)
	elseif gCurrentTool == "_modify" then
		geo.modifyControl(obj)
	elseif gCurrentTool == "_movelocal" then
		geo.moveControl(obj,"local")
	elseif gCurrentTool == "_select" then
		geo.stopControl()
	end
	
	if gCurrentObject~=obj then
		if gCurrentObject then
			gCurrentObject:showBoundingBox(false)
		end		
		obj:showBoundingBox(true);
		gCurrentObject = obj
	end
end

--标记当前工具
function checkControl(old)
	if gCurrentTool ==  old then
		return
	end
	local t = gToolBar:child(gCurrentTool)
	local tt = gMenuBar:child(gCurrentTool)
	local o = gToolBar:child(old)
	local oo = gMenuBar:child(old)
	if t then
		t:setStateSelected(true)
	end
	if tt then
		tt:setItemChecked(true)
	end
	if o then
		o:setStateSelected(false)
	end
	if oo then
		oo:setItemChecked(false)
	end
end

--标记模拟状态
function checkSimulation()
	local r = gMenuBar:child("_run")
	local p = gMenuBar:child("_pause")
	if geo.isSimulation() then
		r:setItemChecked(true)
		p:setItemChecked(false)
	else
		r:setItemChecked(false)
		p:setItemChecked(true)	
	end
end

--显示about对话栏
function about()
	if not gAbout and not gui.isModalAny() then
		gAbout = gui.loadLayout("layout/builder_about.layout")
		gui.addWidgetModal(gAbout)
		gAbout:setScript("eventWindowButtonPressed",
		function(sender,id)
			gAbout:destroy()
			gAbout=nil
		end)
		gAbout:child("_ok"):setScript("eventMouseButtonClick",
		function(sender)
			gAbout:destroy()
			gAbout=nil		
		end)
	end
end

function open(filename)
	geo.clearAllNode()
	if geo.loadScene(filename) then
		currentFile = filename
	end
end

function save(filename)
	if geo.saveScene(filename) then
		currentFile = filename
	end
end

function command(sender)
	local name = sender:getName()
	local rigid
	local isc,ist
	local old = gCurrentTool
	if name=="_quit" then
		game.quit()
	elseif name=="_new" then
		geo.clearAllNode()
		currentFile = nil
	elseif name=="_rigid_sphere" then
		rigid = geo.createRigidSphere(30)
		geo.addNode(rigid)
		isc = true
	elseif name=="_rigid_box" then
		rigid = geo.createRigidBox(50,50,50)
		geo.addNode(rigid)
		isc = true
	elseif name=="_rigid_cylinder" then
		rigid = geo.createRigidCylinder(30,50)
		geo.addNode(rigid)
		isc = true
	elseif name=="_move" then
		gCurrentTool = name
		ist = true
	elseif name=="_move2d" then
		gCurrentTool = name
		ist = true
	elseif name=="_rotate" then
		gCurrentTool = name
		ist = true
	elseif name=="_modify" then
		gCurrentTool = name
		ist = true
	elseif name=="_movelocal" then
		gCurrentTool = name
		ist = true
	elseif name=="_select" then
		gCurrentTool = name
		ist = true
	elseif name=="_delete" then
		geo.removeNode(gCurrentObject)
		gCurrentObject=nil
	elseif name=="_run" then
		geo.simulation(true)
		checkSimulation()
	elseif name=="_pause" then
		geo.simulation(false)
		checkSimulation()
	elseif name=="_about" then
		about()
	elseif name=="_save" then
		if not currentFile then
			utility.openFileDialog(false,save,"xml")
		else
			save(currentFile)
		end
	elseif name=="_open" then
		utility.openFileDialog(true,open,"xml")
	elseif name=="_translate" then
		tool_gettext.translateDialog()
	elseif name=='_options' then
		options.dialog()
	elseif name=="_test2" then
		testFramework2()
	elseif name=="_test1" then
		testFramework()
	end
	
	if isc and rigid then
		rigid:setMaterialName("Ogre/Skin")
		currentControl(rigid)
		print("----")
	end
	if ist then
		checkControl(old)
		currentControl(gCurrentObject)
	end
end

function testFramework2()
	local fwk = geo.createFramework()
	local doc = xml.newDocument()
	if doc:open("/Users/zuzu/Desktop/test1.xml") then
		local root = doc:getRoot()
		fwk:load(root)
		local doc2 = xml.newDocument()
		doc:createDeclaration()
		local ele = doc:createRoot("Framework")
		fwk:save(ele)
		doc:save("/Users/zuzu/Desktop/test2.xml")		
	end
end

function testFramework()
	local fwk = geo.createFramework()
	local jo1 = geo.createJointBall()
	local jo2 = geo.createJointBall()
	local jo3 = geo.createJointBall()
	local jo4 = geo.createJointBall()
	
	local r1 = geo.createRigidBox(10,10,60)
	local r2 = geo.createRigidBox(10,10,60)
	local r3 = geo.createRigidBox(10,10,60)
	local r4 = geo.createRigidBox(10,10,60)
	local b = geo.createRigidBox(30,30,60)

	jo1:linkRigid(b,r1)
	jo2:linkRigid(b,r2)
	jo3:linkRigid(b,r3)
	jo4:linkRigid(b,r4)

	fwk:addJoint(jo1)
	fwk:addJoint(jo2)
	fwk:addJoint(jo3)
	fwk:addJoint(jo4)

	local doc = xml.newDocument()
	doc:createDeclaration()
	local ele = doc:createRoot("Framework")
	fwk:save(ele)
	doc:save("/Users/zuzu/Desktop/test1.xml")
end

--创建主工具条place可以设置工具条位置Top或者Bottom
function createMainTool(place)
	local button={"_rigid_sphere",
	"_rigid_box","_rigid_cylinder",
	"_plane","_select",
	"_move","_rotate","_move2d","_modify",
	"_jointball"}
	local x = 8
	local toolbar = gui.createWidget("Widget","ClientTileSkin","Overlapped","HFull|Top","toolbar")
	toolbar:setSize(64,64)
	
	if place=="Top" then
		local w,h = gMenuBar:getSize()
		toolbar:setPosition(0,h-1)
		toolbar:setAlign("HFull|Top")	
	elseif place=="Bottom" then
		local w,h = gui.getScreenSize()
		toolbar:setPosition(0,h-64)
		toolbar:setAlign("HFull|Bottom")
	end
	
	for i,name in ipairs(button) do
		local b = toolbar:createChild("Button","ButtonImage","HCenter|Left",name)
		b:setSize(60,60)
		b:setPosition(x,0)
		b:setAlign("HCenter|Left")
		b:setImageResource("MainToolBar")
		b:setImageGroup("Objects")
		b:setImageName(name)
		b:setScript("eventMouseButtonClick",command)
		x = x + 64
	end
	gToolBar = toolbar
	checkControl("")
	checkSimulation()
end

function onMousePress(x,y,button)
	if button == gui.MouseButtonLeft then
		local obj = geo.pickObject(x,y)
		currentControl( obj )
	end
end

function openStage()
	gui.setWindowText("iRobot")
	--gui.loadResource("MyGUI_BlackOrangeTheme.xml")
	gui.loadResource("gui/toolbar_images.xml")
	game.setCameraControlet("simple")
	hotkey.load("hotkey.xml")
	gMenuBar = gui.loadLayout("layout/builder_menubar.layout")
	gMenuBar:getName()
	SetCommandHandler(gMenuBar,command)
	gCurrentTool = "_select"
	--创建工具条
	createMainTool("Bottom")
	--创建一个地面
	local ground = geo.createPlane(0,1,0,0,5000,5000,10,10,true)
	geo.addNode(ground)
	ground:setMaterialName("Ogre/Tusks")
	--
	game.setScript("eventMousePress",onMousePress)
	registry.set("builder","iRobot")
end

function closeStage()
end
