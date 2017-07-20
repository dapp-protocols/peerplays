/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <boost/test/unit_test.hpp>
#include <fc/crypto/openssl.hpp>
#include <openssl/rand.h>

#include "../common/database_fixture.hpp"
#include <graphene/utilities/tempdir.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/is_authorized_asset.hpp>
#include <graphene/chain/witness_object.hpp>

#include <graphene/chain/sport_object.hpp>
#include <graphene/chain/event_object.hpp>
#include <graphene/chain/event_group_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/betting_market_object.hpp>

//#include <boost/algorithm/string/replace.hpp>

using namespace graphene::chain;
using namespace graphene::chain::test;


BOOST_FIXTURE_TEST_SUITE( betting_tests, database_fixture )

#define CREATE_ICE_HOCKEY_BETTING_MARKET() \
  const sport_object& ice_hockey = create_sport({{"en", "Ice Hockey"}, {"zh_Hans", "冰球"}, {"ja", "アイスホッケー"}}); \
  const event_group_object& nhl = create_event_group({{"en", "NHL"}, {"zh_Hans", "國家冰球聯盟"}, {"ja", "ナショナルホッケーリーグ"}}, ice_hockey.id); \
  const event_object& capitals_vs_blackhawks = create_event({{"en", "Washington Capitals/Chicago Blackhawks"}, {"zh_Hans", "華盛頓首都隊/芝加哥黑鷹"}, {"ja", "ワシントン・キャピタルズ/シカゴ・ブラックホークス"}}, {{"en", "2016-17"}}, nhl.id); \
  const betting_market_rules_object& betting_market_rules = create_betting_market_rules({{"en", "NHL Rules v1.0"}}, {{"en", "The winner will be the team with the most points at the end of the game.  The team with fewer points will not be the winner."}}); \
  const betting_market_group_object& moneyline_betting_markets = create_betting_market_group({{"en", "Moneyline"}}, capitals_vs_blackhawks.id, betting_market_rules.id, asset_id_type()); \
  const betting_market_object& capitals_win_market = create_betting_market(moneyline_betting_markets.id, {{"en", "Washington Capitals win"}}); \
  const betting_market_object& blackhawks_win_market = create_betting_market(moneyline_betting_markets.id, {{"en", "Chicago Blackhawks win"}});

#if 0
BOOST_AUTO_TEST_CASE(generate_block)
{
   try
   {
      ACTORS( (alice)(bob) );

      // failure if ACTORS
      generate_blocks(10);

   } FC_LOG_AND_RETHROW()
}
#endif

