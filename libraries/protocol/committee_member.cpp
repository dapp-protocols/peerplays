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
#include <graphene/protocol/committee_member.hpp>
#include <graphene/protocol/fee_schedule.hpp>

#include <fc/io/raw.hpp>

namespace graphene { namespace protocol {

void committee_member_create_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT(url.size() < GRAPHENE_MAX_URL_LENGTH );
}

void committee_member_update_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   if( new_url.valid() )
      FC_ASSERT(new_url->size() < GRAPHENE_MAX_URL_LENGTH );
}

void committee_member_update_global_parameters_operation::validate() const
{
   FC_ASSERT( fee.amount >= 0 );
   new_parameters.validate();
}

} } // graphene::protocol

GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::committee_member_create_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::committee_member_update_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::committee_member_update_global_parameters_operation::fee_parameters_type )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::committee_member_create_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::committee_member_update_operation )
GRAPHENE_EXTERNAL_SERIALIZATION( /*not extern*/, graphene::protocol::committee_member_update_global_parameters_operation )

