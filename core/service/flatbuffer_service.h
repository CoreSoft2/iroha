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

#ifndef IROHA_FLATBUFFER_SERVICE_H
#define IROHA_FLATBUFFER_SERVICE_H

#include <functional>
#include <memory>
#include <utils/expected.hpp>
#include <vector>
#include <service/tx_builder.h>

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

namespace flatbuffers {
template <class T>
class Offset;
class FlatBufferBuilder;

// HACK: this typedef is dirty and unstable solution. (might be able to be
// solved by setting dependency for this header)
typedef std::unique_ptr<uint8_t, std::function<void(uint8_t* /* unused */)>>
    unique_ptr_t;
}  // namespace flatbuffers

namespace flatbuffer_service {

  using ::iroha::Peer;
  using ::iroha::PeerAdd;
  using ::iroha::PeerRemove;
  using ::iroha::PeerChangeTrust;
  using ::iroha::PeerSetTrust;
  using ::iroha::PeerSetActive;
  using ::iroha::Transaction;

  Expected<int> hasRequreMember(const iroha::Transaction &tx);

  Expected<flatbuffers::Offset<::iroha::Transaction>> copyTransaction(
    flatbuffers::FlatBufferBuilder &fbb, const ::iroha::Transaction &fromTx);

  Expected<flatbuffers::Offset<::iroha::ConsensusEvent>> copyConsensusEvent(
    flatbuffers::FlatBufferBuilder &fbb, const ::iroha::ConsensusEvent &);

  Expected<std::vector<uint8_t>> GetTxPointer(const iroha::Transaction &tx);

  template<typename T>
  VoidHandler ensureNotNull(T *value) {
    if (value == nullptr) {
      return makeUnexpected(
        exception::connection::NullptrException(typeid(T).name()));
    }
    return {};
  }

  std::string toString(const iroha::Transaction &tx);

  Expected<flatbuffers::unique_ptr_t> addSignature(
    const iroha::ConsensusEvent &event, const std::string &publicKey,
    const std::string &signature);

  Expected<flatbuffers::Offset<::iroha::TransactionWrapper>> toTxWrapper(
    flatbuffers::FlatBufferBuilder &, const ::iroha::Transaction &);

  Expected<flatbuffers::unique_ptr_t> toConsensusEvent(
    const iroha::Transaction &tx);

  Expected<flatbuffers::unique_ptr_t> makeCommit(
    const iroha::ConsensusEvent &event);
};      // namespace flatbuffer_service
#endif  // IROHA_FLATBUFFER_SERVICE_H
