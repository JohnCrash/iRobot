local print = print
local geo = require "geo"
local gui = require "gui"
local game = require "game"
local hotkey = require "hotkey"
local utility = require "utility"
local tool_gettext = require "tool_gettext"
local mat = require "mat"
local ipairs = ipairs

_ENV = module(...)


--Ϊ���������������
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

--��ǵ�ǰ����
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

--���ģ��״̬
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

--��ʾabout�Ի���
function about()
	if not gAbout and not gui.isModalAny() then
		gAbout = gui.loadLayout("builder_about.layout")
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
	geo.loadScene(filename)
end

function save(filename)
	geo.saveScene(filename)
end

function command(sender)
	local name = sender:getName()
	local rigid
	local isc,ist
	local old = gCurrentTool
	if name=="_quit" then
		game:quit()
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
		utility.openFileDialog(false,save,"xml")
	elseif name=="_open" then
		utility.openFileDialog(true,open,"xml")
	elseif name=="_translate" then
		tool_gettext.translateDialog()
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

	print( mat.toRadian(180) )
	print( name )
end

--������������place�������ù�����λ��Top����Bottom
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
	gui.loadResource("toolbar_images.xml")
	game.setCameraControlet("simple")
	hotkey.load("builder.xml")
	gMenuBar = gui.loadLayout("builder_menubar.layout")
	SetCommandHandler(gMenuBar,command)
	gCurrentTool = "_select"
	--����������
	createMainTool("Bottom")
	--����һ������
	local ground = geo.createPlane(0,1,0,0,5000,5000,10,10,true)
	geo.addNode(ground)
	ground:setMaterialName("Ogre/Tusks")
	--
	game.setScript("eventMousePress",onMousePress)
end

function closeStage()
end