BOOST_AUTO_TEST_CASE(try_create_sport)
{
   try
   {
        sport_create_operation sport_create_op;
        sport_create_op.name = {{"en", "Ice Hockey"}, {"zh_Hans", "冰球"}, {"ja", "アイスホッケー"}};

        proposal_id_type proposal_id = propose_operation(sport_create_op);

        const auto& active_witnesses = db.get_global_properties().active_witnesses;
        int voting_count = active_witnesses.size() / 2;

        // 5 for
        std::vector<witness_id_type> witnesses;
        for (const witness_id_type& witness_id : active_witnesses)
        {
            witnesses.push_back(witness_id);
            if (--voting_count == 0)
                break;
        }
        process_proposal_by_witnesses(witnesses, proposal_id);

        // 1st out
        witnesses.clear();
        auto itr = active_witnesses.begin();
        witnesses.push_back(*itr);
        process_proposal_by_witnesses(witnesses, proposal_id, true);

        const auto& sport_index = db.get_index_type<sport_object_index>().indices().get<by_id>();
        // not yet approved
        BOOST_REQUIRE(sport_index.rbegin() == sport_index.rend());

        // 6th for
        witnesses.clear();
        itr += 5;
        witnesses.push_back(*itr);
        process_proposal_by_witnesses(witnesses, proposal_id);

        // not yet approved
        BOOST_REQUIRE(sport_index.rbegin() == sport_index.rend());

        // 7th for
        witnesses.clear();
        ++itr;
        witnesses.push_back(*itr);
        process_proposal_by_witnesses(witnesses, proposal_id);

        // done
        BOOST_REQUIRE(sport_index.rbegin() != sport_index.rend());
        sport_id_type sport_id = (*sport_index.rbegin()).id;
        BOOST_REQUIRE(sport_id == sport_id_type());

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(simple_bet_win)
{
   try
   {
      ACTORS( (alice)(bob) );
      CREATE_ICE_HOCKEY_BETTING_MARKET();

      // give alice and bob 10k each
      transfer(account_id_type(), alice_id, asset(10000));
      transfer(account_id_type(), bob_id, asset(10000));

      // place bets at 10:1
      place_bet(bob_id, capitals_win_market.id, bet_type::lay, asset(100, asset_id_type()), 11 * GRAPHENE_BETTING_ODDS_PRECISION, 2);
      place_bet(alice_id, capitals_win_market.id, bet_type::back, asset(1000, asset_id_type()), 11 * GRAPHENE_BETTING_ODDS_PRECISION, 20);

      // reverse positions at 1:1
      place_bet(bob_id, capitals_win_market.id, bet_type::back, asset(1100, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 22);
      place_bet(alice_id, capitals_win_market.id, bet_type::lay, asset(1100, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 22);

   } FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( peerplays_sport_create_test )
{
   try
   {
      ACTORS( (alice)(bob) );
      CREATE_ICE_HOCKEY_BETTING_MARKET();

      // give alice and bob 10M each
      transfer(account_id_type(), alice_id, asset(10000000));
      transfer(account_id_type(), bob_id, asset(10000000));

      // have bob lay a bet for 1M (+20k fees) at 1:1 odds                  
      place_bet(bob_id, capitals_win_market.id, bet_type::lay, asset(1000000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 1000000 / 50 /* chain defaults to 2% fees */);
      // have alice back a matching bet at 1:1 odds (also costing 1.02M) 
      place_bet(alice_id, capitals_win_market.id, bet_type::back, asset(1000000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 1000000 / 50 /* chain defaults to 2% fees */);

      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000000 - 1000000 - 20000);
      BOOST_CHECK_EQUAL(get_balance(bob_id, asset_id_type()), 10000000 - 1000000 - 20000);

      // caps win
      resolve_betting_market_group(moneyline_betting_markets.id,
                                  {{capitals_win_market.id, betting_market_resolution_type::win},
                                   {blackhawks_win_market.id, betting_market_resolution_type::cancel}});


      uint16_t rake_fee_percentage = db.get_global_properties().parameters.betting_rake_fee_percentage;
      uint32_t rake_value = (-1000000 + 2000000) * rake_fee_percentage / GRAPHENE_1_PERCENT / 100;
      BOOST_TEST_MESSAGE("Rake value " +  std::to_string(rake_value));
      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000000 - 1000000 - 20000 + 2000000 - rake_value);
      BOOST_CHECK_EQUAL(get_balance(bob_id, asset_id_type()), 10000000 - 1000000 - 20000);

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(peerplays_sport_update_test)
{
   try
   {
      ACTORS( (alice) );
      CREATE_ICE_HOCKEY_BETTING_MARKET();
      update_sport(ice_hockey.id, {{"en", "Hockey on Ice"}, {"zh_Hans", "冰"}, {"ja", "アイスホッケ"}});

      transfer(account_id_type(), alice_id, asset(10000000));
      place_bet(alice_id, capitals_win_market.id, bet_type::back, asset(1000000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 1000000 / 50 /* chain defaults to 2% fees */);

      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000000 - 1000000 - 20000);

    } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(peerplays_event_group_update_test)
{
   try
   {
      ACTORS( (alice)(bob) );
      CREATE_ICE_HOCKEY_BETTING_MARKET();

      transfer(account_id_type(), alice_id, asset(10000000));
      transfer(account_id_type(), bob_id, asset(10000000));

      place_bet(alice_id, capitals_win_market.id, bet_type::back, asset(1000000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 1000000 / 50 /* chain defaults to 2% fees */);

      internationalized_string_type n = {{"en", "IBM"}, {"zh_Hans", "國家冰球聯"}, {"ja", "ナショナルホッケーリー"}};
      const sport_object& ice_on_hockey = create_sport({{"en", "Hockey on Ice"}, {"zh_Hans", "冰球"}, {"ja", "アイスホッケー"}}); \

      fc::optional<internationalized_string_type> name = n;

      object_id_type object_id = ice_on_hockey.id;
      fc::optional<object_id_type> sport_id = object_id;

      update_event_group(nhl.id, fc::optional<object_id_type>(), name);
      update_event_group(nhl.id, sport_id, fc::optional<internationalized_string_type>());
      update_event_group(nhl.id, sport_id, name);

#if 0
      // nothing to change
      //GRAPHENE_REQUIRE_THROW(update_event_group(nhl.id, fc::optional<object_id_type>(), fc::optional<internationalized_string_type>()), fc::assert_exception);
      try
      {
          update_event_group(nhl.id, fc::optional<object_id_type>(), fc::optional<internationalized_string_type>());
          FC_ASSERT(false, "expected error hasn't occured");
      }
      catch (fc::exception& e)
      {
        edump((e.code()));
      }

      // no sport object
      //object_id = capitals_win_market.id;
      //sport_id = object_id;
      //GRAPHENE_REQUIRE_THROW(update_event_group(nhl.id, sport_id, name), fc::assert_exception);

      // sport object doesn't exist
#endif

      place_bet(bob_id, capitals_win_market.id, bet_type::lay, asset(1000000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 1000000 / 50 /* chain defaults to 2% fees */);

      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000000 - 1000000 - 20000);
      BOOST_CHECK_EQUAL(get_balance(bob_id, asset_id_type()), 10000000 - 1000000 - 20000);

      // caps win
      resolve_betting_market_group(moneyline_betting_markets.id,
                                  {{capitals_win_market.id, betting_market_resolution_type::win},
                                   {blackhawks_win_market.id, betting_market_resolution_type::cancel}});


      uint16_t rake_fee_percentage = db.get_global_properties().parameters.betting_rake_fee_percentage;
      uint32_t rake_value = (-1000000 + 2000000) * rake_fee_percentage / GRAPHENE_1_PERCENT / 100;
      BOOST_TEST_MESSAGE("Rake value " +  std::to_string(rake_value));
      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000000 - 1000000 - 20000 + 2000000 - rake_value);
      BOOST_CHECK_EQUAL(get_balance(bob_id, asset_id_type()), 10000000 - 1000000 - 20000);

    }
    catch (fc::exception& e)
    {
      edump((e.to_detail_string()));
      throw;
    }
    //} FC_LOG_AND_RETHROW()

}


BOOST_AUTO_TEST_CASE(peerplays_event_update_test)
{
   try
   {
      ACTORS( (alice) );
      CREATE_ICE_HOCKEY_BETTING_MARKET();

      internationalized_string_type n = {{"en", "Washington Capitals vs. Chicago Blackhawks"}, {"zh_Hans", "華盛頓首都隊/芝加哥黑"}, {"ja", "ワシントン・キャピタルズ/シカゴ・ブラックホーク"}};
      internationalized_string_type s = {{"en", "2017-18"}};

      fc::optional<internationalized_string_type> empty;
      fc::optional<internationalized_string_type> name = n;
      fc::optional<internationalized_string_type> season = s;

      update_event(capitals_vs_blackhawks.id, name, empty);
      update_event(capitals_vs_blackhawks.id, empty, season);
      update_event(capitals_vs_blackhawks.id, name, season);

      transfer(account_id_type(), alice_id, asset(10000000));
      place_bet(alice_id, capitals_win_market.id, bet_type::back, asset(1000000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 1000000 / 50 /* chain defaults to 2% fees */);

      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000000 - 1000000 - 20000);

      //GRAPHENE_REQUIRE_THROW(update_event(capitals_vs_blackhawks.id, empty, empty), fc::exception);

    } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE(peerplays_betting_market_rules_update_test)
{
   try
   {
      ACTORS( (alice) );
      CREATE_ICE_HOCKEY_BETTING_MARKET();

      internationalized_string_type n = {{"en", "NHL Rules v1.1"}};
      internationalized_string_type d = {{"en", "The winner will be the team with the most points at the end of the game. The team with fewer points will not be the winner."}};

      fc::optional<internationalized_string_type> empty;
      fc::optional<internationalized_string_type> name = n;
      fc::optional<internationalized_string_type> desc = d;

      update_betting_market_rules(betting_market_rules.id, name, empty);
      update_betting_market_rules(betting_market_rules.id, empty, desc);
      update_betting_market_rules(betting_market_rules.id, name, desc);

      transfer(account_id_type(), alice_id, asset(10000000));
      place_bet(alice_id, capitals_win_market.id, bet_type::back, asset(1000000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 1000000 / 50 /* chain defaults to 2% fees */);

      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000000 - 1000000 - 20000);

      //GRAPHENE_REQUIRE_THROW(update_betting_market_rules(betting_market_rules.id, empty, empty), fc::exception);

    } FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_CASE( cancel_unmatched_in_betting_group_test )
{
   try
   {
      ACTORS( (alice)(bob) );
      CREATE_ICE_HOCKEY_BETTING_MARKET();

      // give alice and bob 10M each
      transfer(account_id_type(), alice_id, asset(10000000));
      transfer(account_id_type(), bob_id, asset(10000000));

      // have bob lay a bet for 1M (+20k fees) at 1:1 odds
      place_bet(bob_id, capitals_win_market.id, bet_type::lay, asset(1000000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 1000000 / 50 /* chain defaults to 2% fees */);
      // have alice back a matching bet at 1:1 odds (also costing 1.02M)
      place_bet(alice_id, capitals_win_market.id, bet_type::back, asset(1000000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 1000000 / 50 /* chain defaults to 2% fees */);
      // place unmatched
      place_bet(alice_id, capitals_win_market.id, bet_type::back, asset(500, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 10);
      place_bet(bob_id, blackhawks_win_market.id, bet_type::lay, asset(600, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 20);

      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000000 - 1000000 - 20000 - 500 - 10);
      BOOST_CHECK_EQUAL(get_balance(bob_id, asset_id_type()), 10000000 - 1000000 - 20000 - 600 - 20);

      // cancel unmatched
      cancel_unmatched_bets(moneyline_betting_markets.id);

      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000000 - 1000000 - 20000);
      BOOST_CHECK_EQUAL(get_balance(bob_id, asset_id_type()), 10000000 - 1000000 - 20000);

   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( chained_market_create_test )
{
   // Often you will want to create several objects that reference each other at the same time.
   // To facilitate this, many of the betting market operations allow you to use "relative" object ids,
   // which let you can create, for example, an event in the 2nd operation in a transaction where the
   // event group id is set to the id of an event group created in the 1st operation in a tranasction.
   try
   {
      {
         const flat_set<witness_id_type>& active_witnesses = db.get_global_properties().active_witnesses;
         
         BOOST_TEST_MESSAGE("Creating a sport and competitors in the same proposal");
         {
            // operation 0 in the transaction
            sport_create_operation sport_create_op;
            sport_create_op.name.insert(internationalized_string_type::value_type("en", "Ice Hockey"));
            sport_create_op.name.insert(internationalized_string_type::value_type("zh_Hans", "冰球"));
            sport_create_op.name.insert(internationalized_string_type::value_type("ja", "アイスホッケー"));

            // operation 1
            event_group_create_operation event_group_create_op;
            event_group_create_op.name.insert(internationalized_string_type::value_type("en", "NHL"));
            event_group_create_op.name.insert(internationalized_string_type::value_type("zh_Hans", "國家冰球聯盟"));
            event_group_create_op.name.insert(internationalized_string_type::value_type("ja", "ナショナルホッケーリーグ"));
            event_group_create_op.sport_id = object_id_type(relative_protocol_ids, 0, 0);

            // operation 2
            // leave name and start time blank
            event_create_operation event_create_op;
            event_create_op.name = {{"en", "Washington Capitals/Chicago Blackhawks"}, {"zh_Hans", "華盛頓首都隊/芝加哥黑鷹"}, {"ja", "ワシントン・キャピタルズ/シカゴ・ブラックホークス"}};
            event_create_op.season.insert(internationalized_string_type::value_type("en", "2016-17"));
            event_create_op.event_group_id = object_id_type(relative_protocol_ids, 0, 3);

            // operation 3
            betting_market_group_create_operation betting_market_group_create_op;
            betting_market_group_create_op.description = {{"en", "Moneyline"}};
            betting_market_group_create_op.event_id = object_id_type(relative_protocol_ids, 0, 2);
            betting_market_group_create_op.asset_id = asset_id_type();

            // operation 4
            betting_market_create_operation caps_win_betting_market_create_op;
            caps_win_betting_market_create_op.group_id = object_id_type(relative_protocol_ids, 0, 3);
            caps_win_betting_market_create_op.payout_condition.insert(internationalized_string_type::value_type("en", "Washington Capitals win"));

            // operation 5
            betting_market_create_operation blackhawks_win_betting_market_create_op;
            blackhawks_win_betting_market_create_op.group_id = object_id_type(relative_protocol_ids, 0, 4);
            blackhawks_win_betting_market_create_op.payout_condition.insert(internationalized_string_type::value_type("en", "Chicago Blackhawks win"));


            proposal_create_operation proposal_op;
            proposal_op.fee_paying_account = (*active_witnesses.begin())(db).witness_account;
            proposal_op.proposed_ops.emplace_back(sport_create_op);
            proposal_op.proposed_ops.emplace_back(event_group_create_op);
            proposal_op.proposed_ops.emplace_back(event_create_op);
            proposal_op.proposed_ops.emplace_back(betting_market_group_create_op);
            proposal_op.proposed_ops.emplace_back(caps_win_betting_market_create_op);
            proposal_op.proposed_ops.emplace_back(blackhawks_win_betting_market_create_op);
            proposal_op.expiration_time =  db.head_block_time() + fc::days(1);

            signed_transaction tx;
            tx.operations.push_back(proposal_op);
            set_expiration(db, tx);
            sign(tx, init_account_priv_key);
            
            db.push_transaction(tx);
         }

         BOOST_REQUIRE_EQUAL(db.get_index_type<proposal_index>().indices().size(), 1);
         {
            const proposal_object& prop = *db.get_index_type<proposal_index>().indices().begin();

            for (const witness_id_type& witness_id : active_witnesses)
            {
               BOOST_TEST_MESSAGE("Approving sport+competitors creation from witness " << fc::variant(witness_id).as<std::string>());
               const witness_object& witness = witness_id(db);
               const account_object& witness_account = witness.witness_account(db);

               proposal_update_operation pup;
               pup.proposal = prop.id;
               pup.fee_paying_account = witness_account.id;
               //pup.key_approvals_to_add.insert(witness.signing_key);
               pup.active_approvals_to_add.insert(witness_account.id);

               signed_transaction tx;
               tx.operations.push_back( pup );
               set_expiration( db, tx );
               sign(tx, init_account_priv_key);

               db.push_transaction(tx, ~0);
               if (db.get_index_type<sport_object_index>().indices().size() == 1)
               {
                  //BOOST_TEST_MESSAGE("The sport creation operation has been approved, new sport object on the blockchain is " << fc::json::to_pretty_string(*db.get_index_type<sport_object_index>().indices().rbegin()));
                break;
               }
            }
         }

      }

   } FC_LOG_AND_RETHROW()
}


BOOST_AUTO_TEST_SUITE_END()

// set up a fixture that places a series of two matched bets, we'll use this fixture to verify
// the result in all three possible outcomes
struct simple_bet_test_fixture : database_fixture {
   betting_market_id_type capitals_win_betting_market_id;
   betting_market_id_type blackhawks_win_betting_market_id;
   betting_market_group_id_type moneyline_betting_markets_id;

   simple_bet_test_fixture()
   {
      ACTORS( (alice)(bob) );
      CREATE_ICE_HOCKEY_BETTING_MARKET();

      // give alice and bob 10k each
      transfer(account_id_type(), alice_id, asset(10000));
      transfer(account_id_type(), bob_id, asset(10000));

      // place bets at 10:1
      place_bet(alice_id, capitals_win_market.id, bet_type::back, asset(100, asset_id_type()), 11 * GRAPHENE_BETTING_ODDS_PRECISION, 2);
      place_bet(bob_id, capitals_win_market.id, bet_type::lay, asset(1000, asset_id_type()), 11 * GRAPHENE_BETTING_ODDS_PRECISION, 20);

      // reverse positions at 1:1
      place_bet(alice_id, capitals_win_market.id, bet_type::lay, asset(1100, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 22);
      place_bet(bob_id, capitals_win_market.id, bet_type::back, asset(1100, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 22);

      capitals_win_betting_market_id = capitals_win_market.id;
      blackhawks_win_betting_market_id = blackhawks_win_market.id;
      moneyline_betting_markets_id = moneyline_betting_markets.id;
   }
};

BOOST_FIXTURE_TEST_SUITE( simple_bet_tests, simple_bet_test_fixture )

BOOST_AUTO_TEST_CASE( win )
{
   try
   {
      resolve_betting_market_group(moneyline_betting_markets_id,
                                  {{capitals_win_betting_market_id, betting_market_resolution_type::win},
                                   {blackhawks_win_betting_market_id, betting_market_resolution_type::cancel}});

      GET_ACTOR(alice);
      GET_ACTOR(bob);

      uint16_t rake_fee_percentage = db.get_global_properties().parameters.betting_rake_fee_percentage;
      uint32_t rake_value;
      //rake_value = (-100 + 1100 - 1100) * rake_fee_percentage / GRAPHENE_1_PERCENT / 100;
      // alice starts with 10000, pays 100 (bet) + 2 (fee), wins 1100, then pays 1100 (bet) + 22 (fee), wins 0
      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000 - 100 - 2 + 1100 - 1100 - 22 + 0);

      rake_value = (-1000 - 1100 + 2200) * rake_fee_percentage / GRAPHENE_1_PERCENT / 100;
      // bob starts with 10000, pays 1000 (bet) + 20 (fee), wins 0, then pays 1100 (bet) + 22 (fee), wins 2200
      BOOST_TEST_MESSAGE("Rake value " +  std::to_string(rake_value));
      BOOST_CHECK_EQUAL(get_balance(bob_id, asset_id_type()), 10000 - 1000 - 20 + 0 - 1100 - 22 + 2200 - rake_value);
   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( not_win )
{
   try
   {
      resolve_betting_market_group(moneyline_betting_markets_id,
                                  {{capitals_win_betting_market_id, betting_market_resolution_type::not_win},
                                   {blackhawks_win_betting_market_id, betting_market_resolution_type::cancel}});

      GET_ACTOR(alice);
      GET_ACTOR(bob);

      uint16_t rake_fee_percentage = db.get_global_properties().parameters.betting_rake_fee_percentage;
      uint32_t rake_value = (-100 - 1100 + 2200) * rake_fee_percentage / GRAPHENE_1_PERCENT / 100;
      // alice starts with 10000, pays 100 (bet) + 2 (fee), wins 0, then pays 1100 (bet) + 22 (fee), wins 2200
      BOOST_TEST_MESSAGE("Rake value " +  std::to_string(rake_value));
      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000 - 100 - 2 + 0 - 1100 - 22 + 2200 - rake_value);

      //rake_value = (-1000 + 1100 - 1100) * rake_fee_percentage / GRAPHENE_1_PERCENT / 100;
      // bob starts with 10000, pays 1000 (bet) + 20 (fee), wins 1100, then pays 1100 (bet) + 22 (fee), wins 0
      BOOST_CHECK_EQUAL(get_balance(bob_id, asset_id_type()), 10000 - 1000 - 20 + 1100 - 1100 - 22 + 0);
   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( cancel )
{
   try
   {
      resolve_betting_market_group(moneyline_betting_markets_id,
                                  {{capitals_win_betting_market_id, betting_market_resolution_type::cancel},
                                   {blackhawks_win_betting_market_id, betting_market_resolution_type::cancel}});

      GET_ACTOR(alice);
      GET_ACTOR(bob);

      // alice and bob both start with 10000, they should end with 10000
      BOOST_CHECK_EQUAL(get_balance(alice_id, asset_id_type()), 10000);
      BOOST_CHECK_EQUAL(get_balance(bob_id, asset_id_type()), 10000);
   } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()

struct simple_bet_test_fixture_2 : database_fixture {
   betting_market_id_type capitals_win_betting_market_id;
   simple_bet_test_fixture_2() 
   {
      ACTORS( (alice)(bob) );
      CREATE_ICE_HOCKEY_BETTING_MARKET();

      // give alice and bob 10k each
      transfer(account_id_type(), alice_id, asset(10000));
      transfer(account_id_type(), bob_id, asset(10000));

      // alice backs 1000 at 1:1, matches
      place_bet(alice_id, capitals_win_market.id, bet_type::back, asset(1000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 20);
      place_bet(bob_id, capitals_win_market.id, bet_type::lay, asset(1000, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 20);

      // now alice lays at 2500 at 1:1.  This should require a deposit of 500, with the remaining 200 being funded from exposure
      place_bet(alice_id, capitals_win_market.id, bet_type::lay, asset(2500, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 50);

      // match the bet bit by bit. bob matches 500 of alice's 2500 bet.  This effectively cancels half of bob's lay position
      // so he immediately gets 500 back.  It reduces alice's back position, but doesn't return any money to her (all 2000 of her exposure
      // was already "promised" to her lay bet, so the 500 she would have received is placed in her refundable_unmatched_bets)
      place_bet(bob_id, capitals_win_market.id, bet_type::back, asset(500, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 10);

      // match another 500, which will fully cancel bob's lay position and return the other 500 he had locked up in his position.  
      // alice's back position is now canceled, 1500 remains of her unmatched lay bet, and the 500 from canceling her position has
      // been moved to her refundable_unmatched_bets
      place_bet(bob_id, capitals_win_market.id, bet_type::back, asset(500, asset_id_type()), 2 * GRAPHENE_BETTING_ODDS_PRECISION, 10);

      capitals_win_betting_market_id = capitals_win_market.id;
   }
};

//#define BOOST_TEST_MODULE "C++ Unit Tests for Graphene Blockchain Database"
#include <cstdlib>
#include <iostream>
#include <boost/test/included/unit_test.hpp>

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[]) {
    std::srand(time(NULL));
    std::cout << "Random number generator seeded to " << time(NULL) << std::endl;
    return nullptr;
}

