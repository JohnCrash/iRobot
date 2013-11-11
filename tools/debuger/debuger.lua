local socket = require "socket"
local io = require "io"

local server
local connect
local def_sign
local last_error_source
local last_error_line
local last_error_msg
local signs = {} --维护一个标号表

--定义一个sign,用来表示断点位置
function sign_init()
	if not def_sign then
		vim.command('sign define breakpoint text=* texthl=Search')	
		def_sign = true
	end
end

--查找标记,如果发现返回true,id(标号)
--如果没找到,分配一个新的id,返回false ,id
function sign_search(source,line)
	for i=1,#signs do
		if signs[i] then
			if signs[i].line ==line and signs[i].source==source then
				return true,i
			end
		end
	end
	--找一个洞
	for i=1,#signs do
		if not signs[i] then
			return false,i
		end
	end
	--增加一个
	return false,#signs+1
end
--设置一个断点标记
function sign(source,line)
	sign_init()
	if focus(source) then
		local has,id = sign_search(source,line)
		if has then
			--删除标记
			vim.command('sign unplace '..id)
			signs[id] = nil
		else
			--增加标记
			vim.command('sign place '..id..' line='..line..' name=breakpoint file='..source)
			signs[id] = {}
			signs[id].source = source
			signs[id].line = line
		end
	end
end

--清除全部断点标记
function unsignall()
	signs = {}
	vim.command('sign unplace *')
end

