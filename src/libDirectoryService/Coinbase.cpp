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
#include <map>
#include <queue>
#include <vector>

#include "DirectoryService.h"
#include "common/Constants.h"
#include "libData/AccountData/Account.h"
#include "libData/AccountData/AccountStore.h"
#include "libMediator/Mediator.h"

using namespace std;
using namespace boost::multiprecision;

template <class Container>
bool DirectoryService::SaveCoinbaseCore(const vector<bool>& b1,
                                        const vector<bool>& b2,
                                        const Container& shard,
                                        const uint32_t& shard_id,
                                        const uint64_t& epochNum) {
  if (LOOKUP_NODE_MODE) {
    LOG_GENERAL(WARNING,
                "DirectoryService::SaveCoinbaseCore not "
                "expected to be called from LookUp node.");
    return true;
  }

  LOG_MARKER();

  auto it = m_coinbaseRewardees.find(epochNum);
  if (it != m_coinbaseRewardees.end()) {
    if (it->second.find(shard_id) != it->second.end()) {
      LOG_GENERAL(INFO, "Already have cosigs of shard " << shard_id);
      return false;
    }
  }

  if (shard.size() != b1.size()) {
    LOG_GENERAL(WARNING, "B1 and shard members pub keys size do not match "
                             << b1.size() << "  " << shard.size());
    return false;
  }
  if (shard.size() != b2.size()) {
    LOG_GENERAL(WARNING, "B2 and shard members pub keys size do not match "
                             << b2.size() << "  " << shard.size());
    return false;
  }

  unsigned int i = 0;
  constexpr uint16_t MAX_REPUTATION =
      4096;  // This means the max priority is 12. A node need to continually
             // run for 5 days to achieve this reputation.

  for (const auto& kv : shard) {
    const auto& pubKey = std::get<SHARD_NODE_PUBKEY>(kv);
    if (b1.at(i)) {
      m_coinbaseRewardees[epochNum][shard_id].push_back(
          Account::GetAddressFromPublicKey(pubKey));
      if (m_mapNodeReputation[pubKey] < MAX_REPUTATION) {
        ++m_mapNodeReputation[pubKey];
      }
    }
    if (b2.at(i)) {
      m_coinbaseRewardees[epochNum][shard_id].push_back(
          Account::GetAddressFromPublicKey(pubKey));
      if (m_mapNodeReputation[pubKey] < MAX_REPUTATION) {
        ++m_mapNodeReputation[pubKey];
      }
    }
    i++;
  }

  /*deque<PubKey> toKeys;

  for (auto it = m_myShardMembers->begin(); it != m_myShardMembers->end();
       ++it)
  {
      toKeys.emplace_back(it->first);
  }

  Reward(lastMicroBlock.GetB1(), lastMicroBlock.GetB2(), toKeys,
         genesisAccount);*/

  return true;
}

void DirectoryService::LookupCoinbase(
    [[gnu::unused]] const DequeOfShard& shards,
    [[gnu::unused]] const MapOfPubKeyPoW& allPow,
    [[gnu::unused]] const map<PubKey, Peer>& powDSWinner,
    [[gnu::unused]] const MapOfPubKeyPoW& dsPow) {
  LOG_MARKER();
  VectorOfLookupNode vecLookup = m_mediator.m_lookup->GetLookupNodes();
  const auto& epochNum = m_mediator.m_currentEpochNum;

  lock_guard<mutex> g(m_mutexCoinbaseRewardees);

  for (const auto& lookupNode : vecLookup) {
    LOG_GENERAL(INFO, " " << lookupNode.first);
    m_coinbaseRewardees[epochNum][CoinbaseReward::LOOKUP_REWARD].push_back(
        Account::GetAddressFromPublicKey(lookupNode.first));
  }

  /*for (const auto& shard : shards) {
    for (const auto& shardNode : shard) {
      auto toFind = get<SHARD_NODE_PUBKEY>(shardNode);
      auto it = allPow.find(toFind);
      if (it == allPow.end()) {
        LOG_GENERAL(INFO, "Could not find the node, maybe it is the oldest DS "
  << toFind); continue;
      }
      const auto& lookupId = it->second.lookupId;
      // Verify

      if (lookupId >= vecLookup.size()) {
        LOG_GENERAL(WARNING, "Lookup id greater than lookup Size " << lookupId);
        continue;
      }
      else
      {
        if(DEBUG_LEVEL >= 5)
        {
          LOG_GENERAL(INFO,"[LCNBSE]"<<"Awarded lookup "<<lookupId);
        }
      }

      const auto& lookupPubkey = vecLookup.at(lookupId).first;
      m_coinbaseRewardees[epochNum][CoinbaseReward::LOOKUP_REWARD].push_back(
          Account::GetAddressFromPublicKey(lookupPubkey));
    }
  }

  for (const auto& dsWinner : powDSWinner) {
    auto toFind = dsWinner.first;
    auto it = dsPow.find(toFind);

    if (it == dsPow.end()) {
      LOG_GENERAL(FATAL, "Could not find " << toFind);
    }
    const auto& lookupId = it->second.lookupId;
    if (lookupId >= vecLookup.size()) {
      LOG_GENERAL(WARNING, "Lookup id greater than lookup Size " << lookupId);
      continue;
    }

    const auto& lookupPubkey = vecLookup.at(lookupId).first;
    m_coinbaseRewardees[epochNum][CoinbaseReward::LOOKUP_REWARD].push_back(
        Account::GetAddressFromPublicKey(lookupPubkey));
  }*/
}

