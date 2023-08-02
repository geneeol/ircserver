#include "StreamHandler.hpp"
#include "ClientRepository.hpp"
#include "LoggingHandler.hpp"
#include "def.h"
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
    size_t crlf_pos;
    std::string requestStr;

    std::memset(tmpBuf, 0, sizeof(tmpBuf));
    nread = read(m_handle, tmpBuf, sizeof(tmpBuf) - 1);
    LOG_TRACE("nread: " << nread);
    if (nread < 0)
    {
        LOG_DEBUG("StreamHandler read failed: " << std::strerror(errno));
        return (OK);
    }
    if (nread == 0)
    {
        LOG_DEBUG("StreamHandler read eof: " << std::strerror(errno));
        // TODO: disconnect(구현 자유롭게 하면 될듯 되도록 disconnect 하도록)
        return (OK);
    }
    std::string tcpStreams(tmpBuf);
    LOG_TRACE("[ " << m_handle << " ]"
                   << " sent data: " << tcpStreams);
    m_readBuf += tcpStreams;
    while (hasRequest(requestStr) != false)
    {
        Request *request;

        try
        {
            LOG_DEBUG("Try parse: " << requestStr);
            request = Parser::parseRequest(requestStr, m_handle);
            LOG_DEBUG("Parsing success");
        }
        catch (std::exception &e)
        {
            // TODO: 에러 핸들러 호출로 처리
            LOG_DEBUG("Failed to parese request: " << requestStr << e.what());
            continue;
        }

        Validator *validator = Validator::GetInstance();
        if (request->Accept(validator))
        {
            Executor *executor = Executor::GetInstance();
            request->Accept(executor);
        }
    }
    return (OK);
}

int StreamHandler::handleWrite(void)
{
    Client *client = ClientRepository::GetInstance()->FindBySocket(m_handle);
    ssize_t nwrite;

    if (!client)
    {
        LOG_ERROR("StreamHandler write event but no client found" << std::strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (m_writeBuf.empty())
    {
        LOG_ERROR("StreamHandler write event but buf is empty");
        exit(EXIT_FAILURE);
    }
    nwrite = write(m_handle, m_writeBuf.c_str(), m_writeBuf.size());
    if (nwrite < 0)
    {
        LOG_WARN("StreamHandler write failed: write again or disconnect: " << std::strerror(errno));
        return (OK);
    }
    if (nwrite < m_writeBuf.size())
    {
        LOG_INFO("Partial write");
        m_writeBuf = m_writeBuf.substr(nwrite);
        return (OK);
    }
    if (nwrite == m_writeBuf.size())
    {
        LOG_DEBUG("StreamHandler write success fully: " << m_writeBuf);
        m_writeBuf.clear();
        return (g_reactor().unregisterEvent(this, WRITE_EVENT));
    }
    LOG_ERROR("StreamHandler write: should not reach here " << std::strerror(errno));
    return (OK);
}

int StreamHandler::handleError(void)
{
    return (0);
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
    LOG_DEBUG("Add response to handler's wrbuf: " << responseStr);
    g_reactor().registerEvent(this, WRITE_EVENT);
    m_writeBuf += responseStr;
}
