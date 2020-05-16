
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

#ifndef __COMMON_LICOTE_H__
#define __COMMON_LICOTE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

/** 断言 */
#ifdef __DEBUG__
	#define ASSERT(exp)			do{	\
		if (!(exp)){				\
			LOG_DEBUG("assert ("#exp") failed!");\
		}							\
		assert(exp);				\
	}while(0)
#else
	#define ASSERT(exp)
#endif


/** 数组大小 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)			(sizeof(arr)/sizeof(arr[0]))
#endif
/** 释放内存 */
#ifndef FREE_EMPTY
#define FREE_EMPTY(ptr)			do{	\
	if(ptr){						\
		free(ptr); ptr=NULL;		\
	}								\
}while(0)
#endif
/** 关闭文件 */
#ifndef FILE_CLOSE
#define FILE_CLOSE(fp)			do{	\
	if(fp){							\
		fclose(fp); fp=NULL;		\
	}								\
}while(0)
#endif

#ifndef MIN
#define MIN(a,b)				((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b)				((a)>(b)?(a):(b))
#endif

/** 置位清位 */
#define SET_BIT(map,bit)		map[(bit)/8] |= (1 << (bit) % 8)
#define CLR_BIT(map,bit)		map[(bit)/8] &=~(1 << (bit) % 8)
#define TST_BIT(map,bit)		(map[(bit)/8] & (1 << (bit) % 8))

/** 字体颜色 */
#define COLOR_RED				"\033[31m"
#define COLOR_GREEN				"\033[32m"
#define COLOR_OK				"\033[0m"
/** 常用的宏 */
#define TMP_BUF_LEN16			(1 << 4)
#define TMP_BUF_LEN32			(1 << 5)
#define TMP_BUF_LEN64			(1 << 6)
#define TMP_BUF_LEN128			(1 << 7)
#define TMP_BUF_LEN256			(1 << 8)
#define TMP_BUF_LEN512			(1 << 9)
#define TMP_BUF_LEN1K			(1 << 10)
#define TMP_BUF_LEN2K			(1 << 11)
#define TMP_BUF_LEN4K			(1 << 12)

/** 单条消息, 日志长度, 日志文件 */
#define MAX_MESSAGE_LENGTH		TMP_BUF_LEN256

#ifndef __DEBUG__
#define LICOTE_SHOW				printf
#else
#define LICOTE_SHOW				LOG_DEBUG
#endif


/***************************************************************
* 日志调试 API
***************************************************************/
/* 提示信息 */
#define LOG_INFO(fmt...)		\
	__log_debug(__FILE__, __FUNCTION__, __LINE__, fmt)
/* 调试信息 */
#define	LOG_DEBUG(fmt...)		\
	__log_debug(__FILE__, __FUNCTION__, __LINE__, fmt)
/* 错误信息 */
#define	LOG_ERROR(fmt...)		\
	__log_error(__FILE__, __FUNCTION__, __LINE__, fmt)
/* 条件打印 */
#define EXP_DEBUG(exp, fmt...)	do{	\
	if(exp){					\
		LICOTE_SHOW(fmt);		\
		fflush(stdout);			\
	}							\
}while(0)


#ifdef __cplusplus
	extern "C" {
#endif
/***************************************************************
* 内部接口
***************************************************************/
/** 告警信息 */
static inline void
__log_error(const char* file,
			const char* func,
			int line,
			const char* fmt, ...)
{
	static char msg[MAX_MESSAGE_LENGTH] = {0};
	if(!fmt){
		return;
	}

	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, sizeof(msg), fmt, args);
	va_end(args);

	printf(COLOR_RED "[%s:%d]" COLOR_OK "\t\t", file, line);
	printf("%s\n", msg);
}

/** 实时调试日志 */
static inline void
__log_debug(const char* file,
			const char* func,
			int line,
			const char* fmt,  ...)
{
	static char msg[MAX_MESSAGE_LENGTH] = {0};
	if(!fmt){
		return;
	}

	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, sizeof(msg), fmt, args);
	va_end(args);

	printf("[%s:%d]\t\t", file, line);
	printf("%s\n", msg);
}

#ifdef __cplusplus
	}
#endif
#endif /* __COMMON_LICOTE_H__ */