--设置断点,当前文件当前行
local bps={} --这里维护一个断点表
function bp(source,line)
	local w = vim.window()
	if not w then return end
	if not source then
		source = w.buffer.fname
		line = w.line
	end
	for i,v in ipairs(bps) do
		if v then
			if v.line == line and v.source == source then
				--断点已经存在删除断点
				send('clear<'..v.source..'>@'..v.line..'\n')
				sign(source,line)
				bps[i] = nil
				return
			end
		end
	end
	--断点不存在增加断点
	send('bp<'..source..'>@'..line..'\n')
	sign(source,line)
	for i=1,#bps do
		if not bps[i] then
			bps[i] = {}
			bps[i].source = source
			bps[i].line = line
			return
		end
	end
	bps[#bps+1] = {}
	bps[#bps].source = source
	bps[#bps].line = line
end
--清除标线
local match_source
function clear_match()
	if match_source then
		local current = vim.window()
		if current and 
			string.upper(current.buffer.fname)==
			string.upper(match_source) then
			vim.command('match none')
		elseif focus(match_source) then
			vim.command('match none')
			if current then
				focus(current.buffer.fname)
			end
		end
		match_source=nil
	end
end
--同步bps
function asynbps()
	for i,v in ipairs(bps) do
		send('bp<'..v.source..'>@'..v.line..'\n')
	end
end
--clearall
function clearall()
	send('clearall\n')
	unsignall()
	bps = {}
end
--stepin
function stepin()
	send('stepin\n')
	clear_match()	
end
--step
function step()
	send('step\n')
	clear_match()
end
--continue
function continue()
	send('continue\n')
	clear_match()
end
--reset
function reset()
	send('reset\n')
	clear_match()
	dbgs()
end
--打印最近的错误信息
function err()
	if last_error_msg then
		if focus(last_error_source,last_error_line) then
			print(last_error_msg)
		end
	end
end
--traceback
function traceback()
	send('traceback\n')
end
--tracefront
function tracefront()
	send('tracefront\n')
end
--get变量信息
function get(var)
	send('get<'..var..'>\n')
end
--搜索直到
function search_until(str,i)
	 local s,e = string.find(str,'[^%w_]',i)
	 if s then
	 	return string.sub(str,i,s-1)
	 end
	 return string.sub(str,i)
end
--取光标位置的变量
function get_it()
	if not connect then
		print('connect is close!')
		return
	end
	local w = vim.window()
	if w and w.buffer then
		local line = w.buffer[w.line]
		if line then
			local name_e = search_until(line,w.col+1)
			local rline = string.reverse(line)
			local rcol = string.len(line)-w.col+1
			local name_s = search_until(rline,rcol)
			name_s = string.reverse(name_s)
			return get(name_s..name_e)
		end
	end
end
--stack返回堆栈信息
function stack()
	send('stack\n')
end
--切换到源文件source,当前行设置为line
--有该文件返回ture,没有该文件返回nil
function focus(source,line)
	local w = vim.window(true)
	while w do
		if string.upper(tostring(source))==string.upper(tostring(w.buffer.fname)) then
			w()
			if line then
				vim.command(line)
			end
			return true
		end
		w = w:next()
	end
end

--高亮文件的一行
function hiLine(group,source,line)
	if focus(source) then
		if line then
			match_source = source
			vim.command('match '..group..' /\\%'..line..'l/')
		else
			match_source = nil
			vim.command('match none')
		end
	end
end

--返回文件的路径
function path(file)
	local s = string.reverse(file)
	local idx = string.find(s,'/',1)
	if idx then
		return string.sub(file,1,-idx)
	end
end

--打开一个源文件source,并且跳转到line行
--如果已经存在切换到该文件
function openSource(group,source,line)
	if focus(source,line) then
		hiLine(group,source,line)
		vim.eval('foreground()')
		return
	else
		--没有找到文件
		vim.command('vne')
		vim.command('e '..source)
		vim.command(tostring(line))
		hiLine(group,source,line)
		vim.command('cd '..path(source))
		vim.eval('foreground()')
	end
end

--错误处理
function errorpoint_handle(source,line)
	openSource('ErrorMsg',source,line)
end

--断点处理
function breakpoint_handle(source,line)
	openSource('Search',source,line)
end

--变量信息,使用\r分割没行
function info_handle(msg)
	local s,e
	if msg then
		local i = 1
		repeat
			s,e = string.find(msg,'\t',i)
			if e then
				print(string.sub(msg,i,s-1))
				i = e + 1
			end
		until not s
		if string.len(msg)>i then
			print(string.sub(msg,i))
		end
	end
end
--错误信息处理
--一般包括source:line:msg
function error_handle(msg)
	local sp,ep = string.find(msg,':%d+:')
	if sp then
		local source = string.sub(msg,0,sp-1)
		local line = string.sub(msg,sp+1,ep-1)
		local error_msg = string.sub(msg,ep+1,-1)
		last_error_msg=error_msg
		last_error_source = source
		last_error_line = line
		openSource('ErrorMsg',source,line)
		print(error_msg)
	else
		print("invalid msg:"..msg)
	end
end

--有数据可以读取
function receiver()
	if connect~=nil then
		local msg,err = connect:receive("*l")
		if msg then
			local idx,source,line
			if string.sub(msg,1,6) == 'error<' then
				idx = string.find(msg,'>@')
				if idx then
					source = string.sub(msg,7,idx-1)
					line = string.sub(msg,idx+2)
					errorpoint_handle(source,line)
				else
					idx = string.find(msg,'>')
					if idx then
						error_handle(string.sub(msg,7,idx-1))
					else
						print('invalid msg:'..msg)
					end
				end
			elseif string.sub(msg,1,6) == 'break<' then
				idx = string.find(msg,'>@')
				if idx~=nil then
					source = string.sub(msg,7,idx-1)
					line = string.sub(msg,idx+2)
					breakpoint_handle(source,line)
				else
					print('invalid msg:'..msg)
				end
			elseif string.sub(msg,1,5) == 'info:' then
				info_handle( string.sub(msg,6) )
			end
		elseif err=='timeout' then
			return
		else
			print('connect:receive '..err)
			connect:close()
			connect = nil
		end
	end
end

function dbgs()
	local err,result
	if not server then
		server,err = socket.tcp()
		if not server then
			print('socket.tcp :'..err)
			return
		end
		result,err = server:bind("127.0.0.1",3030)
		if not result then
			print('server:bind :'..err)
			server:close()
			server=nil
			return
		end
		result,err = server:listen(10)
		if not result then
			print('server:listen :'..err)
			server:close()
			server=nil
			return
		end
		print("connect:127.0.0.1:3030")
		--这里发生阻塞
		server:settimeout(10)
		connect,err = server:accept()
		if connect then
			connect:settimeout(0)
			vim.asyn_socket(connect,receiver)
			--同步以前设置的断点
			asynbps()
		else
			print('server:accept '..err)
		end
	else
		if connect then
			vim.asyn_socket(connect,nil)
			connect:close()
			connect = nil
		end
		print('connect:127.0.0.1:3030')
		server:settimeout(10)
		connect,err = server:accept()
		if connect then
			connect:settimeout(0)
			vim.asyn_socket(connect,receiver)
			--同步以前断点
			asynbps()
		else
			print('server:accept '..err)
		end
	end
end

function send(msg)
	if connect~=nil then
	--	print('send:'..msg)
		connect:send(msg)
	end
end

function dbge()
	if connect then
		vim.asyn_socket(connect,nil)
		connect:close()
		connect = nil
	end
	if server then
		server:close()
		server = nil
	end
	print("disconnect")
end
