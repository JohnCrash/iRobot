local print = print
local geo = require "geo"
local gui = require "gui"
local game = require "game"

_ENV = module(...)

local _dialog

function closeDialog()
	_dialog:destroy()
	_dialog=nil
end

function dialog()
	_dialog = gui.loadLayout('layout/options.layout')
	_dialog['Cancel']:setScript('eventMouseButtonClick',closeDialog)
	_dialog:setScript("eventWindowButtonPressed",closeDialog)

end
