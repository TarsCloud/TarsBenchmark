
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

#ifndef __LICOTE_PRIVATE_H__
#define __LICOTE_PRIVATE_H__


/***************************************************************
* 宏定义
***************************************************************/
#define LICOTE_MAX_OPTIONS			(128)	/* Licote支持的最大选项个数 */
#define LICOTE_HASH_SIZE			(10 + 26 + 26 + 1)/* HASH表大小 */ 
#define LICOTE_USAGE_LEN			(128)	/* 用法最大长度 */
#define LICOTE_USAGE_SIZE			(20)	/* 每行最多选项个数 */
#define LICOTE_MAX_KEY_LEN			(32)	/* 长选项最大字符个数 */
#define LICOTE_BOOL_TRUE			"1"		/* Licote选项是否存在 */

/* Licote版本信息 */
#define __LICOTE_VERSION			"1.3.4"
#define __LICOTE_RELEASE			"2013/11/11"

/* Licote设置密码的选项 */
#define LICOTE_LIXOPT_PWD			"--licote-pwd"
#define LICOTE_WINOPT_PWD			"/licote-pwd"

/* LICOTE选项标记 */
#define	FLAGS_ONOFF					(1 << 0)
#define FLAGS_OPTIONAL				(1 << 1)
#define FLAGS_HOOK					(1 << 2)
#define FLAGS_HELP					(1 << 3)
#define FLAGS_HIDE					(1 << 4)
#define FLAGS_CLASS					(1 << 5)
#define FLAGS_DEPEND				(1 << 6)
#define FLAGS_PASSWD				(1 << 7)
#define FLAGS_APPEND				(1 << 8)
#define FLAGS_VERSION				(1 << 9)
/* ^.^ */
#define FLAGS_EXIST					(1 << 10)	// 标记获得用户输入
#define FLAGS_NONE					(1 << 11)	// 标记非选项节点

enum{
	LINUX = '-',					/* Linux风格 */
	WINDOWS = '/'					/* Windows风格 */
};
/* 隐藏属性 */
enum{
	OPT_PWD_EXIST = (1 << 0),		/* 有声明可隐藏的选项 */
	OPT_PWD_CORRECT = (1 << 1)		/* 用户输入了正确的密码 */
};
/* 字符串索引: 与g_LicoteF[]一一对应 */
enum{
	TIPS = 0,
	DEPICT,
	USAGE,
	HIDE,
	OPTION,
	EXAMPLE,
	ERROR_OPTION,
	MISS_OPTION,
	MISS_VALUE,
	ERROR_USAGE,
	ERROR_ARGC,
	ERROR_PWD
};


/** 显式调用函数标记 */
#define __licote_call
#define __no_call


/***************************************************************
* Licote钩子函数原型
***************************************************************/
/** 
 * @param opt	: 匹配成功的option名称
 * @param argc	: 参数列表个数
 * @param argv	: 参数指针数组
 * @return	: >0: 返回已经处理完成的参数个数, 
 *			  0 : 表示只处理了当前option,而没有处理任何值
 * @brief	: Licote钩子函数原型
 */
typedef int (*licote_hook_t)(const char* opt, 
							 int argc, 
							 char** argv);
/***************************************************************
* 私有接口
***************************************************************/
#ifdef __cplusplus
	extern "C" {
#endif  
/**
 * @brief licote构造函数
*/
__no_call void 
__licote_core_init(void);

/**
 * @brief licote析构函数
*/
__no_call void
licote_option_exit(void)__attribute__((destructor));

/**
 * @brief 注册一个Licote选项
*/
__no_call void
licote_option_add(const char* opt, 
				  const char* flags, 
				  const char* info);

/**
 * @brief 注册选项的别名
*/
__no_call void
licote_option_alias(const char* o,
					const char* a);
	
/**
 * @brief 注册选项的钩子函数
*/
__no_call void
licote_option_hook(const char* patt, 
				   licote_hook_t hook);

#ifdef __cplusplus
}
#endif
#endif /* __LICOTE_PRIVATE_H__ */


