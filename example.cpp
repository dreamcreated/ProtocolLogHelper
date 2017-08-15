#include "ProtocolLogHelper.hpp"

struct fakeReqRsp {
    fakeReqRsp() {
        std::cout << "fakeReq construct" << std::endl;
    }

    ~fakeReqRsp() {
        std::cout << "fakeReq destruct" << std::endl;
    }


};

std::ostream& operator<<(std::ostream& os, const fakeReqRsp& obj) {
    os << "this is a fake class";
    return os;
}


int main() {
    int newRetCode = 0;
    long fakeReq1 = 1, fakeRsp1 = 2;
    std::string fakeReq2 = "3", fakeRsp2 = "4";
    char fakeReq3 = 5, fakeRsp3 = 6;

    fakeReqRsp fakeReq4, fakeRsp4;

    auto logPrinter = ProtocolLogHelper::Reqs(fakeReq1, fakeReq2, fakeReq3, fakeReq4).Rsps(fakeRsp1, fakeRsp2, fakeRsp3, fakeRsp4).Ret(newRetCode, __FUNCTION__);
    auto logPrinter1 = ProtocolLogHelper::Reqs(fakeReq1, fakeReq2, fakeReq3, fakeReq4).Rsps().Ret(newRetCode, __FUNCTION__);
    auto logPrinter2 = ProtocolLogHelper::Reqs().Rsps().Ret(newRetCode, __FUNCTION__);
    auto logPrinter3 = ProtocolLogHelper::Reqs().Rsps(fakeRsp1, fakeRsp2, fakeRsp3, fakeRsp4).Ret(newRetCode, __FUNCTION__);

    fakeRsp1 *= 10;
    fakeRsp2.append("0");
    fakeRsp3 *= 10;
    newRetCode = 10010;

    logPrinter2.printResponse(newRetCode, fakeRsp1, fakeRsp2, fakeRsp3, fakeRsp4);
}
