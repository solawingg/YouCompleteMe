// Copyright (C) 2011, 2012  Google Inc.
//
// This file is part of YouCompleteMe.
//
// YouCompleteMe is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// YouCompleteMe is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with YouCompleteMe.  If not, see <http://www.gnu.org/licenses/>.

#include "standard.h"
#include "Candidate.h"
#include "Result.h"
#include <cctype>
#include <boost/algorithm/string.hpp>

using boost::algorithm::all;
using boost::algorithm::is_lower;

namespace YouCompleteMe {

std::string GetWordBoundaryChars( const std::string &text ) {
  std::string result;

  for ( uint i = 0; i < text.size(); ++i ) {
    bool is_first_char_but_not_underscore = i == 0 && text[ i ] != '_';
    bool is_good_uppercase = i > 0 &&
                             IsUppercase( text[ i ] ) &&
                             !IsUppercase( text[ i - 1 ] );
    bool is_alpha_after_underscore = i > 0 &&
                                     text[ i - 1 ] == '_' &&
                                     isalpha( text[ i ] );

    if ( is_first_char_but_not_underscore ||
         is_good_uppercase ||
         is_alpha_after_underscore ) {
      result.push_back( tolower( text[ i ] ) );
    }
  }

  return result;
}


Bitset LetterBitsetFromString( const std::string &text ) {
  Bitset letter_bitset;
  foreach ( char letter, text ) {
    letter_bitset.set( IndexForChar( letter ) );
  }

  return letter_bitset;
}


Candidate::Candidate( const std::string &text )
  :
  text_( text ),
  word_boundary_chars_( GetWordBoundaryChars( text ) ),
  text_is_lowercase_( all( text, is_lower() ) ),
  letters_present_( LetterBitsetFromString( text ) )
{
}


Result Candidate::QueryMatchResult( const std::string &query,
                                    bool case_sensitive ) const {
  int index_sum = 0;
  std::string::const_iterator query_iter = query.begin(), query_end = query.end();
  if ( query_iter == query_end) 
    return Result( true, &text_, text_is_lowercase_, index_sum,
        word_boundary_chars_, query );

  int index = 0, candidate_len = text_.size();

  if (case_sensitive){
    // only case sensitive when the query char is upper
      
    // When the query letter is uppercase, then we force an uppercase match
    // but when the query letter is lowercase, then it can match both an
    // uppercase and a lowercase letter. This is by design and it's much
    // better than forcing lowercase letter matches.
    char candidate_char, query_char;
    bool query_char_not_upper = false;

    query_char = *query_iter;
    if ( !IsUppercase(query_char) ) query_char_not_upper = true;
    
    while (index < candidate_len){
      candidate_char = text_[index];
      if ( query_char_not_upper && IsUppercase(candidate_char) ) 
        candidate_char += kUpperToLowerCount;

      if ( candidate_char == query_char ){
        index_sum += index;
        ++query_iter;
        if ( query_iter == query_end ) 
          return Result( true, &text_, text_is_lowercase_, index_sum,
              word_boundary_chars_, query );

        query_char = *query_iter;
        if ( !IsUppercase(query_char) ) query_char_not_upper = true;
        else query_char_not_upper = false;
      }
      ++index;
    }
  }else{
    char candidate_char, query_char;

    query_char = *query_iter;
    if ( IsUppercase(query_char) ) query_char += kUpperToLowerCount;

    while (index < candidate_len){
      candidate_char = text_[index];
      if ( IsUppercase(candidate_char) ) candidate_char += kUpperToLowerCount;

      if (candidate_char == query_char){
        index_sum += index;
        ++query_iter;
        if ( query_iter == query_end ) 
          return Result( true, &text_, text_is_lowercase_, index_sum,
              word_boundary_chars_, query );

        query_char = *query_iter;
        if ( IsUppercase(query_char) ) query_char += kUpperToLowerCount;
      }
      ++index;
    }
  }
  return Result(false);
}

} // namespace YouCompleteMe
