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
#pragma once
#include <graphene/protocol/base.hpp>
#include <graphene/protocol/account.hpp>
#include <graphene/protocol/affiliate.hpp>
#include <graphene/protocol/assert.hpp>
#include <graphene/protocol/asset_ops.hpp>
#include <graphene/protocol/lottery_ops.hpp>
#include <graphene/protocol/balance.hpp>
#include <graphene/protocol/custom.hpp>
#include <graphene/protocol/committee_member.hpp>
#include <graphene/protocol/confidential.hpp>
#include <graphene/protocol/fba.hpp>
#include <graphene/protocol/market.hpp>
#include <graphene/protocol/proposal.hpp>
#include <graphene/protocol/transfer.hpp>
#include <graphene/protocol/vesting.hpp>
#include <graphene/protocol/withdraw_permission.hpp>
#include <graphene/protocol/witness.hpp>
#include <graphene/protocol/worker.hpp>
#include <graphene/protocol/sport.hpp>
#include <graphene/protocol/event_group.hpp>
#include <graphene/protocol/event.hpp>
#include <graphene/protocol/betting_market.hpp>
#include <graphene/protocol/tournament.hpp>
#include <graphene/protocol/custom_permission.hpp>
#include <graphene/protocol/custom_account_authority.hpp>
#include <graphene/protocol/offer.hpp>
#include <graphene/protocol/nft_ops.hpp>
#include <graphene/protocol/nft_lottery.hpp>
#include <graphene/protocol/account_role.hpp>
#include <graphene/protocol/son.hpp>
#include <graphene/protocol/sidechain_address.hpp>
#include <graphene/protocol/son_wallet.hpp>
#include <graphene/protocol/son_wallet_deposit.hpp>
#include <graphene/protocol/son_wallet_withdraw.hpp>
#include <graphene/protocol/sidechain_transaction.hpp>
#include <graphene/protocol/random_number.hpp>

namespace graphene { namespace protocol {

   /**
    * @ingroup operations
    *
    * Defines the set of valid operations as a discriminated union type.
    */
   typedef fc::static_variant<
            transfer_operation,
            limit_order_create_operation,
            limit_order_cancel_operation,
            call_order_update_operation,
            fill_order_operation,           // VIRTUAL
            account_create_operation,
            account_update_operation,
            account_whitelist_operation,
            account_upgrade_operation,
            account_transfer_operation,
            asset_create_operation,
            asset_update_operation,
            asset_update_bitasset_operation,
            asset_update_feed_producers_operation,
            asset_issue_operation,
            asset_reserve_operation,
            asset_fund_fee_pool_operation,
            asset_settle_operation,
            asset_global_settle_operation,
            asset_publish_feed_operation,
            witness_create_operation,
            witness_update_operation,
            proposal_create_operation,
            proposal_update_operation,
            proposal_delete_operation,
            withdraw_permission_create_operation,
            withdraw_permission_update_operation,
            withdraw_permission_claim_operation,
            withdraw_permission_delete_operation,
            committee_member_create_operation,
            committee_member_update_operation,
            committee_member_update_global_parameters_operation,
            vesting_balance_create_operation,
            vesting_balance_withdraw_operation,
            worker_create_operation,
            custom_operation,
            assert_operation,
            balance_claim_operation,
            override_transfer_operation,
            transfer_to_blind_operation,
            blind_transfer_operation,
            transfer_from_blind_operation,
            asset_settle_cancel_operation,  // VIRTUAL
            asset_claim_fees_operation,
            fba_distribute_operation,        // VIRTUAL
            tournament_create_operation,
            tournament_join_operation,
            game_move_operation,
            asset_update_dividend_operation,
            asset_dividend_distribution_operation, // VIRTUAL
            tournament_payout_operation, // VIRTUAL
            tournament_leave_operation,
            sport_create_operation,
            sport_update_operation,
            event_group_create_operation,
            event_group_update_operation,
            event_create_operation,
            event_update_operation,
            betting_market_rules_create_operation,
            betting_market_rules_update_operation,
            betting_market_group_create_operation,
            betting_market_create_operation,
            bet_place_operation,
            betting_market_group_resolve_operation,
            betting_market_group_resolved_operation, // VIRTUAL
            bet_adjusted_operation, // VIRTUAL
            betting_market_group_cancel_unmatched_bets_operation,
            bet_matched_operation, // VIRTUAL
            bet_cancel_operation,
            bet_canceled_operation, // VIRTUAL
            betting_market_group_update_operation,
            betting_market_update_operation,
            event_update_status_operation,
            sport_delete_operation,
            event_group_delete_operation,
            affiliate_payout_operation, // VIRTUAL
            affiliate_referral_payout_operation, // VIRTUAL
            lottery_asset_create_operation,
            ticket_purchase_operation,
            lottery_reward_operation,
            lottery_end_operation,
            sweeps_vesting_claim_operation,
            custom_permission_create_operation,
            custom_permission_update_operation,
            custom_permission_delete_operation,
            custom_account_authority_create_operation,
            custom_account_authority_update_operation,
            custom_account_authority_delete_operation,
            offer_operation,
            bid_operation,
            cancel_offer_operation,
            finalize_offer_operation,
            nft_metadata_create_operation,
            nft_metadata_update_operation,
            nft_mint_operation,
            nft_safe_transfer_from_operation,
            nft_approve_operation,
            nft_set_approval_for_all_operation,
            account_role_create_operation,
            account_role_update_operation,
            account_role_delete_operation,
            son_create_operation,
            son_update_operation,
            son_deregister_operation,
            son_heartbeat_operation,
            son_report_down_operation,
            son_maintenance_operation,
            son_wallet_recreate_operation,
            son_wallet_update_operation,
            son_wallet_deposit_create_operation,
            son_wallet_deposit_process_operation,
            son_wallet_withdraw_create_operation,
            son_wallet_withdraw_process_operation,
            sidechain_address_add_operation,
            sidechain_address_update_operation,
            sidechain_address_delete_operation,
            sidechain_transaction_create_operation,
            sidechain_transaction_sign_operation,
            sidechain_transaction_send_operation,
            sidechain_transaction_settle_operation,
            nft_lottery_token_purchase_operation,
            nft_lottery_reward_operation,
            nft_lottery_end_operation,
            random_number_store_operation
         > operation;

   /// @} // operations group

   /**
    *  Appends required authorites to the result vector.  The authorities appended are not the
    *  same as those returned by get_required_auth 
    *
    *  @return a set of required authorities for @ref op
    */
   void operation_get_required_authorities( const operation& op,
                                            flat_set<account_id_type>& active,
                                            flat_set<account_id_type>& owner,
                                            vector<authority>& other,
                                            bool ignore_custom_operation_required_auths );

   void operation_validate( const operation& op );

   /**
    *  @brief necessary to support nested operations inside the proposal_create_operation
    */
   struct op_wrapper
   {
      public:
         op_wrapper(const operation& op = operation()):op(op){}
         operation op;
   };

} } // graphene::protocol

FC_REFLECT_TYPENAME( graphene::protocol::operation )
FC_REFLECT( graphene::protocol::op_wrapper, (op) )

GRAPHENE_EXTERNAL_SERIALIZATION( extern, graphene::protocol::op_wrapper )

