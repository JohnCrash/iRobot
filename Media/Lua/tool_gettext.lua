local gui = require "gui"
local string = string
local lfs = require "lfs"
local io = io
local xml = require "xml"
local utility = require "utility"
local file = file
local L = L
local print = print
local pairs = pairs
local ipairs = ipairs
local http = require("socket.http")

_ENV = module(...)

local _dialog
local _text
local _list
local _msg
local _edit
local _lastSel
local _ischange

--利用Google translate翻译词汇
local function en2cn(word)
	local url="http://www.google.com/translate_t?hl=en&ie=UTF8&text="..word.."&langpair=en|zh-CN"
	local result
	while not result do
		result = http.request(url)
	end
	return string.match(result,"'\">(.*)</span></span>")
end

local function addMsg( msg,str )
	if not msg then return end
	if _msg[msg] then
		return
	else
		_msg[msg]=str
		_list:addItem(msg)
		_ischange = true
	end
--[[	
	for i,v in ipairs(_msg) do
		if v == msg then
			return
		end
	end
	_msg[#_msg+1] = msg
	_list:addItem(msg)
--]]
end

--lua代码可以使用L"
local function scanLine( line,bpat,epat )
	local index = 1
	local begin
	while true do
		local i,j = string.find(line,bpat,index)
		if i == nil then
			--没发现模式
			return
		else
			begin = j+1
			i,j = string.find(line,epat,begin)
			if i == nil then
				print("scanLine Error:"..line)
				return
			else
				addMsg(string.sub(line,begin,i-1),"")
				--继续
				index = j+1
				begin = nil
			end
		end
	end
end

local function scanSource( _file )
	_text:setCaption( _file )
	--[[for line in io.lines(_file) do
		scanLine( line,"L%(\"","\"%)" )
	end
	--]]
	--如果是Lua源代码需要考虑L"的方式
	if string.match(_file,".lua") then
		for line in io.lines(_file) do
			scanLine(line,"L\"","\"")
		end
	end
end

local function scanNode( node )
	if node then
		if node:getName() == "Property" then
			local b,value = node:findAttribute("value")
			local keyb,keyvalue = node:findAttribute("key")
			if keyb and keyvalue=="Caption" and b then
				addMsg(value,"")
			end
		end
		local t = node:getChildren()
		for i,child in pairs(t) do
			scanNode( child )
		end
	end
end

local function scanXml( file )
	_text:setCaption( file )
	local doc = xml.newDocument()
	if doc:open(file) then
		scanNode( doc:getRoot() )
	end
end

local function scanDirectory( path )
	for name,p in lfs.dir(path) do
		local f = path.."\\"..name
		if string.match(name,".cpp") then
			scanSource( f )
		elseif string.match(name,".h") then
			scanSource( f )
		elseif string.match(name,".lua") then
			scanSource( f )
		elseif string.match(name,".xml") then
			scanXml( f )
		elseif string.match(name,".layout") then
			scanXml( f )
		end
	end
end

local function save()
--保存正在编辑的
	if _lastSel then
		local msgid = _list:getItemNameAt(_lastSel)
		local msgstr = _edit:getCaption()
		_msg[msgid] = msgstr
	end
	local doc = xml.newDocument()
	doc:createDeclaration()
	local root = doc:createRoot("Translate")
	for i,v in pairs(_msg) do
		local child = root:createChild("Msg")
		child:addAttribute("msgid",i)
		child:addAttribute("msgstr",v)
	end
	doc:save(lfs.currentdir().."\\..\\zh_CN.xml")
	_ischange = nil
end

local function closeDialog()
	if _ischange then
		utility.YesNo(L"Warning",L"Do you want save this change?",
		function(yes) 
			if yes then
				print("Yes save it")
				save() 
				_dialog:destroy()
				_dialog=nil				
			else
				print("No save")
				_dialog:destroy()
				_dialog=nil				
			end 
		end)
	else
		_dialog:destroy()
		_dialog=nil
	end
end

local function scan()
	if not _msg then
		_msg = {}
		_list:removeAllItems()
	end
	scanDirectory(lfs.currentdir().."\\..\\..\\..\\src")
	scanDirectory(lfs.currentdir().."\\..\\..\\..\\include")
	scanDirectory(lfs.currentdir().."\\..\\..\\..\\Media\\lua")
	scanDirectory(lfs.currentdir().."\\..\\..\\..\\Media\\Layout")
end

local function loadTralateXml(file)
	local doc = xml.newDocument()
	_msg = {}
	if doc:open( file ) then
		local root = doc:getRoot()
		if root:getName() ~= "Translate" then
			print(L"Not is a translate xml document "..file)
			return
		else
			print(L"Translate file "..file)
		end
		local t = root:getChildren()
		for i,m in ipairs(t) do
			if m:getName() == "Msg" then
				local b,msgid = m:findAttribute("msgid")
				local b1,msgstr = m:findAttribute("msgstr")
				if b and b1 then
					addMsg( msgid,msgstr )
				end
			end
		end
	else
		print(L"Can't open file "..file)
	end
end

local function listSelect(sender,index)
	gui.setKeyFocusWidget(_edit)
	--保存上个索引的编辑内容
	if _lastSel and _lastSel<_list:getItemCount() then
		local msgid = _list:getItemNameAt(_lastSel)
		local msgstr = _edit:getCaption()
		if msgstr ~= _msg[msgid] then
			_msg[msgid] = msgstr
			_ischange = true
		end
	end
	local mi = _list:getItemNameAt(index)

	_edit:setCaption(_msg[mi])
	_lastSel = index
end

function translateDialog()
	_dialog = gui.loadLayout("tool_translate.layout")
	gui.addWidgetModal(_dialog)
	_dialog:setScript("eventWindowButtonPressed",closeDialog)
	_dialog:child("_save"):setScript("eventMouseButtonClick",save)	
	_dialog:child("_scan"):setScript("eventMouseButtonClick",scan)	
	_text = _dialog:child("_text")
	_edit = _dialog:child("_edit")
	_list = _dialog:child("_list")
	_list:setScript("eventListChangePosition",listSelect)
	loadTralateXml(lfs.currentdir().."\\..\\zh_CN.xml")
	_ischange = nil
end
