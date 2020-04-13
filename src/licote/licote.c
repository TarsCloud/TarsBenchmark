
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
#include <stdarg.h>
#include <regex.h>
#include "list.h"
#include "licote.h"
#include "private.h"

/* 提取字符串的宏 */
#define __MSG(idx)				g_LicoteM[idx][__g_licote_lan]
/* 获取选项名称 */
#define __NAME(node)			\
	((node)->sopt ? (node)->sopt : (node)->lopt)

/* 判断一个选项是否为长选项 */
#define LINUX_LONG(opt)					\
	((g_LicoteC.style == LINUX) && 		\
	((opt)[0] == LINUX) && 				\
	((opt)[1] == LINUX))

#define WINDOWS_LONG(opt)				\
	((g_LicoteC.style == WINDOWS) && 	\
	((opt)[0] == WINDOWS) &&			\
	(strlen(opt) > 2))

#define IS_LONG_OPTION(opt)				\
	LINUX_LONG(opt) || WINDOWS_LONG(opt)

/* 判断是否集合显示开关型短选项 */
#define IS_GATHER_SHOW()				\
	(g_LicoteC.style == LINUX && 		\
	__g_licote_gather != 0 &&			\
	g_LicoteC.gather >= __g_licote_gather)

#define IS_GATHER(opt)					\
	((g_LicoteC.style == LINUX) &&		\
	strlen(opt) > 2 && opt[1] != LINUX)

/** 循集合短选项 */
#define GATHER_FOR_EACH(opt, buf)	do{	\
	int _i_ = 1;						\
	for(_i_=1; _i_<strlen(opt); _i_++){	\
		snprintf(buf, sizeof(buf), "-%c",\
				 opt[_i_]);

#define GATHER_END()		}}while(0)

/* 格式化LICOTE选项
 * m是在info前面需要加上的信息
 */
#define LICOTE_OPTION_FORMAT(node, m)	do{\
	LICOTE_SHOW("  %2s%1s %-16s :%s%s\n",\
		node->sopt ? node->sopt : "",	\
		(node->sopt && node->lopt) ? "," : "",\
		node->lopt ? node->lopt : "", 	\
		m, node->info);					\
}while(0)
/* 格式化附件选项描述信息 */
#define LICOTE_OPTION_FORMAT2(node)	do{	\
	LICOTE_SHOW("  %-20s :%s\n",		\
		node->sopt,node->info);			\
}while(0)


/***************************************************************
* 类型与结构体声明
***************************************************************/
/* Licote选项 */
typedef struct __licote_option{
	const char*				sopt;	/* 短选项标记符 */
	const char*				lopt;	/* 长选项标记符 */
	const char*				info;	/* 提示信息 */
	uint16_t				flag;	/* 开关型,可选的,版本,帮助等标记 */
	void*					value;	/* 选项值 */
	licote_hook_t			hook;	/* 钩子函数 */
	struct list_head		list0;	/* 原始选项链表 */
	void*					child;	/* 依赖链表ListNode*类型 */
}LicoteOption;

/* 哈希节点 */
typedef struct __hash_node{
	struct hlist_node		hlist;	/* HASH节点 */
	LicoteOption*			data;	/* 选项缓存地址 */
}HashNode;
/* 依赖节点 */
typedef struct __list_node{
	struct __list_node*		next;	/* 链表指针 */
	LicoteOption*			data;	/* 选项缓存地址 */
}ListNode;


/***************************************************************
* 全局变量
***************************************************************/
/* 版本信息 */
int
__g_licote_lan		= CN;			/* 默认为中文 */
const char*
__g_licote_pwd		= NULL;			/* 隐式命令密码 */
const char*
__g_licote_author	= NULL;			/* 命令作者信息 */
const char*
__g_licote_version  = NULL;			/* 命令版本信息 */
const char*
__g_licote_more		= NULL;			/* 设置更多信息 */
const char*
__g_licote_describe = NULL;			/* 命令描述信息 */
const char*
__g_licote_usage	= NULL;			/* 自定义用法信息 */
const char*
__g_licote_example	= NULL;			/* 命令使用例子 */
int
__g_licote_argc 	= 0;			/* 最小参数个数 */
int
__g_licote_gather	= 5;			/* 集合显示个数阀值 */


