#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h> /* strtoumax */
#include <stdbool.h>
#include <errno.h>

#include "date.hpp"
#include "sole.hpp"
#include "json.hpp"

using json = nlohmann::json;

const uint8_t ProtocolVersion = 0x00;
const uint8_t AlgorithmID = 0x01;
const uint8_t SecurityMode = 0x01;
const uint16_t DifficultyBits = 0x0010;

struct ring_t {
  std::string ring_uuid;
  std::string ring_status;
  std::string ring_created;
  std::string ring_updated;
};

std::ostream& operator << (std::ostream &o, const ring_t &r) {
  o << "{" << std::endl;
  o << "  ring_uuid: '" << r.ring_uuid << "'" << std::endl;
  o << "  ring_status: '" << r.ring_status << "'" << std::endl;
  o << "  ring_created: '" << r.ring_created << "'" << std::endl;
  o << "  ring_updated: '" << r.ring_updated << "'" << std::endl;
  o << "}";
  return o;
}

struct block_t {
  std::string uuid;
  std::string ring_uuid;
  std::string prev_hash;
  std::string hash;
  std::string data;
  uint32_t nonce;
  std::string created;
  std::string updated;
};

std::ostream& operator << (std::ostream &o, const block_t &b) {
  o << "{" << std::endl;
  o << "  uuid: '" << b.uuid << "'" << std::endl;
  o << "  ring_uuid: '" << b.ring_uuid << "'" << std::endl;
  o << "  prev_hash: " << b.prev_hash << std::endl;
  o << "  hash: " << b.hash << std::endl;
  o << "  data: '" << b.data << "'" << std::endl;
  o << "  nonce: " << b.nonce << std::endl;
  o << "  created: '" << b.created << "'" << std::endl;
  o << "  updated: '" << b.updated << "'" << std::endl;
  o << "}";
  return o;
}

uint16_t fletcher16(const uint8_t *data, size_t len) {
	uint32_t c0, c1;

	/*  Found by solving for c1 overflow: */
	/* n > 0 and n * (n+1) / 2 * (2^8-1) < (2^32-1). */
	for (c0 = c1 = 0; len > 0; ) {
		size_t blocklen = len;
		if (blocklen > 5002) {
			blocklen = 5002;
		}
		len -= blocklen;
		do {
			c0 = c0 + *data++;
			c1 = c1 + c0;
		} while (--blocklen);
		c0 = c0 % 255;
		c1 = c1 % 255;
   }
   return (c1 << 8 | c0);
}

std::string bytesToHex(const char *data) {
  std::string dataStr(data);
  std::stringstream hexStringStream;
  
  hexStringStream << std::hex << std::setfill('0');
  for(auto d : dataStr) {
    hexStringStream << std::setw(2) << static_cast<int>(d);
  }
  return hexStringStream.str();
}

void stripHyphens(std::string &str) {
  str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
}

uint8_t hexValue(uint8_t c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('a' <= c && c <= 'f') {
    return c - 'a' + 10;
  } else if ('A' <= c && c <= 'F') {
    return c - 'A' + 10;
  } else {
    std::abort();
  }
}

std::string hexToAscii(const std::string in) {
  std::string out;
  out.clear();
  out.reserve(in.length() / 2);
  for (std::string::const_iterator p = in.begin(); p != in.end(); p++) {
      uint8_t c = hexValue(*p);
      p++;
      if (p == in.end()) break;  // incomplete last digit - should report error
      c = (c << 4) + hexValue(*p); // + takes precedence over <<
      out.push_back(c);
  }
  return out;
}

std::string uint16ToHex(uint16_t u) {
  std::stringstream ss;
  ss << std::hex << u;
  std::string hexString = ss.str();
  return hexString;
}

uint16_t stringToUint16(std::string s) {
  std::stringstream ss;
  for (const char c : s) {
    ss << std::hex << static_cast<uint16_t>(c);
  }
  uint16_t s16;
  ss >> s16;
  return s16;
}

template< typename T > std::string decToHex( T i ) {
  std::stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(T)*2) 
         << std::hex << i;
  return stream.str();
}

std::string dateToEpochHex(const std::string d) {
  std::string fmt = "%F %T";
  date::sys_time<std::chrono::milliseconds> tp;
  std::istringstream in(d);
  in >> date::parse(fmt, tp);
  std::stringstream ss;
  ss << std::hex << tp.time_since_epoch().count();
  return ss.str();
}

std::string hashToHex(block_t b) {
  std::string input;
  std::string u;
  std::stringstream ss;

  ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint16_t>(ProtocolVersion);
  ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint16_t>(SecurityMode);
  ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint16_t>(AlgorithmID);
  ss << std::hex << std::setfill('0') << std::setw(4) << static_cast<uint16_t>(DifficultyBits);

  u = b.uuid;
  stripHyphens(u);
  ss << u;

  u = b.ring_uuid;
  stripHyphens(u);
  ss << u;

  u = b.prev_hash;
  ss << u;

  u = bytesToHex(b.data.c_str());
  ss << u;

  u = dateToEpochHex(b.created);
  ss << u;

  u = decToHex<uint32_t>(b.nonce);
  ss << u;
  input = ss.str();
  //std::cout << input << std::endl;

  /*json j = json::object();
  j["uuid"] = b.uuid;
  j["ring_uuid"] = b.ring_uuid;
  j["prev_hash"] = b.prev_hash;
  j["data"] = b.data;
  j["nonce"] = b.nonce;
  j["created"] = b.created;
  j["updated"] = b.updated;
  std::string bJSON = j.dump();*/
  //std::cout << bJSON << std::endl;
  const uint8_t *b8 = reinterpret_cast<const uint8_t *>(input.c_str());
  uint16_t chksum16 = fletcher16(b8, strlen(input.c_str()));
  //std::cout << chksum16 << std::endl;
  //std::string chksum = uint16ToHex(chksum16);
  std::string chksum = decToHex<uint16_t>(chksum16);
  return chksum;
}