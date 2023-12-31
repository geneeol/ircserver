#include "PingRequest.hpp"

PingRequest::PingRequest(handle_t socket, const std::string &token) : Request(socket), mToken(token)
{
    LOG_TRACE("PingRequest constructor called | " << *this);
}

bool PingRequest::Accept(visitor_pattern::Visitor *visitor)
{
    return visitor->Visit(this);
}

void PingRequest::SetToken(const std::string &token)
{
    mToken = token;
}

const std::string &PingRequest::GetToken() const
{
    return mToken;
}

std::stringstream &operator<<(std::stringstream &ss, const PingRequest &pingRequest)
{
    ss << "PingRequest = { Token: " << pingRequest.mToken << " }";

    return ss;
}
