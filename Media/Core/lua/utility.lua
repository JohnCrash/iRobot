local gui = require "gui"
local registry = require "registry"
local string = string
local lfs = lfs
local print = print
local pairs = pairs
local type = type
local pcall = pcall
local L = L
_ENV = module(...)

local _dialog
local _filelist
local _pathhistory
local _currentdir
local _ok
local _callback
local _isopen
local _patt

--实现文件打开对话框
local function closeDialog()
	_dialog:destroy()
	_dialog=nil
end

local function setCurrentDirectory(path)
	_currentdir = path
	
	if _pathhistory:getItemCount() > 0 then
		for i=0, _pathhistory:getItemCount()-1 do
			if _pathhistory:getItemNameAt(i) == path then
				_pathhistory:setIndexSelected(i)
				return
			end
		end
	end
	_pathhistory:addItem(path)
	_pathhistory:setIndexSelected(_pathhistory:getItemCount()-1)
end

--[[对于windows系统，碰到盘符H:需要在后面加入\\
    如果/Users/John 将被改为/Users/John/    
--]]
local function realPath(path)
    local c = string.sub(path,-1)
   	if c ~= '/' then
		return path.."/"
	else
		return path
	end
end

--根据_pat过滤文件名称
local function filiter(name)
	if _patt then
		local pat
		for i,s in pairs(_patt) do
			pat = ".+%."..s
			if string.match(name,pat) then
				return true
			end
		end
	else
		return true
	end
	return false
end

local function loadDirectory(path)
	local _index = 0

	print("path:"..realPath(path))
	local iserr,errmsg = pcall(lfs.dir,realPath(path))
	if not iserr then
		print("loadDirectory invalid director "..path)
		path = lfs.currentdir()
		return
	end

	setCurrentDirectory(path)
	_filelist:removeAllItems()
	--目录放前面
	for f,_path in lfs.dir(realPath(path)) do
		if f ~= "." then
			if lfs.attributes(path.."/"..f,lfs.mode).mode == "directory" then
				_filelist:addItem( "#<B>#000000["..f.."]" )
				_filelist:setItemDataAt(_index,f)
				_index = _index + 1
			end
		end
	end
	--文件放后面
	for f,_path in lfs.dir(realPath(path)) do
		if lfs.attributes(path.."/"..f,lfs.mode).mode ~= "directory" 
			and filiter(f) then
			_filelist:addItem( f )
			_filelist:setItemDataAt(_index,f)
			_index = _index + 1			
		end
	end	
end

--将xml;txt;等等分别放入一个_patt表中
local function initpatt(pat)
	local index = 1
	local k = 1
	if pat then
		_patt = {}
		while true do
			local i,j = string.find(pat,";",index+1)
			if i == nil then
				if index == 1 then
					_patt[k] = pat
				else
					_patt[k] = string.sub(pat,index+1,-1)		
				end		
				return
			else
				_patt[k] = string.sub(pat,index+1,j-1)
				k = k + 1
				index = j
			end
		end
	else
		_patt = nil
	end
end

--[[
    返回上一级目录
    / 返回 /
    /Users 返回 /
    /Users/john 返回 /Users
--]]
local function getUplevel(path)
	local index = 0
	while true do
		local i,j = string.find(path,"/",index+1)
		if i == nil then
			if index == 0 then
                --没有发现/
				return path
			else
                if index == 1 then
                    --形如/Users 返回 /
                    return string.sub(path,1,index)
                else
                    --形如/Users/john 返回 /Users
                    --形如E:/Users 返回E:
				    return string.sub(path,1,index-1)
                end
			end
		else
			index = j
		end
	end
end

local function isDirectory(path)
	local ft = lfs.attributes(realPath(path),mode)
	if ft and ft.mode == "directory" then
		return true
	end
	return false
end

local function ok(sender)
	local path = _currentdir.."/".._dialog:child("_name"):getCaption()
	registry.set("LastOpenFile",_currentdir)
	_callback(path)
	closeDialog()
end

local function doubleClick(sender)
	if _currentdir then
		local sel = _filelist:getIndexSelected()
		if sel ~= gui.ITEM_NONE then
			local p
			local filename = _filelist:getItemDataAt(sel)
			if filename == ".." then
				p = getUplevel(_currentdir)
			else
				p = realPath(_currentdir)..filename
			end
            print("open dir:"..p)
			if isDirectory(p) then
				loadDirectory(p)
			else
				ok(sender)
			end
		end
	end
end

local function up(sender)
	loadDirectory(getUplevel(_currentdir))
end

local function selectpath(sender,index)
	if index ~= gui.ITEM_NONE then
		loadDirectory(_pathhistory:getItemNameAt(index))
	end
end

local function enterpath(sender,index)
	if isDirectory(_pathhistory:getCaption()) then
		loadDirectory(_pathhistory:getCaption())
	end
end

local function selectfile(sender,index)
	if index == gui.ITEM_NONE then
		return
	end
    local p = realPath(_currentdir).._filelist:getItemDataAt(index)
    if not isDirectory(p) then
		_dialog:child("_name"):setCaption(_filelist:getItemDataAt(index))
	end
end

--isopen是打开还是存储，callback选定对象的回调函数
function openFileDialog(isopen,callback,pat)
	_dialog = gui.loadLayout("layout/OpenSaveDialog.layout")
	gui.addWidgetModal(_dialog)
	_callback = callback
	_ok = _dialog:child("_ok")
	if isopen then
		_dialog:setCaption(L"Open file")
		_ok:setCaption(L"Open")
	else
		_dialog:setCaption(L"Save file")
		_ok:setCaption(L"Save")
	end
	
	_isopen = isopen
	initpatt(pat)
	
	_ok:setScript("eventMouseButtonClick",ok)
	_filelist = _dialog:child("_filelist")
	_pathhistory = _dialog:child("_path")
	_dialog:setScript("eventWindowButtonPressed",closeDialog)
	_dialog:child("_cancel"):setScript("eventMouseButtonClick",
		closeDialog)
	_dialog:child("_up"):setScript("eventMouseButtonClick",up)
	
	_filelist:setScript("eventListSelectAccept",doubleClick)
	_filelist:setScript("eventListChangePosition",selectfile)
	_pathhistory:setScript("eventComboChangePosition",selectpath)
	_pathhistory:setScript("eventComboAccept",enterpath)
	
	local path = registry.get("LastOpenFile",lfs.currentdir())
	loadDirectory(path)
end

--实现一个Yes No对话框
function YesNo(caption,msg,callback)
	local _msgbox = gui.loadLayout("layout/yesno.layout")
	gui.addWidgetModal(_msgbox)
	local _yes = _msgbox:child("_yes")
	local _no = _msgbox:child("_no")
	local _txt = _msgbox:child("_msg")
	_msgbox:setCaption(caption)
	_txt:setCaption(msg)
	if callback and type(callback) == "function" then
		_yes:setScript("eventMouseButtonClick",
			function(sender)
				callback(true)
				_msgbox:destroy()
				_msgbox=nil
			end
			)
		_no:setScript("eventMouseButtonClick",
			function(sender)
				callback(nil)
				_msgbox:destroy()
				_msgbox=nil				
			end
			)
	end
end

--实现一个确定对话框
