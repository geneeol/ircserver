#pragma once

#include <vector>
#include "Request.hpp"

class JoinRequest : public Request {

public:
    JoinRequest(handle_t socket);

    void SetChannelName(const std::string &channelName);
    void SetKey(const std::string &key);

    const std::vector<std::string> &GetChannelNames() const;
    const std::vector<std::string> &GetKeys() const;

private:
    std::vector<std::string> mChannelNames;
    std::vector<std::string> mKeys;

};