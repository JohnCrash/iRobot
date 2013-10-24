vim lua 调试器插件

需要对vim进行修改,目前从macvim 7.3 806版本修改而来
应用 path.7.3.806.diff.

将luadebug.vim,debuger.lua拷贝到vim插件目录
mac下在~/.vim/plugin

插件的使用:
启动vim会有菜单Debug
Debug/New 等待启动iRobot,进行连接.timeout=10秒
Debug/Close 关闭连接,包括服务器socket,一般不使用
Debug/Continue F5 中断继续
Debug/Step F6 单步
Debug/StepIn F7 单步进入
Debug/Breakpoint F4 在当前光标行设置断点
Debug/Clear all 删除全部断点
Debug/Track back 跳到调用堆栈的上一级
Debug/Track front 跳到调用堆栈的下一级
Debug/Reset 重新启动iRobot
Debug/Last error 显示最近的错误信息

热键
Ctrl+Z 取光标位置变量的内容

菜单对应命令见luadebug.vim


插件使用luaSocket和iRobot通讯,并且参加vim netbeans接口将
socket接收方式该为事务模式.vim.asyn_socket(connect,receiver)
完成设置,当连接connect有数据接收时调用receiver
另外vim.vim_msg(callback) 将使得vim空闲时会调用callback

