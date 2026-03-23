#include "HistoSetEE.h"
#include "jet.h"

#include "TFile.h"
#include "TLorentzVector.h"
#include "TH1.h"


void HistoSetEE::Init() {

  TH1::SetDefaultSumw2();
  fSuffix.clear();

  SetHisto("h_EventInfo", std::vector<double>{-9999, 5, 0.5, 5.5});
  SetHisto("h_GenWeight", std::vector<double>{-9999, 20000, -10000., 10000.});
  SetHisto("h_LHEDielecMass", std::vector<double>{-9999, 6000, 0., 6000.});
  SetHisto("h_LHEnElec", std::vector<double>{-9999, 10, 0., 10.});

  SetHisto("h_nPVGood_Count", std::vector<double>{-9999, 100, 0., 100.});
  SetHisto("h_PileUp_Count_Interaction_before", std::vector<double>{-9999, 100, 0., 100.});
  SetHisto("h_PileUp_Count_Interaction_after", std::vector<double>{-9999, 100, 0., 100.});

  fPtBins = { 0,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95,  
            100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165, 170, 175, 180, 185, 
            190, 195, 200, 205, 210, 215, 220, 225, 230, 235, 240, 245, 250, 255, 260, 265, 270, 275, 
            280, 285, 290, 295, 300, 305, 310, 315, 320, 325, 330, 335, 340, 345, 350, 355, 360, 365, 
            370, 375, 380, 385, 390, 395, 400, 405, 410, 415, 420, 425, 430, 435, 440, 445, 450, 455,
            460, 465, 470, 475, 480, 485, 490, 495, 500, 510, 520, 530, 540, 550, 560, 570, 580, 590, 600,
            610, 620, 630, 640, 650, 660, 670, 680, 690, 700, 710, 720, 730, 740, 750, 760, 770, 780, 790, 800,
            810, 820, 830, 840, 850, 860, 870, 880, 890, 900, 910, 920, 930, 940, 950, 960, 970, 980, 990, 1000,
            1020, 1040, 1060, 1080, 1100, 1120, 1140, 1160, 1180, 1200, 1220, 1240, 1260, 1280, 1300, 1320, 1340, 1360, 1380, 1400,
            1420, 1440, 1460, 1480, 1500, 1520};

  fEtaBins = {-9999, 60, -3., 3.};
  fPhiBins = {-9999, 60, -3.141593, 3.141593};
  fMassBins = {199, 200,  220,  243, 273, 320, 380, 440, 510, 600, 700, 830, 1000, 1500, 4000, 4001};
  // fMassBins = {39, 40, 45, 50, 55, 60, 64, 68, 72, 76, 81, 86, 91, 96, 101, 106, 110, 115, 120, 126, 133, 141, 150, 160, 171, 185, 200, 220, 243, 273, 320, 380, 440, 510, 600, 700, 830, 1000, 1500, 4000, 4001};

  fDeltaRBins = {-9999, 100, 0.0, 6.0};
  fNJetBins = {-9999, 20, 0, 20};

  SetHisto("h_GenAcc_Denom", fMassBins);
  SetHisto("h_GenAcc_Numer", fMassBins);
  // Denominator is identical to h_GenAcc_Numer (gen fiducial)
  SetHisto("h_GenEff_Numer", fMassBins);

  SetHisto("h_nHardGenElec", std::vector<double>{-9999, 10, 0., 10.});

  // ------------------------------------------------------------------
  // Reco–gen matching / charge mis-id study histograms (global, no suffix)
  // ------------------------------------------------------------------
  // Reco–gen ΔR: use finer variable binning near 0
  std::vector<double> tRecoGenDeltaRBins;
  for (int i = 0; i <= 200; ++i) tRecoGenDeltaRBins.push_back(0.0 + 0.001 * i);  // 0.000 ... 0.200
  for (int i = 1; i <= 80;  ++i) tRecoGenDeltaRBins.push_back(0.2 + 0.01  * i);  // 0.210 ... 1.000
  for (int i = 1; i <= 50;  ++i) tRecoGenDeltaRBins.push_back(1.0 + 0.1   * i);  // 1.100 ... 6.000
  SetHisto("h_RecoGenDeltaR", tRecoGenDeltaRBins);

  std::vector<std::string> fAddonMass = {""};
  std::vector<std::string> fAddonJet = {"", "_0J", "_1J", "_mt1J", "_0BJ", "_1BJ", "_mt1BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"};
  
  for (int i = 0; i < fMassBins.size() -1; i++) {
    fAddonMass.push_back("_m" + std::to_string((int)fMassBins[i]) + "_" + std::to_string((int)fMassBins[i+1]));
  }

  for (int i = 0; i < fAddonMass.size(); i++) {
    for (int j = 0; j < fAddonJet.size(); j++) {
      std::string tHistSuffix = fAddonJet[j] + fAddonMass[i];
      fSuffix.push_back(tHistSuffix);

      SetHisto("h_nJet" + tHistSuffix);    
      SetHisto("h_JetPt" + tHistSuffix);
      SetHisto("h_JetEta" + tHistSuffix);
      SetHisto("h_JetPhi" + tHistSuffix);

      SetHisto("h_nBJet" + tHistSuffix);
      SetHisto("h_BJetPt" + tHistSuffix);
      SetHisto("h_BJetEta" + tHistSuffix);
      SetHisto("h_BJetPhi" + tHistSuffix);

      SetHisto("h_LeadingElecPt" + tHistSuffix);
      SetHisto("h_LeadingElecEta" + tHistSuffix);
      SetHisto("h_LeadingElecPhi" + tHistSuffix);

      SetHisto("h_SubleadingElecPt" + tHistSuffix);
      SetHisto("h_SubleadingElecEta" + tHistSuffix);
      SetHisto("h_SubleadingElecPhi" + tHistSuffix);

      SetHisto("h_ElecPt" + tHistSuffix);
      SetHisto("h_ElecEta" + tHistSuffix);
      SetHisto("h_ElecPhi" + tHistSuffix);
      SetHisto("h_ElecDeltaR" + tHistSuffix, fDeltaRBins);

      SetHisto("h_dielecMass" + tHistSuffix);
      SetHisto("h_dielecMass_1GeVBin" + tHistSuffix, std::vector<double>{-9999, 6000, 0., 6000.});
      SetHisto("h_dielecPt" + tHistSuffix);
      SetHisto("h_dielecRap" + tHistSuffix, "Eta");
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

void HistoSetEE::FillHisto(std::string name, double value, double weight) {

  if (fHistSet.find(name) == fHistSet.end()) {
    std::cout << "Error: HistoSetEE::FillHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }

  fHistSet[name]->Fill(value, weight);
}

void HistoSetEE::FillHisto(std::string name, float value, double weight) {

  if (fHistSet.find(name) == fHistSet.end()) {
    std::cout << "Error: HistoSetEE::FillHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }

  fHistSet[name]->Fill(value, weight);
}

void HistoSetEE::FillHisto(std::string name, int value, double weight) {

  if (fHistSet.find(name) == fHistSet.end()) {
    std::cout << "Error: HistoSetEE::FillHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }

  fHistSet[name]->Fill(value, weight);
}

void HistoSetEE::SetHisto(std::string name, std::vector<double> bins) {

  if (fHistSet.find(name) == fHistSet.end()) {
    if (bins[0] == -9999) fHistSet[name] = new TH1D(name.c_str(), name.c_str(), bins[1], bins[2], bins[3]);
    else                 fHistSet[name] = new TH1D(name.c_str(), name.c_str(), bins.size() - 1, &(bins[0]));
  } else {
    if (bins[0] == -9999) fHistSet[name]->SetBins(bins[1], bins[2], bins[3]);
    else                 fHistSet[name]->SetBins(bins.size() - 1, &(bins[0]));
  }
}

void HistoSetEE::SetHisto(std::string name, std::vector<double> bins1, std::vector<double> bins2) {

  if (fHistSet2D.find(name) == fHistSet2D.end()) {
    if (bins1[0] == -9999 && bins2[0] == -9999) fHistSet2D[name] = new TH2D(name.c_str(), name.c_str(), bins1[1], bins1[2], bins1[3], bins2[1], bins2[2], bins2[3]);
    else fHistSet2D[name] = new TH2D(name.c_str(), name.c_str(), bins1.size() - 1, &(bins1[0]), bins2.size() - 1, &(bins2[0]));
  } else {
    if (bins1[0] == -9999 && bins2[0] == -9999) fHistSet2D[name]->SetBins(bins1[1], bins1[2], bins1[3], bins2[1], bins2[2], bins2[3]);
    else fHistSet2D[name]->SetBins(bins1.size() - 1, &(bins1[0]), bins2.size() - 1, &(bins2[0]));
  }
}

void HistoSetEE::SetHisto(std::string name) {
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
    std::cout << "Error: HistoSetEE::SetHisto: Unknown histogram name: " << name << std::endl;
    exit(1);
  }
}

void HistoSetEE::SetHisto(std::string name, std::string binning) {
  
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
    std::cout << "Error: HistoSetEE::SetHisto: Unknown histogram name: " << name << " " << binning << std::endl;
    exit(1);
  }
}

void HistoSetEE::SetHisto(std::string name, std::string binning1, std::string binning2) {

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
    std::cout << "Error: HistoSetEE::SetHisto: Unknown histogram name: " << name << " " << binning1 << std::endl;
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
    std::cout << "Error: HistoSetEE::SetHisto: Unknown histogram name: " << name << " " << binning2 << std::endl;
    exit(1);
  }

  SetHisto(name, binEdge1, binEdge2);

}

std::string HistoSetEE::GetMassBin(double fDimuonMass) {
  for (int i = 0; i < fMassBins.size() - 1; i++) {
    if (fDimuonMass >= fMassBins[i] && fDimuonMass < fMassBins[i+1]) {
      return "_m" + std::to_string((int)fMassBins[i]) + "_" + std::to_string((int)fMassBins[i+1]);
    }
  }
  return "";
}

std::string HistoSetEE::GetJetBin(double fNJet) {
  
  if (fNJet == 0) return "_0J";
  else if (fNJet == 1) return "_1J";
  else if (fNJet >= 2) return "_mt1J";
  else return "";
}

std::string HistoSetEE::GetBJetBin(double fNBJet) {
  
  if (fNBJet == 0) return "_0BJ";
  else if (fNBJet == 1) return "_1BJ";
  else if (fNBJet >= 2) return "_mt1BJ";
  else return "";
}

std::string HistoSetEE::GetbVetoJetBin(double fNJet) {
  
  if (fNJet == 0) return "_bVeto_0J";
  else if (fNJet == 1) return "_bVeto_1J";
  else if (fNJet >= 2) return "_bVeto_mt1J";
  else return "";
}

double HistoSetEE::SetPtOverflow(double fPt) {
  if (fPt > 1500) return 1510;
  else return fPt;
}
double HistoSetEE::SetMassOverflow(double fMass) {
  if (fMass < 200) return 199.5;
  // if (fMass < 40) return 39.5;
  if (fMass >= 4000) return 4000.5;
  else return fMass;
}

void HistoSetEE::FillElec(TLorentzVector& fLeadingElec, TLorentzVector& fSubleadingElec, int nJet, int nBJet, double weight) {

  TLorentzVector fDielec = fLeadingElec + fSubleadingElec;

  std::string tMassSuffix = GetMassBin(SetMassOverflow(fDielec.M()));
  std::string tJetSuffix = GetJetBin(nJet);
  std::string tBJetSuffix = GetBJetBin(nBJet);
  std::string tbVetoJetSuffix = GetbVetoJetBin(nJet);

  std::vector<std::string> tHistSuffix;
  if (nBJet == 0) tHistSuffix = {"", tMassSuffix, tJetSuffix, tJetSuffix + tMassSuffix, tBJetSuffix, tBJetSuffix + tMassSuffix, tbVetoJetSuffix, tbVetoJetSuffix + tMassSuffix};
  else tHistSuffix = {"", tMassSuffix, tJetSuffix, tJetSuffix + tMassSuffix, tBJetSuffix, tBJetSuffix + tMassSuffix};
  
  for (auto suffix : tHistSuffix) {

    fHistSet["h_LeadingElecPt" + suffix]->Fill(SetPtOverflow(fLeadingElec.Pt()), weight);
    fHistSet["h_LeadingElecEta" + suffix]->Fill(fLeadingElec.Eta(), weight);
    fHistSet["h_LeadingElecPhi" + suffix]->Fill(fLeadingElec.Phi(), weight);

    fHistSet["h_SubleadingElecPt" + suffix]->Fill(SetPtOverflow(fSubleadingElec.Pt()), weight);
    fHistSet["h_SubleadingElecEta" + suffix]->Fill(fSubleadingElec.Eta(), weight);
    fHistSet["h_SubleadingElecPhi" + suffix]->Fill(fSubleadingElec.Phi(), weight);

    fHistSet["h_ElecPt" + suffix]->Fill(SetPtOverflow(fLeadingElec.Pt()), weight);
    fHistSet["h_ElecEta" + suffix]->Fill(fLeadingElec.Eta(), weight);
    fHistSet["h_ElecPhi" + suffix]->Fill(fLeadingElec.Phi(), weight);

    fHistSet["h_ElecPt" + suffix]->Fill(SetPtOverflow(fSubleadingElec.Pt()), weight);
    fHistSet["h_ElecEta" + suffix]->Fill(fSubleadingElec.Eta(), weight);
    fHistSet["h_ElecPhi" + suffix]->Fill(fSubleadingElec.Phi(), weight);

    fHistSet["h_ElecDeltaR" + suffix]->Fill(fLeadingElec.DeltaR(fSubleadingElec), weight);

    fHistSet["h_dielecMass" + suffix]->Fill(SetMassOverflow(fDielec.M()), weight);
    fHistSet["h_dielecMass_1GeVBin" + suffix]->Fill(fDielec.M(), weight);
    fHistSet["h_dielecPt" + suffix]->Fill(SetPtOverflow(fDielec.Pt()), weight);
    fHistSet["h_dielecRap" + suffix]->Fill(fDielec.Rapidity(), weight);
  }
}

void HistoSetEE::FillJet(std::vector<JET::StdJet>* fJet, std::vector<JET::StdJet>* fBJet, double fDielecMass, double weight) {
  
  std::string tMassSuffix = GetMassBin(SetMassOverflow(fDielecMass));
  std::string tJetSuffix = GetJetBin(fJet->size());
  std::string tBJetSuffix = GetBJetBin(fBJet->size());
  std::string tbVetoJetSuffix = GetbVetoJetBin(fJet->size());

  std::vector<std::string> tHistSuffix;
  if (fBJet->size() == 0) tHistSuffix = {"", tMassSuffix, tJetSuffix, tJetSuffix + tMassSuffix, tBJetSuffix, tBJetSuffix + tMassSuffix, tbVetoJetSuffix, tbVetoJetSuffix + tMassSuffix};
  else tHistSuffix = {"", tMassSuffix, tJetSuffix, tJetSuffix + tMassSuffix, tBJetSuffix, tBJetSuffix + tMassSuffix};

  for (auto suffix : tHistSuffix) {

    fHistSet["h_nJet" + suffix]->Fill(fJet->size(), weight);
    for (int i = 0; i < fJet->size(); i++) {
      fHistSet["h_JetPt" + suffix]->Fill(SetPtOverflow(fJet->at(i).fVec.Pt()), weight);
      fHistSet["h_JetEta" + suffix]->Fill(fJet->at(i).fVec.Eta(), weight);
      fHistSet["h_JetPhi" + suffix]->Fill(fJet->at(i).fVec.Phi(), weight);
    }

    fHistSet["h_nBJet" + suffix]->Fill(fBJet->size(), weight);
    for (int i = 0; i < fBJet->size(); i++) {
      fHistSet["h_BJetPt" + suffix]->Fill(SetPtOverflow(fBJet->at(i).fVec.Pt()), weight);
      fHistSet["h_BJetEta" + suffix]->Fill(fBJet->at(i).fVec.Eta(), weight);
      fHistSet["h_BJetPhi" + suffix]->Fill(fBJet->at(i).fVec.Phi(), weight);
    }
  }
}

void HistoSetEE::WriteHisto(TString fEra, TString fSampleName, TString fOutputDir, bool fIsData) {
  
  if (fSampleName.Contains("NNLO_EE_10to50"))
    fSampleName = "NNLO_EE_10to50";

  TFile* fOutputFile = new TFile(fOutputDir, "RECREATE");
  
  fOutputFile->mkdir(fEra + '/' + fSampleName);

  for (auto tSuffix : fSuffix)
    if (tSuffix != "")
      fOutputFile->mkdir(fEra + '/' + fSampleName + '/' + tSuffix);

  fOutputFile->cd(fEra + '/' + fSampleName);
  fHistSet["h_EventInfo"]->Write();
  fHistSet["h_GenWeight"]->Write();
  fHistSet["h_LHEDielecMass"]->Write();
  fHistSet["h_LHEnElec"]->Write();
  fHistSet["h_nPVGood_Count"]->Write();
  fHistSet["h_PileUp_Count_Interaction_before"]->Write();
  fHistSet["h_PileUp_Count_Interaction_after"]->Write();
  fHistSet["h_GenAcc_Denom"]->Write();
  fHistSet["h_GenAcc_Numer"]->Write();
  fHistSet["h_GenEff_Numer"]->Write();
  fHistSet["h_nHardGenElec"]->Write();
  fHistSet["h_RecoGenDeltaR"]->Write();

  for (auto tSuffix : fSuffix) {
    if (tSuffix != "")
      fOutputFile->cd(fEra + '/' + fSampleName + '/' + tSuffix);

    fHistSet["h_LeadingElecPt" + tSuffix]->Write();
    fHistSet["h_LeadingElecEta" + tSuffix]->Write();
    fHistSet["h_LeadingElecPhi" + tSuffix]->Write();

    fHistSet["h_SubleadingElecPt" + tSuffix]->Write();
    fHistSet["h_SubleadingElecEta" + tSuffix]->Write();
    fHistSet["h_SubleadingElecPhi" + tSuffix]->Write();

    fHistSet["h_ElecPt" + tSuffix]->Write();  
    fHistSet["h_ElecEta" + tSuffix]->Write();
    fHistSet["h_ElecPhi" + tSuffix]->Write();

    fHistSet["h_ElecDeltaR" + tSuffix]->Write();
    fHistSet["h_dielecMass" + tSuffix]->Write();
    fHistSet["h_dielecMass_1GeVBin" + tSuffix]->Write();
    fHistSet["h_dielecPt" + tSuffix]->Write();
    fHistSet["h_dielecRap" + tSuffix]->Write();
    
    fHistSet["h_nJet" + tSuffix]->Write();
    fHistSet["h_JetPt" + tSuffix]->Write();
    fHistSet["h_JetEta" + tSuffix]->Write();
    fHistSet["h_JetPhi" + tSuffix]->Write();
    
    fHistSet["h_nBJet" + tSuffix]->Write();
    fHistSet["h_BJetPt" + tSuffix]->Write();
    fHistSet["h_BJetEta" + tSuffix]->Write();
    fHistSet["h_BJetPhi" + tSuffix]->Write();
  }

  if (fIsData) {
    
    fOutputFile->mkdir(fEra + "/Data");
    fOutputFile->cd(fEra + "/Data");

    fHistSet["h_EventInfo"]->Write();
    fHistSet["h_GenWeight"]->Write();
    fHistSet["h_LHEDielecMass"]->Write();
    fHistSet["h_LHEnElec"]->Write();
    fHistSet["h_nPVGood_Count"]->Write();
    fHistSet["h_PileUp_Count_Interaction_before"]->Write();
    fHistSet["h_PileUp_Count_Interaction_after"]->Write();
    fHistSet["h_GenAcc_Denom"]->Write();
    fHistSet["h_GenAcc_Numer"]->Write();
    fHistSet["h_GenEff_Numer"]->Write();
    fHistSet["h_RecoGenDeltaR"]->Write();

    for (auto tSuffix : fSuffix) 
      if (tSuffix != "")
        fOutputFile->mkdir(fEra + "/Data/" + tSuffix);

    for (auto tSuffix : fSuffix) {
      if (tSuffix != "") fOutputFile->cd(fEra + "/Data/" + tSuffix);
      else               fOutputFile->cd(fEra + "/Data");

      fHistSet["h_LeadingElecPt" + tSuffix]->Write();
      fHistSet["h_LeadingElecEta" + tSuffix]->Write();
      fHistSet["h_LeadingElecPhi" + tSuffix]->Write();

      fHistSet["h_SubleadingElecPt" + tSuffix]->Write();
      fHistSet["h_SubleadingElecEta" + tSuffix]->Write();
      fHistSet["h_SubleadingElecPhi" + tSuffix]->Write();

      fHistSet["h_ElecPt" + tSuffix]->Write();  
      fHistSet["h_ElecEta" + tSuffix]->Write();
      fHistSet["h_ElecPhi" + tSuffix]->Write();

      fHistSet["h_ElecDeltaR" + tSuffix]->Write();
      fHistSet["h_dielecMass" + tSuffix]->Write();
      fHistSet["h_dielecMass_1GeVBin" + tSuffix]->Write();
      fHistSet["h_dielecPt" + tSuffix]->Write();
      fHistSet["h_dielecRap" + tSuffix]->Write();
      
      fHistSet["h_nJet" + tSuffix]->Write();
      fHistSet["h_JetPt" + tSuffix]->Write();
      fHistSet["h_JetEta" + tSuffix]->Write();
      fHistSet["h_JetPhi" + tSuffix]->Write();
      
      fHistSet["h_nBJet" + tSuffix]->Write();
      fHistSet["h_BJetPt" + tSuffix]->Write();
      fHistSet["h_BJetEta" + tSuffix]->Write();
      fHistSet["h_BJetPhi" + tSuffix]->Write();
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

