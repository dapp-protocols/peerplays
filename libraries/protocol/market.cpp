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
#include <fc/io/raw.hpp>
#include <graphene/protocol/market.hpp>

#include <fc/io/raw.hpp>

namespace graphene { namespace protocol {

void limit_order_create_operation::validate()const
{
   FC_ASSERT( amount_to_sell.asset_id != min_to_receive.asset_id );
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( amount_to_sell.amount > 0 );
   FC_ASSERT( min_to_receive.amount > 0 );
}

void limit_order_cancel_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
}

void call_order_update_operation::validate()const
{ try {
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( delta_collateral.asset_id != delta_debt.asset_id );
   FC_ASSERT( delta_collateral.amount != 0 || delta_debt.amount != 0 );
} FC_CAPTURE_AND_RETHROW((*this)) }

} } // graphene::protocol

GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::limit_order_create_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::limit_order_cancel_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::call_order_update_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::limit_order_create_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::limit_order_cancel_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::call_order_update_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::fill_order_operation )