/* Licote支持的标记 */
static struct __option_flag{
	char		f;
	uint16_t		v;
}g_LicoteF[] = {
	{':', FLAGS_ONOFF},					/* 修饰选项为开关型 */
	{'>', FLAGS_OPTIONAL|FLAGS_DEPEND},	/* 修饰选项为条件依赖[优先级不能低于下面的普通字符] */
	{'o', FLAGS_OPTIONAL},				/* 修饰选项为可选的 */
	{'v', FLAGS_OPTIONAL|FLAGS_VERSION|FLAGS_ONOFF},/* 修饰选项为显示版本信息 */
	{'h', FLAGS_OPTIONAL|FLAGS_HELP|FLAGS_ONOFF},	/* 修饰选项为显示版主信息 */
	{'H', FLAGS_OPTIONAL|FLAGS_HIDE},	/* 修饰选项为可隐藏 */
//--下面为LICOTE内部使用标记,非导出标记--//
	{'W', FLAGS_OPTIONAL|FLAGS_PASSWD|FLAGS_HIDE},	/* 修饰选项为LICOTE密码选项 */
	{'C', FLAGS_OPTIONAL|FLAGS_NONE|FLAGS_CLASS},	/* 特殊选项，用于分类 */
	{'A', FLAGS_OPTIONAL|FLAGS_NONE|FLAGS_APPEND}	/* 附加描述信息 */
};

/* Licote管理结构体 */
static struct __licote_core{
	uint32_t				argc;		/* 参数个数 */
	char**					argv;		/* 参数缓存 */
	const char*				path;		/* 命令路径 */
	const char*				name;		/* 程序名称 */
	const char*				optpwd;		/* 隐藏选项的名称 */
	uint16_t				hide;		/* 隐藏选项标记 */
	uint16_t				count;		/* 选项的个数(包括分隔符/附件选项等) */
	uint8_t					style;		/* Licote风格 */
	uint8_t					gather;		/* 开关型短选项的个数 */
}g_LicoteC;

/* 所有选项链表 */
static struct list_head		g_list;		/* 原始选项链表 */
static struct list_head		g_regex;	/* HOOK选项链表(单独存放) */
/* 哈希链表和缓存 */
static struct hlist_head	g_hash[LICOTE_HASH_SIZE] = {{0}};	/* 用于检索 */
static LicoteOption			g_pool[LICOTE_MAX_OPTIONS] = {{0}};

/* Licote LOGO */
static const char* g_LicoteL[] = {
" _         _____   _____  _______  _____ (R)",
"/ |  \033[31m0ooo\033[0m /  _  \\ /  _  \\|__   __\\/  ___\\",
"| |  (  ) | | |_| | | | |   | |   | |__",
"| |  ) /  | |  _  | | | |   | |   |  __|",
"| |_(_/___| |_| |_| |_| |___| |___| |___",
"\\____|____\\_____/_\\_____/___\\_|___\\_____/"
};

/* 用户提示信息 */
static const char* g_LicoteM[][2] = {
	{"notice",               "Tips"},
	{"description",          "Depict"},
	{"usage",                "Usage"},
	{"hide",                 "Hide"},
	{"option",               "Option"},
	{"example",              "Example"},
	{"不支持的命令选项",   	  "Unsupported option"},
	{"请输入必须的选项",   "Please input the option"},
	{"请设置选项的值",     "The option missing argument"},
	{"错误的用法",         "Invalid usage"},
	{"此命令至少需要%d个参数",  "Options less than %d"},
	{"密码错误,请重新输入","Error password, input again"}
};


/***************************************************************
* 本地函数声明
***************************************************************/
static void
__licote_core_exit(void);
/** 真正的选项解析过程 */
static int
__licote_parse_input(int argc, char** argv);
/** 用于解析长选项 @return !NULL: 成功,需要为返回的选项添加值 */
static LicoteOption*
__licote_parse_long(const char* opt);
/** 用于解析短选项 @return !NULL: 成功,需要为返回的选项添加值 */
static LicoteOption*
__licote_parse_short(const char* opt);
/** 选项标记解析 */
static void
__licote_parse_flags(LicoteOption* node,
					const char* opt,
					const char* flags);
/** 处理选项的依赖列表 */
static void
__licote_parse_depend(LicoteOption* node,
					const char* opt,
					const char* buf);
/** 处理每一个依赖选项 */
static int
__licote_parse_each(LicoteOption* node,
					const char* opt,
					const char* buf);
/** 显示帮助信息 */
static void
__licote_show_help(void);
static void
__licote_show_depict(void);
static void
__licote_show_usage(void);
static void
__licote_show_hide(void);
static void
__licote_show_option(void);
static void
__licote_show_example(void);
/** 显示版本信息 */
static void
__licote_show_version(void);

/** 计算选项对应的HASH值 */
static int
__licote_hash_key(const char* opt);
/** 根据opt获取LicoteOption */
static LicoteOption*
__licote_hash_get(const char* opt);
/** 将选项插入HASH链表 */
static int
__licote_hash_insert(uint32_t key, LicoteOption* popt);
/** 将node插入opt所在选项的依赖列表 */
static int
__licote_list_insert(LicoteOption* node,
					const char* opt,
					const char* depend);
