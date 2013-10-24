if exists("g:loaded_luadebug")
	finish
endif

let g:loaded_luadebug = 1

"启动调试器
luafile ~/.vim/plugin/debuger.lua

"定义命令将命令和lua函数关联
command Dbg :lua dbgs()
command Dbge :lua dbge()
command Err :lua err()
command Step :lua step()
command Stepin :lua stepin()
command Continue :lua continue()
command Traceback :lua traceback()
command Tracefront :lua tracefront()
command Reset :lua reset()
command Clearall :lua clearall()
command Stack :lua stack()
command -nargs=1 G :lua get("<args>")
command Bp :lua bp()
command Get :lua get_it()

"定义热键
map <F5> :Continue<CR>
map <F6> :Step<CR>
map <F7> :Stepin<CR>
map <F4> :Bp<CR>
map <F3> :Reset<CR>

"定义菜单
menu Debug.New :Dbg<CR>
menu Debug.Close :Dbge<CR>
amenu Debug.-sep1- :
menu Debug.Continue<Tab>F5 :Continue<CR>
menu Debug.Step<Tab>F6 :Step<CR>
menu Debug.StepIn<Tab>F7 :Stepin<CR>
amenu Debug.-sep2- :
menu Debug.Breakpoint<Tab>F4 :Bp<CR>
menu Debug.Clear\ all :Clearall<CR> 
amenu Debug.-sep3- :
menu Debug.Trace\ Back :Traceback<CR>
menu Debug.Trace\ Front :Tracefront<CR>
amenu Debug.-sep4- :
menu Debug.Reset<Tab>F3 :Reset<CR>
menu Debug.Last\ error :Err<CR>

"定义一个取光标下的变量的功能
map <C-Z> :Get<CR>
