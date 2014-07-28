--[[module版本1 ,使用setmetatable(M,{__index=_G})来输入外部函数，速度上面会受到影响
function module( modname )
	local M={}
	_G[modname] = M
	package.loaded[modname] = M
	setmetatable(M,{__index=_G})
	return M
end
--]]
--module版本2,空间换速度
local game = require "game"
local type = type
local _G = _G

--注意：函数和它所属的chunk(代码块)的_ENV绑定。这意味着别的chunk在定义模块时
--调用module用的是init.lua的chunk环境_ENV
function module( modname )
	local M={}
	--_G[modname] = M
	_G.package.loaded[modname] = M
	return M
end

--为C socket附加lua函数
require "socket.luasocket"
require "socket.luamime"
require "mygui"
require "console"
require "errorhandle"

_ENV = module(...)
