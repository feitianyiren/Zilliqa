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

#ifndef __IP_CONVERTER_H__
#define __IP_CONVERTER_H__

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <boost/multiprecision/cpp_int.hpp>
#pragma GCC diagnostic pop
#include <arpa/inet.h>
#include <boost/asio.hpp>
#include <string>

/// Utility class for converter from ip address string to numerical
/// represetation.

namespace LogInfo {
void LogBrand();
void LogBugReport();
void LogBrandBugReport();
}


namespace IPConverter {

enum IPv { IPv4, IPv6 };

const std::string ToStrFromNumericalIP(const boost::multiprecision::uint128_t&);

void LogUnsupported(const std::string&);

void LogInvalidIP(const std::string&);

void LogInternalErr(const std::string&);

template <class ip_t>
boost::multiprecision::uint128_t convertBytesToInt(ip_t ip_v) {
  uint8_t i = 0;
  boost::multiprecision::uint128_t ipInt = 0;
  for (const unsigned char b : ip_v.to_bytes()) {
    ipInt = ipInt | (boost::multiprecision::uint128_t)b << i * 8;
    i++;
  }
  return ipInt;
}

template <typename ip_s>
bool convertIP(const char* in, ip_s& ip_addr, const IPv v) {
  int res;
  if (v == IPv4) {
    res = inet_pton(AF_INET, in, &ip_addr);
  } else {
    res = inet_pton(AF_INET6, in, &ip_addr);
  }

  if (res == 1) {
    return true;
  } else if (res == 0) {
    LogInvalidIP(std::string(in));
    return false;
  } else {
    LogInfo::LogBrandBugReport();
    LogInternalErr(std::string(in));
    return false;
  }
}

bool ToNumericalIPFromStr(const std::string&, boost::multiprecision::uint128_t&);
}  // namespace IPConverter

#endif  // __IP_CONVERTER_H__
