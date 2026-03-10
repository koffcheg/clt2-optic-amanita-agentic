
#pragma once

#include "commandsQueue.hpp"
#include "slip.hpp"
#include "tcpServer.hpp"

namespace turret {
class ControlServerClient : public TcpServerClient {
public:
  ControlServerClient(int socketfd, CommandsQueue &commandsQueue);
  ~ControlServerClient() override;

protected:
  void processData(const void *data, size_t size) override;

private:
  void onFrame(const slip::Data &decodedData);

  slip::Decoder m_slipDecoder;
  CommandsQueue &m_commandsQueue;
};

class ControlServer : public TcpServer {
public:
  ControlServer(const std::string &host, const std::string &port,
                CommandsQueue &commandsQueue);

protected:
  std::unique_ptr<TcpServerClient> newClient(int socket) const override;

private:
  CommandsQueue &m_commandsQueue;
};

// todo: KeepAlive functionality

} // namespace turret
