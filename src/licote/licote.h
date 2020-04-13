
/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#ifndef __LICOTE_H__
#define __LICOTE_H__

#include "common.h"
#include "private.h"

enum{
	CN = 0,	/* 中文 */
	EN = 1	/* English */
};

/** LICOTE命令选项声明宏 */
#define LICOTE_OPTION_BEGIN					\
	static int __licote_startup(void)		\
	__attribute__((constructor));			\
	static int __licote_startup(void){		\
	__licote_core_init();

/** 选项声明 */
#define LICOTE_OPTION_DECL(opt, flag, info)	\
	licote_option_add(opt, flag, info)
/** 选项别名 */
#define LICOTE_OPTION_ALIAS(o1, o2)			\
	licote_option_alias(o1, o2)
/** 插入钩子回调函数 */
#define LICOTE_OPTION_HOOK(pat,hook)		\
	licote_option_hook(pat,hook)
/** 选项分隔符 */
#define LICOTE_OPTION_CLASS(info)			\
	LICOTE_OPTION_DECL(0, "C", info)
/** 附加选项描述信息 */
#define LICOTE_OPTION_APPEND(opt,msg)		\
	LICOTE_OPTION_DECL(opt, "A", msg)
/** 声明结束 */
#define LICOTE_OPTION_END					\
		return 0;							\
	}

/** 设置命令描述信息 */
#define LICOTE_SET_DESCRIPTION(msg)		do{	\
	extern __no_call const char*			\
	__g_licote_describe;					\
	__g_licote_describe = msg;				\
}while(0)

/** 替换默认的USAGE */
#define LICOTE_SET_USAGE(msg)			do{	\
	extern __no_call const char*			\
	__g_licote_usage;						\
	__g_licote_usage = msg;					\
}while(0)

/** 设置命令例子 */
#define LICOTE_SET_EXAMPLE(msg)			do{	\
	extern __no_call const char*			\
	__g_licote_example;						\
	__g_licote_example = msg;				\
}while(0)

/** 设置命令最小参数个数 */
#define LICOTE_SET_ARGC(nbr)			do{	\
	extern __no_call int					\
	__g_licote_argc;						\
	__g_licote_argc = nbr;					\
}while(0)

/** 设置集合显示短选项个数 */
#define LICOTE_SET_GATHER(nbr)			do{	\
	extern __no_call int					\
	__g_licote_gather;						\
	__g_licote_gather = nbr;				\
}while(0)

/** 设置作者姓名 */
#define LICOTE_SET_AUTHOR(name)			do{	\
	extern __no_call const char*			\
	__g_licote_author;						\
	__g_licote_author = name;				\
}while(0)

/** 设置命令版本信息 */
#define LICOTE_SET_VERSION(msg)			do{	\
	extern __no_call const char*			\
	__g_licote_version;						\
	__g_licote_version = msg;				\
}while(0)

/** 设置更多的相关信息 */
#define LICOTE_SET_MORE(msg)			do{	\
	extern __no_call const char*			\
	__g_licote_more;						\
	__g_licote_more = msg;					\
}while(0)

/** 设置licote密码[用于显示隐藏选项] */
#define LICOTE_SET_PWD(pwd)				do{	\
	extern __no_call const char*			\
	__g_licote_pwd;							\
	__g_licote_pwd = pwd;					\
	LICOTE_OPTION_DECL(0, "W",				\
		"used to show the hide option.");	\
}while(0)

/** 设置提示的语言 */
#define LICOTE_SET_LANGUAGE(lan)		do{	\
	extern __no_call int   					\
	__g_licote_lan;							\
	__g_licote_lan = !!lan;					\
}while(0)


#ifdef __cplusplus
	extern "C" {
#endif
/***************************************************************
* 导出接口
***************************************************************/
/**
 * @param	argc: 命令参数个数
 * @param	argv: 命令参数列表
 * @brief	初始化Licote库程序
 * @return	0: 成功 -1: 失败
*/
__licote_call int
licote_option_init(int argc, char** argv);

/**
 * @parem	opt: 选项的名称，可以是短选项或长选项
 * @brief	根据选项的名称获取相应的值
 * @return	NULL:不存在  VALUE:KV型选项的值 
 * 			LICOTE_BOOL_TRUE:特殊的开关型选项的值
*/
__licote_call const char*
licote_option_get(const char* opt);

/**
 * @param	opt: 选项的名称，可以是短选项或长选项
 * @brief	判断选项是否存在
 * @return	1:存在 0:不存在
*/
__licote_call int
licote_option_exist(const char* opt);

/**
 * @param	fmt:帮助信息前的提示信息
 * @brief	导出显示帮助信息想借口
*/
__licote_call void
licote_option_help(const char* fmt,...);

/**
 * @brief 打印Licote选项解析的结果，用于调试;
*/
__licote_call void
licote_option_debug(void);


#ifdef __cplusplus
}
#endif
#endif /* __LICOTE_H__ */


 
