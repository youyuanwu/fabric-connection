#pragma once

#include <fabrictransport_.h>
#include <string>

// convert std::string data payload on wire to IFabricTransportMessage

namespace fabricconnection {

bool data_to_transport_msg(std::string data, IFabricTransportMessage **message);

bool transport_msg_to_data(IFabricTransportMessage *message, std::string &data);

} // namespace fabricconnection