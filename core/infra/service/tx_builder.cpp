/**
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <flatbuffers/flatbuffers.h>
#include <service/flatbuffer_service.h>
#include <service/tx_builder.h>
#include <commands_generated.h>
#include <endpoint_generated.h>
#include <primitives_generated.h>
#include <transaction_generated.h>
#include <crypto/hash.hpp>
#include <crypto/signature.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <membership_service/peer_service.hpp>
#include <utils/datetime.hpp>

namespace tx_builder {

namespace command {

  flatbuffers::Offset<void> CreateCommandFromTx(
    flatbuffers::FlatBufferBuilder& fbb, const Transaction& tx) {

    using iroha::Command;

    switch (tx.command_type()) {
      case Command::NONE: {
        assert(false && "Command::NONE");
      }
      case Command::Add: {
        auto p = tx.command_as_Add();
        std::vector<uint8_t> asset(p->asset()->begin(), p->asset()->end());
        return ::iroha::CreateAddDirect(
          fbb, p->accPubKey()->c_str(), &asset).Union();
      }
      case Command::Subtract: {
        auto p = tx.command_as_Subtract();
        std::vector<uint8_t> asset(p->asset()->begin(), p->asset()->end());
        return ::iroha::CreateSubtractDirect(
          fbb, p->accPubKey()->c_str(), &asset).Union();
      }
      case Command::Transfer: {
        auto p = tx.command_as_Transfer();
        std::vector<uint8_t> asset(p->asset()->begin(), p->asset()->end());
        return ::iroha::CreateTransferDirect(
          fbb, &asset, p->sender()->c_str(), p->receiver()->c_str()).Union();
      }
      case Command::AssetCreate: {
        auto p = tx.command_as_AssetCreate();
        return ::iroha::CreateAssetCreateDirect(
          fbb, p->asset_name()->c_str(), p->domain_name()->c_str(),
          p->ledger_name()->c_str()).Union();
      }
      case Command::AssetRemove: {
        auto p = tx.command_as_AssetRemove();
        return ::iroha::CreateAssetRemoveDirect(
          fbb, p->asset_name()->c_str(), p->domain_name()->c_str(),
          p->ledger_name()->c_str()).Union();
      }
      case Command::PeerAdd: {
        auto p = tx.command_as_PeerAdd();
        std::vector<uint8_t> peer(p->peer()->begin(), p->peer()->end());
        return ::iroha::CreatePeerAddDirect(fbb, &peer).Union();
      }
      case Command::PeerRemove: {
        auto p = tx.command_as_PeerRemove();
        return ::iroha::CreatePeerRemoveDirect(
          fbb, p->peerPubKey()->c_str()).Union();
      }
      case Command::PeerSetActive: {
        auto p = tx.command_as_PeerSetActive();
        return ::iroha::CreatePeerSetActiveDirect(
          fbb, p->peerPubKey()->c_str(), p->active()).Union();
      }
      case Command::PeerSetTrust: {
        auto p = tx.command_as_PeerSetTrust();
        return ::iroha::CreatePeerSetTrustDirect(
          fbb, p->peerPubKey()->c_str(), p->trust()).Union();
      }
      case Command::PeerChangeTrust: {
        auto p = tx.command_as_PeerChangeTrust();
        return ::iroha::CreatePeerChangeTrustDirect(
          fbb, p->peerPubKey()->c_str(), p->delta()).Union();
      }
      case Command::AccountAdd: {
        auto p = tx.command_as_AccountAdd();
        std::vector<uint8_t> account(p->account()->begin(), p->account()->end());
        return ::iroha::CreateAccountAddDirect(fbb, &account).Union();
      }
      case Command::AccountRemove: {
        auto p = tx.command_as_AccountRemove();
        return ::iroha::CreateAccountRemoveDirect(fbb, p->pubkey()->c_str()).Union();
      }

        // WILL CHECK
      case Command::AccountAddSignatory: {
        auto p = tx.command_as_AccountAddSignatory();
        std::vector<flatbuffers::Offset<flatbuffers::String>>
          signatory(p->signatory()->begin(), p->signatory()->end());
        return ::iroha::CreateAccountAddSignatoryDirect(
          fbb, p->account()->c_str(), &signatory).Union();
      }
      case Command::AccountRemoveSignatory: {
        auto p = tx.command_as_AccountRemoveSignatory();
        std::vector<flatbuffers::Offset<flatbuffers::String>>
          signatory(p->signatory()->begin(), p->signatory()->end());
        return ::iroha::CreateAccountRemoveSignatoryDirect(
          fbb, p->account()->c_str(), &signatory).Union();
      }
      case Command::AccountSetUseKeys: {
        auto p = tx.command_as_AccountSetUseKeys();
        std::vector<flatbuffers::Offset<flatbuffers::String>>
          accounts(p->accounts()->begin(), p->accounts()->end());
        return ::iroha::CreateAccountSetUseKeysDirect(fbb, &accounts, p->useKeys()).Union();
      }
      case Command::AccountMigrate: {
        auto p = tx.command_as_AccountMigrate();
        std::vector<uint8_t> account(p->account()->begin(), p->account()->end());
        return ::iroha::CreateAccountMigrateDirect(fbb, &account, p->prevPubKey()->c_str()).Union();
      }
      case Command::ChaincodeAdd: {
        throw exception::NotImplementedException("Command", __FILE__);
      }
      case Command::ChaincodeRemove: {
        throw exception::NotImplementedException("Command", __FILE__);
      }
      case Command::ChaincodeExecute: {
        throw exception::NotImplementedException("Command", __FILE__);
      }
      case Command::PermissionRemove: {
        /*
        auto p = tx.command_as_PermissionRemove();
        ::iroha::CreateP
        return ::iroha::CreatePermissionRemoveDirect(fbb, p->targetAccount()->c_str(), p->permission_type(), );
         */
        throw exception::NotImplementedException("Command", __FILE__);
      }
      case Command::PermissionAdd: {
        /*
        auto p = tx.command_as_AccountMigrate();
        std::vector<uint8_t> account(p->account()->begin(), p->account()->end());
        return ::iroha::CreateAccountMigrateDirect(fbb, &account, p->prevPubKey()->c_str()).Union();
         */
        throw exception::NotImplementedException("Command", __FILE__);
      }
      default: return flatbuffers::Offset<void>();
    }
  }

} // namespace command