/** 更新选项标记 */
static void
__licote_node_update(LicoteOption* popt);
/** 检查选项中的特殊标记 */
static void
__licote_node_check(LicoteOption* popt);
/** 检测是否输入所有的必须选项 */
static void
__licote_valid_check(void);


/***************************************************************
* Export API
***************************************************************/
int
licote_option_init(int argc, char* argv[])
{
	/* 检查参数合法性 */
	if (!argv){
		LOG_ERROR("invalid parameter,argv=null!");
		return -1;
	}
	/* 获取程序的名称 */
	g_LicoteC.argc = argc;
	g_LicoteC.argv = argv;

	char* ptr = strrchr(argv[0], '/');
	if (!ptr){
		g_LicoteC.path = ".";
		g_LicoteC.name = argv[0];
	}else{
		*ptr++ = '\0';
		g_LicoteC.path = argv[0];
		g_LicoteC.name = ptr;
	}

  	/* 检查最小参数个数 */
	if (__g_licote_argc && (argc - 1) <  __g_licote_argc){
		licote_option_help(__MSG(ERROR_ARGC), __g_licote_argc);
	}
	/* 判断当前风格 */
	if (g_LicoteC.style == LINUX){
		g_LicoteC.optpwd = LICOTE_LIXOPT_PWD;
	}else{
		g_LicoteC.optpwd = LICOTE_WINOPT_PWD;
	}

	/* 执行参数解析 */
	return __licote_parse_input(argc, argv);
}

void
licote_option_exit(void)
{
	__licote_core_exit();
}

void
licote_option_add(const char* opt,
				  const char* flags,
				  const char* info)
{
	/* 参数检测: opt,flags可能为空 */
	if(!info){
		LOG_ERROR("Invalid param, opt=null or info=null!");
		return;
	}

	if (g_LicoteC.count >= LICOTE_MAX_OPTIONS){
		LOG_ERROR("options reach the limit: %d, real: %d", LICOTE_MAX_OPTIONS, g_LicoteC.count);
		return;
	}

    /* 命令已注册，更新它 */
    LicoteOption* popt = __licote_hash_get(opt);
    if (popt != NULL) {
        if (flags) { /* 转换flags标记 */
            __licote_parse_flags(popt, opt, flags);
        }
        popt->info = info;
        return;
    }

	int len = 0;
	int key = 0;
    popt = &g_pool[g_LicoteC.count];
	popt->info = info;
	if (flags){/* 转换flags标记 */
		__licote_parse_flags(popt, opt, flags);
	}
	if (popt->flag & FLAGS_CLASS){	/* 分类符 */
		goto REGISTER_SUCC;
	}
	if (popt->flag & FLAGS_APPEND){	/* 附加描述 */
		popt->sopt = opt;
		goto REGISTER_SUCC;
	}
	if (popt->flag & FLAGS_PASSWD){	/* 设置密码 */
		switch(g_LicoteC.style){
		case LINUX:
			opt = LICOTE_LIXOPT_PWD;
			popt->lopt = opt;
			goto INSERT_HASH;
		case WINDOWS:
			opt = LICOTE_WINOPT_PWD;
			popt->lopt = opt;
			goto INSERT_HASH;
		default:
			LOG_ERROR("LICOTE_SET_PWD must be call after LICOTE_OPTION_DECL.");
			goto REGISTER_FAIL;
		}
	}

	/* 检查选项的合法性: opt可能为空,因此放到这里检查 */
	len = opt ? strlen(opt) : 0;
	if (len < 2 || (opt[0] != LINUX && opt[0] != WINDOWS) ||
		(len == 2 && opt[1] == '-')){	//不允许--, /- 这的选项存在
		LOG_ERROR("Invalid opt: %s, opt[0] must be '-','/'", opt);
		goto REGISTER_FAIL;
	}
	if (len  > 2 && opt[0] == LINUX && opt[1] != '-'){
		LOG_ERROR("The short option did't meet the rule of licote:%s", opt);
		goto REGISTER_FAIL;
	}

	/* 如果是第一个真实的选项, 则识别选项的风格 */
	if (g_LicoteC.style == 0){
		g_LicoteC.style = opt[0];
	}
	if (opt[0] != g_LicoteC.style){
		LOG_ERROR("register option %s failed: can't mix %s with %s!", opt,
					g_LicoteC.style == LINUX ? "Linux" : "Windows",
					g_LicoteC.style != LINUX ? "Linux" : "Windows");
		goto REGISTER_FAIL;
	}

	/* 判断选项的类别 */
	if (len == 2){
		popt->sopt = opt;
		if (popt->flag & FLAGS_ONOFF){
			g_LicoteC.gather += 1;	/* 统计开关型短选项的个数 */
		}
	}else{
		popt->lopt = opt;
	}

INSERT_HASH:
	/* 插入HASH链表:只有真实的选项才应走到这一步 */
	key = __licote_hash_key(opt);
	if (__licote_hash_insert(key, popt) != 0){
		goto REGISTER_FAIL;
	}

REGISTER_SUCC:
    /* 添加原始链表:分类符/附件描述会直接跳转到这里 */
    list_add_tail(&popt->list0, &g_list);
    g_LicoteC.count += 1;
    return;

REGISTER_FAIL:
	memset(popt, 0, sizeof(LicoteOption));
	return;
}

