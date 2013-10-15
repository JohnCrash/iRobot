local print = print
local game = require "game"

_ENV=module(...)

local function help( param )
	for i,v in pairs(_cmd) do
		print( i )
	end
end

local function reset( param )
	game.reset()
end

local function quit( param )
	game.quit()
end

local _cmd={
	["help"]=help,
	["reset"]=reset,
	["quit"]=quit
}

local function command( cmd )
	if _cmd[cmd] then
		_cmd[cmd]()
	end
	print( cmd )
end

print("console load")
game.setScript("eventConsole",command)
