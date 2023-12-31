#include "StreamHandler.hpp"
#include "ClientRepository.hpp"
#include "LoggingHandler.hpp"
#include "ParseException.hpp"
#include "disconnect.h"
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>

const std::string StreamHandler::CRLF = "\r\n";

StreamHandler::StreamHandler(handle_t handle) : EventHandler(OFF_EVENT), m_handle(handle) {}

StreamHandler::~StreamHandler() {}

handle_t StreamHandler::getHandle(void) const
{
    return (m_handle);
}

int StreamHandler::handleRead(void)
{
    char tmpBuf[TMP_BUF_SIZE];
    ssize_t nread;
    std::string requestStr;

    std::memset(tmpBuf, 0, sizeof(tmpBuf));
    nread = read(m_handle, tmpBuf, sizeof(tmpBuf) - 1);
    LOG_TRACE("nread: " << nread);
    if (nread < 0)
    {
        LOG_DEBUG("[ " << m_handle << " ] "
                       << "StreamHandler read failed: " << std::strerror(errno));
        return (CODE_OK);
    }
    if (nread == 0)
    {
        LOG_WARN("[ " << m_handle << " ] "
                      << "StreamHandler read eof: " << std::strerror(errno));
        // nread가 0이면 여기로 안오고 무조건 EOF로 플래그에 먼저 잡힘
        return (CODE_OK);
    }
    std::string tcpStreams(tmpBuf);
    LOG_TRACE("[ " << m_handle << " ]"
                   << " sent data: " << tcpStreams);
    m_readBuf += tcpStreams;
    while (hasRequest(requestStr))
    {
        Request *request;

        try
        {
            LOG_DEBUG("Try parse: " << requestStr);
            request = Parser::parseRequest(requestStr, m_handle);
            LOG_DEBUG("Parsing success");
        }
        catch (const Parser::ParseException &e)
        {
            LOG_DEBUG("Failed to parese request: " << requestStr << e.what());
            e.handleError();
            continue;
        }

        Validator *validator = Validator::GetInstance();
        if (request->Accept(validator))
        {
            LOG_DEBUG("Request is valid, execute it");
            Executor *executor = Executor::GetInstance();
            request->Accept(executor);
            LOG_DEBUG("Request executed");
        }
        delete request;
        if (m_disconnectFlag)
            return (CLIENT_DISCONNECT);
    }
    return (CODE_OK);
}

int StreamHandler::handleWrite(void)
{
    SharedPtr< Client > client = ClientRepository::GetInstance()->FindBySocket(m_handle);
    ssize_t nwrite;

    if (!client)
    {
        LOG_WARN("[ " << m_handle << " ] "
                      << "StreamHandler write event but no client found " << std::strerror(errno));
        return CODE_OK;
    }
    if (m_writeBuf.empty())
    {
        LOG_WARN("[ " << m_handle << " ] "
                      << "StreamHandler write event but buf is empty ");
        return CODE_OK;
    }
    nwrite = write(m_handle, m_writeBuf.c_str(), m_writeBuf.size());
    if (nwrite < 0)
    {
        LOG_INFO("[ " << m_handle << " ] "
                      << "StreamHandler write failed: write again or disconnectClient: " << std::strerror(errno));
        return (CODE_OK);
    }
    if ((size_t)nwrite < m_writeBuf.size())
    {
        LOG_INFO("[ " << m_handle << " ] "
                      << "StreamHandler Partial write, nwrite: " << nwrite);
        m_writeBuf = m_writeBuf.substr(nwrite);
        return (CODE_OK);
    }
    if ((size_t)nwrite == m_writeBuf.size())
    {
        LOG_DEBUG("[ " << m_handle << " ] "
                       << "StreamHandler write success fully: " << m_writeBuf);
        m_writeBuf.clear();
        return (Reactor::GetInstance()->unregisterEvent(this, WRITE_EVENT));
    }
    LOG_ERROR("[ " << m_handle << " ] "
                   << "StreamHandler write: should not reach here " << std::strerror(errno));
    return (CODE_OK);
}

int StreamHandler::handleDisconnect(void)
{
    disconnectClient(m_handle);
    return (CODE_OK);
}

bool StreamHandler::hasRequest(std::string &requestStr)
{
    size_t crlf_pos = m_readBuf.find(CRLF);
    if (crlf_pos == std::string::npos)
        return (false);
    LOG_TRACE("crlf_pos: " << crlf_pos)
    while (crlf_pos > MAX_MSG_SIZE - CRLF_LEN)
    {
        m_readBuf = m_readBuf.substr(MAX_MSG_SIZE - CRLF_LEN);
        crlf_pos = m_readBuf.find(CRLF);
        LOG_TRACE("crlf_pos: " << crlf_pos)
    }
    requestStr = m_readBuf.substr(0, crlf_pos + CRLF_LEN);
    LOG_DEBUG("Extracted request: " << requestStr);
    m_readBuf = m_readBuf.substr(crlf_pos + CRLF_LEN);
    LOG_DEBUG("Reamin buffer len: " << m_readBuf.size() << ", " << m_readBuf);
    return (true);
}

void StreamHandler::addResponseToBuf(const std::string &responseStr)
{
    LOG_DEBUG("[ " << m_handle << " ]: "
                   << "Add response to handler's wrbuf: " << responseStr);
    Reactor::GetInstance()->registerEvent(this, WRITE_EVENT);
    m_writeBuf += responseStr;
}