void
licote_option_hook(const char* patt, licote_hook_t hook)
{
	static char buf[TMP_BUF_LEN128] = {0};
	if (!hook){ /* 允许patt为空,表示匹配最后一个选项 */
		LOG_ERROR("invalid param, hook=null");
		return;
	}

	int ret = 0;
	LicoteOption* node = (LicoteOption*)malloc(sizeof(LicoteOption));
	if (!node){
		LOG_ERROR("register option failed: malloc failed.");
		return;
	}
	memset(node, 0, sizeof(LicoteOption));

	/* LICOTE 钩子选项将使用选项中的
	 * info保存模式字符串, value保持regex_t指针
	*/
	node->flag  = FLAGS_HOOK|FLAGS_OPTIONAL;
	node->hook  = hook;
	node->info  = patt;
	if (node->info){
		node->value = malloc(sizeof(regex_t));
		if (!node->value){
			LOG_ERROR("register option %s failed:malloc failed.", patt);
			goto REGISTER_FAIL;
		}
		/* 编译正则表达式 */
		ret = regcomp(node->value, node->info, REG_NOSUB|REG_EXTENDED);
		if (ret != 0){
			goto REGISTER_FAIL;
		}
	}

	/* 添加到钩子链表 */
	list_add_tail(&node->list0, &g_regex);
	return;

REGISTER_FAIL:
	if (ret != 0){
		regerror(ret, node->value, buf, sizeof(buf));
		LOG_ERROR("%s: '%s'\n", buf, node->info);
	}
	FREE_EMPTY(node);
	FREE_EMPTY(node->value);
}

void
licote_option_alias(const char* ori,
					const char* ali)
{
	if (!ori || !ali){
		LOG_ERROR("error:alias %s as %s failed!", ori, ali);
		return;
	}

	LicoteOption* popt = __licote_hash_get(ori);
	if (!popt){
		LOG_ERROR("error:alias %s as %s failed: %s not exist!", ori, ali, ori);
		return;
	}

	if (popt->lopt == NULL){
		popt->lopt = ali;
	}else{
		popt->sopt = ali;
		if (popt->flag & FLAGS_ONOFF){
			g_LicoteC.gather += 1;	/* 统计开关型短选项的个数 */
		}
	}

	/* 插入一个别名HASH节点 */
	int key = __licote_hash_key(ali);
	__licote_hash_insert(key, popt);
}

const char*
licote_option_get(const char* opt)
{
	if (!opt){
		return NULL;
	}

	LicoteOption* node = __licote_hash_get(opt);
	if (node && (node->flag & FLAGS_EXIST)){
		return (const char*)(node->value ? node->value : LICOTE_BOOL_TRUE);
	}

	return NULL;
}

int
licote_option_exist(const char* opt)
{
	if (!opt){
		return 0;
	}

	LicoteOption* node = __licote_hash_get(opt);
	if (node && (node->flag & FLAGS_EXIST)){
		return 1;
	}else{
		return 0;
	}
}

void
licote_option_help(const char* fmt,...)
{
	if (!g_LicoteC.argc || !g_LicoteC.argv){
		LOG_ERROR("Please don't call %s before licote_option_init.",
					__FUNCTION__);
		exit(0);
	}

	/* 显示LICOTE帮助信息 */
	__licote_show_help();

	/* 显示提示信息 */
	if (fmt){
		LICOTE_SHOW("%s%s: ", COLOR_RED, __MSG(TIPS));
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		LICOTE_SHOW(COLOR_OK"\n");
	}

	/* 直接退出 */
	exit(0);
}

