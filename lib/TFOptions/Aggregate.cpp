/************************************************************************
 * Copyright(c) 2021, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

/*
 * File:    Aggregate.cpp
 * Author:  raymond@burkholder.net
 * Project: TFOptions
 * Created on May 29, 2021, 20:09
 */

#include <stdexcept>

#include "Aggregate.h"

namespace ou { // One Unified
namespace tf { // TradeFrame
namespace option { // options

Aggregate::Aggregate(
  pWatch_t pWatchUnderlying
)
: m_pWatchUnderlying( pWatchUnderlying )
{
}

void Aggregate::LoadChains( fGatherOptions_t&& fGatherOptions ) {
  std::cout << "Aggregate::LoadChains" << std::endl;
  fGatherOptions(
    m_pWatchUnderlying->GetInstrument()->GetInstrumentName( ou::tf::Instrument::eidProvider_t::EProviderIQF ),
    [this]( pOption_t pOption ){

      mapChains_iterator_t iterChains;

      const std::string& sInstrumentName( pOption->GetInstrumentName() );
      const std::string& sIQFeedSymbolName( pOption->GetInstrument()->GetInstrumentName( ou::tf::Instrument::eidProvider_t::EProviderIQF ) );

      { // find existing expiry, or create new one

        chain_t chain;

        const boost::gregorian::date expiry( pOption->GetExpiry() );

        iterChains = m_mapChains.find( expiry ); // see if expiry date exists
        if ( m_mapChains.end() == iterChains ) { // insert new expiry set if not
          std::cout
            << "Aggregate chain: "
            << ou::tf::Instrument::BuildDate( expiry )
            << "," << sIQFeedSymbolName
            << "," << sInstrumentName
            << std::endl;
          iterChains = m_mapChains.insert(
            m_mapChains.begin(),
             mapChains_t::value_type( expiry, std::move( chain ) )
            );
        }
      }

      { // populate new call or put, no test for pre-existance

        //std::cout << "  option: " << row.sSymbol << std::endl;

        chain_t& chain( iterChains->second );
        chain_t::strike_t& strike( chain.GetStrike( pOption->GetStrike() ) );

        try {
          switch ( pOption->GetOptionSide() ) {
            case ou::tf::OptionSide::Call:
              try {
                chain.SetIQFeedNameCall( pOption->GetStrike(), sIQFeedSymbolName );
                strike.call.pOption = pOption;
              }
              catch ( std::runtime_error& e ) {
                // don't do anything with the duplicate
              }
              break;
            case ou::tf::OptionSide::Put:
              try {
                chain.SetIQFeedNamePut( pOption->GetStrike(), sIQFeedSymbolName );
                strike.put.pOption = pOption;
              }
              catch( std::runtime_error& e ) {
                // don't do anything with the duplicate
              }
              break;
          }
        }
        catch ( std::runtime_error& e ) {
          std::cout << "LoadChains::fGatherOptions error" << std::endl;
        }
      }
    }
  );
}

void Aggregate::FilterChains() {

  assert( 0 < m_mapChains.size() );

  std::vector<boost::gregorian::date> vChainsToBeRemoved;

  size_t nStrikesSum {};
  for ( const mapChains_t::value_type& vt: m_mapChains ) { // only use chains where all calls/puts available

    size_t nStrikesTotal {};
    size_t nStrikesMatch {};

    std::vector<double> vMisMatch;
    vMisMatch.reserve( 10 );

    vt.second.Strikes(
      [&nStrikesTotal,&nStrikesMatch,&vMisMatch]( double strike, const chain_t::strike_t& options ){
        nStrikesTotal++;
        if ( options.call.sIQFeedSymbolName.empty() || options.put.sIQFeedSymbolName.empty() ) {
          vMisMatch.push_back( strike );
        }
        else {
          nStrikesMatch++;
        }
    } );

    const std::string sStrikeDate( ou::tf::Instrument::BuildDate( vt.first ) );

    bool bChainAdded( true );
    if ( nStrikesTotal == nStrikesMatch ) {
      nStrikesSum += nStrikesTotal;
      std::cout << "chain " << sStrikeDate << " added with " << nStrikesTotal << " strikes" << std::endl;
    }
    else {
      if ( 0 == nStrikesMatch ) {
        std::cout << "chain " << sStrikeDate << " skipped with " << nStrikesMatch << '/' << nStrikesTotal << " strikes" << std::endl;
        vChainsToBeRemoved.push_back( vt.first );
        bChainAdded = false;
      }
      else {
        std::cout
          << "chain " << sStrikeDate << " added " << nStrikesMatch << " strikes without";
        for ( double strike: vMisMatch ) {
          std::cout << " " << strike;
          const_cast<chain_t&>( vt.second ).Erase( strike );
        }
        std::cout << std::endl;
        assert( 0 < vt.second.Size() );
      }
    }
  }

  assert( vChainsToBeRemoved.size() != m_mapChains.size() );
  for ( auto date: vChainsToBeRemoved ) { // remove chains with incomplete info
    mapChains_t::iterator iter = m_mapChains.find( date );
    m_mapChains.erase( iter );
  }

  size_t nAverageStrikes = nStrikesSum / m_mapChains.size();
  std::cout << "chain size average: " << nAverageStrikes << std::endl;

}

void Aggregate::WalkChains( fDate_t&& fDate ) const {
  for ( const mapChains_t::value_type& vt: m_mapChains ) {
    fDate( vt.first );
  }
}

void Aggregate::WalkChains( fOption_t&& fOption ) const {
  for ( const mapChains_t::value_type& vt: m_mapChains ) {
    const chain_t& chain( vt.second );
    chain.Strikes(
      [ fOption ]( double strike, const chain_t::strike_t& options ){
        if ( options.call.pOption ) fOption( options.call.pOption );
        if ( options.put.pOption ) fOption( options.put.pOption );
      });
  }
}

void Aggregate::WalkChain( boost::gregorian::date date, fOption_t&& fOption ) const {
  mapChains_t::const_iterator iter = m_mapChains.find( date );
  if ( m_mapChains.end() == iter ) {
    throw std::runtime_error( "Aggregate::WalkChain: date not found" );
  }
  else {
    iter->second.Strikes(
      [ fOption = std::move( fOption ) ]( double strike, const chain_t::strike_t& options ){
        fOption( options.call.pOption );
        fOption( options.put.pOption );
      } );
  }
}


} // namespace option
} // namespace tf
} // namespace ou
