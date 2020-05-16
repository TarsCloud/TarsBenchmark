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
#include "monitor.h"
namespace bm
{
    int Monitor::initialize(int iShmKey, int iShmSize)
    {
        bool bCreate = false;
        int iShmID = shmget(iShmKey, iShmSize, IPC_CREAT | IPC_EXCL | 0666);
        if (iShmID >= 0)
        {
            bCreate = true;
        }
        else if ((iShmID = shmget(iShmKey, iShmSize, 0666)) < 0)  //有可能是已经存在同样的key_shm,则试图连接
        {
            return BM_SHM_ERR_GET;
        }

        pData = (char *)shmat(iShmID, NULL, 0);
        if (pData == (char *)(-1))
        {
            return BM_SHM_ERR_ATTACH;
        }

        tmpStat.clear();
        StatCache* ch = (StatCache*)pData;
        uint32_t queueSize = (iShmSize - 4 - sizeof(IntfStat)) / sizeof(IntfStat);
        if (bCreate || queueSize != ch->itemSize)
        {
            // 首次创建 or 数据紊乱，清空内存
            memset(pData, 0, iShmSize);
            ch->itemSize = queueSize;
        }

        ch->headIdx  = 0;
        ch->tailIdx  = 0;
        return 0;
    }

    void Monitor::clear()
    {
        vector<IntfStat> vis;
        fetch(vis);
    }

    void Monitor::report(int retCode)
    {
        retCount[retCode] += 1;
    }

    void Monitor::reportSend(int64_t sndTime, int sndBytes)
    {
        tmpStat.totalCount++;
        tmpStat.totalSendBytes += sndBytes;
        if (tmpStat.staIndex == 0)
        {
            tmpStat.staIndex = sndTime;
        }
    }

    void Monitor::reportRecv(int64_t rcvTime, int rcvBytes)
    {
        tmpStat.totalRecvBytes += rcvBytes;
    }

    void Monitor::report(int retCode, int costTime)
    {
        if (BM_SUCC == retCode || BM_UNEXPECT == retCode)
        {
            tmpStat.succCount++;
        }
        else
        {
            tmpStat.failCount++;
        }

        report(retCode);
        queueCost.push_back(costTime);
        double dCostTime  = (double)costTime;
        tmpStat.totalTime += dCostTime;
        tmpStat.maxTime = std::max<double>(dCostTime, tmpStat.maxTime);
        tmpStat.minTime = std::min<double>(dCostTime, tmpStat.minTime);
        static int costStep[MAX_STEP_COST] = {10, 30, 50, 100, 500, 3000, 5000, 0, 0, 0};
        for (size_t i = 0; i < MAX_STEP_COST; i++)
        {
            if (costStep[i] > costTime || costStep[i] == 0)
            {
                tmpStat.costTimes[i] += 1;
                break;
            }
        }
    }

    void Monitor::syncStat(int64_t rTime)
    {
        if (abs(rTime - tmpStat.staIndex) >= 1000)
        {
            string sRetMsg = map2str(retCount);
            sort(queueCost.begin(), queueCost.end());
            tmpStat.p90Time = calcPercent(900);
            tmpStat.p99Time = calcPercent(990);
            tmpStat.p999Time = calcPercent(999);

            RMB(); // sync read
            StatCache* ch = (StatCache*)pData;
            uint32_t oldHead = ch->headIdx;
            uint32_t oldTail = ch->tailIdx;
            uint32_t newTail = (ch->tailIdx + 1) % ch->itemSize;
            if (oldHead == newTail)
            {
                return;
            }

		    if (!CAS(&ch->tailIdx, oldTail, newTail))
            {
                return;
            }

            memcpy(&ch->itemList[oldTail], &tmpStat, sizeof(IntfStat));
            if (sRetMsg.length() < sizeof(ch->itemList[oldTail].retCount))
            {
                memcpy(ch->itemList[oldTail].retCount, sRetMsg.c_str(), sRetMsg.length());
                ch->itemList[oldTail].retCount[sRetMsg.length()] = 0;
            }

            WMB(); // sync write with other processors
            tmpStat.clear();
            retCount.clear();
            queueCost.clear();
            tmpStat.staIndex = rTime;
        }
    }

    bool Monitor::fetch(vector<IntfStat>& itemList)
    {
        RMB();
        StatCache* ch = (StatCache*)pData;
        uint32_t oldHead = ch->headIdx;
        uint32_t oldTail = ch->tailIdx;
        while (oldHead != oldTail)
        {
            uint32_t newHead = (ch->headIdx + 1) % ch->itemSize;
            if (CAS(&ch->headIdx, oldHead, newHead))
		    {
			    WMB();
                IntfStat tmpStat;
                memcpy(&tmpStat, &ch->itemList[oldHead], sizeof(tmpStat));
                if (tmpStat.staFlag == STA_FLAG && tmpStat.endFlag == END_FLAG)
                {
                    itemList.push_back(tmpStat);
                }
		    }

            RMB();
            oldHead = ch->headIdx;
            oldTail = ch->tailIdx;
        }
        return itemList.size() > 0;
    }



    double Monitor::calcPercent(size_t percent)
    {
        if (queueCost.size() == 0 || percent > 1000)
        {
            return 0.00;
        }
        return (double)queueCost[percent * queueCost.size() / 1000];
    }
};
