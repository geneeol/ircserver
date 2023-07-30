#include "InviteRequest.hpp"

InviteRequest::InviteRequest(handle_t socket, const std::string &nickName, const std::string &channelName)
    : Request(socket), mNickName(nickName), mChannelName(channelName)
{
    LOG_TRACE("InviteRequest constructor: " << *this);
}

bool InviteRequest::Accept(visitor_pattern::Visitor *visitor)
{
    return visitor->Visit(this);
}

void InviteRequest::SetNickName(const std::string &nickName)
{
    mNickName = nickName;
}

void InviteRequest::SetChannelName(const std::string &channelName)
{
    mChannelName = channelName;
}

const std::string &InviteRequest::GetNickName() const
{
    return mNickName;
}

const std::string &InviteRequest::GetChannelName() const
{
    return mChannelName;
}

std::ostream &operator<<(std::ostream &os, const InviteRequest &inviteRequest)
{
    os << "InviteRequest - NickName: " << inviteRequest.mNickName << ", ChannelName: " << inviteRequest.mChannelName;

    return os;
}
