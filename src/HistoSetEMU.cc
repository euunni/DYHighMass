#include "HistoSetEMU.h"
#include "jet.h"

#include "TFile.h"
#include "TLorentzVector.h"
#include "TH1.h"


void HistoSetEMU::Init() {

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

        SetHisto("h_" + fType + "_ElecPt" + tHistSuffix);
        SetHisto("h_" + fType + "_ElecEta" + tHistSuffix);
        SetHisto("h_" + fType + "_ElecPhi" + tHistSuffix);

        SetHisto("h_" + fType + "_MuonPt" + tHistSuffix);
        SetHisto("h_" + fType + "_MuonEta" + tHistSuffix);
        SetHisto("h_" + fType + "_MuonPhi" + tHistSuffix);

        SetHisto("h_" + fType + "_PairMass" + tHistSuffix);
        SetHisto("h_" + fType + "_PairPt" + tHistSuffix);
        SetHisto("h_" + fType + "_PairRap" + tHistSuffix, "Eta");
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

void HistoSetEMU::FillHistoSet(std::string name, double mass, int nJet, int nBJet, double value, double weight) {

  std::string tMassSuffix = GetMassBin(SetMassOverflow(mass));

  std::vector<std::string> tHistSuffix;
  if (nBJet == 0) tHistSuffix = {"", tMassSuffix, "_0BJ", "_0BJ" + tMassSuffix};
  else tHistSuffix = {"", tMassSuffix};
  
  for (auto suffix : tHistSuffix) {
    if (!(fHistSet.find(name + suffix) == fHistSet.end())) {
      fHistSet[name + suffix]->Fill(value, weight);
    }
  }
}

void HistoSetEMU::FillHistoSet(std::string name, double mass, int nJet, int nBJet, float value, double weight) {

  std::string tMassSuffix = GetMassBin(SetMassOverflow(mass));

  std::vector<std::string> tHistSuffix;
  if (nBJet == 0) tHistSuffix = {"", tMassSuffix, "_0BJ", "_0BJ" + tMassSuffix};
  else tHistSuffix = {"", tMassSuffix};
  
  for (auto suffix : tHistSuffix) {
    if (!(fHistSet.find(name + suffix) == fHistSet.end())) {
      fHistSet[name + suffix]->Fill(value, weight);
    }
  }
}

void HistoSetEMU::FillHistoSet(std::string name, double mass, int nJet, int nBJet, int value, double weight) {

  std::string tMassSuffix = GetMassBin(SetMassOverflow(mass));

  std::vector<std::string> tHistSuffix;
  if (nBJet == 0) tHistSuffix = {"", tMassSuffix, "_0BJ", "_0BJ" + tMassSuffix};
  else tHistSuffix = {"", tMassSuffix};
  
  for (auto suffix : tHistSuffix) {
    if (!(fHistSet.find(name + suffix) == fHistSet.end())) {
      fHistSet[name + suffix]->Fill(value, weight);
    }
  }
}

void HistoSetEMU::FillHisto(std::string name, double value, double weight) {

  if (fHistSet.find(name) == fHistSet.end()) {
    std::cout << "Error: HistoSetEMU::FillHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }

  fHistSet[name]->Fill(value, weight);
}

void HistoSetEMU::FillHisto(std::string name, float value, double weight) {

  if (fHistSet.find(name) == fHistSet.end()) {
    std::cout << "Error: HistoSetEMU::FillHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }

  fHistSet[name]->Fill(value, weight);
}

void HistoSetEMU::FillHisto(std::string name, int value, double weight) {

  if (fHistSet.find(name) == fHistSet.end()) {
    std::cout << "Error: HistoSetEMU::FillHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }

  fHistSet[name]->Fill(value, weight);
}

void HistoSetEMU::SetHisto(std::string name, std::vector<double> bins) {

  if (fHistSet.find(name) == fHistSet.end()) {
    if (bins[0] == -9999) fHistSet[name] = new TH1D(name.c_str(), name.c_str(), bins[1], bins[2], bins[3]);
    else                 fHistSet[name] = new TH1D(name.c_str(), name.c_str(), bins.size() - 1, &(bins[0]));
  } else {
    if (bins[0] == -9999) fHistSet[name]->SetBins(bins[1], bins[2], bins[3]);
    else                 fHistSet[name]->SetBins(bins.size() - 1, &(bins[0]));
  }
}

void HistoSetEMU::SetHisto(std::string name, std::vector<double> bins1, std::vector<double> bins2) {

  if (fHistSet2D.find(name) == fHistSet2D.end()) {
    if (bins1[0] == -9999 && bins2[0] == -9999) fHistSet2D[name] = new TH2D(name.c_str(), name.c_str(), bins1[1], bins1[2], bins1[3], bins2[1], bins2[2], bins2[3]);
    else fHistSet2D[name] = new TH2D(name.c_str(), name.c_str(), bins1.size() - 1, &(bins1[0]), bins2.size() - 1, &(bins2[0]));
  } else {
    if (bins1[0] == -9999 && bins2[0] == -9999) fHistSet2D[name]->SetBins(bins1[1], bins1[2], bins1[3], bins2[1], bins2[2], bins2[3]);
    else fHistSet2D[name]->SetBins(bins1.size() - 1, &(bins1[0]), bins2.size() - 1, &(bins2[0]));
  }
}

void HistoSetEMU::SetHisto(std::string name) {
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
    std::cout << "Error: HistoSetEMU::SetHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }
}

void HistoSetEMU::SetHisto(std::string name, std::string binning) {
  
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
    std::cout << "Error: HistoSetEMU::SetHisto: Unknown histogram name: " << name << " " << binning << std::endl;
    exit(1);
  }
}

void HistoSetEMU::SetHisto(std::string name, std::string binning1, std::string binning2) {

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
    std::cout << "Error: HistoSetEMU::SetHisto: Unknown histogram name: " << name << " " << binning1 << std::endl;
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
    std::cout << "Error: HistoSetEMU::SetHisto: Unknown histogram name: " << name << " " << binning2 << std::endl;
    exit(1);
  }

  SetHisto(name, binEdge1, binEdge2);

}

std::string HistoSetEMU::GetMassBin(double fDimuonMass) {
  for (int i = 0; i < fMassBins.size() - 1; i++) {
    if (fDimuonMass >= fMassBins[i] && fDimuonMass < fMassBins[i+1]) {
      return "_m" + std::to_string((int)fMassBins[i]) + "_" + std::to_string((int)fMassBins[i+1]);
    }
  }
  return "";
}

double HistoSetEMU::SetPtOverflow(double fPt) {
  if (fPt > 1500) return 1510;
  else return fPt;
}
double HistoSetEMU::SetMassOverflow(double fMass) {
  if (fMass < 40) return 39.5;
  if (fMass >= 3000) return 3000.5;
  else return fMass;
}

void HistoSetEMU::FillEMUPair(
  const TLorentzVector& fMuon, 
  const TLorentzVector& fElec, 
  const int& nJet, 
  const int& nBJet, 
  const double& weight,
  const std::string& fType
) {

  TLorentzVector tEMuPair = fMuon + fElec;

  std::string tMassSuffix = GetMassBin(SetMassOverflow(tEMuPair.M()));

  std::vector<std::string> tHistSuffix;
  if (nBJet == 0) tHistSuffix = {"", tMassSuffix, "_0BJ", "_0BJ" + tMassSuffix};
  else tHistSuffix = {"", tMassSuffix};
  
  for (auto suffix : tHistSuffix) {
    
    fHistSet["h_" + fType + "_PairMass" + suffix]->Fill(SetMassOverflow(tEMuPair.M()), weight);

    if (tEMuPair.M() > 40) {
      fHistSet["h_" + fType + "_MuonPt" + suffix]->Fill(fMuon.Pt(), weight);
      fHistSet["h_" + fType + "_MuonEta" + suffix]->Fill(fMuon.Eta(), weight);
      fHistSet["h_" + fType + "_MuonPhi" + suffix]->Fill(fMuon.Phi(), weight);
      
      fHistSet["h_" + fType + "_ElecPt" + suffix]->Fill(fElec.Pt(), weight);
      fHistSet["h_" + fType + "_ElecEta" + suffix]->Fill(fElec.Eta(), weight);
      fHistSet["h_" + fType + "_ElecPhi" + suffix]->Fill(fElec.Phi(), weight);

      fHistSet["h_" + fType + "_PairPt" + suffix]->Fill(tEMuPair.Pt(), weight);
      fHistSet["h_" + fType + "_PairRap" + suffix]->Fill(tEMuPair.Rapidity(), weight);
    }
  }
}

void HistoSetEMU::FillJet(
  const std::vector<JET::StdJet>& fJet, 
  const std::vector<JET::StdJet>& fBJet, 
  const double& fDimuonMass, 
  const double& weight,
  const std::string& fType
) {

  std::string tMassSuffix = GetMassBin(SetMassOverflow(fDimuonMass));

  std::vector<std::string> tHistSuffix;
  if (fBJet.empty()) tHistSuffix = {"", tMassSuffix, "_0BJ", "_0BJ" + tMassSuffix};
  else tHistSuffix = {"", tMassSuffix};

  for (auto suffix : tHistSuffix) {

    fHistSet["h_" + fType + "_nJet" + suffix]->Fill(fJet.size(), weight);
    for (int i = 0; i < fJet.size(); i++) {
      fHistSet["h_" + fType + "_JetPt" + suffix]->Fill(SetPtOverflow(fJet.at(i).fVec.Pt()), weight);
      fHistSet["h_" + fType + "_JetEta" + suffix]->Fill(fJet.at(i).fVec.Eta(), weight);
      fHistSet["h_" + fType + "_JetPhi" + suffix]->Fill(fJet.at(i).fVec.Phi(), weight);
    }

    fHistSet["h_" + fType + "_nBJet" + suffix]->Fill(fBJet.size(), weight);
    for (int i = 0; i < fBJet.size(); i++) {
      fHistSet["h_" + fType + "_BJetPt" + suffix]->Fill(SetPtOverflow(fBJet.at(i).fVec.Pt()), weight);
      fHistSet["h_" + fType + "_BJetEta" + suffix]->Fill(fBJet.at(i).fVec.Eta(), weight);
      fHistSet["h_" + fType + "_BJetPhi" + suffix]->Fill(fBJet.at(i).fVec.Phi(), weight);
    }
  }
}

void HistoSetEMU::WriteHisto(TString fEra, TString fSampleName, TString fOutputDir, bool fIsData) {

  if (fSampleName.Contains("NNLO_MUMU_10to50"))
    fSampleName = "NNLO_MUMU_10to50";

  TFile* fOutputFile = new TFile(fOutputDir, "RECREATE");
  
  fOutputFile->mkdir(fEra + '/' + fSampleName);

  for (auto tSuffix : fSuffix)
    if (tSuffix != "")
      fOutputFile->mkdir(fEra + '/' + fSampleName + '/' + tSuffix);

  fOutputFile->cd(fEra + '/' + fSampleName);
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
      if (tSuffix != "") fOutputFile->cd(fEra + '/' + fSampleName + '/' + tSuffix);
     else                fOutputFile->cd(fEra + '/' + fSampleName);

      fHistSet["h_" + fType + "_ElecPt" + tSuffix]->Write();
      fHistSet["h_" + fType + "_ElecEta" + tSuffix]->Write();
      fHistSet["h_" + fType + "_ElecPhi" + tSuffix]->Write();

      fHistSet["h_" + fType + "_MuonPt" + tSuffix]->Write();
      fHistSet["h_" + fType + "_MuonEta" + tSuffix]->Write();
      fHistSet["h_" + fType + "_MuonPhi" + tSuffix]->Write();

      fHistSet["h_" + fType + "_PairMass" + tSuffix]->Write();
      fHistSet["h_" + fType + "_PairPt" + tSuffix]->Write();
      fHistSet["h_" + fType + "_PairRap" + tSuffix]->Write();

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

        fHistSet["h_" + fType + "_ElecPt" + tSuffix]->Write();
        fHistSet["h_" + fType + "_ElecEta" + tSuffix]->Write();
        fHistSet["h_" + fType + "_ElecPhi" + tSuffix]->Write();

        fHistSet["h_" + fType + "_MuonPt" + tSuffix]->Write();
        fHistSet["h_" + fType + "_MuonEta" + tSuffix]->Write();
        fHistSet["h_" + fType + "_MuonPhi" + tSuffix]->Write();

        fHistSet["h_" + fType + "_PairMass" + tSuffix]->Write();
        fHistSet["h_" + fType + "_PairPt" + tSuffix]->Write();
        fHistSet["h_" + fType + "_PairRap" + tSuffix]->Write();

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


  if (fHistSet2D.size() > 0) {
    fOutputFile->mkdir("Hist2D");

    for (auto [name, hist] : fHistSet2D) {
      hist->SetDirectory(fOutputFile);
      fOutputFile->cd("Hist2D");
      hist->Write();
    }
  }

  fOutputFile->Close();
}

