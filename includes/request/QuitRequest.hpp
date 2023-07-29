#pragma once

#include "Request.hpp"

class QuitRequest : public Request
{

  public:
    QuitRequest(handle_t socket, const std::string &reason);

    bool Accept(visitor_pattern::Visitor *visitor);

    void SetReason(const std::string &reason);

    const std::string &GetReason() const;

  private:
    std::string mReason;
};