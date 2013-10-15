local print = print
local game = require "game"
local string = require "string"
local os = require "os"
local lfs=lfs

_ENV=module(...)

local function help( param )
	for i,v in pairs(_cmd) do
		print( i )
	end
end

local function scall( param,func )
	local p = string.match(param,"\"(%a+)\"")
	if p then
		func(p)
	else
		func(param)
	end
end

local function reset( param )
	--scall(param,game.reset)
	game.reset("NORMAL")
end

local function quit( param )
	game.quit()
end

local function load( param )
	scall(param,game.execute)
end


local function iconv( param )
    print("转换编码，目录："..param)
	local path = param 
      for f,_path in lfs.dir(path,lfs.mode) do
			if string.match(f,".+%.txt") then
				print( path.."/"..f )
				local command = "iconv -f GB18030 -t UTF-8 "..path.."/"..f.." > /Users/john/temp/"..f
				print( command )
				local reslut = os.execute( command )
				if reslut then
					print("ok")
				else
					print("no")
					--改名
					command = "rm /Users/john/temp/"..f
					os.execute(command)
					print(command)
				end
			end  
      end
end

local _cmd={
	["help"]=help,
	["reset"]=reset,
	["quit"]=quit,
	["load"]=load,
	["iconv"]=iconv
}

local function command( cmd )
	local c,p = string.match(cmd,"(%a+)%s*([^\n]*)")
	if _cmd[c] then
		_cmd[c](p)
		print( cmd )
	else
		print( "Invalid command:"..cmd )
	end
end

print("console load")
game.setScript("eventConsole",command)