void
licote_option_debug(void)
{
	struct list_head* pos = NULL;

	list_for_each(pos, &g_list){
		LicoteOption* node = list_entry(pos, LicoteOption, list0);
		// 跳过非选项
		if (node->flag & FLAGS_NONE){
			continue;
		}

		LICOTE_SHOW("  %2s %-16s :0x%X",
					node->sopt ? node->sopt : "",
					node->lopt ? node->lopt : "",
					node->flag);

		// 如果有依赖列表，则继续打印依赖列表
		ListNode* ptr = node->child;
		if (ptr){
			LICOTE_SHOW(" --> ");
		}
		while(ptr){
			LICOTE_SHOW("%s, ", __NAME(ptr->data));
			ptr = ptr->next;
		}
		LICOTE_SHOW("\n");
	}
}


/***************************************************************
* Local API
***************************************************************/
/* 命令启动执行函数 */
void
__licote_core_init(void)
{
	memset(&g_LicoteC, 0, sizeof(g_LicoteC));

	INIT_LIST_HEAD(&g_list);
	INIT_LIST_HEAD(&g_regex);

	/* 初始化HASH链表 */
	int i = 0;
	for(i=0; i<ARRAY_SIZE(g_hash); i++){
		INIT_HLIST_HEAD(&g_hash[i]);
	}
}
/* 命令退出执行函数 */
static void
__licote_core_exit(void)
{
	struct list_head* pos = NULL;
	list_for_each(pos, &g_list){
		/* 释放regex_t结构体 */
		LicoteOption* node = list_entry(pos, LicoteOption, list0);
		if (node->flag & FLAGS_HOOK){
			regfree(node->value);
			FREE_EMPTY(node->value);/* regex_t结构体 */
		}

		/* 释放依赖链表 */
		ListNode* ptr = (ListNode*)node->child;
		while(ptr){
			ListNode* next = ptr->next;
			FREE_EMPTY(ptr);
			ptr = next;
		}
	}
	INIT_LIST_HEAD(&g_list);

	/* 释放哈希节点 */
	int i = 0;
	for(i=0; i<LICOTE_HASH_SIZE; i++){
		struct hlist_node* hpos = NULL;
		struct hlist_node* hnxt = NULL;
		hlist_for_each_safe(hpos, hnxt, &g_hash[i]){
			HashNode* node = hlist_entry(hpos, HashNode, hlist);
			FREE_EMPTY(node);/* 释放哈希节点 */
		}
	}

	memset(&g_LicoteC, 0, sizeof(g_LicoteC));
}

static int
__licote_parse_input(int argc, char** argv)
{
	ASSERT(argv);

	int i = 1;
	for(i=1; i<argc; i++){
		struct list_head*	pos = NULL;
		const char*			opt = argv[i];
		int match = 0;
		int nbr = 0;
		/* 优先进行正则匹配 */
		list_for_each(pos, &g_regex){
			LicoteOption* node = list_entry(pos, LicoteOption, list0);
			if (!node->info && i == g_LicoteC.argc - 1){
				nbr = node->hook(opt, 0, 0);
				match = 1;
				break;
			}
			if (regexec(node->value, opt, 0, NULL, 0) == 0){/* 匹配成功 */
				nbr = node->hook(opt, argc - i - 1, argv + i + 1);
				match = 1;
				break;
			}
		}
		if (match){/* 选项被HOOK匹配,不用LICOTE处理 */
			if (nbr >= 0){
				i += nbr;
				continue;
			}else{/* 回调函数处理失败 */
				return -1;
			}
		}

		/* LICOTE选项匹配 */
		if(opt[0] != g_LicoteC.style){
			licote_option_help("%s -- %s", __MSG(ERROR_OPTION), opt);
		}

		/* 判断选项类型 */
		LicoteOption* node = NULL;
		if (IS_LONG_OPTION(opt)){/* 长选项 */
			node = __licote_parse_long(opt);
		}else{/* 短选项 */
			node = __licote_parse_short(opt);
		}

        if (++i == argc) {
            licote_option_help("%s -- %s", __MSG(MISS_VALUE), __NAME(node));
        } else if (node) { /* KV型，获取其值 */
            node->value = argv[i];
            __licote_node_update(node);
        }
        continue;
	}

	/* 检查是否输入所有必须的选项 */
	__licote_valid_check();
	return 0;
}

static LicoteOption*
__licote_parse_long(const char* opt)
{
	ASSERT(opt);

	/* 解析<key, value> 例: --file-path=/home/ */
	char  buf[LICOTE_MAX_KEY_LEN] = {0};	/* key */
	char* val = strchr(opt, '=');
	if (val){
		strncpy(buf, opt, val - opt);
		val += 1;
	}else{
		strncpy(buf, opt, sizeof(buf) - 1);
		val = NULL;
	}

	/* 查找支持的选项 */
	LicoteOption* node = __licote_hash_get(buf);
	if (!node){
		licote_option_help("%s -- %s", __MSG(ERROR_OPTION), buf);
	}
	if (!(node->flag & FLAGS_ONOFF) && g_LicoteC.style == LINUX &&
		(!val|| !val[0])){
		/* LINUX:非开关型选项不允许val为空 */
		licote_option_help("%s -- %s", __MSG(MISS_VALUE), buf);
	}
	node->value = val;

	/* 插入输入链表 */
	if (node->flag & FLAGS_ONOFF ||	//WINDOWS选项需做进一步处理
		g_LicoteC.style == LINUX){	//LINUX长选项无需做进一步处理
		__licote_node_update(node);
		return NULL;
	}

	return node;/* 返回node,以便进一步获取其值 */
}

