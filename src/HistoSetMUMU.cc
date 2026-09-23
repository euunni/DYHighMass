#include "HistoSetMUMU.h"
#include "jet.h"

#include "TFile.h"
#include "TLorentzVector.h"
#include "TH1.h"


void HistoSetMUMU::Init() {

  TH1::SetDefaultSumw2();

  SetHisto("h_EventInfo", std::vector<double>{-9999, 5, 0.5, 5.5});
  SetHisto("h_GenWeight", std::vector<double>{-9999, 20000, -10000., 10000.});
  SetHisto("h_LHEDimuonMass", std::vector<double>{-9999, 6000, 0., 6000.});
  SetHisto("h_LHEnMuon", std::vector<double>{-9999, 10, 0., 10.});

  SetHisto("h_nPVGood_Count", std::vector<double>{-9999, 100, 0., 100.});
  SetHisto("h_PileUp_Count_Interaction_before", std::vector<double>{-9999, 100, 0., 100.});
  SetHisto("h_PileUp_Count_Interaction_after", std::vector<double>{-9999, 100, 0., 100.});

  fPtBins = {-9999, 300, 0., 1500.};
  fEtaBins = {-9999, 60, -3., 3.};
  fPhiBins = {-9999, 60, -3.141593, 3.141593};
  fMassBins = {39, 40, 45, 50, 55, 60, 64, 68, 72, 76, 81, 86, 91, 96, 101,
               106, 110, 115, 120, 126, 133, 141, 150, 160, 171, 185, 200, 220,
               243, 273, 320, 380, 440, 510, 600, 700, 830, 1000, 1500, 2000,
               3000, 3001};
  fDeltaRBins = {-9999, 100, 0.0, 6.0};
  fNJetBins = {-9999, 20, 0, 20};
  fChargeBins = {-9999, 2, -1, 1};

  std::vector<std::string> fAddonMass = {""};
  std::vector<std::string> fAddonJet = {"", "_0BJ"};
  std::vector<std::string> fAddonType = {"OS", "SS", "OS_inverted", "SS_inverted"};
  
  for (int i = 0; i < fMassBins.size() -1; i++) {
    fAddonMass.push_back("_m" + std::to_string((int)fMassBins[i]) + "_" + std::to_string((int)fMassBins[i+1]));
  }

  for (int i = 0; i < fAddonMass.size(); i++) {
    for (int j = 0; j < fAddonJet.size(); j++) {
      std::string tHistSuffix = fAddonJet[j] + fAddonMass[i];
      fSuffix.push_back(tHistSuffix);

      for (auto fType : fAddonType) {

        SetHisto("h_" + fType + "_nJet" + tHistSuffix);    
        SetHisto("h_" + fType + "_JetPt" + tHistSuffix);
        SetHisto("h_" + fType + "_JetEta" + tHistSuffix);
        SetHisto("h_" + fType + "_JetPhi" + tHistSuffix);

        SetHisto("h_" + fType + "_nBJet" + tHistSuffix);
        SetHisto("h_" + fType + "_BJetPt" + tHistSuffix);
        SetHisto("h_" + fType + "_BJetEta" + tHistSuffix);
        SetHisto("h_" + fType + "_BJetPhi" + tHistSuffix);

        SetHisto("h_" + fType + "_LeadingMuonPt" + tHistSuffix);
        SetHisto("h_" + fType + "_LeadingMuonEta" + tHistSuffix);
        SetHisto("h_" + fType + "_LeadingMuonPhi" + tHistSuffix);

        SetHisto("h_" + fType + "_SubleadingMuonPt" + tHistSuffix);
        SetHisto("h_" + fType + "_SubleadingMuonEta" + tHistSuffix);
        SetHisto("h_" + fType + "_SubleadingMuonPhi" + tHistSuffix);

        SetHisto("h_" + fType + "_MuonPt" + tHistSuffix);
        SetHisto("h_" + fType + "_MuonEta" + tHistSuffix);
        SetHisto("h_" + fType + "_MuonPhi" + tHistSuffix);
        SetHisto("h_" + fType + "_MuonCharge" + tHistSuffix, fChargeBins);

        SetHisto("h_" + fType + "_dimuonMass" + tHistSuffix);
        SetHisto("h_" + fType + "_dimuonMassFailGen" + tHistSuffix);
        SetHisto("h_" + fType + "_dimuonPt" + tHistSuffix);
        SetHisto("h_" + fType + "_dimuonRap" + tHistSuffix, "Eta");
      }
    }
  }

  std::cout << "######################################################################" << std::endl;
  std::cout << "                             Hist setting                             " << std::endl;
  std::cout << "----------------------------------------------------------------------" << std::endl;
  std::cout << " Total size: " << fHistSet.size() << std::endl;
  std::cout << " Mass binning: " << std::endl;

  for (int i = 0; i < fMassBins.size() - 1; i++)
    std::cout << "    m" << (int)fMassBins[i] << "_" << (int)fMassBins[i+1] << std::endl;

  std::cout << "######################################################################" << std::endl;
  std::cout << " " << std::endl;
}

