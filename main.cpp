#include "util.hpp"

int main() {
  sole::uuid u4 = sole::uuid4();
  std::cout << "uuid:     " << u4 << std::endl;
  std::string uStr = u4.str();
  stripHyphens(uStr);
  std::cout << "uuidHex:  " << uStr << std::endl;
  std::string data = "Lorem Ipsum";
  std::string dataHex = bytesToHex(data.c_str());
  std::cout << "data:     " << data << std::endl;
  std::cout << "dataHex:  " << dataHex << std::endl;

  std::string d = "2020-10-21 05:34:09.581";
  std::string dHex = dateToEpochHex(d);
  std::cout << "date:     " << d << '\n';
  std::cout << "dataHex:  " << dHex << '\n';

  std::string datum = "000101001022845014d6b246699a0374de7ac8562d3c1fec6e739c4ce4b0b9a46b2e61e33212894c6f72656d20697073756d33175b29943ff0000000000000000";
  const uint8_t *datum8 = reinterpret_cast<const uint8_t *>(datum.c_str());
  uint16_t chk = fletcher16(datum8, strlen(datum.c_str()));
  std::cout << "datum: " << datum << std::endl;
  std::cout << "chk: " << std::hex << chk << std::endl;
  std::string datum2 = "000101001022845014d6b246699a0374de7ac8562d3c1fec6e739c4ce4b0b9a46b2e61e33212894c6f72656d20697073756d33175b29943ff0000000000000001";
  const uint8_t *datum28 = reinterpret_cast<const uint8_t *>(datum2.c_str());
  uint16_t chk2 = fletcher16(datum28, strlen(datum2.c_str()));
  std::cout << "datum2: " << datum2 << std::endl;
  std::cout << "chk2: " << std::hex << chk2 << std::endl;

  ring_t r;
  r.ring_uuid = u4.str();
  r.ring_status = "open";
  r.ring_created = "2020-10-20 12:00:00";
  r.ring_updated = "2020-10-20 13:00:00";
  std::cout << "r: " << r << std::endl;

  std::vector<block_t> blocks;
  block_t b;
  u4 = sole::uuid4();
  b.uuid = u4.str();
  b.ring_uuid = r.ring_uuid;
  b.prev_hash = "7fff";
  b.hash = "";
  b.data = "Lorem Ipsum";
  b.nonce = 0x0;
  b.created = "2020-10-20 12:00:00";
  b.updated = "2020-10-20 13:00:00";
  blocks.push_back(b);

  std::string initPrevHash = b.prev_hash;
  std::string prevHash = initPrevHash;

  u4 = sole::uuid4();
  b.uuid = u4.str();
  blocks.push_back(b);

  u4 = sole::uuid4();
  b.uuid = u4.str();
  blocks.push_back(b);

  block_t lastBlock;
  for (auto b : blocks) {
    b.prev_hash = prevHash;
    std::string hash = hashToHex(b);
    b.hash = hash;
    std::cout << b << std::endl;
    prevHash = hash;
    lastBlock = b;
  }

  for (int i = 0x0; i < 0xFFFF; i++) {
    if (lastBlock.hash != initPrevHash) {
      lastBlock.nonce++;
      lastBlock.hash = hashToHex(lastBlock);
      std::cout << std::hex << "nonce = " << lastBlock.nonce;
      std::cout << ", hash = " << lastBlock.hash << std::endl;
    } else {
      std::cout << "Hash match found!" << std::endl;
      std::cout << lastBlock << std::endl;
      break;
    }
  }
}
