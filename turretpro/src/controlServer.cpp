#include "controlServer.hpp"
#include "turretCommands.hpp"
#include <logHelper.hpp>

static auto logger = log4cxx::Logger::getLogger("config");

namespace turret {

ControlServerClient::ControlServerClient(int socketfd,
                                         CommandsQueue &commandsQueue)
    : TcpServerClient(socketfd),
      m_slipDecoder(std::bind(&ControlServerClient::onFrame, this,
                              std::placeholders::_1)),
      m_commandsQueue(commandsQueue) {}

ControlServerClient::~ControlServerClient() {}

void ControlServerClient::onFrame(const slip::Data &decodedData) {
  Deserializer deserializer(decodedData);
  CommandPtr cmd = Command::deserialize(deserializer);

  if (cmd) {
    m_commandsQueue.push_back(std::move(cmd));    
  } else {
    // todo: log Unknown command received
    return;
  }
}

void ControlServerClient::processData(const void *data, size_t size) {
  m_slipDecoder.decode(data, size);
}

ControlServer::ControlServer(const std::string &host, const std::string &port,
                             CommandsQueue &commandsQueue)
    : TcpServer(host, port), m_commandsQueue(commandsQueue) {}

std::unique_ptr<TcpServerClient> ControlServer::newClient(int socket) const {
  return std::make_unique<ControlServerClient>(socket, m_commandsQueue);
}

} // namespace turret