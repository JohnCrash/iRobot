--[[module�汾1 ,ʹ��setmetatable(M,{__index=_G})�������ⲿ�������ٶ�������ܵ�Ӱ��
function module( modname )
	local M={}
	_G[modname] = M
	package.loaded[modname] = M
	setmetatable(M,{__index=_G})
	return M
end
--]]
--module�汾2,�ռ任�ٶ�
local game = require "game"
local type = type
local _G = _G

--ע�⣺��������������chunk(�����)��_ENV�󶨡�����ζ�ű��chunk�ڶ���ģ��ʱ
--����module�õ���init.lua��chunk����_ENV
function module( modname )
	local M={}
	--_G[modname] = M
	_G.package.loaded[modname] = M
	return M
end

--ΪC socket����lua����
require "socket.luasocket"
require "socket.luamime"
require "mygui"
require "console"
require "errorhandle"

_ENV = module(...)