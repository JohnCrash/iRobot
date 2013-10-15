local gui = require "gui"
local string = string
local lfs = lfs
local print = print
local pairs = pairs
local type = type
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

--ʵ���ļ��򿪶Ի���
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

--����windowsϵͳ�������̷�H:��Ҫ�ں������\\
local function realPath(path)
	if string.find(path,"\\") == nil then
		return path.."\\"
	else
		return path
	end
end

--����_pat�����ļ�����
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

	setCurrentDirectory(path)
	_filelist:removeAllItems()
	--Ŀ¼��ǰ��
	for f,_path in lfs.dir(realPath(path)) do
		if f ~= "." then
			if lfs.attributes(path.."\\"..f,mode).mode == "directory" then
				_filelist:addItem( "#00Cf00["..f.."]" )
				_filelist:setItemDataAt(_index,f)
				_index = _index + 1
			end
		end
	end
	--�ļ��ź���
	for f,_path in lfs.dir(realPath(path)) do
		if lfs.attributes(path.."\\"..f,mode).mode ~= "directory" 
			and filiter(f) then
			_filelist:addItem( f )
			_filelist:setItemDataAt(_index,f)
			_index = _index + 1			
		end
	end	
end

--��xml;txt;�ȵȷֱ����һ��_patt����
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

--������һ��Ŀ¼
local function getUplevel(path)
	local index = 1
	while true do
		local i,j = string.find(path,"\\",index+1)
		if i == nil then
			if index == 1 then
				return path
			else
				return string.sub(path,1,index-1)
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

local function doubleClick(sender)
	if _currentdir then
		local sel = _filelist:getIndexSelected()
		if sel ~= gui.ITEM_NONE then
			local p
			local filename = _filelist:getItemDataAt(sel)
			if filename == ".." then
				p = getUplevel(_currentdir)
			else
				p = _currentdir.."\\"..filename
			end

			if isDirectory(p) then
				loadDirectory(p)
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
	local p = _currentdir.."\\".._filelist:getItemDataAt(index)
	if not isDirectory(p) then
		_dialog:child("_name"):setCaption(_filelist:getItemDataAt(index))
	end
end

local function ok(sender)
	_callback(_currentdir.."\\".._dialog:child("_name"):getCaption())
	closeDialog()
end

--isopen�Ǵ򿪻��Ǵ洢��callbackѡ������Ļص�����
function openFileDialog(isopen,callback,pat)
	_dialog = gui.loadLayout("OpenSaveDialog.layout")
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
	
	local path =lfs.currentdir()
	loadDirectory(path)
end

--ʵ��һ��Yes No�Ի���
function YesNo(caption,msg,callback)
	local _msgbox = gui.loadLayout("yesno.layout")
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

--ʵ��һ��ȷ���Ի���