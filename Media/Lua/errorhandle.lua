local game = require "game"
local print = print
_ENV = module(...)

function error_parse( msg )
	local error_line,error_file,error_msg
	local prev_line
	for v in string.gmatch(msg,"[^\n]+") do
		if not error_file then
			error_file,error_line = string.match(v,"([^%s]+):(%d+): in function")
		end
		if string.find(v,"stack traceback:") then
			error_msg = prev_line
		end
		prev_line = v
	end
	return error_msg,error_file,error_line
end

function error_handle( msg )
	if not errorDialog then
		local info,file,line = error_parse( msg )
		if info then
			openErrorHandleDialog(info,file,line)
		end
	else
		print( "#FF0000"..msg )
	end
	print( "#FF0000"..msg )
end

function closeErrorDialog_handle( sender,cmd )
	if cmd == "close" then
		errorDialog:destroy()
		errorDialog = nil
	elseif sender:getName() == "save" then
		errorDialog:destroy()
		errorDialog = nil
	end
end

function kayword( s )
end

function openErrorHandleDialog( info,file,line )
	if not errorDialog then
		errorDialog = gui.loadLayout("error.layout")
		if not errorDialog then
			print("errorDialog==nil")
			return
		end
		errorInfo = errorDialog:child("text")
		errorSource = errorDialog:child("edit")
		saveButton = errorDialog:child("save")
		errorDialog:setScript("eventWindowButtonPressed",closeErrorDialog_handle)
		saveButton:setScript("eventMouseButtonClick",closeErrorDialog_handle)
	end
	errorInfo:setCaption( info )
	local f = io.open(file)
	if f then
		local s = f:read("*all")
		text = string.gsub(s,"#","##");
		errorSource:setCaption( text )
		f:close()
	else
		print( L"Can't open file '"..file.."'" )
	end
end

game.setScript("eventErrorHandler",error_handle)