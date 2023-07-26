#pragma once

#include "Request.hpp"

class QuitRequest : public Request {

public:
    QuitRequest(handle_t socket);

    void AcceptValidator(Validator *validator);
    void AcceptCommand(Command *command);

    void SetReason(const std::string &reason);

    const std::string &GetReason() const;

private:
    std::string mReason;

};