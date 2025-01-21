#include "RequestStatus.h"

using namespace reqstatus;

bool isCleanStatus(RequestStatus& status) {
    std::string emptyStr{""};
    return status.isEmpty == true && (status.type == 0) && (status.code == 0)
            && (status.msg.compare(emptyStr) == 0);
}
