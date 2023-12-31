#pragma once

#include "EventHandler.hpp"
#include "LoggingHandler.hpp"
#include "SharedPtr.hpp"
#include <bitset>
#include <iostream>
#include <queue>
#include <sstream>

class Channel;

class Client
{

  public:
    Client(handle_t socket);
    ~Client();

    std::string GetClientInfo() const;
    handle_t GetSocket() const;

    void SetChannel(SharedPtr< Channel > channel);
    void ResetChannel();
    SharedPtr< Channel > GetChannel() const;

    void SetNickName(const std::string &nickName);
    void SetUserName(const std::string &userName);
    void SetHostName(const std::string &hostName);
    void SetServerName(const std::string &serverName);
    void SetRealName(const std::string &realName);

    void SetRegistered();
    void SetPasswordEntered();
    void SetNickNameEntered();
    void SetUserInfoEntered();

    const std::string &GetNickName() const;
    const std::string &GetUserName() const;
    const std::string &GetHostName() const;
    const std::string &GetServerName() const;
    const std::string &GetRealName() const;

    bool HasRegistered() const;
    bool HasEnteredPassword() const;
    bool HasEnteredNickName() const;
    bool HasEnteredUserInfo() const;

    void AddResponseToBuf(const std::string &response);

    friend std::stringstream &operator<<(std::stringstream &ss, const Client &client);

  private:
    static const int REGISTERED_FLAG = 1 << 0;
    static const int PASSWORD_ENTERED_FLAG = 1 << 1;
    static const int NICKNAME_ENTERED_FLAG = 1 << 2;
    static const int USER_INFO_ENTERED_FLAG = 1 << 3;

    static const std::string CRLF;

    handle_t mSocket;

    SharedPtr< Channel > mChannel;

    std::string mNickName;
    std::string mUserName;
    std::string mHostName;
    std::string mServerName;
    std::string mRealName;

    int mRegistrationFlags;
};