namespace peer {  // namespace peer

  flatbuffers::Offset<PeerAdd> CreateAdd(flatbuffers::FlatBufferBuilder &fbb,
                                         const ::peer::Node &peer) {
    return iroha::CreatePeerAdd(fbb,
                                fbb.CreateVector(primitives::CreatePeer(peer)));
  }

  flatbuffers::Offset<PeerRemove> CreateRemove(
      flatbuffers::FlatBufferBuilder &fbb, const std::string &pubKey) {
    return iroha::CreatePeerRemove(fbb, fbb.CreateString(pubKey));
  }

  flatbuffers::Offset<PeerChangeTrust> CreateChangeTrust(
      flatbuffers::FlatBufferBuilder &fbb, const std::string &pubKey,
      double delta) {
    return iroha::CreatePeerChangeTrust(fbb, fbb.CreateString(pubKey), delta);
  }

  flatbuffers::Offset<PeerSetTrust> CreateSetTrust(
      flatbuffers::FlatBufferBuilder &fbb, const std::string &pubKey,
      double trust) {
    return iroha::CreatePeerSetTrust(fbb, fbb.CreateString(pubKey), trust);
  }

  flatbuffers::Offset<PeerSetActive> CreateSetActive(
      flatbuffers::FlatBufferBuilder &fbb, const std::string &pubKey,
      bool active) {
    return iroha::CreatePeerSetActive(fbb, fbb.CreateString(pubKey), active);
  }

};  // namespace peer

namespace primitives {  // namespace primitives

  std::vector<uint8_t> CreatePeer(const ::peer::Node &peer) {
    flatbuffers::FlatBufferBuilder fbb;
    auto peer_cp = iroha::CreatePeer(
        fbb, fbb.CreateString(peer.ledger_name), fbb.CreateString(peer.publicKey),
        fbb.CreateString(peer.ip), peer.trust, peer.active, peer.join_ledger);
    fbb.Finish(peer_cp);

    uint8_t *ptr = fbb.GetBufferPointer();
    return {ptr, ptr + fbb.GetSize()};
  }

  flatbuffers::Offset<::iroha::Signature> CreateSignature(
      flatbuffers::FlatBufferBuilder &fbb, const std::string &hash,
      uint64_t timestamp) {
    // In oreder to use variable hash and create signature with timestamp,
    // we need hashed string and timestamp in arguments.
    const auto signature = signature::sign(
        hash, config::PeerServiceConfig::getInstance().getMyPublicKey(),
        config::PeerServiceConfig::getInstance().getMyPrivateKey());
    const std::vector<uint8_t> sigblob(signature.begin(), signature.end());
    return ::iroha::CreateSignatureDirect(
        fbb, config::PeerServiceConfig::getInstance().getMyPublicKey().c_str(),
        &sigblob, timestamp);
  }

}  // namespace primitives

namespace account {

