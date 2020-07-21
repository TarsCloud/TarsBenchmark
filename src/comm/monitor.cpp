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
    int Monitor::initialize(int shm_key, int shm_size)
    {
        bool bCreate = false;
        int shm_id = shmget(shm_key, shm_size, IPC_CREAT | IPC_EXCL | 0666);
        if (shm_id >= 0)
        {
            bCreate = true;
        }
        else if ((shm_id = shmget(shm_key, shm_size, 0666)) < 0) //有可能是已经存在同样的key_shm,则试图连接
        {
            return BM_SHM_ERR_GET;
        }

        _set_speed = DEF_SPEED_LEVEL;
        _data_base = (char *)shmat(shm_id, NULL, 0);
        if (_data_base == (char *)(-1))
        {
            return BM_SHM_ERR_ATTACH;
        }

        _cache_stat.clear();
        StatCache *ch = (StatCache *)_data_base;
        uint32_t max_cnt = (shm_size - 4 - sizeof(IntfStat)) / sizeof(IntfStat);
        if (bCreate || max_cnt != ch->max_cnt)
        {
            // 首次创建 or 数据紊乱，清空内存
            memset(_data_base, 0, shm_size);
            ch->max_cnt = max_cnt;
        }

        ch->head = 0;
        ch->tail = 0;
        return 0;
    }

    void Monitor::clear()
    {
        vector<IntfStat> vis;
        fetch(vis);
    }

    void Monitor::reportSend(int64_t snd_time, int snd_bytes)
    {
        _cache_stat.totalCount++;
        _cache_stat.totalSendBytes += snd_bytes;
        if (_cache_stat.staIndex == 0)
        {
            _cache_stat.staIndex = snd_time;
            _cache_stat.execKey = TC_Port::getpid();
        }
    }

    void Monitor::report(int ret_code)
    {
        _count_ret[ret_code] += 1;
    }

    void Monitor::reportRecv(int64_t rcv_time, int rcv_bytes)
    {
        _cache_stat.totalRecvBytes += rcv_bytes;
    }

    void Monitor::report(int ret_code, int cost_time)
    {
        if (BM_SUCC == ret_code)
        {
            _cache_stat.succCount++;
        }
        else
        {
            _cache_stat.failCount++;
        }

        report(ret_code);
        _queue_cost.push_back(cost_time);
        _cache_stat.totalTime += double(cost_time);
        _cache_stat.maxTime = std::max<double>(double(cost_time), _cache_stat.maxTime);
        _cache_stat.minTime = std::min<double>(double(cost_time), _cache_stat.minTime);
        static int costStep[MAX_STEP_COST] = {10, 30, 50, 100, 500, 3000, 5000, 0, 0, 0};
        for (size_t i = 0; i < MAX_STEP_COST; i++)
        {
            if (costStep[i] > cost_time || costStep[i] == 0)
            {
                _cache_stat.costTimes[i] += 1;
                break;
            }
        }
    }

    void Monitor::adapteSpeed(int64_t cur_time, int64_t time_out)
    {
        int64_t decimal = max(_cache_stat.failCount + _cache_stat.succCount, size_t(1));
        int64_t fail_rate = _cache_stat.failCount * 1000 / decimal;
        static int64_t last_rate = 1000;
        static int count = 0;
        if (fail_rate > RATE_FAIL_LEVEL)
        {
            _set_speed = _set_speed * (_workmode == MODEL_QUICK ? 500 : 995) / 1000 + 10; // 最低到10
            if (last_rate < RATE_FAIL_LEVEL && _workmode == MODEL_QUICK)
            {
                _workmode = MODEL_SLOW;
            }

            // 上一个错误率高于20%并持续10个周期，转换状态
            if (last_rate > RATE_ERROR_LEVEL && (++count * 100 > time_out))
            {
                count = 0;
                _workmode = MODEL_QUICK;
            }
            last_rate = fail_rate;
        }
        else if (++count * 1000 > time_out)
        {
            count = 0;
            int64_t percent = 1000;
            if (last_rate < RATE_FAIL_LEVEL && _workmode == MODEL_SLOW)
            {
                percent = 1001; // 慢模式下以0.1%去逼进
            }
            else if (_workmode == MODEL_QUICK)
            {
                percent = 2000;
                int64_t curr_cost = _cache_stat.totalTime * 1000 / decimal;
                static int64_t init_cost = _cache_stat.totalTime * 100 / decimal;
                static int64_t step_cost[][2] = {{100, 80}, {10, 50}, {5, 20}, {2, 15}};
                for (size_t i = 0; i < sizeof(step_cost) / sizeof(step_cost[0]); i++)
                {
                    if (step_cost[i][0] * init_cost < 100 * time_out)
                    {
                        if (step_cost[i][1] * init_cost < curr_cost)
                        {
                            percent = 1100;
                            _workmode = MODEL_SLOW;
                        }
                        break;
                    }
                }
            }
            last_rate = fail_rate;
            _set_speed = _set_speed * percent / 1000;
        }
    }

    bool Monitor::syncStat(int64_t cur_time, int64_t time_out)
    {
        bool is_overload = false;
        if (abs(cur_time - _cache_stat.staIndex) >= 1000)
        {
            // 第一步，计算失败率
            if (_workmode != MODEL_FIXED && time_out != 0)
            {
                adapteSpeed(cur_time, time_out);
            }

            // 第二步，同步数据给主进程
            string ret_msg = map2str(_count_ret);
            sort(_queue_cost.begin(), _queue_cost.end());
            _cache_stat.p90Time = calcPercent(900);
            _cache_stat.p99Time = calcPercent(990);
            _cache_stat.p999Time = calcPercent(999);
            is_overload = _cache_stat.p90Time >= time_out && time_out != 0;

            RMB(); // sync read
            StatCache *ch = (StatCache *)_data_base;
            uint32_t old_head = ch->head;
            uint32_t old_tail = ch->tail;
            uint32_t newTail = (ch->tail + 1) % ch->max_cnt;
            if (old_head == newTail)
            {
                return is_overload;
            }

            if (!CAS(&ch->tail, old_tail, newTail))
            {
                return is_overload;
            }

            memcpy(&ch->item_list[old_tail], &_cache_stat, sizeof(IntfStat));
            if (ret_msg.length() < sizeof(ch->item_list[old_tail].retCount))
            {
                memcpy(ch->item_list[old_tail].retCount, ret_msg.c_str(), ret_msg.length());
                ch->item_list[old_tail].retCount[ret_msg.length()] = 0;
            }

            WMB(); // sync write with other processors
            _count_ret.clear();
            _queue_cost.clear();
            _cache_stat.clear();
            _cache_stat.staIndex = cur_time;
            _cache_stat.execKey = TC_Port::getpid();
        }
        return is_overload;
    }

    bool Monitor::fetch(vector<IntfStat> &item_list)
    {
        RMB();
        StatCache *ch = (StatCache *)_data_base;
        uint32_t old_head = ch->head;
        uint32_t old_tail = ch->tail;
        while (old_head != old_tail)
        {
            uint32_t new_head = (ch->head + 1) % ch->max_cnt;
            if (CAS(&ch->head, old_head, new_head))
            {
                WMB();
                IntfStat stat;
                memcpy(&stat, &ch->item_list[old_head], sizeof(stat));
                if (stat.staFlag == STA_FLAG && stat.endFlag == END_FLAG)
                {
                    item_list.push_back(stat);
                }
            }

            RMB();
            old_head = ch->head;
            old_tail = ch->tail;
        }
        return item_list.size() > 0;
    }

    double Monitor::calcPercent(size_t percent)
    {
        if (_queue_cost.size() == 0 || percent > 1000)
        {
            return 0.00;
        }
        return (double)_queue_cost[percent * _queue_cost.size() / 1000];
    }

    size_t Monitor::calcInterval(size_t intval, size_t cons)
    {
        // 用户已设置，以用户设置为准，否则根据失败率自动调整
        if (intval != 0)
        {
            return intval;
        }

        // 改变工作模式
        _workmode = _workmode == 0 ? MODEL_QUICK : _workmode;
        return (1000000 * cons) / _set_speed;
    }
}; // namespace bm
