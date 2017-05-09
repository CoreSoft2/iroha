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
//
// Created by Takumi Yamashita on 2017/05/03.
//


#include <endpoint_generated.h>
#include <gtest/gtest.h>
#include <service/flatbuffer_service.h>
#include <ametsuchi/repository.hpp>
#include <membership_service/peer_service.hpp>
#include <membership_service/synchronizer.hpp>
#include <service/connection.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>


class synchornizer_connection_part_test : public ::testing::Test {
 protected:
  void serverSyncReceive() { connection::run(); }
  std::thread server_thread_torii;

  static void SetUpTestCase() { connection::initialize_peer(); }

  static void TearDownTestCase() { connection::finish(); }

  virtual void SetUp() {
    server_thread_torii = std::thread(
      &synchornizer_connection_part_test::serverSyncReceive, this);
    connection::initialize_peer();
    connection::wait_till_ready();
  }

  virtual void TearDown() { server_thread_torii.detach(); }
};


TEST_F(synchornizer_connection_part_test, checkHashAllTest) {
  std::string ip = ::peer::myself::getIp();
  std::string hash = repository::getMerkleRoot();
  std::cout << ip << " " << hash << std::endl;
  auto vec = tx_builder::endpoint::CreatePing(hash, ip);
  auto &ping = *flatbuffers::GetRoot<iroha::Ping>(vec.data());
  ASSERT_TRUE(connection::memberShipService::SyncImpl::checkHash::send(ip, ping));

  std::string dummy_hash = "ng_hash";
  auto vec2 = tx_builder::endpoint::CreatePing(dummy_hash, ip);
  auto &ping2 = *flatbuffers::GetRoot<iroha::Ping>(vec2.data());
  ASSERT_FALSE(connection::memberShipService::SyncImpl::checkHash::send(ip, ping2));
}

TEST_F(synchornizer_connection_part_test, getPeersTest) {
  std::string default_leader_ip =
      ::peer::myself::getIp();  // change -> getConfig::LeaderIp()
  std::string message = "getPing!";
  std::string myip = ::peer::myself::getIp();
  auto vec = tx_builder::endpoint::CreatePing(message, myip);
  auto &ping = *flatbuffers::GetRoot<iroha::Ping>(vec.data());
  connection::memberShipService::SyncImpl::getPeers::send(default_leader_ip,
                                                          ping);
  for (auto &&peer : ::peer::service::getActivePeerList()) {
    std::cout << peer->ip << std::endl;
    std::cout << peer->publicKey << std::endl;
    std::cout << peer->ledger_name << std::endl;
    std::cout << peer->trust << std::endl;
    std::cout << peer->active << std::endl;
    std::cout << peer->join_ledger << std::endl;
  }
}