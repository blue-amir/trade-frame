/************************************************************************
 * Copyright(c) 2023, One Unified. All rights reserved.                 *
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
 * File:    PanelComboOrder.cpp
 * Author:  raymond@burkholder.net
 * Project: TFVuTrading
 * Created: April 10, 2023 13:10:29
 */

#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/tglbtn.h>
#include <wx/listbook.h>

#include "PanelComboOrder.hpp"

namespace ou { // One Unified
namespace tf { // TradeFrame

PanelComboOrder::PanelComboOrder(): wxPanel() {
  Init();
}

PanelComboOrder::PanelComboOrder( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name )
: wxPanel()
{
  Init();
  Create(parent, id, pos, size, style, name );
}

PanelComboOrder::~PanelComboOrder() {
}

void PanelComboOrder::Init() {

  m_pBookOptionChains = nullptr;
  m_pGridComboOrder = nullptr;
  m_btnUpgdateGreeks = nullptr;
  m_btnClearOrder = nullptr;
  m_btnPlaceOrder = nullptr;

  m_fOnPageChanged = nullptr;
  m_fOnPageChanging = nullptr;
}

bool PanelComboOrder::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {

////@begin PanelComboOrder creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    //Centre();
////@end PanelComboOrder creation
    return true;
}

void PanelComboOrder::CreateControls() {

    PanelComboOrder* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_pBookOptionChains = new wxListbook( itemPanel1, ID_BOOK_OptionChains, wxDefaultPosition, wxDefaultSize, wxBK_LEFT|wxNO_BORDER );

    itemBoxSizer2->Add(m_pBookOptionChains, 1, wxGROW|wxALL, 1);

    m_pGridComboOrder = new wxGrid( itemPanel1, ID_GRID_ComboOrder, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxVSCROLL );
    m_pGridComboOrder->SetDefaultColSize(50);
    m_pGridComboOrder->SetDefaultRowSize(25);
    m_pGridComboOrder->SetColLabelSize(25);
    m_pGridComboOrder->SetRowLabelSize(50);
    m_pGridComboOrder->CreateGrid(5, 6, wxGrid::wxGridSelectRows);
    itemBoxSizer2->Add(m_pGridComboOrder, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    m_btnUpgdateGreeks = new wxToggleButton( itemPanel1, ID_BTN_UpdateGreeks, _("Update Greeks"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnUpgdateGreeks->SetValue(false);
    itemBoxSizer4->Add(m_btnUpgdateGreeks, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_btnClearOrder = new wxButton( itemPanel1, ID_BTN_ClearOrder, _("Clear Order"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_btnClearOrder, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_btnPlaceOrder = new wxButton( itemPanel1, ID_BTN_PlaceOrder, _("Place Order"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnPlaceOrder->SetDefault();
    itemBoxSizer4->Add(m_btnPlaceOrder, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

  BindEvents();
}

void PanelComboOrder::BindEvents() {
  // Page Change events cause issues during OnDestroy
  Bind( wxEVT_LISTBOOK_PAGE_CHANGING, &PanelComboOrder::OnBOOKOptionChainsPageChanging, this );
  Bind( wxEVT_LISTBOOK_PAGE_CHANGED, &PanelComboOrder::OnBOOKOptionChainsPageChanged, this );
  Bind( wxEVT_DESTROY, &PanelComboOrder::OnDestroy, this );
}

void PanelComboOrder::UnbindEvents() {
  // Page change events occur during Deletion of Pages, causing problems
  assert( Unbind( wxEVT_DESTROY, &PanelComboOrder::OnDestroy, this ) );
  assert( Unbind( wxEVT_LISTBOOK_PAGE_CHANGING, &PanelComboOrder::OnBOOKOptionChainsPageChanging, this ) );
  assert( Unbind( wxEVT_LISTBOOK_PAGE_CHANGED, &PanelComboOrder::OnBOOKOptionChainsPageChanged, this ) );
}

// add specific put/call-at-strike pair to Notebook of OptionChaines
void PanelComboOrder::Add( boost::gregorian::date date, double strike, ou::tf::OptionSide::EOptionSide side, const std::string& sSymbol ) {

  mapOptionExpiry_t::iterator iterExpiry = m_mapOptionExpiry.find( date );

  if ( m_mapOptionExpiry.end() == iterExpiry ) {
    // add another panel
    std::string sDate = boost::lexical_cast<std::string>( date.year() );
    sDate += std::string( "/" )
      + ( date.month().as_number() < 10 ? "0" : "" )
      + boost::lexical_cast<std::string>( date.month().as_number() );
    sDate += std::string( "/" ) + ( date.day()   < 10 ? "0" : "" ) + boost::lexical_cast<std::string>( date.day() );

    auto* pPanel = new wxPanel( this, wxID_ANY );
    auto* pSizer = new wxBoxSizer(wxVERTICAL);
    pPanel->SetSizer( pSizer );
    auto* pGridOptionChain = new GridOptionChain( pPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, sSymbol );
    pSizer->Add( pGridOptionChain, 1, wxALL|wxEXPAND, 1 );

    // a control right click will signal through that strike should watch/unwatch
    // TODO: maybe the signal through should return a boolean of whether it turned out to be watch or unwatch
    pGridOptionChain->m_fOnRowClicked =
      [this, date](double strike, bool bSelected, const GridOptionChain::OptionUpdateFunctions& funcsCall, const GridOptionChain::OptionUpdateFunctions& funcsPut  ){
        if ( nullptr != m_fOnRowClicked) {
          m_fOnRowClicked( date, strike, bSelected, funcsCall, funcsPut );
        }
    };
    pGridOptionChain->m_fOnOptionUnderlyingRetrieveInitiate =
      [this, date]( const std::string& sIQFeedOptionName, double strike, GridOptionChain::fOnOptionUnderlyingRetrieveComplete_t&& f ){
        if ( nullptr != m_fOnOptionUnderlyingRetrieve ) {
          m_fOnOptionUnderlyingRetrieve(sIQFeedOptionName, date, strike, f );
        }
    };

    auto pair = m_mapOptionExpiry.emplace( mapOptionExpiry_t::value_type( date, Tab( sDate, pPanel, pGridOptionChain ) ) );
    assert( pair.second );

    struct Reindex {
      size_t ix;
      Reindex(): ix{} {}
      void operator()( Tab& tab ) { tab.ixTab = ix; ix++; }
    };

    // renumber the pages
    Reindex reindex;
    std::for_each(
      m_mapOptionExpiry.begin(), m_mapOptionExpiry.end(),
        [&reindex](mapOptionExpiry_t::value_type& v){ reindex( v.second ); } );

    m_pBookOptionChains->InsertPage( iterExpiry->second.ixTab, pPanel, sDate );

    m_pBookOptionChains->SetSelection( 0 );
  } // end add panel

  mapStrike_t& mapStrike( iterExpiry->second.mapStrike ); // assumes single thread
  mapStrike_t::iterator iterStrike = mapStrike.find( strike );

  if ( mapStrike.end() == iterStrike ) {
    iterStrike = mapStrike.insert( mapStrike.begin(), mapStrike_t::value_type( strike, Row( mapStrike.size() ) ) );
  }

  switch ( side ) {
    case ou::tf::OptionSide::Call:
      assert( "" == iterStrike->second.sCall );
      iterStrike->second.sCall = sSymbol;
      break;
    case ou::tf::OptionSide::Put:
      assert( "" == iterStrike->second.sPut );
      iterStrike->second.sPut = sSymbol;
      break;
  }

  // add option set to the expiry panel
  iterExpiry->second.pWinOptionChain->Add( strike, side, sSymbol );

}

void PanelComboOrder::OnDestroy( wxWindowDestroyEvent& event ) {

  UnbindEvents();

  std::for_each(
    m_mapOptionExpiry.begin(), m_mapOptionExpiry.end(),
    [](mapOptionExpiry_t::value_type& value){
     value.second.pWinOptionChain->PreDestroy();
     value.second.pWinOptionChain->Destroy();
     value.second.pPanel->Destroy();
   });

  //DeleteAllPages();
  while ( 0 != m_pBookOptionChains->GetPageCount() ) {
   //DeletePage( 0 );
   m_pBookOptionChains->RemovePage( 0 );
  }

  event.Skip();
}

void PanelComboOrder::OnBOOKOptionChainsPageChanging( wxListbookEvent& event ) {
  int ixTab = event.GetOldSelection();
  if ( -1 != ixTab ) {
    //std::cout << "page changing: " << ixTab << std::endl;
    mapOptionExpiry_t::iterator iter
     = std::find_if( m_mapOptionExpiry.begin(), m_mapOptionExpiry.end(), [ixTab,this](mapOptionExpiry_t::value_type& vt) {
       return ixTab == vt.second.ixTab;
    });
    if ( m_mapOptionExpiry.end() == iter ) {
      std::cout << "PanelComboOrder::OnPageChanging: couldn't find tab index: " << ixTab << std::endl;
    }
    else {
      iter->second.pWinOptionChain->TimerDeactivate();
      if ( nullptr != m_pgcsGridOptionChain ) {
        iter->second.pWinOptionChain->SaveColumnSizes( *m_pgcsGridOptionChain );
      }
      if ( nullptr != m_fOnPageChanging ) {
        m_fOnPageChanging( iter->first );
      }
    }
  }
  event.Skip();
}

void PanelComboOrder::OnBOOKOptionChainsPageChanged( wxListbookEvent& event ) {
  int ixTab = event.GetSelection();
  //std::cout << "page changed: " << ixTab << std::endl;
  mapOptionExpiry_t::iterator iter
    = std::find_if( m_mapOptionExpiry.begin(), m_mapOptionExpiry.end(), [ixTab,this](mapOptionExpiry_t::value_type& vt) {
      return ixTab == vt.second.ixTab;
      });
  if ( m_mapOptionExpiry.end() == iter ) {
    std::cout << "PanelComboOrder::OnPageChanged: couldn't find tab index: " << ixTab << std::endl;
  }
  else {
    if ( nullptr != m_pgcsGridOptionChain ) {
      iter->second.pWinOptionChain->SetColumnSizes( *m_pgcsGridOptionChain );
    }
    iter->second.pWinOptionChain->TimerActivate();
    if ( nullptr != m_fOnPageChanged ) {
      m_fOnPageChanged( iter->first );
    }
  }
  event.Skip();
}

void PanelComboOrder::OnBTNUpdateGreeksClick( wxCommandEvent& event ) {
  event.Skip();
}

void PanelComboOrder::OnBTNClearOrderClick( wxCommandEvent& event ) {
  event.Skip();
}

void PanelComboOrder::OnBTNPlaceOrderClick( wxCommandEvent& event ) {
  event.Skip();
}

void PanelComboOrder::SetGridOptionChain_ColumnSaver( ou::tf::GridColumnSizer* pgcs ) {
  m_pgcsGridOptionChain = pgcs;
  int ixTab = m_pBookOptionChains->GetSelection();
  mapOptionExpiry_t::iterator iter
    = std::find_if( m_mapOptionExpiry.begin(), m_mapOptionExpiry.end(), [ixTab,this](mapOptionExpiry_t::value_type& vt) {
      return ixTab == vt.second.ixTab;
      });
  if ( m_mapOptionExpiry.end() == iter ) {
    std::cout << "PanelComboOrder::SetGridOptionChain_ColumnSaver: couldn't find tab index: " << ixTab << std::endl;
  }
  else {
    if ( nullptr != m_pgcsGridOptionChain ) {
      iter->second.pWinOptionChain->SetColumnSizes( *m_pgcsGridOptionChain );
    }
  }
}

wxBitmap PanelComboOrder::GetBitmapResource( const wxString& name ) {
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon PanelComboOrder::GetIconResource( const wxString& name ) {
    wxUnusedVar(name);
    return wxNullIcon;
}


} // namespace tf
} // namespace ou
