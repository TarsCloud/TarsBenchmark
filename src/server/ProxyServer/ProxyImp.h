#ifndef _ProxyImp_H_
#define _ProxyImp_H_

#include "Proxy.h"


using namespace bm;
using namespace tars;

/**
 *
 *
 */
class ProxyImp : public Proxy
{
public:
    /**
     *
     */
    virtual ~ProxyImp() {}

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
    int _sendsize;
    int _recvsize;
};
/////////////////////////////////////////////////////
#endif
