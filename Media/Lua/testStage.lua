_ENV=module(...)

function pt( t )
	local s
	for i,v in pairs(t) do
		if s then
			s = s..tostring(v)
		else
			s = tostring(v)
		end
		s = s..","
	end
	return s
end

function pressed(window,name)
	if window:getName() == "reset" then
		print("reset...")
		ResetScene()
	end
	if name == "close" then
		--window:setVisibleSmooth(false)
		window:destroy()
		collectgarbage("collect")
	elseif window:getName() == "Ok" then
		print(window:getCaption())
		print("Ok")
	end
	if window:getTypeName()=="Button" and window:getCaption()=="createWindow"  then
		local w = gui.createWidget("Window","WindowCX","Overlapped" ,"Center" )
		w:setCaption("#Inew window")
		w:setSize(320,200)
		local cbut = w:createChild("Button","Button","Center","close")
		cbut:setSize(128,64)
		cbut:setPosition(20,20)
		cbut:setAlign("HCenter|VCenter")
		cbut:setCaption("Close")
		w:setScript( "eventWindowButtonPressed",pressed )
		print( "createWidget")
	end
	
	if list then
		list:addItem( edit:getCaption() )
	end
end

function printchild( t )
	print("Widget count:"..#t)
	for i,v in ipairs(t) do
		print( "'"..v:getName().."'"..v:getTypeName() )
	end
end

function getFindCaption( t,text )
	for i,v in ipairs(t) do
		if v:getTypeName()=="Button" and v:getCaption() == text then
			print("found button caption == "..text )
			return v
		end
	end
	return nil
end

function listChange( sender,index )
	if index == gui.ITEM_NONE then
		print( "index == ITEM_NONE" )
	--	return
	end
	text:setCaption( list:getItemNameAt(index) )
	tab:setIndexSelected(index)
end

function eventKeyButtonPressed( sender,_key,_char )
	if _key == gui.key.Return then
		local text = edit:getCaption()
		us = gui.newUString(text)
		local t = string.gsub(text,"##","#").."("..us:size()..")"
		for i=0,us:size()-1 do
			print( us:at(i) )
		end
		list:addItem( t )
		tab:addItem( t )
		list:addItem("ÖÐÎÄ")
		print("ÄãºÃ")
	elseif _key == gui.key.Home then
		print("HOME")
	end
end

function eventTabChangeSelect( sender,index )
	list:setIndexSelected(index)
end

function eventScrollChangePosition( sender,pos )
	list:addItem( tostring(pos) )
	print( tostring(pos) )
end

function openStage()
	print( "luaStage is open....")
	local w = gui.loadLayout("test.layout")
	if w then
		w:setScript("eventWindowButtonPressed",pressed)
		w:setVisibleSmooth(true);
		w:setCaption("!Test list and edit!")
		list = w:child("list")
		edit = w:child("edit")
		text = w:child("text")
		tab = w:child("tab")
		menu= w:child("menu")
		hbar = w:child("hbar")
		
		hbar:setScrollRange(100)
		hbar:setScrollPage(1)
		hbar:setTrackSize(100)
		hbar:setScrollPosition(50)
		hbar:setScript("eventScrollChangePosition",eventScrollChangePosition)
		
		w:child("reset"):setScript("eventMouseButtonClick",pressed)
		list:setScript("eventListChangePosition",listChange)
		edit:setScript("eventKeyButtonPressed",eventKeyButtonPressed)
		tab:setScript("eventTabChangeSelect",eventTabChangeSelect)
		printchild( w:childs() )
		cbut = getFindCaption( w:childs(),"createWindow" )
		cbut:setScript("eventMouseButtonClick",pressed)
		
		local but = w:child("ok")
		if but then
			but:setScript("eventMouseButtonClick",pressed)
		else
			print("no found button ok")
		end
		print( "visible:"..tostring(w:getVisible()) )
		print( "name:"..tostring( w:getName()) )
		print( "align:"..tostring( w:getAlign() ))
		print( "alpha:"..tostring( w:getAlpha() ))
		print( "size:"..tostring(pt{w:getSize()}))
		print( "position:"..tostring(pt{w:getPosition()} ))
		if w:getParent() then
			print("parent:not nil")
		else
			print("parent:nil")
		end
	else
		print( "LoadLayout('test.layout') return nil" )
	end
end

function closeStage()
	print( "luaStage is close...." )
end

function test()
	print("test call")
end

game.addStage("testStage",{["openStage"]=openStage,["closeStage"]=closeStage})
