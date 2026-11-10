#include "HistoSetEMU.h"

#include <iostream>

void HistoSetEMU::Init() {
  BookHisto(HistoSetBase::HistGroup::EventInfo, "EventInfo", "h_EventInfo", std::vector<double>{-9999, 5, 0.5, 5.5});
  BookHisto(HistoSetBase::HistGroup::EventInfo, "GenWeight", "h_GenWeight", std::vector<double>{-9999, 20000, -10000., 10000.});

  std::vector<std::string> fAddonMass = {"inc"};
  std::vector<std::string> fAddonJet = {"inc", "0J", "1J", "mt1J", "0BJ", "1BJ", "mt1BJ", "bVeto", "bVeto_0J", "bVeto_1J", "bVeto_mt1J"};
  const std::vector<std::string> fAddonType = {"OS", "SS", "OS_inverted", "SS_inverted"};
  
  if (GetMassExclusive())
    for (int i = 0; i < fMassBins.size() -1; i++)
      fAddonMass.push_back("m" + std::to_string((int)fMassBins[i]) + "_" + std::to_string((int)fMassBins[i+1]));

  for (const auto& tMassBin : fAddonMass) {
    for (const auto& tJetBin : fAddonJet) {
      for (const auto& tType : fAddonType) {
        BookHisto(tType, tJetBin, tMassBin, "nJet", "h_nJet");
        BookHisto(tType, tJetBin, tMassBin, "JetPt", "h_JetPt");
        BookHisto(tType, tJetBin, tMassBin, "JetEta", "h_JetEta");
        BookHisto(tType, tJetBin, tMassBin, "JetPhi", "h_JetPhi");

        BookHisto(tType, tJetBin, tMassBin, "nBJet", "h_nBJet");
        BookHisto(tType, tJetBin, tMassBin, "BJetPt", "h_BJetPt");
        BookHisto(tType, tJetBin, tMassBin, "BJetEta", "h_BJetEta");
        BookHisto(tType, tJetBin, tMassBin, "BJetPhi", "h_BJetPhi");

        BookHisto(tType, tJetBin, tMassBin, "ElecPt", "h_ElecPt");
        BookHisto(tType, tJetBin, tMassBin, "ElecEta", "h_ElecEta");
        BookHisto(tType, tJetBin, tMassBin, "ElecPhi", "h_ElecPhi");

        BookHisto(tType, tJetBin, tMassBin, "MuonPt", "h_MuonPt");
        BookHisto(tType, tJetBin, tMassBin, "MuonEta", "h_MuonEta");
        BookHisto(tType, tJetBin, tMassBin, "MuonPhi", "h_MuonPhi");

        BookHisto(tType, tJetBin, tMassBin, "PairMass", "h_PairMass");
        BookHisto(tType, tJetBin, tMassBin, "PairPt", "h_PairPt");
        BookHisto(tType, tJetBin, tMassBin, "PairRap", "h_PairRap");
      }
    }
  }

  std::cout << "######################################################################\n"
            << "                             Hist setting                             \n"
            << "----------------------------------------------------------------------\n";
  for (int i = 0; i < static_cast<int>(fMassBins.size()) - 1; ++i) {
    std::cout << "    m" << static_cast<int>(fMassBins[i])
              << "_" << static_cast<int>(fMassBins[i + 1]) << '\n';
  }
  std::cout << "######################################################################\n\n";
}

void HistoSetEMU::FillEMUPair(
  const TLorentzVector& fMuon,
  const TLorentzVector& fElec,
  const int& nJet,
  const int& nBJet,
  const double& fWeight,
  const std::string& fType
) {
  const TLorentzVector tEMuPair = fMuon + fElec;

  std::vector<TString> vMassSuffix = {"inc"};
  if (GetMassExclusive())
    vMassSuffix.push_back(GetMassBin(SetMassOverflow(tEMuPair.M())));

  std::vector<TString> vJetSuffix = {
    "inc",
    GetJetBin(nJet).c_str(),
    GetBJetBin(nBJet).c_str()
  };
  if (nBJet == 0) {
    vJetSuffix.push_back("bVeto");
    vJetSuffix.push_back(GetbVetoJetBin(nJet).c_str());
  }

  for (const auto& tMassSuffix : vMassSuffix) {
    for (const auto& tJetSuffix : vJetSuffix) {
      FillHisto(fType, tJetSuffix, tMassSuffix, "PairMass", SetMassOverflow(tEMuPair.M()), fWeight);

      if (tEMuPair.M() > 200.) {
        FillHisto(fType, tJetSuffix, tMassSuffix, "MuonPt", SetPtOverflow(fMuon.Pt()), fWeight);
        FillHisto(fType, tJetSuffix, tMassSuffix, "MuonEta", fMuon.Eta(), fWeight);
        FillHisto(fType, tJetSuffix, tMassSuffix, "MuonPhi", fMuon.Phi(), fWeight);

        FillHisto(fType, tJetSuffix, tMassSuffix, "ElecPt", SetPtOverflow(fElec.Pt()), fWeight);
        FillHisto(fType, tJetSuffix, tMassSuffix, "ElecEta", fElec.Eta(), fWeight);
        FillHisto(fType, tJetSuffix, tMassSuffix, "ElecPhi", fElec.Phi(), fWeight);

        FillHisto(fType, tJetSuffix, tMassSuffix, "PairPt", SetPtOverflow(tEMuPair.Pt()), fWeight);
        FillHisto(fType, tJetSuffix, tMassSuffix, "PairRap", tEMuPair.Rapidity(), fWeight);
      }
    }
  }
}

