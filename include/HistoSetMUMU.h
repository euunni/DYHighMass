#ifndef HistoSetMUMU_h
#define HistoSetMUMU_h 1

#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

#include "jet.h"
#include "muon.h"

#include "TFile.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TChain.h"
#include "TLorentzVector.h"
#include "TUnfoldBinning.h"
#include "HistoSetBase.h"
#include "TH1.h"
#include "TH2.h"

class HistoSetMUMU : public HistoSetBase
{
public:
  HistoSetMUMU(TString fEra_, TString fSampleName_) : HistoSetBase(fEra_, fSampleName_) {
    // SetMassExclusive();
    Init();
    if (GetSampleName().Contains("NNLO_MUMU")) InitGenInfo();
  }

  void Init();
  void InitGenInfo();

  /**
    * \brief Get binning definition
    *
    * \param name: target binning name
    *
    * \throws No exception is thrown for this function, return {0.} when search failed
    */
  std::vector<double> GetBinning(std::string name) override {

    if (name.find("Pt") != std::string::npos) return fPtBins;
    else if (name.find("Eta") != std::string::npos || name.find("Rap") != std::string::npos) return fEtaBins;
    else if (name.find("Phi") != std::string::npos) return fPhiBins;
    else if (name.find("Mass") != std::string::npos) return fMassBins;
    else if (name.find("DeltaR") != std::string::npos) return fDeltaRBins;
    else if (name.find("nJet") != std::string::npos || name.find("nBJet") != std::string::npos) return fNJetBins;
    else if (name.find("Charge") != std::string::npos) return fChargeBins;
    else return {};
  }

  /**
    * \brief Filling the muon related histograms 
    *
    * \param fLeadingMuon: 4-vector of the leading muon
    * \param fSubleadingMuon: 4-vector of the subleading muon
    * \param fCharge: charge of the dimuon (eg. OS > 0, SS < 0)
    * \param nJet: number of jets
    * \param nBJet: number of b-jets
    * \param weight: weight of the event
    * \param fType: type of muon pair event (eg. "OS", "SS", "OS_inverted", "SS_inverted")
    *
    * \throws No exception is thrown for this function
    */
  void FillMuon(
    const TLorentzVector& fLeadingMuon, 
    const TLorentzVector& fSubleadingMuon, 
    const float& tCharge, 
    const int& nJet, 
    const int& nBJet, 
    const double& weight,
    const std::string& fType,
    const bool& fHasGen
  );

  /**
    * \brief Filling the muon related histograms 
    *
    * \param fJet: vector of jets in form of JET::StdJet
    * \param fBJet: vector of b-jets in form of JET::StdJet
    * \param fDimuonMass: mass of the dimuon
    * \param fWeight: weight of the event
    * \param fType: type of jet event (eg. "OS", "SS", "OS_inverted", "SS_inverted")
    *
    * \throws No exception is thrown for this function
    */
  void FillJet(
    const std::vector<JET::StdJet>& fJet, 
    const std::vector<JET::StdJet>& fBJet, 
    const double& fDimuonMass, 
    const double& fWeight,
    const std::string& fType
  );


  /**
    * \brief Filling dressed level Gen Info with events has reco information
    * \param tDressedLeptons: pair of dressed muon pair
    * \param tGenJets: number of gen jets
    * \param tMuon_OS: pair of offline muon pair
    * \param nJets: number of offline jets
    * \param nBJets: number of offline b-jets
    * \param fMCWeight: MC Weight
    * \param fRecoWeight: Reco Weight
    * \throws No exception is thrown for this function
    */
  void FillGenInfo(
    const std::vector<TLorentzVector>& tDressedLeptons,
    const int& tGenJets,
    const std::vector<MUON::StdMuon>& tMuon_OS,
    const int& nJets,
    const int& nBJets,
    const double& fMCWeight,
    const double& fRecoWeight
  );

  /**
    * \brief Filling dressed level Gen Info
    * \param tDressedLeptons: pair of dressed muon pair
    * \param tGenJets: number of gen jets
    * \param fMCWeight: MC Weight
    * \throws No exception is thrown for this function
    */
  void FillGenInfoDressedLevel(
    const std::vector<TLorentzVector>& tDressedLeptons,
    const int& tGenJets,
    const double& fMCWeight
  );

  void FillResponseMatrix(
    const double& tGenMass,
    const int& tNGenJets,
    const double& tRecoMass,
    const int& nJets,
    const double& fMCWeight,
    const double& fRecoWeight,
    const bool& tPassingReco
  );

  std::string GetMassBin(double fDimuonMass);
  std::string GetJetBin(double fNJet);
  std::string GetBJetBin(double fNBJet);
  std::string GetbVetoJetBin(double fNJet);

  double SetPtOverflow(double fPt);
  double SetMassOverflow(double fMass);

  double GetNJetBinIndex(int nJet) const {
    if (nJet <= 0) return 0.;
    if (nJet == 1) return 1.;
    return 2.;
  }

private:
  TUnfoldBinning fGlobalBinningGen{"generator"};
  TUnfoldBinning fGlobalBinningReco{"reco"};
};

#endif
