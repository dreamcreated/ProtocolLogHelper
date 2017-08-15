#ifndef __H_PROTOCOLLOGHELPER__
#define __H_PROTOCOLLOGHELPER__
#include <iostream>
#ifdef __USE_MSGID
#include <uuid/uuid.h>
#endif
#include <memory>
#include <string>
#include <sstream>
#include <tuple>
#include <chrono>


/* example:
   auto logPrinter = ProtocolLogHelper::Reqs(req1, req2, req3).Rsps(rsp1, rsp2, rsp3).Ret(retCode, __FUNCTION__);
   其中 req1-3表示你的request结构体实例,可以是任意类型任意个数, rsp1-3表示你的返回结果结构体实例,可以是任意类型任意个数,retCode是单个任意类型的接口返回值.__FUNCTION__是当前函数名,如果接口名称和函数名称不一样,这里可以自行传入正确的接口名称字符串.
   logPrinter会持有rsp1-3实例以及retCode实例的引用,在logPrinter的生命周期内,需要保证传入的rsp和retcode实例有效,在定义logPrinter之后,可以随意修改rsp1-3和retCode,因为logPrinter持有的是引用,所以后面的修改都将影响最终logPrinter析构时的最终返回结果的打印.
   如果你的接口中使用异步方式调用了其他接口,并且在异步接口返回之前无法返回当前接口,那么则需要使用下面的代码初始化日志打印器:
   auto logPrinter = ProtocolLogHelper::Reqs(req1, req2, req3).Rsps(rsp1, rsp2, rsp3).RetPtr(retCode, __FUNCTION__);
   和上面的例子惟一的不同是, 最后的Ret函数变成了RetPtr函数,返回值是一个智能指针,那么在整个异步调用过程中,都需要持有logPrinter,保证其不被析构,通常做法是把logPrinter作为callback类中的一个成员变量,通过decttype可以提取logPrinter的类型, 如果callback是一个lambda表达式,那么只需要把logPrinter填入caputer list中即可, 例如: [logPrinter](...){...}
*/

#ifndef __PROTOCOL_LOGGER
#define __PROTOCOL_LOGGER std::cout
#endif

#define TNOWMS std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

template<typename T>
void getStringStreamByMultiParam(std::ostringstream& oss, const T &t) {
    oss << t;
}


template<typename T, typename... Args>
void getStringStreamByMultiParam(std::ostringstream& oss, const T &t, const Args&... args) {
    oss << t;
    oss << "|";
    getStringStreamByMultiParam(oss, args...);
}

inline void getStringStreamByMultiParam(std::ostringstream& oss) {

}

template<typename TTuple, std::size_t... Index>
    void getStringStreamByMultiParam(std::ostringstream& oss, TTuple& tup, std::index_sequence<Index...>) {
    getStringStreamByMultiParam(oss, std::get<Index>(tup)...);
}


namespace ProtocolLogHelper {
    template<typename... TREQS>
    class REQ {
    public:
        REQ(const TREQS&... reqs) : m_tupReqs(reqs...) {}
        template<typename... TRSPS>
        class RSP {
        public:
            RSP(const REQ<TREQS...>& req, const TRSPS&... rsps) : m_reqs(req), m_tupRsps(rsps...) {}
            template<typename TRET>
            class ReturnValue {
            public:
                ReturnValue(const TRET& ret,
                            const std::string& funcName,
                            const std::tuple<const TREQS&...> &tupReqs,
                            const std::tuple<const TRSPS&...> &tupRsps) : m_ret(ret), m_rsps(tupRsps), m_funcName(funcName), m_isAsync(false) {
#ifdef __USE_MSGID
                    uuid_t uuidMsgId;
                    char msgId[50];
                    uuid_generate_random(uuidMsgId);
                    uuid_unparse_upper(uuidMsgId, msgId);
                    m_msgId = msgId;
#else
                    static int msgIdSeed = 0;
                    m_msgId = std::to_string(__sync_fetch_and_add(&msgIdSeed, 1));
#endif
                    std::ostringstream oss;
                    getStringStreamByMultiParam(oss, tupReqs, std::make_index_sequence<std::tuple_size<std::tuple<const TREQS&...>>::value>());
                    m_reqs = oss.str();
                    __PROTOCOL_LOGGER << m_funcName << "|" << m_msgId << "|REQ|" << "|" << m_reqs << std::endl;
                    m_startTime = TNOWMS;
                }

                //for async call
                template<typename... TASYNCRSPS>
                void printResponse(const TRET& ret, const TASYNCRSPS&... rsps) {
                    m_isAsync = true;
                    std::ostringstream oss;
                    getStringStreamByMultiParam(oss, rsps...);
                    _printResponse(oss.str(), ret);
                }
                virtual ~ReturnValue() {
                    if (m_isAsync) {
                        return;
                    }
                    std::ostringstream oss;
                    getStringStreamByMultiParam(oss, m_rsps, std::make_index_sequence<std::tuple_size<std::tuple<TRSPS*...>>::value>());
                    _printResponse(oss.str(), m_ret);
                }
            private:
                void _printResponse(const std::string& strRsps, const TRET& ret) {
                    __PROTOCOL_LOGGER << m_funcName << "|" << m_msgId << "|REQ|" << "|" << m_reqs << "|RSP|" << strRsps << "|" << (TNOWMS - m_startTime) <<  "|" << ret << std::endl;
                }
                const TRET& m_ret;
                std::tuple<const TRSPS&...> m_rsps;
                std::string m_msgId;
                int64_t m_startTime;
                std::string m_funcName;
                bool m_isAsync;
                std::string m_reqs;
            };
            template<typename TRET>
            ReturnValue<TRET> Ret(const TRET& ret, const std::string& funcName) {
                return ReturnValue<TRET>(ret, funcName, m_reqs.m_tupReqs, m_tupRsps);
            }
            template<typename TRET>
            std::shared_ptr<ReturnValue<TRET>> RetPtr(const TRET& ret, const std::string& funcName) {
                return std::make_shared<ReturnValue<TRET>>(ret, funcName, m_reqs.m_tupReqs, m_tupRsps);
            }
        private:
            const REQ<TREQS...>& m_reqs;
            std::tuple<const TRSPS&...> m_tupRsps;
        };
        template<typename... TRSPS>
        RSP<TRSPS...> Rsps(const TRSPS&... rsps) {
            return RSP<TRSPS...>(*this, rsps...);
        }
        template<typename... TRSPS> friend class RSP;
    private:
        std::tuple<const TREQS&...> m_tupReqs;
    };

    template<typename... TREQS>
    REQ<TREQS...> Reqs(const TREQS&... reqs) {
        return REQ<TREQS...>(reqs...);
    }
}

#endif