static LicoteOption*
__licote_parse_short(const char* opt)
{/** 例: -abc or /a */
	ASSERT(opt);

	char  buf[LICOTE_MAX_KEY_LEN] = {0};
	int	  len = strlen(opt), i=1;

	for(i=1; i<len; i++){
		snprintf(buf, sizeof(buf), "%c%c", g_LicoteC.style, opt[i]);
		/* 查找支持的选项 */
		LicoteOption* node = __licote_hash_get(buf);
		if (!node){
            continue;
			// 不要退出licote_option_help("%s -- %s", __MSG(ERROR_OPTION), buf);
		}

		/* 非开关型短选项，必须位列最后一个位置 */
		if (!(node->flag & FLAGS_ONOFF)){
			if (i == len - 1){
				return node;
			}
			licote_option_help("%s -- %s", __MSG(ERROR_USAGE), buf);
		}

		/* 特殊选项处理 */
		__licote_node_check(node);

		/* 插入输入链表 */
		__licote_node_update(node);
	}

	/* 无需做进一步处理 */
	return NULL;
}

static void
__licote_parse_flags(LicoteOption* node,
					const char* opt,
					const char* flags)
{
	ASSERT(node && flags);

	/* 选项标记转换 */
	int i = 0;
	for(i=0; i<ARRAY_SIZE(g_LicoteF); i++){
		if (strchr(flags, g_LicoteF[i].f)){
			node->flag |= g_LicoteF[i].v;
		}
		if (node->flag & FLAGS_DEPEND){
			__licote_parse_depend(node, opt, flags);
			break;
		}
		if (node->flag & FLAGS_HIDE){
			g_LicoteC.hide |= OPT_PWD_EXIST;
		}
	}
}

static void
__licote_parse_depend(LicoteOption* node,
					const char* opt,
					const char* flags)
{
	ASSERT(node && flags);
	static char buf[TMP_BUF_LEN64] = {0};

	if (flags[0] != '>'){
		LOG_ERROR("Invalid depend option flags:%s", flags);
		return;
	}
	strncpy(buf, flags + 1, sizeof(buf) - 1);

	char* ptr = strtok(buf, ">");
	while(ptr){
		int ret = __licote_parse_each(node, opt, ptr);
		if (ret != 0){
			return;
		}
		ptr = strtok(NULL, ">");
	};
}

static int
__licote_parse_each(LicoteOption* node,
					const char* opt,
					const char* buf)
{
	ASSERT(node && buf);

	if (IS_GATHER(buf)){
		//Linux集合选项
		char chr[4] = {0};
		GATHER_FOR_EACH(buf, chr){
			int ret = __licote_list_insert(node, opt, chr);
			if (ret != 0){
				return ret;
			}
		}GATHER_END();
	}else{
		//非集合选项
		return __licote_list_insert(node, opt, buf);
	}

	return 0;
}

static int
__licote_hash_key(const char* opt)
{
	ASSERT(opt);
	if (strlen(opt) < 2){
		goto DEFAULT;
	}

	char cid = 0;
	/* 计算HASH KEY */
	if (opt[1] != '-'){
		cid = opt[1];
	}else{
		cid = opt[2];
	}

	if (cid >= 'a' && cid <= 'z'){
		return (10 + cid - 'a');
	}
	if (cid >= 'A' && cid <= 'Z'){
		return (36 + cid - 'A');
	}
	if (cid >= '0' && cid <= '9'){
		return (cid - '0');
	}

DEFAULT:
	return LICOTE_HASH_SIZE - 1;
}

static LicoteOption*
__licote_hash_get(const char* opt)
{
	ASSERT(opt);

	int key = __licote_hash_key(opt);
	struct hlist_node* hpos = NULL;
	hlist_for_each(hpos, &g_hash[key]){
		HashNode* node = hlist_entry(hpos, HashNode, hlist);
		/* 命令支持的选项 */
		if (node->data->sopt && strcmp(node->data->sopt, opt) == 0){
			return node->data;
		}
		if (node->data->lopt && strcmp(node->data->lopt, opt) == 0){
			return node->data;
		}
	}

	return NULL;
}

