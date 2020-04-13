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
#ifndef _MONITOR_H_
#define _MONITOR_H_
#include "commdefs.h"

namespace bm
{
#pragma pack(1)
    typedef struct tagIntfStat
    {
        int8_t      staFlag;
        int64_t     staIndex;
        double      minTime;
        double      maxTime;
        double      p90Time;
        double      p99Time;
        double      p999Time;
        double      totalTime;
        int32_t     costTimes[10];
        int32_t     succCount;
        int32_t     failCount;
        int32_t     totalCount;
        size_t      totalSendBytes;
        size_t      totalRecvBytes;
        int8_t      retCount[1024];
        int8_t      endFlag;

        tagIntfStat()
        {
            clear();
        }

        void clear()
        {
            memset(this, 0, sizeof(struct tagIntfStat));
            staFlag = STA_FLAG;
            endFlag = END_FLAG;
            minTime = 1000.00;
        }

        inline tagIntfStat& operator+=(const tagIntfStat& src)
        {
            for (size_t i = 0; i < 10; i++)
            {
                costTimes[i] += src.costTimes[i];
            }

            int32_t totalCnt = totalCount + src.totalCount;
            if ((totalCount + src.totalCount) > 0)
            {
                p90Time  = (p90Time*totalCount + src.p90Time*src.totalCount) / totalCnt;
                p99Time  = (p99Time*totalCount + src.p99Time*src.totalCount) / totalCnt;
                p999Time = (p999Time*totalCount + src.p999Time*src.totalCount) / totalCnt;
            }

            totalTime  += src.totalTime;
            totalCount += src.totalCount;
            failCount  += src.failCount;
            succCount  += src.succCount;
            totalSendBytes += src.totalSendBytes;
            totalRecvBytes += src.totalRecvBytes;
            maxTime = std::max(src.maxTime, maxTime);
            minTime = std::min(src.minTime, minTime);
            return *this;
        }

    } IntfStat;

    typedef struct tagStatCache
    {
        volatile uint32_t   itemSize;
        volatile uint32_t   headIdx;
        volatile uint32_t   tailIdx;
        IntfStat     itemList[0];
    } StatCache;
#pragma pack()

    class Monitor
    {
    public:
        Monitor() {};
        virtual ~Monitor() {};

        /**
         * @brief  实例化
         *
         */
        static Monitor* getInstance()
        {
            static Monitor* m = NULL;
            if (m == NULL)
            {
                m = new Monitor;
            }
            return m;
        }

        /**
         * @brief  初始化
         *
         */
        int initialize(int iShmKey = 0x19453959, int iShmSize = 1024 * 1024);

        /**
         * @brief  析构
         *
         */
        void destroy();


        /**
         * @brief  接口上报
         *
         * @param retCode   返回码
         * @param costTime  消耗的时间
         * @param sndTime   上报的时间点
         */
        void report(int retCode);
        void report(int retCode, int costTime);
        void reportSend(int64_t sndTime, int sndBytes);
        void reportRecv(int64_t rcvTime, int rcvBytes);

        /**
         * @brief  状态同步
         *
         * @param @param rTime     上报的时间点
         */
        void syncStat(int64_t rTime);

        /**
         * @brief  获取上报的数据
         *
         * @param itemList  返回的数据
         *
         * @return bool
         */
         bool fetch(vector<IntfStat>& itemList);

        /**
         * @brief  计算百分比
         *
         * @param @param percent     上报的时间点
         */
        double calcPercent(size_t percent);

    private:
        char*           pData;
        IntfStat        tmpStat;
        map<int, int>   retCount;
        vector<int>     queueCost;
    };
};
#endif
