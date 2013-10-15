print("Hello")
local lfs = lfs
local gui = require "gui"
local game = require "game"
local builder = require "builder"

local function keyPress( key,text )
	if gui.getKeyState( gui.key.LeftControl ) and key == gui.key.R then
		print( "You press Ctrl+R" )
	end
end

game.applyTranslate(lfs.currentdir().."\\..\\zh_CN.xml")
game.setScript("eventKeyPress",keyPress)
builder.openStage()