void HistoSetEMU::FillJet(
  const std::vector<JET::StdJet>& fJet,
  const std::vector<JET::StdJet>& fBJet,
  const double& fEMUMass,
  const double& fWeight,
  const std::string& fType
) {
  std::vector<TString> vMassSuffix = {"inc"};
  if (GetMassExclusive())
    vMassSuffix.push_back(GetMassBin(SetMassOverflow(fEMUMass)));

  std::vector<TString> vJetSuffix = {
    "inc",
    GetJetBin(fJet.size()).c_str(),
    GetBJetBin(fBJet.size()).c_str()
  };
  if (fBJet.empty()) {
    vJetSuffix.push_back("bVeto");
    vJetSuffix.push_back(GetbVetoJetBin(fJet.size()).c_str());
  }

  for (const auto& tMassSuffix : vMassSuffix) {
    for (const auto& tJetSuffix : vJetSuffix) {
      FillHisto(fType, tJetSuffix, tMassSuffix, "nJet", fJet.size(), fWeight);
      for (const auto& tJet : fJet) {
        FillHisto(fType, tJetSuffix, tMassSuffix, "JetPt", SetPtOverflow(tJet.fVec.Pt()), fWeight);
        FillHisto(fType, tJetSuffix, tMassSuffix, "JetEta", tJet.fVec.Eta(), fWeight);
        FillHisto(fType, tJetSuffix, tMassSuffix, "JetPhi", tJet.fVec.Phi(), fWeight);
      }

      FillHisto(fType, tJetSuffix, tMassSuffix, "nBJet", fBJet.size(), fWeight);
      for (const auto& tBJet : fBJet) {
        FillHisto(fType, tJetSuffix, tMassSuffix, "BJetPt", SetPtOverflow(tBJet.fVec.Pt()), fWeight);
        FillHisto(fType, tJetSuffix, tMassSuffix, "BJetEta", tBJet.fVec.Eta(), fWeight);
        FillHisto(fType, tJetSuffix, tMassSuffix, "BJetPhi", tBJet.fVec.Phi(), fWeight);
      }
    }
  }
}

std::string HistoSetEMU::GetMassBin(double fPairMass) {
  for (int i = 0; i < static_cast<int>(fMassBins.size()) - 1; ++i) {
    if (fPairMass >= fMassBins[i] && fPairMass < fMassBins[i + 1]) {
      return "m" + std::to_string(static_cast<int>(fMassBins[i])) +
             "_" + std::to_string(static_cast<int>(fMassBins[i + 1]));
    }
  }
  return "";
}

std::string HistoSetEMU::GetJetBin(double fNJet) {
  if (fNJet == 0) return "0J";
  if (fNJet == 1) return "1J";
  if (fNJet >= 2) return "mt1J";
  return "";
}

std::string HistoSetEMU::GetBJetBin(double fNBJet) {
  if (fNBJet == 0) return "0BJ";
  if (fNBJet == 1) return "1BJ";
  if (fNBJet >= 2) return "mt1BJ";
  return "";
}

std::string HistoSetEMU::GetbVetoJetBin(double fNJet) {
  if (fNJet == 0) return "bVeto_0J";
  if (fNJet == 1) return "bVeto_1J";
  if (fNJet >= 2) return "bVeto_mt1J";
  return "";
}

double HistoSetEMU::SetPtOverflow(double fPt) {
  return fPt > 1500. ? 1510. : fPt;
}

double HistoSetEMU::SetMassOverflow(double fMass) {
  if (fMass < 200.) return 199.5;
  if (fMass >= 4000.) return 4000.5;
  return fMass;
}
