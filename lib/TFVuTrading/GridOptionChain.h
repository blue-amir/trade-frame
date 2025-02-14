/************************************************************************
 * Copyright(c) 2014, One Unified. All rights reserved.                 *
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
// Started 2014/09/10

#pragma once

#include <functional>

#include <wx/grid.h>

#include <TFTrading/Instrument.h>

#include <TFOptions/Option.h>

#include <TFVuTrading/GridColumnSizer.h>
#include <TFVuTrading/DragDropInstrument.h>

namespace ou { // One Unified
namespace tf { // TradeFrame

#define GRID_OPTIONCHAIN_STYLE wxWANTS_CHARS
#define GRID_OPTIONCHAIN_TITLE _("Grid Option Chain")
#define GRID_OPTIONCHAIN_IDNAME ID_GRID_OPTIONCHAIN
#define GRID_OPTIONCHAIN_SIZE wxSize(-1, -1)
#define GRID_OPTIONCHAIN_POSITION wxDefaultPosition

class GridOptionChain_impl;  // Forward Declaration

class GridOptionChain: public wxGrid {
  friend GridOptionChain_impl;
public:

  GridOptionChain();
  GridOptionChain(
    wxWindow* parent, wxWindowID id = GRID_OPTIONCHAIN_IDNAME,
    const wxPoint& pos = GRID_OPTIONCHAIN_POSITION,
    const wxSize& size = GRID_OPTIONCHAIN_SIZE,
    long style = GRID_OPTIONCHAIN_STYLE,
    const wxString& = GRID_OPTIONCHAIN_TITLE );
  virtual ~GridOptionChain(void);

  bool Create( wxWindow* parent,
    wxWindowID id = GRID_OPTIONCHAIN_IDNAME,
    const wxPoint& pos = GRID_OPTIONCHAIN_POSITION,
    const wxSize& size = GRID_OPTIONCHAIN_SIZE,
    long style = GRID_OPTIONCHAIN_STYLE,
    const wxString& = GRID_OPTIONCHAIN_TITLE );

  void Add( double strike, ou::tf::OptionSide::EOptionSide side, const std::string& sSymbol );

  void SetSelected( double strike, bool bSelected );

  void TimerActivate();
  void TimerDeactivate();

  void SaveColumnSizes( ou::tf::GridColumnSizer& ) const;
  void SetColumnSizes( ou::tf::GridColumnSizer& );

  void PreDestroy();

  struct OptionUpdateFunctions {
    std::string sSymbolName;
    fastdelegate::FastDelegate<void(const ou::tf::Quote&)> fQuote;
    fastdelegate::FastDelegate<void(const ou::tf::Trade&)> fTrade;
    fastdelegate::FastDelegate<void(const ou::tf::Greek&)> fGreek;
  };

  using fOnRowClicked_t = std::function<void(double, bool bSelected, const OptionUpdateFunctions&, const OptionUpdateFunctions& )>;
  fOnRowClicked_t m_fOnRowClicked; // called when a row is clicked (on/off)

  //using pInstrument_t = Instrument::pInstrument_t;

  using fOnOptionUnderlyingRetrieveComplete_t = DragDropInstrument::fOnOptionUnderlyingRetrieveComplete_t;
  using fOnOptionUnderlyingRetrieveInitiate_t = std::function<void(const std::string&, double, fOnOptionUnderlyingRetrieveComplete_t&&)>; // IQFeed Option Symbol, strike, completion function
  fOnOptionUnderlyingRetrieveInitiate_t m_fOnOptionUnderlyingRetrieveInitiate;  // called when DropTarget wants instrument

protected:

  void Init();
  void CreateControls();

private:
  enum {
    ID_Null=wxID_HIGHEST, ID_GRID_OPTIONCHAIN
  };

  std::unique_ptr<GridOptionChain_impl> m_pimpl;

  template<class Archive>
  void serialize(Archive & ar, const unsigned int file_version);

  void HandleSize( wxSizeEvent& event );
  void OnDestroy( wxWindowDestroyEvent& event );

  wxBitmap GetBitmapResource( const wxString& name );
  wxIcon GetIconResource( const wxString& name );
  static bool ShowToolTips() { return true; };

};

} // namespace tf
} // namespace ou
