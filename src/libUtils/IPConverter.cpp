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

#include "IPConverter.h"


using namespace std;


namespace IPConverterr {

const std::string ToStrFromNumericalIP(
    const boost::multiprecision::uint128_t& ip) {
  char str[INET_ADDRSTRLEN];
  struct sockaddr_in serv_addr;
  serv_addr.sin_addr.s_addr = ip.convert_to<unsigned long>();
  inet_ntop(AF_INET, &(serv_addr.sin_addr), str, INET_ADDRSTRLEN);
  return std::string(str);
}

void LogBrand() {
  cout << "Copyright (C) Zilliqa. Version 1.0 (Durian). "
          "<https://www.zilliqa.com/> "
       << endl;
}

void LogBugReport() {
  cout << "For bug reporting, please create an issue at "
          "<https://github.com/Zilliqa/Zilliqa> \n"
       << endl;
}

void LogUnsupported() {
  LogBrand();
  LogBugReport();
  cout << "Error: Unknown address type - unsupported protocol\n"
       << endl;
}

void LogInvalidIP() {
  LogBrand();
  LogBugReport();
  cout << "Error: listen_ip_address does not contain a character string "
          "representing a valid network address\n"
       << endl;
}

void LogInternalErr() {
  LogBrand();
  LogBugReport();
  std::cout << "Internal Error: cannot process the input IP address.\n" << std::endl;
}

int ToNumericalIPFromStr(
    const std::string& ipStr, boost::multiprecision::uint128_t& ipInt) {
        boost::asio::ip::address Addr;
        try{
          Addr = boost::asio::ip::address::from_string(ipStr);
        }
        catch(const std::exception& e){
          LogInvalidIP();
          return -1;
        }

        if(Addr.is_v4()) {
          ipInt = (boost::multiprecision::uint128_t)Addr.to_v4().to_ulong();
          return 0;
        }
        else if(Addr.is_v6()) {
          boost::multiprecision::uint128_t addr = 0;
          uint8_t i = 0;
          for (const unsigned char b : Addr.to_v6().to_bytes()) {
            addr = (boost::multiprecision::uint128_t) b << i *8;
            i++;
          }
          return 0;
        }

        LogUnsupported();
        return -1;
}
}

const std::string IPConverter::ToStrFromNumericalIP(
    const boost::multiprecision::uint128_t& ip) {
  char str[INET_ADDRSTRLEN];
  struct sockaddr_in serv_addr;
  serv_addr.sin_addr.s_addr = ip.convert_to<unsigned long>();
  inet_ntop(AF_INET, &(serv_addr.sin_addr), str, INET_ADDRSTRLEN);
  return std::string(str);
}

//void IPConverter::LogBrand() {
//  cout << "Copyright (C) Zilliqa. Version 1.0 (Durian). "
//          "<https://www.zilliqa.com/> "
//       << endl;
//}
//
//void IPConverter::LogBugReport() {
//  cout << "For bug reporting, please create an issue at "
//          "<https://github.com/Zilliqa/Zilliqa> \n"
//       << endl;
//}
//
//void IPConverter::LogUnsupported() {
//  LogBrand();
//  LogBugReport();
//  cout << "Error: Unknown address type - unsupported protocol\n"
//       << endl;
//}
//
//void IPConverter::LogInvalidIP() {
//  LogBrand();
//  LogBugReport();
//  cout << "Error: listen_ip_address does not contain a character string "
//          "representing a valid network addres\n"
//       << endl;
//}
//
//void IPConverter::LogInternalErr() {
//  LogBrand();
//  LogBugReport();
//  std::cout << "Internal Error: cannot process the input IP address.\n" << std::endl;
//}

int IPConverter::ToNumericalIPFromStr(
    const std::string& ipStr, boost::multiprecision::uint128_t& ipInt) {
        boost::asio::ip::address Addr;
        try{
          Addr = boost::asio::ip::address::from_string(ipStr);
        }
        catch(const std::exception& e){
          return -1;
        }

        if(Addr.is_v4()) {
          ipInt = (boost::multiprecision::uint128_t)Addr.to_v4().to_ulong();
          return 0;
        }
        else if(Addr.is_v6()) {
          boost::multiprecision::uint128_t addr = 0;
          uint8_t i = 0;
          for (const unsigned char b : Addr.to_v6().to_bytes()) {
            addr = (boost::multiprecision::uint128_t) b << i *8;
            i++;
          }
          return 0;
        }

        return -1;
}