bool DirectoryService::SaveCoinbase(const vector<bool>& b1,
                                    const vector<bool>& b2,
                                    const int32_t& shard_id,
                                    const uint64_t& epochNum) {
  if (LOOKUP_NODE_MODE) {
    LOG_GENERAL(WARNING,
                "DirectoryService::SaveCoinbase not "
                "expected to be called from LookUp node.");
    return true;
  }

  LOG_MARKER();
  LOG_GENERAL(INFO, "Save coin base for shardId: " << shard_id << ", epochNum: "
                                                   << epochNum);

  if (shard_id == (int32_t)m_shards.size()) {
    LOG_GENERAL(INFO, "Skip the micro block with shardId = shard size.");
    return true;
  }

  if (shard_id == CoinbaseReward::FINALBLOCK_REWARD) {
    // DS
    lock(m_mediator.m_mutexDSCommittee, m_mutexCoinbaseRewardees);
    lock_guard<mutex> g(m_mediator.m_mutexDSCommittee, adopt_lock);
    lock_guard<mutex> g1(m_mutexCoinbaseRewardees, adopt_lock);
    return SaveCoinbaseCore(b1, b2, *m_mediator.m_DSCommittee, shard_id,
                            epochNum);
  } else {
    lock_guard<mutex> g(m_mutexCoinbaseRewardees);
    return SaveCoinbaseCore(b1, b2, m_shards.at(shard_id), shard_id, epochNum);
  }
}

