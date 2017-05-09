/*
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *      http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IROHA_TX_BUILDER_H
#define IROHA_TX_BUILDER_H

#include <flatbuffers/flatbuffers.h>
#include <functional>
#include <memory>
#include <utils/expected.hpp>
#include <vector>

namespace iroha {
struct Transaction;
struct TransactionWrapper;
struct ConsensusEvent;
struct Peer;
struct PeerAdd;
struct PeerRemove;
struct PeerChangeTrust;
struct PeerSetTrust;
struct PeerSetActive;
struct Signature;
struct Sumeragi;
struct Attachment;
enum class Command : uint8_t;
}  // namespace iroha

namespace peer {
  struct Node;
}

namespace tx_builder {

using ::iroha::Peer;
using ::iroha::PeerAdd;
using ::iroha::PeerRemove;
using ::iroha::PeerChangeTrust;
using ::iroha::PeerSetTrust;
using ::iroha::PeerSetActive;
using ::iroha::Transaction;

namespace command {
  flatbuffers::Offset<void> CreateCommandFromTx(
    flatbuffers::FlatBufferBuilder &, const Transaction &);
} // namespace command

namespace peer {  // namespace peer

  flatbuffers::Offset<PeerAdd> CreateAdd(flatbuffers::FlatBufferBuilder &fbb, const ::peer::Node &peer);

  flatbuffers::Offset<PeerRemove> CreateRemove(flatbuffers::FlatBufferBuilder &fbb, const std::string &pubKey);

  flatbuffers::Offset<PeerChangeTrust> CreateChangeTrust(
    flatbuffers::FlatBufferBuilder &fbb,
    const std::string &pubKey, double delta);

  flatbuffers::Offset<PeerSetTrust> CreateSetTrust(
    flatbuffers::FlatBufferBuilder &fbb,
    const std::string &pubKey,
    double trust);

  flatbuffers::Offset<PeerSetActive> CreateSetActive(
    flatbuffers::FlatBufferBuilder &fbb,
    const std::string &pubKey,
    bool active);

};  // namespace peer

namespace primitives {
  std::vector<uint8_t> CreatePeer(const ::peer::Node &peer);

  flatbuffers::Offset<::iroha::Signature> CreateSignature(
    flatbuffers::FlatBufferBuilder &fbb, const std::string &hash, uint64_t timestamp);
}  // namespace primitives

namespace account {

  std::vector<uint8_t> CreateAccount(const std::string &publicKey,
                                     const std::string &alias,
                                     const std::string &prevPubKey,
                                     const std::vector<std::string> &signatories,
                                     uint16_t useKeys);

}  // namespace account

namespace asset {

  std::vector<uint8_t> CreateCurrency(const std::string &currencyName,
                                      const std::string &domainName,
                                      const std::string &ledgerName,
                                      const std::string &description,
                                      const std::string &amount,
                                      uint8_t precision);

}  // namespace asset


namespace transaction {  // namespace transaction

  std::vector<uint8_t> CreateTransaction(
    flatbuffers::FlatBufferBuilder& fbb,
    const std::string& creatorPubKey,
    iroha::Command cmd_type,
    const flatbuffers::Offset<void>& command
  );

  std::vector<uint8_t> CreateTransaction(
    flatbuffers::FlatBufferBuilder& fbb,
    const std::string& creatorPubKey,
    iroha::Command cmd_type,
    const flatbuffers::Offset<void>& command,
    flatbuffers::Offset<iroha::Attachment> attachment
  );
}

namespace endpoint {
  std::vector<uint8_t> CreatePing(
    const std::string &message,
    const std::string &sender
  );
}

} // namespace tx_builder
#endif