  // Note: This function is used mainly for debug because Sumeragi doesn't create
  // Account.
  std::vector<uint8_t> CreateAccount(const std::string &publicKey,
                                     const std::string &alias,
                                     const std::string &prevPubKey,
                                     const std::vector<std::string> &signatories,
                                     uint16_t useKeys) {
    flatbuffers::FlatBufferBuilder fbb;

    std::vector<flatbuffers::Offset<flatbuffers::String>> signatoryOffsets;
    for (const auto &e : signatories) {
      signatoryOffsets.push_back(fbb.CreateString(e));
    }

    auto accountOffset =
        ::iroha::CreateAccountDirect(fbb, publicKey.c_str(), prevPubKey.c_str(),
                                     alias.c_str(), &signatoryOffsets, 1);
    fbb.Finish(accountOffset);

    auto buf = fbb.GetBufferPointer();
    return {buf, buf + fbb.GetSize()};
  }

}  // namespace account

namespace asset {

  // Note: This function is used mainly for debug because Sumeragi doesn't create
  // Currency.
  std::vector<uint8_t> CreateCurrency(const std::string &currencyName,
                                      const std::string &domainName,
                                      const std::string &ledgerName,
                                      const std::string &description,
                                      const std::string &amount,
                                      uint8_t precision) {
    flatbuffers::FlatBufferBuilder fbb;
    auto currency = iroha::CreateCurrencyDirect(
        fbb, currencyName.c_str(), domainName.c_str(), ledgerName.c_str(),
        description.c_str(), amount.c_str(), precision);
    auto asset =
        iroha::CreateAsset(fbb, ::iroha::AnyAsset::Currency, currency.Union());
    fbb.Finish(asset);
    auto buf = fbb.GetBufferPointer();
    return {buf, buf + fbb.GetSize()};
  }

}  // namespace asset

namespace transaction {  // namespace transaction

  std::vector<uint8_t> CreateTransaction(
      flatbuffers::FlatBufferBuilder &fbb, const std::string &creatorPubKey,
      iroha::Command cmd_type, const flatbuffers::Offset<void> &command) {
    return CreateTransaction(fbb, creatorPubKey, cmd_type, command, 0);
  }

  /**
   *  CreateTransaction()
   *  Notice: This function call fbb.Finish()
   */
  std::vector<uint8_t> CreateTransaction(
      flatbuffers::FlatBufferBuilder &fbb, const std::string &creatorPubKey,
      iroha::Command cmd_type, const flatbuffers::Offset<void> &command,
      flatbuffers::Offset<iroha::Attachment> attachment) {
    const auto timestamp = datetime::unixtime();
    /*
     * sha256(creatorPubKey + command_type + timestamp + attachment)
     * Future work: not command_type but command
     */
    std::string hashable;
    auto appendStr = [&](const std::string &s) {
      if (s.empty()) return;
      for (const auto &e : s) {
        hashable.push_back((char)e);
      }
    };

    auto appendVec = [&](const std::vector<uint8_t> &v) {
      if (v.empty()) return;
      for (const auto &e : v) {
        hashable.push_back((char)e);
      }
    };

    appendStr(creatorPubKey);
    appendStr(::iroha::EnumNameCommand(cmd_type));
    appendStr(std::to_string(timestamp));


    if (attachment.o != 0) {
      auto atc = flatbuffers::GetTemporaryPointer(fbb, attachment);
      std::string attachstr =
          atc->mime()->str() +
          std::string(atc->data()->begin(), atc->data()->end());
      appendStr(attachstr);
    }

    const auto hash = hash::sha3_256_hex(hashable);

    std::vector<flatbuffers::Offset<::iroha::Signature>> signatures{
        tx_builder::primitives::CreateSignature(fbb, hash, timestamp)};

    std::vector<uint8_t> hsvec(hash.begin(), hash.end());

    auto txOffset = ::iroha::CreateTransactionDirect(
        fbb, creatorPubKey.c_str(), cmd_type, command, &signatures, &hsvec,
        timestamp, attachment);

    fbb.Finish(txOffset);

    auto bufptr = fbb.GetBufferPointer();
    return {bufptr, bufptr + fbb.GetSize()};
  }

};  // namespace transaction

namespace endpoint {
  std::vector<uint8_t> CreatePing(const std::string &message,
                                  const std::string &sender) {
    flatbuffers::FlatBufferBuilder fbb;
    auto ping = ::iroha::CreatePing(fbb, fbb.CreateString(message),
                                    fbb.CreateString(sender));
    fbb.Finish(ping);
    return {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()};
  }
}  // namespace endpoint

}  // namespace tx_builder