/*
 * Copyright (c) 2018 Zilliqa
 * This source code is being disclosed to you solely for the purpose of your
 * participation in testing Zilliqa. You may view, compile and run the code for
 * that purpose and pursuant to the protocols and algorithms that are programmed
 * into, and intended by, the code. You may not do anything else with the code
 * without express permission from Zilliqa Research Pte. Ltd., including
 * modifying or publishing the code (or any part of it), and developing or
 * forming another public or private blockchain network. This source code is
 * provided 'as is' and no warranties are given as to title or non-infringement,
 * merchantability or fitness for purpose and, to the extent permitted by law,
 * all liability for your use of the code is disclaimed. Some programs in this
 * code are governed by the GNU General Public License v3.0 (available at
 * https://www.gnu.org/licenses/gpl-3.0.en.html) ('GPLv3'). The programs that
 * are governed by GPLv3.0 are those programs that are located in the folders
 * src/depends and tests/depends and which include a reference to GPLv3 in their
 * program files.
 */

#include <execinfo.h>  // for backtrace
#include <signal.h>

#include <arpa/inet.h>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include "boost/program_options.hpp"

#include "depends/NAT/nat.h"
#include "libNetwork/P2PComm.h"
#include "libNetwork/PeerStore.h"
#include "libUtils/DataConversion.h"
#include "libUtils/IPConverter.h"
#include "libUtils/Logger.h"
#include "libZilliqa/Zilliqa.h"

using namespace std;
using namespace boost::multiprecision;

#define SUCCESS 0
#define ERROR_IN_COMMAND_LINE -1
#define ERROR_UNHANDLED_EXCEPTION -2

namespace po = boost::program_options;

int main(int argc, const char* argv[]) {
  try {
    INIT_FILE_LOGGER("zilliqa");
    INIT_STATE_LOGGER("state");
    INIT_EPOCHINFO_LOGGER("epochinfo");

    Peer my_network_info;
    string privK;
    string pubK;
    string address;
    int port = -1;
    uint8_t synctype = 0;
    const char* synctype_descr =
        "0(default) for no, 1 for new, 2 for normal, 3 for ds, 4 for lookup";

    po::options_description desc("Options");

    desc.add_options()("help,h", "Print help messages")(
        "privk,i", po::value<string>(&privK)->required(),
        "32-byte private key")("pubk,u", po::value<string>(&pubK)->required(),
                               "32-byte public key")(
        "address,a", po::value<string>(&address)->required(),
        "Listen IPv4/6 address in standard \"dotted decimal\" or optionally "
        "\"dotted decimal:portnumber\" format, otherwise \"NAT\"")(
        "port,p", po::value<int>(&port),
        "Specifies port to bind to, if not specified in address")(
        "loadconfig,l", "Loads configuration if set")(
        "synctype,s", po::value<uint8_t>(&synctype), synctype_descr)(
        "recovery,r", "Runs in recovery mode if set");

    po::variables_map vm;
    try {
      po::store(po::parse_command_line(argc, argv, desc), vm);

      /** --help option
       */
      if (vm.count("help")) {
        SWInfo::LogBrandBugReport();
        cout << desc << endl;
        return SUCCESS;
      }
      po::notify(vm);

      if (privK.length() != 32) {
        SWInfo::LogBrandBugReport();
        std::cerr << "Invalid length of private key" << endl;
        return ERROR_IN_COMMAND_LINE;
      }

      if (pubK.length() != 32) {
        SWInfo::LogBrandBugReport();
        std::cerr << "Invalid length of public key" << endl;
        return ERROR_IN_COMMAND_LINE;
      }

      if (synctype > 4) {
        SWInfo::LogBrandBugReport();
        std::cerr << "Invalid synctype, please select: " << synctype_descr
                  << "." << endl;
      }

      if (address != "NAT") {
        std::vector<std::string> socket_pair;
        boost::algorithm::split(socket_pair, address,
                                boost::algorithm::is_any_of(":"));
        if (socket_pair.size() == 2) {
          address = socket_pair[0];
          try {
            port = boost::lexical_cast<int>(socket_pair[1]);
          } catch (boost::bad_lexical_cast) {
            SWInfo::LogBrandBugReport();
            std::cerr << "Invalid port number" << endl;
            return ERROR_IN_COMMAND_LINE;
          }
        }
      }

      if ((port < 0) || (port > 65535)) {
        SWInfo::LogBrandBugReport();
        std::cerr << "Invalid or missing port number" << endl;
        return ERROR_IN_COMMAND_LINE;
      }
    } catch (boost::program_options::required_option& e) {
      SWInfo::LogBrandBugReport();
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
      std::cout << desc;
      return ERROR_IN_COMMAND_LINE;
    } catch (boost::program_options::error& e) {
      SWInfo::LogBrandBugReport();
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
      return ERROR_IN_COMMAND_LINE;
    }

    unique_ptr<NAT> nt;
    uint128_t ip;

    if (address == "NAT") {
      nt = make_unique<NAT>();
      nt->init();

      int mappedPort = nt->addRedirect(port);

      if (mappedPort <= 0) {
        SWInfo::LogBrandBugReport();
        LOG_GENERAL(WARNING, "NAT ERROR");
        return -1;
      } else {
        LOG_GENERAL(INFO, "My external IP is " << nt->externalIP().c_str()
                                               << " and my mapped port is "
                                               << mappedPort);
      }

      if (!IPConverter::ToNumericalIPFromStr(nt->externalIP().c_str(), ip)) {
        return ERROR_IN_COMMAND_LINE;
      }
      my_network_info = Peer(ip, mappedPort);
    } else {
      if (!IPConverter::ToNumericalIPFromStr(address, ip)) {
        return ERROR_IN_COMMAND_LINE;
      }
      my_network_info = Peer(ip, port);
    }

    vector<unsigned char> tmPrivkey =
        DataConversion::HexStrToUint8Vec(privK.c_str());
    vector<unsigned char> tmpPubkey =
        DataConversion::HexStrToUint8Vec(pubK.c_str());

    PrivKey privkey;
    if (privkey.Deserialize(tmPrivkey, 0) != 0) {
      LOG_GENERAL(WARNING, "We failed to deserialize PrivKey.");
      return -1;
    }

    PubKey pubkey;
    if (pubkey.Deserialize(tmpPubkey, 0) != 0) {
      LOG_GENERAL(WARNING, "We failed to deserialize PubKey.");
      return -1;
    }

    Zilliqa zilliqa(make_pair(privkey, pubkey), my_network_info,
                    vm.count("loadconfig"), synctype, vm.count("recovery"));
    auto dispatcher =
        [&zilliqa](pair<vector<unsigned char>, Peer>* message) mutable -> void {
      zilliqa.Dispatch(message);
    };
    auto broadcast_list_retriever =
        [&zilliqa](unsigned char msg_type, unsigned char ins_type,
                   const Peer& from) mutable -> vector<Peer> {
      return zilliqa.RetrieveBroadcastList(msg_type, ins_type, from);
    };

    P2PComm::GetInstance().StartMessagePump(
        my_network_info.m_listenPortHost, dispatcher, broadcast_list_retriever);

  } catch (std::exception& e) {
    std::cerr << "Unhandled Exception reached the top of main: " << e.what()
              << ", application will now exit" << std::endl;
    return ERROR_UNHANDLED_EXCEPTION;
  }

  return SUCCESS;
}