static int
__licote_hash_insert(uint32_t key, LicoteOption* popt)
{
	ASSERT(key < LICOTE_HASH_SIZE && popt);

	HashNode* node = (HashNode*)malloc(sizeof(HashNode));
	if (!node){
		LOG_ERROR("malloc failed:%s", strerror(errno));
		return -1;
	}
	node->data = popt;
	hlist_add_head(&node->hlist, &g_hash[key]);
	return 0;
}

static int
__licote_list_insert(LicoteOption* node,
					const char* opt,
					const char* depend)
{
	ASSERT(node && depend);

	LicoteOption* father = __licote_hash_get(depend);
	if (!father){
		LOG_ERROR("The option '%s' depend by '%s' is not declare!",
					depend, opt);
		return -1;
	}

	ListNode* ln = (ListNode*)malloc(sizeof(ListNode));
	if (!ln){
		LOG_ERROR("malloc failed:%s", strerror(errno));
		return -1;
	}

	ln->data = node;
	ln->next = father->child;
	father->child = ln;
	return 0;
}

static void
__licote_node_update(LicoteOption* node)
{
	ASSERT(node);

	/* 直到插入的时候才算是完整的Node,因此在这里检测特殊选项 */
	__licote_node_check(node);

	node->flag |= FLAGS_EXIST;
}

static void
__licote_node_check(LicoteOption* node)
{
	/* 特殊处理-v,-h */
	if (node->flag & FLAGS_VERSION){
		__licote_show_version();
	}
	if (node->flag & FLAGS_HELP){
		licote_option_help(NULL);
	}

	/* 检查是否设置--licote-pwd */
	if (g_LicoteC.hide && __g_licote_pwd && node->lopt){
		if (strcmp(node->lopt, g_LicoteC.optpwd) != 0){/* --licote-pwd */
			return;
		}
		if (node->value &&
			strcmp(node->value, __g_licote_pwd) == 0){
			g_LicoteC.hide |= OPT_PWD_CORRECT;
			licote_option_help(NULL);
		}else{
			licote_option_help("%s -- %s", __MSG(ERROR_PWD), node->lopt);
		}
	}
}

static void
__licote_valid_check(void)
{
	struct list_head* pos = NULL;
	list_for_each(pos, &g_list){
		LicoteOption* node = list_entry(pos, LicoteOption, list0);
		// 跳过非选项
		if (node->flag & FLAGS_NONE){
			continue;
		}

		if (!(node->flag & FLAGS_OPTIONAL) &&
			!(node->flag & FLAGS_EXIST)){
			licote_option_help("%s -- %s", __MSG(MISS_OPTION), __NAME(node));
		}

		if (node->flag & FLAGS_EXIST){
			ListNode* ptr = node->child;
			while(ptr){
				if (ptr->data->flag & FLAGS_EXIST){
					ptr = ptr->next;
					continue;
				}
				licote_option_help("%s -- %s", __MSG(MISS_OPTION), __NAME(ptr->data));
				ptr = ptr->next;
			}
		}
	}
}

static void
__licote_show_help(void)
{
	/* 显示描述信息 */
	__licote_show_depict();

	/* 显示用法信息 */
	__licote_show_usage();

	/* 显示选项信息 */
	__licote_show_hide();
	__licote_show_option();

	/* 显示用例信息 */
	__licote_show_example();
}

static void
__licote_show_version(void)
{
	/* 显示漂亮的LOGO */
	uint32_t i = 0;
	for(i=0; i<ARRAY_SIZE(g_LicoteL); i++){
		LICOTE_SHOW("%s\n", g_LicoteL[i]);
	}
	LICOTE_SHOW("Licote version: "__LICOTE_VERSION" "__LICOTE_RELEASE"(BSD);\n");
	LICOTE_SHOW("Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.\n\n");

	if (__g_licote_author){
		LICOTE_SHOW("%s author : %s\n", g_LicoteC.name, __g_licote_author);
	}
	if (__g_licote_version){
		LICOTE_SHOW("%s version: %s\n", g_LicoteC.name, __g_licote_version);
	}
	if (__g_licote_more){
		LICOTE_SHOW("%s\n", __g_licote_more);
	}
	LICOTE_SHOW("\n");

	exit(0);
}

static void
__licote_show_depict(void)
{
	if (!__g_licote_describe){
		return;
	}

	LICOTE_SHOW("%s: \n", __MSG(DEPICT));
	LICOTE_SHOW("  %s\n\n", __g_licote_describe);
}

