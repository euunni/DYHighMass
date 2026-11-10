#ifndef HistoSetEMU_h
#define HistoSetEMU_h 1

#include <string>
#include <vector>

#include "HistoSetBase.h"
#include "jet.h"

#include "TLorentzVector.h"

class HistoSetEMU : public HistoSetBase
{
public:
  HistoSetEMU(TString fEra_, TString fSampleName_)
    : HistoSetBase(fEra_, fSampleName_) {
    // SetMassExclusive();
    Init();
  }

  void Init();

  std::vector<double> GetBinning(std::string name) override {
    if (name.find("Pt") != std::string::npos) return fPtBins;
    if (name.find("Eta") != std::string::npos || name.find("Rap") != std::string::npos) return fEtaBins;
    if (name.find("Phi") != std::string::npos) return fPhiBins;
    if (name.find("Mass") != std::string::npos) return fMassBins;
    if (name.find("DeltaR") != std::string::npos) return fDeltaRBins;
    if (name.find("nJet") != std::string::npos || name.find("nBJet") != std::string::npos) return fNJetBins;
    return {};
  }

  void FillEMUPair(
    const TLorentzVector& fMuon,
    const TLorentzVector& fElec,
    const int& nJet,
    const int& nBJet,
    const double& fWeight,
    const std::string& fType
  );

  void FillJet(
    const std::vector<JET::StdJet>& fJet,
    const std::vector<JET::StdJet>& fBJet,
    const double& fEMUMass,
    const double& fWeight,
    const std::string& fType
  );

  std::string GetMassBin(double fPairMass);
  std::string GetJetBin(double fNJet);
  std::string GetBJetBin(double fNBJet);
  std::string GetbVetoJetBin(double fNJet);

  double SetPtOverflow(double fPt);
  double SetMassOverflow(double fMass);
};

#endif