void DirectoryService::InitCoinbase() {
  if (LOOKUP_NODE_MODE) {
    LOG_GENERAL(WARNING,
                "DirectoryService::InitCoinbase not "
                "expected to be called from LookUp node.");
    return;
  }

  LOG_MARKER();

  const auto& vecLookup = m_mediator.m_lookup->GetLookupNodes();
  const auto& epochNum = m_mediator.m_currentEpochNum;

  lock_guard<mutex> g(m_mutexCoinbaseRewardees);

  for (const auto& lookupNode : vecLookup) {
    m_coinbaseRewardees[epochNum][CoinbaseReward::LOOKUP_REWARD].push_back(
        Account::GetAddressFromPublicKey(lookupNode.first));
  }

  if (m_coinbaseRewardees.size() < NUM_FINAL_BLOCK_PER_POW - 1) {
    LOG_GENERAL(INFO, "[CNBSE]"
                          << "Less then expected epoch rewardees "
                          << m_coinbaseRewardees.size());
  } else if (m_coinbaseRewardees.size() > NUM_FINAL_BLOCK_PER_POW - 1) {
    LOG_GENERAL(INFO, "[CNBSE]"
                          << "More then expected epoch rewardees "
                          << m_coinbaseRewardees.size());
  }

  if (GENESIS_WALLETS.empty()) {
    LOG_GENERAL(WARNING, "no genesis wallet");
    return;
  }

  Address genesisAccount(GENESIS_WALLETS[0]);

  uint128_t sig_count = 0;
  uint32_t lookup_count = 0;
  for (auto const& epochNum : m_coinbaseRewardees) {
    for (auto const& shardId : epochNum.second) {
      if (shardId.first == CoinbaseReward::LOOKUP_REWARD) {
        lookup_count += shardId.second.size();
      } else {
        sig_count += shardId.second.size();
      }
    }
  }
  LOG_GENERAL(INFO, "Total signatures count: " << sig_count << " lookup count "
                                               << lookup_count);

  uint128_t total_reward;

  if (!SafeMath<uint128_t>::add(COINBASE_REWARD, m_totalTxnFees,
                                total_reward)) {
    LOG_GENERAL(WARNING, "total_reward addition unsafe!");
    return;
  }

  LOG_GENERAL(INFO, "Total reward: " << total_reward);

  uint128_t lookupReward = (total_reward / 100) * LOOKUP_REWARD_IN_PERCENT;
  uint128_t nodeReward = total_reward - lookupReward;
  uint128_t reward_each;
  uint128_t reward_each_lookup;

  if (!SafeMath<uint128_t>::div(nodeReward, sig_count, reward_each)) {
    LOG_GENERAL(WARNING, "reward_each dividing unsafe!");
    return;
  }

  if (!SafeMath<uint128_t>::div(lookupReward, lookup_count,
                                reward_each_lookup)) {
    LOG_GENERAL(WARNING, "reward_each_lookup dividing unsafe");
    return;
  }

  LOG_GENERAL(INFO, "Each reward: " << reward_each << " lookup each "
                                    << reward_each_lookup);

  uint128_t suc_counter = 0;
  uint128_t suc_lookup_counter = 0;
  for (auto const& epochNum : m_coinbaseRewardees) {
    LOG_GENERAL(INFO, "[CNBSE] Rewarding " << epochNum.first << " epoch");

    for (auto const& shardId : epochNum.second) {
      LOG_GENERAL(INFO, "[CNBSE] Rewarding " << shardId.first << " shard");
      if (shardId.first == CoinbaseReward::LOOKUP_REWARD) {
        for (auto const& addr : shardId.second) {
          if (!AccountStore::GetInstance().UpdateCoinbaseTemp(
                  addr, genesisAccount, reward_each_lookup)) {
            LOG_GENERAL(WARNING, "Could Not reward " << addr);
          } else {
            suc_lookup_counter++;
          }
        }
      } else {
        for (auto const& addr : shardId.second) {
          if (!AccountStore::GetInstance().UpdateCoinbaseTemp(
                  addr, genesisAccount, reward_each)) {
            LOG_GENERAL(WARNING, "Could Not reward " << addr);
          } else {
            if (addr ==
                Account::GetAddressFromPublicKey(m_mediator.m_selfKey.second)) {
              LOG_EPOCH(INFO,
                        std::to_string(m_mediator.m_currentEpochNum).c_str(),
                        "[REWARD]"
                            << "Rewarded " << reward_each);
              LOG_STATE("[REWARD]["
                        << setw(15) << left
                        << m_mediator.m_selfPeer.GetPrintableIPAddress() << "]["
                        << m_mediator.m_currentEpochNum << "][" << reward_each
                        << "]");
            }
            suc_counter++;
          }
        }
      }
    }
  }

  uint128_t balance_left = total_reward - (suc_counter * reward_each) -
                           (suc_lookup_counter * reward_each_lookup);

  LOG_GENERAL(INFO, "Left reward: " << balance_left);

  uint16_t lastBlockHash = DataConversion::charArrTo16Bits(
      m_mediator.m_txBlockChain.GetLastBlock().GetBlockHash().asBytes());
  uint16_t shardIndex =
      lastBlockHash % m_coinbaseRewardees[m_mediator.m_currentEpochNum].size();
  uint16_t count = 0;
  for (const auto& shard : m_coinbaseRewardees[m_mediator.m_currentEpochNum]) {
    if (count == shardIndex) {
      uint16_t rdm_index = lastBlockHash % shard.second.size();
      const Address& winnerAddr = shard.second[rdm_index];
      LOG_GENERAL(INFO, "Lucky draw winner: " << winnerAddr);
      if (!AccountStore::GetInstance().UpdateCoinbaseTemp(
              winnerAddr, genesisAccount, balance_left)) {
        LOG_GENERAL(WARNING, "Could not reward lucky draw!");
      }
      LOG_STATE("[REWARD][" << setw(15) << left
                            << m_mediator.m_selfPeer.GetPrintableIPAddress()
                            << "][" << m_mediator.m_currentEpochNum << "]["
                            << balance_left << "]");
      return;
    } else {
      ++count;
    }
  }

  LOG_GENERAL(INFO, "Didn't find any miner to reward the lucky draw");
}
