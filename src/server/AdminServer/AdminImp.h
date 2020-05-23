#ifndef _ADMIN_IMP_H_
#define _ADMIN_IMP_H_

#include "Admin.h"
#include "proto_factory.h"

using namespace bm;
using namespace tars;

/**
 *
 *
 */
class AdminImp : public Admin
{
public:
    /**
     *
     */
    virtual ~AdminImp() {}

    /**
     *
     */
    virtual void initialize();

    /**
     *
     */
    virtual void destroy();

    /**
     * @brief  启动压测接口
     *
     */
    int startup(const BenchmarkUnit& req, TarsCurrentPtr curr);

    /**
     * @brief  查询压测接口
     *
     */
    int query(const BenchmarkUnit& req, ResultStat& stat, TarsCurrentPtr curr);

    /**
     * @brief  关停压测接口
     *
     */
    int shutdown(const BenchmarkUnit& req, ResultStat& stat, TarsCurrentPtr curr);

    /**
     * @brief  测试接口
     *
     */
    int test(const BenchmarkUnit& req, string& rsp, string& errmsg, TarsCurrentPtr curr);

private:
    /**
     * @brief  基本参数检查
     * 
     * @param req       请求结构体
     * @param conf      任务配置
     * 
     * @return TC_Socket指针
     */
    int check(const BenchmarkUnit &req, TaskConf &conf, int timieout = 0);

private:
    ProtoFactory  _factory;
};
/////////////////////////////////////////////////////
#endif
