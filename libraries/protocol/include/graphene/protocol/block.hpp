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
#include <graphene/protocol/transaction.hpp>

namespace graphene { namespace protocol {

   struct block_header
   {
      digest_type                   digest()const;
      block_id_type                 previous;
      uint32_t                      block_num()const { return num_from_id(previous) + 1; }
      fc::time_point_sec            timestamp;
      witness_id_type               witness;
      secret_hash_type              next_secret_hash;
      secret_hash_type              previous_secret;
      checksum_type                 transaction_merkle_root;
      extensions_type               extensions;

      static uint32_t num_from_id(const block_id_type& id);
   };

   struct signed_block_header : public block_header
   {
      block_id_type              id()const;
      fc::ecc::public_key        signee()const;
      void                       sign( const fc::ecc::private_key& signer );
      bool                       validate_signee( const fc::ecc::public_key& expected_signee )const;

      signature_type             witness_signature;
   };

   struct signed_block : public signed_block_header
   {
      checksum_type calculate_merkle_root()const;
      vector<processed_transaction> transactions;
   };

} } // graphene::protocol

FC_REFLECT( graphene::protocol::block_header, 
            (previous)
            (timestamp)
            (witness)
            (next_secret_hash)
            (previous_secret)
            (transaction_merkle_root)
            (extensions) )
FC_REFLECT_DERIVED( graphene::protocol::signed_block_header, (graphene::protocol::block_header), (witness_signature) )
FC_REFLECT_DERIVED( graphene::protocol::signed_block, (graphene::protocol::signed_block_header), (transactions) )


GRAPHENE_EXTERNAL_SERIALIZATION(extern, graphene::protocol::block_header)
GRAPHENE_EXTERNAL_SERIALIZATION(extern, graphene::protocol::signed_block_header)
GRAPHENE_EXTERNAL_SERIALIZATION(extern, graphene::protocol::signed_block)

