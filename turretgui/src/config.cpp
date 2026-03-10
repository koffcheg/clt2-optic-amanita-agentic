#include "config.hpp"
#include "common/model/m_json_cfg_reader.h"
#include <fstream>
#include <jansson.h>
#include <log4cxx/logger.h>
#include <stdexcept>

namespace turret {
Config::Config(const std::string &filename)
    : m_configStr{load(readConfigFile(filename))} {}

std::string Config::readConfigFile(const std::string &filename) {
  std::ifstream ifs(filename);
  if (!ifs) {
    // todo: log
    throw std::runtime_error("Can not open configuration file: " + filename);
  }

  std::string file_content((std::istreambuf_iterator<char>(ifs)),
                           (std::istreambuf_iterator<char>()));
  return file_content;
}

const std::string &Config::load(const std::string &config) {
  // Parse the JSON string into a JSON object
  json_error_t error;
  json_t *root = json_loads(config.c_str(), 0, &error);

  if (root) {
    json_t *infoServer = json_object_get(root, "infoServer");

    if (json_is_object(infoServer)) {
      json_t *host = json_object_get(infoServer, "host");
      if (json_is_string(host)) {
        Config::infoHost = json_string_value(host);
      } else {
        throw std::runtime_error("infoServer host is misconfigured");
      }
      json_t *port = json_object_get(infoServer, "port");
      if (json_is_string(port)) {
        Config::infoPort = json_string_value(port);
      } else {
        throw std::runtime_error("infoServer port is misconfigured");
      }
    } else {
      throw std::runtime_error("infoServer is misconfigured");
    }

    json_t *controlServer = json_object_get(root, "controlServer");
    if (json_is_object(controlServer)) {
      json_t *host = json_object_get(controlServer, "host");
      if (json_is_string(host)) {
        Config::controlHost = json_string_value(host);
      } else {
        throw std::runtime_error("controlServer host is misconfigured");
      }
      json_t *port = json_object_get(controlServer, "port");
      if (json_is_string(port)) {
        Config::controlPort = json_string_value(port);
      } else {
        throw std::runtime_error("controlServer port is misconfigured");
      }
    } else {
      throw std::runtime_error("controlServer is misconfigured");
    }


    json_t *jturret = json_object_get(root, "turret");
    if (json_is_object(jturret)) {
      json_t *jreconnectIntervalms = json_object_get(jturret, "reconnectIntervalms");
      
      if (jreconnectIntervalms) {
        if (json_is_integer(jreconnectIntervalms)) {
          turret.reconnectIntervalms = json_integer_value(jreconnectIntervalms);
        } else {
          throw std::runtime_error("turret.reconnectIntervalms has invalid value");
        }
        //turret.reconnectWaitms = 
      }

    }

	  {
		  jansson_cfg_obj_reader dp_data_rc_cfg_reader(root, "dp_data_server");
		  dp_data_port = dp_data_rc_cfg_reader.read_int_param("port");
	  }
    json_decref(root);
  } else {
    throw std::runtime_error("Invalid configuration file");
  }

  return config;
}

} // namespace turret