void HistoSetMUMU::InitGenInfo() {

  std::vector<std::string> fAddonJet = {"", "_0J", "_1J", "_mt1J"};

  for (int i = 0; i < fAddonJet.size(); i++) {
    
    std::string tHistSuffix = fAddonJet[i];
    fSuffixGenInfo.emplace_back(tHistSuffix);

    SetHistoGenInfo("h_nJet_WithReco" + tHistSuffix, fNJetBins);    

    SetHistoGenInfo("h_LeadingMuonPt_WithReco" + tHistSuffix, fPtBins);
    SetHistoGenInfo("h_LeadingMuonEta_WithReco" + tHistSuffix, fEtaBins);
    SetHistoGenInfo("h_LeadingMuonPhi_WithReco" + tHistSuffix, fPhiBins);

    SetHistoGenInfo("h_SubleadingMuonPt_WithReco" + tHistSuffix, fPtBins);
    SetHistoGenInfo("h_SubleadingMuonEta_WithReco" + tHistSuffix, fEtaBins);
    SetHistoGenInfo("h_SubleadingMuonPhi_WithReco" + tHistSuffix, fPhiBins);

    SetHistoGenInfo("h_MuonPt_WithReco" + tHistSuffix, fPtBins);
    SetHistoGenInfo("h_MuonEta_WithReco" + tHistSuffix, fEtaBins);
    SetHistoGenInfo("h_MuonPhi_WithReco" + tHistSuffix, fPhiBins);

    SetHistoGenInfo("h_dimuonMass_WithReco" + tHistSuffix, fMassBins);
    SetHistoGenInfo("h_dimuonPt_WithReco" + tHistSuffix, fPtBins);
    SetHistoGenInfo("h_dimuonRap_WithReco" + tHistSuffix, fEtaBins);

    SetHistoGenInfo("h_nJet" + tHistSuffix, fNJetBins);    

    SetHistoGenInfo("h_LeadingMuonPt" + tHistSuffix, fPtBins);
    SetHistoGenInfo("h_LeadingMuonEta" + tHistSuffix, fEtaBins);
    SetHistoGenInfo("h_LeadingMuonPhi" + tHistSuffix, fPhiBins);

    SetHistoGenInfo("h_SubleadingMuonPt" + tHistSuffix, fPtBins);
    SetHistoGenInfo("h_SubleadingMuonEta" + tHistSuffix, fEtaBins);
    SetHistoGenInfo("h_SubleadingMuonPhi" + tHistSuffix, fPhiBins);

    SetHistoGenInfo("h_MuonPt" + tHistSuffix, fPtBins);
    SetHistoGenInfo("h_MuonEta" + tHistSuffix, fEtaBins);
    SetHistoGenInfo("h_MuonPhi" + tHistSuffix, fPhiBins);

    SetHistoGenInfo("h_dimuonMass" + tHistSuffix, fMassBins);
    SetHistoGenInfo("h_dimuonPt" + tHistSuffix, fPtBins);
    SetHistoGenInfo("h_dimuonRap" + tHistSuffix, fEtaBins);
  }
  
  const int nMassBins = fMassBins.size() - 1;
  const int nMassJetBins = 3 * nMassBins;
  fHistSet2D["h_ResponseMatrix"] = new TH2D("h_ResponseMatrix", "h_ResponseMatrix", nMassJetBins, 0., nMassJetBins, nMassJetBins, 0., nMassJetBins);
  fHistSet2D["h_ResponseMatrix_inc"] = new TH2D("h_ResponseMatrix_inc", "h_ResponseMatrix_inc", nMassBins, 0., nMassBins, nMassBins, 0., nMassBins);
  fHistSet2D["h_ResponseMatrix_merged"] = new TH2D("h_ResponseMatrix_merged", "h_ResponseMatrix_merged", 4 * nMassJetBins, 0., 4 * nMassJetBins, nMassJetBins, 0., nMassJetBins);

}