static void
__licote_show_usage(void)
{
	static char buf1[LICOTE_USAGE_LEN] = {0};	//填充必须选项
	static char buf2[TMP_BUF_LEN32] = {0};		//填充集合选项
	static char buf3[LICOTE_USAGE_LEN] = {0};	//填充可选选项
	LICOTE_SHOW("%s:\n", __MSG(USAGE));

	if (__g_licote_usage){
		LICOTE_SHOW("  %s\n\n", __g_licote_usage);
		return;
	}

	int len1 = 0, len3 = 0, i = 0;
	int nbr = 0;
	struct list_head* pos = NULL;

	LicoteOption* node  = NULL;
	const char* value = g_LicoteC.style == LINUX ? "=val" : " val";
	list_for_each(pos, &g_list){
		node = list_entry(pos, LicoteOption, list0);
		/* 跳过分隔符 */
		if (node->flag & FLAGS_NONE){
			continue;
		}

		/* 是否展示隐藏选项 */
		if (node->flag & FLAGS_HIDE &&	/* 此Node具有隐藏属性 */
			__g_licote_pwd && 			/* 且设置了密码保护 */
			!(g_LicoteC.hide & OPT_PWD_CORRECT)){/* 但用户没有输入密码或密码错误则跳过 */
			continue;
		}

		/* 是否集合显示 */
		if (IS_GATHER_SHOW()){
			if (node->sopt && strlen(node->sopt) == 2 && node->flag & FLAGS_ONOFF){
				buf2[i++] = node->sopt[1];
				continue;
			}
		}

		if (node->flag & FLAGS_OPTIONAL){/* 可选选项 */
			len3 += snprintf(buf3 + len3, sizeof(buf3) - len3,
							"[%s%s] ",  __NAME(node),
							node->flag & FLAGS_ONOFF ? "" :
										(node->sopt ? " val" : value));
		}else{/* 必需选项 */
			len1 += snprintf(buf1 + len1, sizeof(buf1) - len1,
							"%s%s ", __NAME(node),
							node->flag & FLAGS_ONOFF ? "" :
										(node->sopt ? " val" : value));
		}

		if ((++nbr % LICOTE_USAGE_SIZE) == 0){/* 换行FIXME */
			len3 += snprintf(buf3 + len3, sizeof(buf3) - len3, "\n");
		}
	}

	LICOTE_SHOW("  %s/%s ", g_LicoteC.path, g_LicoteC.name);
	if (IS_GATHER_SHOW()){
		LICOTE_SHOW("[-%s] ", buf2);
	}
	LICOTE_SHOW("%s", buf1);
	LICOTE_SHOW("%s ", buf3);
	LICOTE_SHOW("\n\n");
}

static void
__licote_show_hide(void)
{
	struct list_head* pos = NULL;
	if (!(g_LicoteC.hide & OPT_PWD_EXIST)){
		return;
	}

	if (__g_licote_pwd &&
		!(g_LicoteC.hide & OPT_PWD_CORRECT)){
		return;
	}

	LICOTE_SHOW("%s: \n", __MSG(HIDE));
	list_for_each(pos, &g_list){
		LicoteOption* node = list_entry(pos, LicoteOption, list0);
		if (node->flag & FLAGS_HIDE){ /* 此Node具有隐藏属性 */
			LICOTE_OPTION_FORMAT(node, "\033[31m[H]\033[0m");
		}
	}
}

static void
__licote_show_option(void)
{
	struct list_head* pos = g_list.next;	/* 第一个选项 */
	LicoteOption* node = list_entry(pos, LicoteOption, list0);

	if (!(node->flag & FLAGS_CLASS)){
		LICOTE_SHOW("%s: \n", __MSG(OPTION));
	}
	list_for_each(pos, &g_list){
		node = list_entry(pos, LicoteOption, list0);
		if (node->flag & FLAGS_HIDE){	/* 不显示隐藏的选项 */
			continue;
		}

		if (node->flag & FLAGS_CLASS){	/* 显示分类 */
			LICOTE_SHOW("%s\n", node->info);
		}else if(node->flag & FLAGS_APPEND){
			LICOTE_OPTION_FORMAT2(node);
		}else{
			LICOTE_OPTION_FORMAT(node, "");
		}
	}
}

static void
__licote_show_example(void)
{
	if (!__g_licote_example){
		return;
	}
	static char buf[TMP_BUF_LEN512] = {0};
	strncpy(buf, __g_licote_example, sizeof(buf));

	/* 每一个例子采用";"作为分隔符 */
	LICOTE_SHOW("\n%s: \n", __MSG(EXAMPLE));
	char* ptr = strtok(buf, ";");
	while(ptr){
		LICOTE_SHOW("  %s;\n", ptr);
		ptr = strtok(NULL, ";");
	}
	LICOTE_SHOW("\n");
}
