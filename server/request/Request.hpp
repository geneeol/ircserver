#pragma once

#include <iostream>
#include "EventHandler.hpp"
#include "Client.hpp"
#include "ClientRepository.hpp"

typedef enum eRequestType {
    PASS,
    NICK,
    USER,
    PRIVMSG,
    JOIN,
    KICK,
    INVITE,
    QUIT,
    TOPIC,
    MODE,
    PING,
} eRequestType;

///////////////////////////////////////////////////////////////////////////////////

//Exception 정의도 각각 request에 할지 생각
class Request {

public:
    Request(handle_t socket);

    Client *GetClient() const;
    eRequestType GetType() const;

protected:
    virtual ~Request() {}

private:
    Client *mClient;
    eRequestType mRequestType;

};