void HistoSetMUMU::FillHisto(std::string name, double value, double weight) {

  if (fHistSet.find(name) == fHistSet.end()) {
    std::cout << "Error: HistoSetMUMU::FillHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }

  fHistSet[name]->Fill(value, weight);
}

void HistoSetMUMU::FillHisto(std::string name, float value, double weight) {

  if (fHistSet.find(name) == fHistSet.end()) {
    std::cout << "Error: HistoSetMUMU::FillHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }

  fHistSet[name]->Fill(value, weight);
}

void HistoSetMUMU::FillHisto(std::string name, int value, double weight) {

  if (fHistSet.find(name) == fHistSet.end()) {
    std::cout << "Error: HistoSetMUMU::FillHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }

  fHistSet[name]->Fill(value, weight);
}

void HistoSetMUMU::Fill2DHisto(std::string name, double value1, double value2, double weight) {

  if (fHistSet2D.find(name) == fHistSet2D.end()) {
    std::cout << "Error: HistoSetMUMU::Fill2DHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }

  fHistSet2D[name]->Fill(value1, value2, weight);
}

void HistoSetMUMU::SetHistoGenInfo(std::string name, std::vector<double> bins) {

  if (fHistSetGenInfo.find(name) == fHistSetGenInfo.end()) {
    if (bins[0] == -9999) fHistSetGenInfo[name] = new TH1D(name.c_str(), name.c_str(), bins[1], bins[2], bins[3]);
    else                 fHistSetGenInfo[name] = new TH1D(name.c_str(), name.c_str(), bins.size() - 1, &(bins[0]));
  } else {
    if (bins[0] == -9999) fHistSetGenInfo[name]->SetBins(bins[1], bins[2], bins[3]);
    else                 fHistSetGenInfo[name]->SetBins(bins.size() - 1, &(bins[0]));
  }
}

void HistoSetMUMU::SetHisto(std::string name, std::vector<double> bins) {

  if (fHistSet.find(name) == fHistSet.end()) {
    if (bins[0] == -9999) fHistSet[name] = new TH1D(name.c_str(), name.c_str(), bins[1], bins[2], bins[3]);
    else                 fHistSet[name] = new TH1D(name.c_str(), name.c_str(), bins.size() - 1, &(bins[0]));
  } else {
    if (bins[0] == -9999) fHistSet[name]->SetBins(bins[1], bins[2], bins[3]);
    else                 fHistSet[name]->SetBins(bins.size() - 1, &(bins[0]));
  }
}

void HistoSetMUMU::SetHisto(std::string name, std::vector<double> bins1, std::vector<double> bins2) {

  if (fHistSet2D.find(name) == fHistSet2D.end()) {
    if (bins1[0] == -9999 && bins2[0] == -9999) fHistSet2D[name] = new TH2D(name.c_str(), name.c_str(), bins1[1], bins1[2], bins1[3], bins2[1], bins2[2], bins2[3]);
    else fHistSet2D[name] = new TH2D(name.c_str(), name.c_str(), bins1.size() - 1, &(bins1[0]), bins2.size() - 1, &(bins2[0]));
  } else {
    if (bins1[0] == -9999 && bins2[0] == -9999) fHistSet2D[name]->SetBins(bins1[1], bins1[2], bins1[3], bins2[1], bins2[2], bins2[3]);
    else fHistSet2D[name]->SetBins(bins1.size() - 1, &(bins1[0]), bins2.size() - 1, &(bins2[0]));
  }
}

void HistoSetMUMU::SetHisto(std::string name) {
  if (name.find("Pt") != std::string::npos) {
    SetHisto(name, fPtBins);
  }
  else if (name.find("Eta") != std::string::npos) {
    SetHisto(name, fEtaBins);
  }
  else if (name.find("Phi") != std::string::npos) {
    SetHisto(name, fPhiBins);
  }
  else if (name.find("Mass") != std::string::npos) {
    SetHisto(name, fMassBins);
  }
  else if (name.find("DeltaR") != std::string::npos) {
    SetHisto(name, fDeltaRBins);
  }
  else if (name.find("nJet") != std::string::npos || name.find("nBJet") != std::string::npos) {
    SetHisto(name, fNJetBins);
  }
  else {
    std::cout << "Error: HistoSetMUMU::SetHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }
}

void HistoSetMUMU::SetHisto(std::string name, std::string binning) {
  
  if (binning.find("Pt") != std::string::npos) {
    SetHisto(name, fPtBins);
  }
  else if (binning.find("Eta") != std::string::npos) {
    SetHisto(name, fEtaBins);
  }
  else if (binning.find("Phi") != std::string::npos) {
    SetHisto(name, fPhiBins);
  }
  else if (binning.find("Mass") != std::string::npos) {
    SetHisto(name, fMassBins);
  }
  else if (binning.find("DeltaR") != std::string::npos) {
    SetHisto(name, fDeltaRBins);
  }
  else if (binning.find("nJet") != std::string::npos || binning.find("nBJet") != std::string::npos) {
    SetHisto(name, fNJetBins);
  }
  else {
    std::cout << "Error: HistoSetMUMU::SetHisto: Unknown histogram name: " << name << " " << binning << std::endl;
    exit(1);
  }
}

void HistoSetMUMU::SetHisto(std::string name, std::string binning1, std::string binning2) {

  std::vector<double> binEdge1; 
  if (binning1.find("Pt") != std::string::npos) {
    binEdge1 = fPtBins;
  }
  else if (binning1.find("Eta") != std::string::npos) {
    binEdge1 = fEtaBins;
  }
  else if (binning1.find("Phi") != std::string::npos) {
    binEdge1 = fPhiBins;
  }
  else if (binning1.find("Mass") != std::string::npos) {
    binEdge1 = fMassBins;
  }
  else if (binning1.find("DeltaR") != std::string::npos) {
    binEdge1 = fDeltaRBins;
  }
  else if (binning1.find("nJet") != std::string::npos || binning1.find("nBJet") != std::string::npos) {
    binEdge1 = fNJetBins;
  }
  else {
    std::cout << "Error: HistoSetMUMU::SetHisto: Unknown histogram name: " << name << " " << binning1 << std::endl;
    exit(1);
  }

  std::vector<double> binEdge2; 
  if (binning2.find("Pt") != std::string::npos) {
    binEdge2 = fPtBins;
  }
  else if (binning2.find("Eta") != std::string::npos) {
    binEdge2 = fEtaBins;
  }
  else if (binning2.find("Phi") != std::string::npos) {
    binEdge2 = fPhiBins;
  }
  else if (binning2.find("Mass") != std::string::npos) {
    binEdge2 = fMassBins;
  }
  else if (binning2.find("DeltaR") != std::string::npos) {
    binEdge2 = fDeltaRBins;
  }
  else if (binning2.find("nJet") != std::string::npos || binning2.find("nBJet") != std::string::npos) {
    binEdge2 = fNJetBins;
  }
  else {
    std::cout << "Error: HistoSetMUMU::SetHisto: Unknown histogram name: " << name << " " << binning2 << std::endl;
    exit(1);
  }

  SetHisto(name, binEdge1, binEdge2);

}

std::string HistoSetMUMU::GetMassBin(double fDimuonMass) {
  for (int i = 0; i < fMassBins.size() - 1; i++) {
    if (fDimuonMass >= fMassBins[i] && fDimuonMass < fMassBins[i+1]) {
      return "_m" + std::to_string((int)fMassBins[i]) + "_" + std::to_string((int)fMassBins[i+1]);
    }
  }
  return "";
}

std::string HistoSetMUMU::GetJetBin(double fNJet) {
  
  if (fNJet == 0) return "_0J";
  else if (fNJet == 1) return "_1J";
  else if (fNJet >= 2) return "_mt1J";
  else return "";
}

double HistoSetMUMU::SetPtOverflow(double fPt) {
  if (fPt > 1500) return 1510;
  else return fPt;
}
double HistoSetMUMU::SetMassOverflow(double fMass) {
  if (fMass < 0) return -0.5;
  if (fMass < 40) return 39.5;
  if (fMass >= 3000) return 3000.5;
  else return fMass;
}

void HistoSetMUMU::FillMuon(
  const TLorentzVector& fLeadingMuon, 
  const TLorentzVector& fSubleadingMuon, 
  const float& fCharge, 
  const int& nJet, 
  const int& nBJet, 
  const double& fWeight,
  const std::string& fType,
  const bool& fHasGen
) {

  TLorentzVector fDimuon = fLeadingMuon + fSubleadingMuon;

  std::string tMassSuffix = GetMassBin(SetMassOverflow(fDimuon.M()));

  std::vector<std::string> tHistSuffix;
  if (nBJet == 0) tHistSuffix = {"", tMassSuffix, "_0BJ", "_0BJ" + tMassSuffix};
  else tHistSuffix = {"", tMassSuffix};
  
  for (auto suffix : tHistSuffix) {

    if (fDimuon.M() > 40) {
      fHistSet["h_" + fType + "_LeadingMuonPt" + suffix]->Fill(SetPtOverflow(fLeadingMuon.Pt()), fWeight);
      fHistSet["h_" + fType + "_LeadingMuonEta" + suffix]->Fill(fLeadingMuon.Eta(), fWeight);
      fHistSet["h_" + fType + "_LeadingMuonPhi" + suffix]->Fill(fLeadingMuon.Phi(), fWeight);

      fHistSet["h_" + fType + "_SubleadingMuonPt" + suffix]->Fill(SetPtOverflow(fSubleadingMuon.Pt()), fWeight);
      fHistSet["h_" + fType + "_SubleadingMuonEta" + suffix]->Fill(fSubleadingMuon.Eta(), fWeight);
      fHistSet["h_" + fType + "_SubleadingMuonPhi" + suffix]->Fill(fSubleadingMuon.Phi(), fWeight);

      fHistSet["h_" + fType + "_MuonPt" + suffix]->Fill(SetPtOverflow(fLeadingMuon.Pt()), fWeight);
      fHistSet["h_" + fType + "_MuonEta" + suffix]->Fill(fLeadingMuon.Eta(), fWeight);
      fHistSet["h_" + fType + "_MuonPhi" + suffix]->Fill(fLeadingMuon.Phi(), fWeight);

      fHistSet["h_" + fType + "_MuonPt" + suffix]->Fill(SetPtOverflow(fSubleadingMuon.Pt()), fWeight);
      fHistSet["h_" + fType + "_MuonEta" + suffix]->Fill(fSubleadingMuon.Eta(), fWeight);
      fHistSet["h_" + fType + "_MuonPhi" + suffix]->Fill(fSubleadingMuon.Phi(), fWeight);
      fHistSet["h_" + fType + "_MuonCharge" + suffix]->Fill(fCharge, fWeight);

      fHistSet["h_" + fType + "_dimuonPt" + suffix]->Fill(SetPtOverflow(fDimuon.Pt()), fWeight);
      fHistSet["h_" + fType + "_dimuonRap" + suffix]->Fill(fDimuon.Rapidity(), fWeight);
    }

    fHistSet["h_" + fType + "_dimuonMass" + suffix]->Fill(SetMassOverflow(fDimuon.M()), fWeight);
    if (!fHasGen) fHistSet["h_" + fType + "_dimuonMassFailGen" + suffix]->Fill(SetMassOverflow(fDimuon.M()), fWeight);
  }
}

void HistoSetMUMU::FillJet(
  const std::vector<JET::StdJet>& fJet, 
  const std::vector<JET::StdJet>& fBJet, 
  const double& fDimuonMass, 
  const double& fWeight,
  const std::string& fType
) {
  
  std::string tMassSuffix = GetMassBin(SetMassOverflow(fDimuonMass));

  std::vector<std::string> tHistSuffix;
  if (fBJet.empty()) tHistSuffix = {"", tMassSuffix, "_0BJ", "_0BJ" + tMassSuffix};
  else tHistSuffix = {"", tMassSuffix};

  if (fDimuonMass > 40) {
    for (auto suffix : tHistSuffix) {

      fHistSet["h_" + fType + "_nJet" + suffix]->Fill(fJet.size(), fWeight);
      for (int i = 0; i < fJet.size(); i++) {
        fHistSet["h_" + fType + "_JetPt" + suffix]->Fill(SetPtOverflow(fJet.at(i).fVec.Pt()), fWeight);
        fHistSet["h_" + fType + "_JetEta" + suffix]->Fill(fJet.at(i).fVec.Eta(), fWeight);
        fHistSet["h_" + fType + "_JetPhi" + suffix]->Fill(fJet.at(i).fVec.Phi(), fWeight);
      }

      fHistSet["h_" + fType + "_nBJet" + suffix]->Fill(fBJet.size(), fWeight);
      for (int i = 0; i < fBJet.size(); i++) {
        fHistSet["h_" + fType + "_BJetPt" + suffix]->Fill(SetPtOverflow(fBJet.at(i).fVec.Pt()), fWeight);
        fHistSet["h_" + fType + "_BJetEta" + suffix]->Fill(fBJet.at(i).fVec.Eta(), fWeight);
        fHistSet["h_" + fType + "_BJetPhi" + suffix]->Fill(fBJet.at(i).fVec.Phi(), fWeight);
      }
    }
  }
}

void HistoSetMUMU::FillGenInfo(
  const std::vector<TLorentzVector>& tDressedLeptons,
  const int& tNGenJets,
  const std::vector<MUON::StdMuon>& tMuon_OS,
  const int& nJets,
  const int& nBJets,
  const double& fMCWeight,
  const double& fRecoWeight
) {

  if (nBJets != 0)
    return;
  
  double tTotalWeight = fMCWeight * fRecoWeight;

  std::string tGenJetSuffix = GetJetBin(tNGenJets);

  auto tGenDiMuon = tDressedLeptons.at(0) + tDressedLeptons.at(1);
  auto tGenMass = SetMassOverflow(tGenDiMuon.M());

  auto tRecoDiMuon = tMuon_OS.at(0).fVec + tMuon_OS.at(1).fVec;
  auto tRecoMass = SetMassOverflow(tRecoDiMuon.M());

  std::vector<std::string> fAddonGenJet = {""};
  fAddonGenJet.emplace_back(tGenJetSuffix);

  for (auto suffix : fAddonGenJet) {

    fHistSetGenInfo["h_dimuonMass_WithReco" + suffix]->Fill(tGenMass, tTotalWeight);
    
    if (tGenMass > 40) {
      fHistSetGenInfo["h_nJet_WithReco" + suffix]->Fill(tNGenJets, tTotalWeight);

      fHistSetGenInfo["h_LeadingMuonPt_WithReco" + suffix]->Fill(tDressedLeptons.at(0).Pt(), tTotalWeight);
      fHistSetGenInfo["h_LeadingMuonEta_WithReco" + suffix]->Fill(tDressedLeptons.at(0).Eta(), tTotalWeight);
      fHistSetGenInfo["h_LeadingMuonPhi_WithReco" + suffix]->Fill(tDressedLeptons.at(0).Phi(), tTotalWeight);

      fHistSetGenInfo["h_SubleadingMuonPt_WithReco" + suffix]->Fill(tDressedLeptons.at(1).Pt(), tTotalWeight);
      fHistSetGenInfo["h_SubleadingMuonEta_WithReco" + suffix]->Fill(tDressedLeptons.at(1).Eta(), tTotalWeight);
      fHistSetGenInfo["h_SubleadingMuonPhi_WithReco" + suffix]->Fill(tDressedLeptons.at(1).Phi(), tTotalWeight);

      fHistSetGenInfo["h_MuonPt_WithReco" + suffix]->Fill(tDressedLeptons.at(0).Pt(), tTotalWeight);
      fHistSetGenInfo["h_MuonEta_WithReco" + suffix]->Fill(tDressedLeptons.at(0).Eta(), tTotalWeight);
      fHistSetGenInfo["h_MuonPhi_WithReco" + suffix]->Fill(tDressedLeptons.at(0).Phi(), tTotalWeight);
  
      fHistSetGenInfo["h_MuonPt_WithReco" + suffix]->Fill(tDressedLeptons.at(1).Pt(), tTotalWeight);
      fHistSetGenInfo["h_MuonEta_WithReco" + suffix]->Fill(tDressedLeptons.at(1).Eta(), tTotalWeight);
      fHistSetGenInfo["h_MuonPhi_WithReco" + suffix]->Fill(tDressedLeptons.at(1).Phi(), tTotalWeight);

      fHistSetGenInfo["h_dimuonPt_WithReco" + suffix]->Fill(tGenDiMuon.Pt(), tTotalWeight);
      fHistSetGenInfo["h_dimuonRap_WithReco" + suffix]->Fill(tGenDiMuon.Rapidity(), tTotalWeight);
    }
  }

}

void HistoSetMUMU::FillGenInfoIndependently(
  const std::vector<TLorentzVector>& tDressedLeptons,
  const int& tNGenJets,
  const double& fMCWeight
) {
  
  std::string tGenJetSuffix = GetJetBin(tNGenJets);

  auto tGenDiMuon = tDressedLeptons.at(0) + tDressedLeptons.at(1);
  auto tGenMass = SetMassOverflow(tGenDiMuon.M());

  std::vector<std::string> fAddonGenJet = {""};
  fAddonGenJet.emplace_back(tGenJetSuffix);

  for (auto suffix : fAddonGenJet) {

    fHistSetGenInfo["h_dimuonMass" + suffix]->Fill(tGenMass, fMCWeight);
    
    if (tGenMass > 40) {
      fHistSetGenInfo["h_nJet" + suffix]->Fill(tNGenJets, fMCWeight);

      fHistSetGenInfo["h_LeadingMuonPt" + suffix]->Fill(tDressedLeptons.at(0).Pt(), fMCWeight);
      fHistSetGenInfo["h_LeadingMuonEta" + suffix]->Fill(tDressedLeptons.at(0).Eta(), fMCWeight);
      fHistSetGenInfo["h_LeadingMuonPhi" + suffix]->Fill(tDressedLeptons.at(0).Phi(), fMCWeight);

      fHistSetGenInfo["h_SubleadingMuonPt" + suffix]->Fill(tDressedLeptons.at(1).Pt(), fMCWeight);
      fHistSetGenInfo["h_SubleadingMuonEta" + suffix]->Fill(tDressedLeptons.at(1).Eta(), fMCWeight);
      fHistSetGenInfo["h_SubleadingMuonPhi" + suffix]->Fill(tDressedLeptons.at(1).Phi(), fMCWeight);

      fHistSetGenInfo["h_MuonPt" + suffix]->Fill(tDressedLeptons.at(0).Pt(), fMCWeight);
      fHistSetGenInfo["h_MuonEta" + suffix]->Fill(tDressedLeptons.at(0).Eta(), fMCWeight);
      fHistSetGenInfo["h_MuonPhi" + suffix]->Fill(tDressedLeptons.at(0).Phi(), fMCWeight);
  
      fHistSetGenInfo["h_MuonPt" + suffix]->Fill(tDressedLeptons.at(1).Pt(), fMCWeight);
      fHistSetGenInfo["h_MuonEta" + suffix]->Fill(tDressedLeptons.at(1).Eta(), fMCWeight);
      fHistSetGenInfo["h_MuonPhi" + suffix]->Fill(tDressedLeptons.at(1).Phi(), fMCWeight);

      fHistSetGenInfo["h_dimuonPt" + suffix]->Fill(tGenDiMuon.Pt(), fMCWeight);
      fHistSetGenInfo["h_dimuonRap" + suffix]->Fill(tGenDiMuon.Rapidity(), fMCWeight);
    }
  }
}

void HistoSetMUMU::FillResponseMatrix(
  const double& tGenMass,
  const int& tNGenJets,
  const double& tRecoMass,
  const int& nJets,
  const int& nBJets,
  const double& fMCWeight,
  const double& fRecoWeight,
  const bool& tPassingOfflineEventSelection
) {

  double tTotalWeight = fMCWeight * fRecoWeight;
  
  double tGenMassIndex = GetMassBinIndex(SetMassOverflow(tGenMass));
  double tRecoMassIndex = GetMassBinIndex(SetMassOverflow(tRecoMass));
  double tGenJetIndex = GetNJetBinIndex(tNGenJets);
  double tRecoJetIndex = GetNJetBinIndex(nJets);
  const double nMassBins = fMassBins.size() - 1;

  bool tPassingOffline = (nBJets == 0) && (tRecoMass > 0) && tPassingOfflineEventSelection;

  double tXbin = nMassBins * tRecoJetIndex + tRecoMassIndex + 0.5;
  double tYbin = nMassBins * tGenJetIndex + tGenMassIndex + 0.5;

  double tXbin_inc = tRecoMassIndex + 0.5;
  double tYbin_inc = tGenMassIndex + 0.5;
  
  if (!tPassingOffline) {
    tXbin = -0.5;
    tXbin_inc = -0.5;
  }

  std::cout << "PassingOffline: " << tPassingOffline << " " << tRecoMass << " " << nBJets << " " << tPassingOfflineEventSelection << std::endl;

  std::cout << "GenMassIndex: " << tGenMassIndex << " " << tGenMass << std::endl;
  std::cout << "RecoMassIndex: " << tRecoMassIndex << " " << tRecoMass << std::endl;
  std::cout << "GenJetIndex: " << tGenJetIndex << " " << tNGenJets << std::endl;
  std::cout << "RecoJetIndex: " << tRecoJetIndex << " " << nJets << std::endl;
  std::cout << "Xbin: " << tXbin << " " << tYbin << std::endl;
  std::cout << "Xbin_inc: " << tXbin_inc << " " << tYbin_inc << std::endl;

  std::cout << "TotalWeight: " << tTotalWeight << std::endl;
  std::cout << "MCWeight: " << fMCWeight << std::endl;
  std::cout << "RecoWeight: " << fRecoWeight << std::endl;

  if (tPassingOffline) {
    fHistSet2D["h_ResponseMatrix"]->Fill(tXbin, tYbin, tTotalWeight);
    fHistSet2D["h_ResponseMatrix_inc"]->Fill(tXbin_inc, tYbin_inc, tTotalWeight);
  }

  if (!tPassingOffline) {
    fHistSet2D["h_ResponseMatrix"]->Fill(-0.5, tYbin, fMCWeight); 
    fHistSet2D["h_ResponseMatrix_inc"]->Fill(-0.5, tYbin_inc, fMCWeight); 
  }

  if (tPassingOffline && fRecoWeight != 1) {
    fHistSet2D["h_ResponseMatrix"]->Fill(-0.5, tYbin, fMCWeight * (1. - fRecoWeight));
    fHistSet2D["h_ResponseMatrix_inc"]->Fill(-0.5, tYbin_inc, fMCWeight * (1. - fRecoWeight));
  }
}

void HistoSetMUMU::FillResponseMatrix_v2(
  const double& tGenMass,
  const int& tNGenJets,
  const double& tRecoMass,
  const int& nJets,
  const double& fMCWeight,
  const double& fRecoWeight,
  const bool& tPassingReco
) {

  const double nMassBins = fMassBins.size() - 1;
  const double nMassJetBins = 3. * nMassBins;
  double fEraOffset = 0;
  if (fEra == "2016_postVFP") fEraOffset = nMassJetBins;
  if (fEra == "2017") fEraOffset = 2. * nMassJetBins;
  if (fEra == "2018") fEraOffset = 3. * nMassJetBins;

  double tGenMassIndex = GetMassBinIndex(SetMassOverflow(tGenMass));
  double tRecoMassIndex = GetMassBinIndex(SetMassOverflow(tRecoMass));
  double tGenJetIndex = GetNJetBinIndex(tNGenJets);
  double tRecoJetIndex = GetNJetBinIndex(nJets);

  double tXbin = nMassBins * tRecoJetIndex + tRecoMassIndex + 0.5;
  double tYbin = nMassBins * tGenJetIndex + tGenMassIndex + 0.5;

  double tXbin_inc = tRecoMassIndex + 0.5;
  double tYbin_inc = tGenMassIndex + 0.5;

  if (tPassingReco) {
    fHistSet2D["h_ResponseMatrix"]->Fill(tXbin, tYbin, fMCWeight * fRecoWeight);
    fHistSet2D["h_ResponseMatrix_inc"]->Fill(tXbin_inc, tYbin_inc, fMCWeight * fRecoWeight);
    fHistSet2D["h_ResponseMatrix_merged"]->Fill(tXbin + fEraOffset, tYbin, fMCWeight * fRecoWeight);
  }

  if (!tPassingReco) {
    fHistSet2D["h_ResponseMatrix"]->Fill(-0.5, tYbin, fMCWeight); 
    fHistSet2D["h_ResponseMatrix_inc"]->Fill(-0.5, tYbin_inc, fMCWeight); 
    fHistSet2D["h_ResponseMatrix_merged"]->Fill(-0.5, tYbin, fMCWeight); 
  }

  if (tPassingReco && fRecoWeight != 1) {
    fHistSet2D["h_ResponseMatrix"]->Fill(-0.5, tYbin, fMCWeight * (1. - fRecoWeight));
    fHistSet2D["h_ResponseMatrix_inc"]->Fill(-0.5, tYbin_inc, fMCWeight * (1. - fRecoWeight));
    fHistSet2D["h_ResponseMatrix_merged"]->Fill(-0.5, tYbin, fMCWeight * (1. - fRecoWeight));
  }
}

void HistoSetMUMU::WriteHisto(TString fEra, TString fSampleName, TFile* fOutputFile, bool fIsData) {

  if (fSampleName.Contains("NNLO_MUMU_10to50"))
    fSampleName = "NNLO_MUMU_10to50";
    
  fOutputFile->mkdir(fEra + "/" + fSampleName);

  for (auto tSuffix : fSuffix)
    if (tSuffix != "")
      fOutputFile->mkdir(fEra + "/" + fSampleName + "/" + tSuffix);

  fOutputFile->cd(fEra + "/" + fSampleName);
  fHistSet["h_EventInfo"]->Write();
  fHistSet["h_GenWeight"]->Write();
  fHistSet["h_LHEDimuonMass"]->Write();
  fHistSet["h_LHEnMuon"]->Write();
  fHistSet["h_nPVGood_Count"]->Write();
  fHistSet["h_PileUp_Count_Interaction_before"]->Write();
  fHistSet["h_PileUp_Count_Interaction_after"]->Write();

  std::vector<std::string> fPairType = {"OS", "SS", "OS_inverted", "SS_inverted"};
  
  for (const auto& fType : fPairType) {
    for (auto tSuffix : fSuffix) {
      if (tSuffix != "") fOutputFile->cd(fEra + "/" + fSampleName + "/" + tSuffix);
      else fOutputFile->cd(fEra + "/" + fSampleName);
        

      fHistSet["h_" + fType + "_LeadingMuonPt" + tSuffix]->Write();
      fHistSet["h_" + fType + "_LeadingMuonEta" + tSuffix]->Write();
      fHistSet["h_" + fType + "_LeadingMuonPhi" + tSuffix]->Write();

      fHistSet["h_" + fType + "_SubleadingMuonPt" + tSuffix]->Write();
      fHistSet["h_" + fType + "_SubleadingMuonEta" + tSuffix]->Write();
      fHistSet["h_" + fType + "_SubleadingMuonPhi" + tSuffix]->Write();

      fHistSet["h_" + fType + "_MuonPt" + tSuffix]->Write();  
      fHistSet["h_" + fType + "_MuonEta" + tSuffix]->Write();
      fHistSet["h_" + fType + "_MuonPhi" + tSuffix]->Write();
      fHistSet["h_" + fType + "_MuonCharge" + tSuffix]->Write();

      fHistSet["h_" + fType + "_dimuonMass" + tSuffix]->Write();
      fHistSet["h_" + fType + "_dimuonMassFailGen" + tSuffix]->Write();
      fHistSet["h_" + fType + "_dimuonPt" + tSuffix]->Write();
      fHistSet["h_" + fType + "_dimuonRap" + tSuffix]->Write();

      fHistSet["h_" + fType + "_nJet" + tSuffix]->Write();
      fHistSet["h_" + fType + "_JetPt" + tSuffix]->Write();
      fHistSet["h_" + fType + "_JetEta" + tSuffix]->Write();
      fHistSet["h_" + fType + "_JetPhi" + tSuffix]->Write();
      
      fHistSet["h_" + fType + "_nBJet" + tSuffix]->Write();
      fHistSet["h_" + fType + "_BJetPt" + tSuffix]->Write();
      fHistSet["h_" + fType + "_BJetEta" + tSuffix]->Write();
      fHistSet["h_" + fType + "_BJetPhi" + tSuffix]->Write();
    }
  }

  if (fIsData) {
    
    fOutputFile->mkdir(fEra + "/Data");
    fOutputFile->cd(fEra + "/Data");
    
    fHistSet["h_EventInfo"]->Write();
    fHistSet["h_GenWeight"]->Write();
    fHistSet["h_LHEDimuonMass"]->Write();
    fHistSet["h_LHEnMuon"]->Write();
    fHistSet["h_nPVGood_Count"]->Write();
    fHistSet["h_PileUp_Count_Interaction_before"]->Write();
    fHistSet["h_PileUp_Count_Interaction_after"]->Write();

    for (auto tSuffix : fSuffix)
      if (tSuffix != "")
        fOutputFile->mkdir(fEra + "/Data/" + tSuffix);

    for (const auto& fType : fPairType) {
      for (auto tSuffix : fSuffix) {
        if (tSuffix != "") fOutputFile->cd(fEra + "/Data/" + tSuffix);
        else               fOutputFile->cd(fEra + "/Data");

        fHistSet["h_" + fType + "_LeadingMuonPt" + tSuffix]->Write();
        fHistSet["h_" + fType + "_LeadingMuonEta" + tSuffix]->Write();
        fHistSet["h_" + fType + "_LeadingMuonPhi" + tSuffix]->Write();

        fHistSet["h_" + fType + "_SubleadingMuonPt" + tSuffix]->Write();
        fHistSet["h_" + fType + "_SubleadingMuonEta" + tSuffix]->Write();
        fHistSet["h_" + fType + "_SubleadingMuonPhi" + tSuffix]->Write();

        fHistSet["h_" + fType + "_MuonPt" + tSuffix]->Write();  
        fHistSet["h_" + fType + "_MuonEta" + tSuffix]->Write();
        fHistSet["h_" + fType + "_MuonPhi" + tSuffix]->Write();
        fHistSet["h_" + fType + "_MuonCharge" + tSuffix]->Write();

        fHistSet["h_" + fType + "_dimuonMass" + tSuffix]->Write();
        fHistSet["h_" + fType + "_dimuonPt" + tSuffix]->Write();
        fHistSet["h_" + fType + "_dimuonRap" + tSuffix]->Write();

        fHistSet["h_" + fType + "_nJet" + tSuffix]->Write();
        fHistSet["h_" + fType + "_JetPt" + tSuffix]->Write();
        fHistSet["h_" + fType + "_JetEta" + tSuffix]->Write();
        fHistSet["h_" + fType + "_JetPhi" + tSuffix]->Write();
        
        fHistSet["h_" + fType + "_nBJet" + tSuffix]->Write();
        fHistSet["h_" + fType + "_BJetPt" + tSuffix]->Write();
        fHistSet["h_" + fType + "_BJetEta" + tSuffix]->Write();
        fHistSet["h_" + fType + "_BJetPhi" + tSuffix]->Write();
      }
    }
  }

  // if (fHistSet2D.size() > 0) {
  //   fOutputFile->mkdir("Hist2D/" + fEra + "/" + fSampleName);

  //   for (auto [name, hist] : fHistSet2D) {
  //     hist->SetDirectory(fOutputFile);
  //     fOutputFile->cd("Hist2D/" + fEra + "/" + fSampleName);
  //     hist->Write();
  //   }
  // }
}

void HistoSetMUMU::WriteGenHisto(TString fEra, TString fSampleName, TFile* fOutputFile) {
  
  if (fSampleName.Contains("NNLO_MUMU_10to50"))
    fSampleName = "NNLO_MUMU_10to50";

  fOutputFile->mkdir(fEra + "/" + fSampleName + "/GenInfo");
  fOutputFile->cd(fEra + "/" + fSampleName + "/GenInfo");
  fHistSet2D["h_ResponseMatrix"]->Write();
  fHistSet2D["h_ResponseMatrix_inc"]->Write();
  fHistSet2D["h_ResponseMatrix_merged"]->Write();

  for (auto tSuffix : fSuffixGenInfo) {
    if (tSuffix != "") {
      fOutputFile->mkdir(fEra + "/" + fSampleName + "/GenInfo/" + tSuffix);
      fOutputFile->cd(fEra + "/" + fSampleName + "/GenInfo/" + tSuffix);
    }

    fHistSetGenInfo["h_nJet_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_LeadingMuonPt_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_LeadingMuonEta_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_LeadingMuonPhi_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_SubleadingMuonPt_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_SubleadingMuonEta_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_SubleadingMuonPhi_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_MuonPt_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_MuonEta_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_MuonPhi_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_dimuonMass_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_dimuonPt_WithReco" + tSuffix]->Write();
    fHistSetGenInfo["h_dimuonRap_WithReco" + tSuffix]->Write();

    fHistSetGenInfo["h_nJet" + tSuffix]->Write();
    fHistSetGenInfo["h_LeadingMuonPt" + tSuffix]->Write();
    fHistSetGenInfo["h_LeadingMuonEta" + tSuffix]->Write();
    fHistSetGenInfo["h_LeadingMuonPhi" + tSuffix]->Write();
    fHistSetGenInfo["h_SubleadingMuonPt" + tSuffix]->Write();
    fHistSetGenInfo["h_SubleadingMuonEta" + tSuffix]->Write();
    fHistSetGenInfo["h_SubleadingMuonPhi" + tSuffix]->Write();
    fHistSetGenInfo["h_MuonPt" + tSuffix]->Write();
    fHistSetGenInfo["h_MuonEta" + tSuffix]->Write();
    fHistSetGenInfo["h_MuonPhi" + tSuffix]->Write();
    fHistSetGenInfo["h_dimuonMass" + tSuffix]->Write();
    fHistSetGenInfo["h_dimuonPt" + tSuffix]->Write();
    fHistSetGenInfo["h_dimuonRap" + tSuffix]->Write();
  }
}
