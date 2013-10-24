//MMBackend.m 增加如下行

/**search 查找定位
    } else if (NetBeansMsgID == msgid) {
#ifdef FEAT_NETBEANS_INTG
        netbeans_read();
#endif
 下面是增加的内容
 */
    } else if (LuaSocketMsgID == msgid ) {
	lua_read_event();



