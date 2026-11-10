#ifndef HistoSetBase_h
#define HistoSetBase_h 1

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <stdexcept>
#include <vector>

#include "TFile.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TLorentzVector.h"
#include "TH1.h"
#include "TH2.h"

/*
  HistCollection {sample name, e.g.) NNLO_MUMU_inc}
  |-- EventInfo
  |   |-- TH1D histos
  |-- GenInfo
  |   |-- TH1D histos
  |-- RegionCollection {OS, SS, OS_inverted, SS_inverted}
  |   |-- JetCollection {inc, 0J, 1J, mt1J, bVeto_0J, bVeto_1J, bVeto_mt1J}
  |   |   |-- MassBinCollection {m200_220, ... }
  |   |   |   |-- ObservableCollection {nJet, MuonPt, ...}
  |   |   |   |   |-- histogram (TH1 -> TH1D or TH2D)
*/

// key: observable, value: histogram
using ObservableCollection = std::map<TString, std::unique_ptr<TH1>>;
// key: massbin, value: Observable
using MassBinCollection = std::map<TString, ObservableCollection>;
// key: Jet case, value: MassBin
using JetCollection = std::map<TString, MassBinCollection>;
// key: region, value: JetCollection
using RegionCollection = std::map<TString, JetCollection>;

struct SampleCollection {
  ObservableCollection fEventInfo;
  ObservableCollection fGenInfo;
  RegionCollection fRegionCollection;
};

// key: sample, value: SampleCollection
using HistCollection = std::map<TString, SampleCollection>;

class HistoSetBase
{
public:
  enum class HistGroup {
    EventInfo,
    GenInfo
  };

  HistoSetBase(TString fEra_, TString fSampleName_)
  : fEra(fEra_), fSampleName(fSampleName_), fAddMassExclusive(false) {

    fPtBins = {-9999, 300, 0., 1500.};
    fEtaBins = {-9999, 60, -3., 3.};
    fPhiBins = {-9999, 60, -3.141593, 3.141593};
    fMassBins = {199, 200,  220,  243, 273, 320, 380, 440, 510, 600, 700, 830, 1000, 1500, 4000, 4001};
    fDeltaRBins = {-9999, 100, 0.0, 6.0};
    fNJetBins = {-9999, 20, 0, 20};
    fChargeBins = {-9999, 2, -1, 1};

    if (fSampleName.Contains("NNLO_MUMU_10to50"))
      fSampleName = "NNLO_MUMU_10to50";
  }

  virtual ~HistoSetBase() = default;

  virtual std::vector<double> GetBinning(std::string name) = 0;

  /**
    * \brief Filling the muon related histograms 
    *
    * \param fRegionName: region name {OS, SS, OS_inverted, SS_inverted}
    * \param fJetBin: jet bin {inc, 0J, 1J, mt1J, bVeto, bVeto_0J, bVeto_1J, bVeto_mt1J}
    * \param fMassBin: mass bin {m200_220, ... }
    * \param fObservable: observable name {nJet, MuonPt, ...}
    * \param fHistoName: Name of the histogram
    * \param fBins: binning definition in std::vector<double>
    * 
    * \throws No exception is thrown for this function
    */
  void BookHisto(
    TString fRegionName,
    TString fJetBin,
    TString fMassBin,
    TString fObservable,
    TString fHistoName
  );
  void BookHisto(
    TString fRegionName,
    TString fJetBin,
    TString fMassBin,
    TString fObservable,
    TString fHistoName,
    std::vector<double> fBins
  );

  /**
    * \brief Booking histogram for event info or gen info
    *
    * \param fHistGroup: flag between event info and gen info
    * \param fObservable: observable name {nJet, MuonPt, ...}
    * \param fHistoName: Name of the histogram
    * \param fBins: binning definition in std::vector<double>
    *
    * \throws No exception is thrown for this function
    */
  void BookHisto(
    HistGroup fHistGroup,
    TString fObservable,
    TString fHistoName,
    std::vector<double> fBins
  );

  void BookHisto(
    HistGroup fHistGroup,
    TString fObservable,
    TH2D* fHisto
  );


  /**
    * \brief Filling the histogram
    *
    * \param fRegionName: region name {OS, SS, OS_inverted, SS_inverted}
    * \param fJetBin: jet bin {inc, 0J, 1J, mt1J, bVeto, bVeto_0J, bVeto_1J, bVeto_mt1J}
    * \param fMassBin: mass bin {m200_220, ... }
    * \param fObservable: observable name {nJet, MuonPt, ...}
    * \param fValue: value of the observable
    * \param fWeight: weight of the event
    *
    * \throws No exception is thrown for this function
    */
  void FillHisto(
    TString fRegionName,
    TString fJetBin,
    TString fMassBin,
    TString fObservable,
    double fValue,
    double fWeight
  );

  /**
    * \brief Filling the histogram for event info or gen info
    *
    * \param fHistGroup: flag between event info and gen info
    * \param fObservable: observable name {nJet, MuonPt, ...}
    * \param fValue: value of the observable
    * \param fWeight: weight of the event
    *
    * \throws No exception is thrown for this function
    */
  void FillHisto(
    HistGroup fHistGroup,
    TString fObservable,
    double fValue,
    double fWeight
  );

  /**
    * \brief Filling the 2D histrogram
    *
    * \param fHistGroup: flag between event info and gen info
    * \param fObservable: observable name {nJet, MuonPt, ...}
    * \param fXValue: value of the x-axis
    * \param fYValue: value of the y-axis
    * \param fWeight: weight of the event
    *
    * \throws No exception is thrown for this function
    */
  void FillHisto(
    HistGroup fHistGroup,
    TString fObservable,
    double fXValue,
    double fYValue,
    double fWeight
  );

  /**
    * \brief Writing the histogram to the output file
    *
    * \param fOutputFile: output file
    * \param fIsData: flag to write data histogram
    *
    * \throws No exception is thrown for this function
    */
  void WriteHisto(
    TFile* fOutputFile,
    bool fIsData
  );

  /**
    * \brief Turn of the flag to add mass exclusive histogram
    */
  void SetMassExclusive() { 
    fAddMassExclusive = true; 
  }
  bool GetMassExclusive() { return fAddMassExclusive; }

  TString GetSampleName() { return fSampleName; }
  TString GetEra() { return fEra; }

protected:
  HistCollection fHistSet;

  std::vector<double> fPtBins;
  std::vector<double> fEtaBins;
  std::vector<double> fPhiBins;
  std::vector<double> fMassBins;
  std::vector<double> fDeltaRBins;
  std::vector<double> fNJetBins;
  std::vector<double> fChargeBins;

private:

  TString fEra;
  TString fSampleName;

  bool fAddMassExclusive;
};

#endif
