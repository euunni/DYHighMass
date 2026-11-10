#include "HistoSetMUMU.h"
#include "jet.h"

#include "TFile.h"
#include "TLorentzVector.h"
#include "TH1.h"


void HistoSetMUMU::Init() {

  BookHisto(HistoSetBase::HistGroup::EventInfo, "EventInfo", "h_EventInfo", std::vector<double>{-9999, 5, 0.5, 5.5});
  BookHisto(HistoSetBase::HistGroup::EventInfo, "GenWeight", "h_GenWeight", std::vector<double>{-9999, 1000, -10000., 10000.});
  BookHisto(HistoSetBase::HistGroup::EventInfo, "LHEDimuonMass", "h_LHEDimuonMass", std::vector<double>{-9999, 6000, 0., 6000.});
  BookHisto(HistoSetBase::HistGroup::EventInfo, "nLHEMuon", "h_LHEnMuon", std::vector<double>{-9999, 10, 0., 10.});
  
  // SetHisto("h_nPVGood_Count", std::vector<double>{-9999, 100, 0., 100.});
  // SetHisto("h_PileUp_Count_Interaction_before", std::vector<double>{-9999, 100, 0., 100.});
  // SetHisto("h_PileUp_Count_Interaction_after", std::vector<double>{-9999, 100, 0., 100.});

  std::vector<std::string> fAddonMass = {"inc"};
  std::vector<std::string> fAddonJet = {"inc", "0J", "1J", "mt1J", "0BJ", "1BJ", "mt1BJ", "bVeto", "bVeto_0J", "bVeto_1J", "bVeto_mt1J"};
  std::vector<std::string> fAddonType = {"OS", "SS", "OS_inverted", "SS_inverted"};

  if (GetMassExclusive())
    for (int i = 0; i < fMassBins.size() -1; i++)
      fAddonMass.push_back("m" + std::to_string((int)fMassBins[i]) + "_" + std::to_string((int)fMassBins[i+1]));

  for (auto tMassBin : fAddonMass) {
    for (auto tJetBin : fAddonJet) {
      for (auto tType : fAddonType) {

        BookHisto(tType, tJetBin, tMassBin, "nJet", "h_nJet");
        BookHisto(tType, tJetBin, tMassBin, "JetPt", "h_JetPt");
        BookHisto(tType, tJetBin, tMassBin, "JetEta", "h_JetEta");
        BookHisto(tType, tJetBin, tMassBin, "JetPhi", "h_JetPhi");

        BookHisto(tType, tJetBin, tMassBin, "nBJet", "h_nBJet");
        BookHisto(tType, tJetBin, tMassBin, "BJetPt", "h_BJetPt");
        BookHisto(tType, tJetBin, tMassBin, "BJetEta", "h_BJetEta");
        BookHisto(tType, tJetBin, tMassBin, "BJetPhi", "h_BJetPhi");

        BookHisto(tType, tJetBin, tMassBin, "LeadingMuonPt", "h_LeadingMuonPt");
        BookHisto(tType, tJetBin, tMassBin, "LeadingMuonEta", "h_LeadingMuonEta");
        BookHisto(tType, tJetBin, tMassBin, "LeadingMuonPhi", "h_LeadingMuonPhi");

        BookHisto(tType, tJetBin, tMassBin, "SubleadingMuonPt", "h_SubleadingMuonPt");
        BookHisto(tType, tJetBin, tMassBin, "SubleadingMuonEta", "h_SubleadingMuonEta");
        BookHisto(tType, tJetBin, tMassBin, "SubleadingMuonPhi", "h_SubleadingMuonPhi");

        BookHisto(tType, tJetBin, tMassBin, "MuonPt", "h_MuonPt");
        BookHisto(tType, tJetBin, tMassBin, "MuonEta", "h_MuonEta");
        BookHisto(tType, tJetBin, tMassBin, "MuonPhi", "h_MuonPhi");
        BookHisto(tType, tJetBin, tMassBin, "MuonCharge", "h_MuonCharge");

        BookHisto(tType, tJetBin, tMassBin, "dimuonMass", "h_dimuonMass");
        BookHisto(tType, tJetBin, tMassBin, "dimuonMassFailGen", "h_dimuonMassFailGen");
        BookHisto(tType, tJetBin, tMassBin, "dimuonPt", "h_dimuonPt");
        BookHisto(tType, tJetBin, tMassBin, "dimuonRap", "h_dimuonRap");

      }
    }
  }

  std::cout << "######################################################################" << std::endl;
  std::cout << "                             Hist setting                             " << std::endl;
  std::cout << "----------------------------------------------------------------------" << std::endl;
  for (int i = 0; i < fMassBins.size() - 1; i++)
    std::cout << "    m" << (int)fMassBins[i] << "_" << (int)fMassBins[i+1] << std::endl;
  std::cout << "######################################################################" << std::endl;
  std::cout << " " << std::endl;
}

void HistoSetMUMU::InitGenInfo() {

  double fMassBin[] = {200,  220,  243, 273, 320, 380, 440, 510, 600, 700, 830, 1000, 1500, 4000};
  double fJetBin[] = {-0.5, 0.5, 1.5, 2.5};


  // fGlobalBinningGen`("generator");
  fGlobalBinningGen.AddAxis("mass", 13, fMassBin, true, true);
  fGlobalBinningGen.AddAxis("njet", 3, fJetBin, false, false);


  // fGlobalBinningReco("reco");
  fGlobalBinningReco.AddAxis("mass", 13, fMassBin, true, true);
  fGlobalBinningReco.AddAxis("njet", 3, fJetBin, false, false);

  auto fResponseMatrix = TUnfoldBinning::CreateHistogramOfMigrations(&fGlobalBinningReco, &fGlobalBinningGen, "h_ResponseMatrix");
  BookHisto(HistoSetBase::HistGroup::GenInfo, "ResponseMatrix", fResponseMatrix);


  std::vector<std::string> fAddonJet = {"", "_0J", "_1J", "_mt1J"};
  for (int i = 0; i < fAddonJet.size(); i++) {

    std::string tHistSuffix = fAddonJet[i];

    BookHisto(HistoSetBase::HistGroup::GenInfo, "nJet_WithReco" + tHistSuffix, "h_nJet_WithReco" + tHistSuffix, fNJetBins);

    BookHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonPt_WithReco" + tHistSuffix, "h_LeadingMuonPt_WithReco" + tHistSuffix, fPtBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonEta_WithReco" + tHistSuffix, "h_LeadingMuonEta_WithReco" + tHistSuffix, fEtaBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonPhi_WithReco" + tHistSuffix, "h_LeadingMuonPhi_WithReco" + tHistSuffix, fPhiBins);

    BookHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonPt_WithReco" + tHistSuffix, "h_SubleadingMuonPt_WithReco" + tHistSuffix, fPtBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonEta_WithReco" + tHistSuffix, "h_SubleadingMuonEta_WithReco" + tHistSuffix, fEtaBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonPhi_WithReco" + tHistSuffix, "h_SubleadingMuonPhi_WithReco" + tHistSuffix, fPhiBins);

    BookHisto(HistoSetBase::HistGroup::GenInfo, "MuonPt_WithReco" + tHistSuffix, "h_MuonPt_WithReco" + tHistSuffix, fPtBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "MuonEta_WithReco" + tHistSuffix, "h_MuonEta_WithReco" + tHistSuffix, fEtaBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "MuonPhi_WithReco" + tHistSuffix, "h_MuonPhi_WithReco" + tHistSuffix, fPhiBins);

    BookHisto(HistoSetBase::HistGroup::GenInfo, "dimuonMass_WithReco" + tHistSuffix, "h_dimuonMass_WithReco" + tHistSuffix, fMassBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "dimuonPt_WithReco" + tHistSuffix, "h_dimuonPt_WithReco" + tHistSuffix, fPtBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "dimuonRap_WithReco" + tHistSuffix, "h_dimuonRap_WithReco" + tHistSuffix, fEtaBins);

    BookHisto(HistoSetBase::HistGroup::GenInfo, "nJet" + tHistSuffix, "h_nJet" + tHistSuffix, fNJetBins);

    BookHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonPt" + tHistSuffix, "h_LeadingMuonPt" + tHistSuffix, fPtBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonEta" + tHistSuffix, "h_LeadingMuonEta" + tHistSuffix, fEtaBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonPhi" + tHistSuffix, "h_LeadingMuonPhi" + tHistSuffix, fPhiBins);

    BookHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonPt" + tHistSuffix, "h_SubleadingMuonPt" + tHistSuffix, fPtBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonEta" + tHistSuffix, "h_SubleadingMuonEta" + tHistSuffix, fEtaBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonPhi" + tHistSuffix, "h_SubleadingMuonPhi" + tHistSuffix, fPhiBins);

    BookHisto(HistoSetBase::HistGroup::GenInfo, "MuonPt" + tHistSuffix, "h_MuonPt" + tHistSuffix, fPtBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "MuonEta" + tHistSuffix, "h_MuonEta" + tHistSuffix, fEtaBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "MuonPhi" + tHistSuffix, "h_MuonPhi" + tHistSuffix, fPhiBins);

    BookHisto(HistoSetBase::HistGroup::GenInfo, "dimuonMass" + tHistSuffix, "h_dimuonMass" + tHistSuffix, fMassBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "dimuonPt" + tHistSuffix, "h_dimuonPt" + tHistSuffix, fPtBins);
    BookHisto(HistoSetBase::HistGroup::GenInfo, "dimuonRap" + tHistSuffix, "h_dimuonRap" + tHistSuffix, fEtaBins);
  }
}

void HistoSetMUMU::FillMuon(
  const TLorentzVector& fLeadingMuon, 
  const TLorentzVector& fSubleadingMuon, 
  const float& fCharge, 
  const int& nJet, 
  const int& nBJet, 
  const double& fWeight,
  const std::string& tType,
  const bool& fHasGen
) {

  TLorentzVector fDimuon = fLeadingMuon + fSubleadingMuon;

  std::vector<TString> vMassSuffix = {"inc"};
  if (GetMassExclusive()) vMassSuffix.push_back(GetMassBin(SetMassOverflow(fDimuon.M())));

  std::vector<TString> vJetSuffix = {
    "inc",
    GetJetBin(nJet).c_str(),
    GetBJetBin(nBJet).c_str(),
  };
  if (nBJet == 0) {
    vJetSuffix.push_back("bVeto");
    vJetSuffix.push_back(GetbVetoJetBin(nJet).c_str());
  }
  
  for (auto massSuffix : vMassSuffix) {
    for (auto jetSuffix : vJetSuffix) {

      if (fDimuon.M() > 200) {

        FillHisto(tType, jetSuffix, massSuffix, "LeadingMuonPt", SetPtOverflow(fLeadingMuon.Pt()), fWeight);
        FillHisto(tType, jetSuffix, massSuffix, "LeadingMuonEta", fLeadingMuon.Eta(), fWeight);
        FillHisto(tType, jetSuffix, massSuffix, "LeadingMuonPhi", fLeadingMuon.Phi(), fWeight);

        FillHisto(tType, jetSuffix, massSuffix, "SubleadingMuonPt", SetPtOverflow(fSubleadingMuon.Pt()), fWeight);
        FillHisto(tType, jetSuffix, massSuffix, "SubleadingMuonEta", fSubleadingMuon.Eta(), fWeight);
        FillHisto(tType, jetSuffix, massSuffix, "SubleadingMuonPhi", fSubleadingMuon.Phi(), fWeight);

        FillHisto(tType, jetSuffix, massSuffix, "MuonPt", SetPtOverflow(fLeadingMuon.Pt()), fWeight);
        FillHisto(tType, jetSuffix, massSuffix, "MuonEta", fLeadingMuon.Eta(), fWeight);
        FillHisto(tType, jetSuffix, massSuffix, "MuonPhi", fLeadingMuon.Phi(), fWeight);

        FillHisto(tType, jetSuffix, massSuffix, "MuonPt", SetPtOverflow(fSubleadingMuon.Pt()), fWeight);
        FillHisto(tType, jetSuffix, massSuffix, "MuonEta", fSubleadingMuon.Eta(), fWeight);
        FillHisto(tType, jetSuffix, massSuffix, "MuonPhi", fSubleadingMuon.Phi(), fWeight);
        FillHisto(tType, jetSuffix, massSuffix, "MuonCharge", fCharge, fWeight);

        FillHisto(tType, jetSuffix, massSuffix, "dimuonPt", SetPtOverflow(fDimuon.Pt()), fWeight);
        FillHisto(tType, jetSuffix, massSuffix, "dimuonRap", fDimuon.Rapidity(), fWeight);

      }

      FillHisto(tType, jetSuffix, massSuffix, "dimuonMass", SetMassOverflow(fDimuon.M()), fWeight);
      if (!fHasGen) FillHisto(tType, jetSuffix, massSuffix, "dimuonMassFailGen", SetMassOverflow(fDimuon.M()), fWeight);
    }
  }
}

void HistoSetMUMU::FillJet(
  const std::vector<JET::StdJet>& fJet, 
  const std::vector<JET::StdJet>& fBJet, 
  const double& fDimuonMass, 
  const double& fWeight,
  const std::string& tType
) {

  std::vector<TString> vMassSuffix = {"inc"};
  if (GetMassExclusive()) vMassSuffix.push_back(GetMassBin(SetMassOverflow(fDimuonMass)));

  std::vector<TString> vJetSuffix = {
    "inc",
    GetJetBin(fJet.size()).c_str(),
    GetBJetBin(fBJet.size()).c_str()
  };
  if (fBJet.empty()) {
    vJetSuffix.push_back("bVeto");
    vJetSuffix.push_back(GetbVetoJetBin(fJet.size()).c_str());
  }

  if (fDimuonMass > 200) {
    for (auto massSuffix : vMassSuffix) {
      for (auto jetSuffix : vJetSuffix) {

        FillHisto(tType, jetSuffix, massSuffix, "nJet", fJet.size(), fWeight);
        for (int i = 0; i < fJet.size(); i++) {
          FillHisto(tType, jetSuffix, massSuffix, "JetPt", SetPtOverflow(fJet.at(i).fVec.Pt()), fWeight);
          FillHisto(tType, jetSuffix, massSuffix, "JetEta", fJet.at(i).fVec.Eta(), fWeight);
          FillHisto(tType, jetSuffix, massSuffix, "JetPhi", fJet.at(i).fVec.Phi(), fWeight);
        }

        FillHisto(tType, jetSuffix, massSuffix, "nBJet", fBJet.size(), fWeight);
        for (int i = 0; i < fBJet.size(); i++) {
          FillHisto(tType, jetSuffix, massSuffix, "BJetPt", SetPtOverflow(fBJet.at(i).fVec.Pt()), fWeight);
          FillHisto(tType, jetSuffix, massSuffix, "BJetEta", fBJet.at(i).fVec.Eta(), fWeight);
          FillHisto(tType, jetSuffix, massSuffix, "BJetPhi", fBJet.at(i).fVec.Phi(), fWeight);
        }
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

  std::string tGenJetSuffix = "_" + GetJetBin(tNGenJets);

  auto tGenDiMuon = tDressedLeptons.at(0) + tDressedLeptons.at(1);
  auto tGenMass = SetMassOverflow(tGenDiMuon.M());

  auto tRecoDiMuon = tMuon_OS.at(0).fVec + tMuon_OS.at(1).fVec;
  auto tRecoMass = SetMassOverflow(tRecoDiMuon.M());

  std::vector<std::string> fAddonGenJet = {""};
  fAddonGenJet.emplace_back(tGenJetSuffix);

  for (auto suffix : fAddonGenJet) {

    FillHisto(HistoSetBase::HistGroup::GenInfo, "dimuonMass_WithReco" + suffix, tGenMass, tTotalWeight);
    
    if (tGenMass > 200) {
      FillHisto(HistoSetBase::HistGroup::GenInfo, "nJet_WithReco" + suffix, tNGenJets, tTotalWeight);

      FillHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonPt_WithReco" + suffix, tDressedLeptons.at(0).Pt(), tTotalWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonEta_WithReco" + suffix, tDressedLeptons.at(0).Eta(), tTotalWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonPhi_WithReco" + suffix, tDressedLeptons.at(0).Phi(), tTotalWeight);

      FillHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonPt_WithReco" + suffix, tDressedLeptons.at(1).Pt(), tTotalWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonEta_WithReco" + suffix, tDressedLeptons.at(1).Eta(), tTotalWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonPhi_WithReco" + suffix, tDressedLeptons.at(1).Phi(), tTotalWeight);

      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonPt_WithReco" + suffix, tDressedLeptons.at(0).Pt(), tTotalWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonEta_WithReco" + suffix, tDressedLeptons.at(0).Eta(), tTotalWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonPhi_WithReco" + suffix, tDressedLeptons.at(0).Phi(), tTotalWeight);
  
      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonPt_WithReco" + suffix, tDressedLeptons.at(1).Pt(), tTotalWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonEta_WithReco" + suffix, tDressedLeptons.at(1).Eta(), tTotalWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonPhi_WithReco" + suffix, tDressedLeptons.at(1).Phi(), tTotalWeight);

      FillHisto(HistoSetBase::HistGroup::GenInfo, "dimuonPt_WithReco" + suffix, tGenDiMuon.Pt(), tTotalWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "dimuonRap_WithReco" + suffix, tGenDiMuon.Rapidity(), tTotalWeight);
    }
  }
}

void HistoSetMUMU::FillGenInfoDressedLevel(
  const std::vector<TLorentzVector>& tDressedLeptons,
  const int& tNGenJets,
  const double& fMCWeight
) {
  
  std::string tGenJetSuffix = "_" + GetJetBin(tNGenJets);

  auto tGenDiMuon = tDressedLeptons.at(0) + tDressedLeptons.at(1);
  auto tGenMass = SetMassOverflow(tGenDiMuon.M());

  std::vector<std::string> fAddonGenJet = {""};
  fAddonGenJet.emplace_back(tGenJetSuffix);

  for (auto suffix : fAddonGenJet) {

    FillHisto(HistoSetBase::HistGroup::GenInfo, "dimuonMass" + suffix, tGenMass, fMCWeight);
    
    if (tGenMass > 200) {
      FillHisto(HistoSetBase::HistGroup::GenInfo, "nJet" + suffix, tNGenJets, fMCWeight);

      FillHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonPt" + suffix, tDressedLeptons.at(0).Pt(), fMCWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonEta" + suffix, tDressedLeptons.at(0).Eta(), fMCWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "LeadingMuonPhi" + suffix, tDressedLeptons.at(0).Phi(), fMCWeight);

      FillHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonPt" + suffix, tDressedLeptons.at(1).Pt(), fMCWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonEta" + suffix, tDressedLeptons.at(1).Eta(), fMCWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "SubleadingMuonPhi" + suffix, tDressedLeptons.at(1).Phi(), fMCWeight);

      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonPt" + suffix, tDressedLeptons.at(0).Pt(), fMCWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonEta" + suffix, tDressedLeptons.at(0).Eta(), fMCWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonPhi" + suffix, tDressedLeptons.at(0).Phi(), fMCWeight);
  
      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonPt" + suffix, tDressedLeptons.at(1).Pt(), fMCWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonEta" + suffix, tDressedLeptons.at(1).Eta(), fMCWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "MuonPhi" + suffix, tDressedLeptons.at(1).Phi(), fMCWeight);

      FillHisto(HistoSetBase::HistGroup::GenInfo, "dimuonPt" + suffix, tGenDiMuon.Pt(), fMCWeight);
      FillHisto(HistoSetBase::HistGroup::GenInfo, "dimuonRap" + suffix, tGenDiMuon.Rapidity(), fMCWeight);
    }
  }
}

void HistoSetMUMU::FillResponseMatrix(
  const double& tGenMass,
  const int& tNGenJets,
  const double& tRecoMass,
  const int& nJets,
  const double& fMCWeight,
  const double& fRecoWeight,
  const bool& tPassingReco
) {

  double tGenJetIndex = GetNJetBinIndex(tNGenJets);
  double tRecoJetIndex = GetNJetBinIndex(nJets);

  double tXbin = fGlobalBinningReco.GetGlobalBinNumber(SetMassOverflow(tRecoMass), tRecoJetIndex);
  double tYbin = fGlobalBinningGen.GetGlobalBinNumber(SetMassOverflow(tGenMass), tGenJetIndex);

  if (tPassingReco) {

    FillHisto(HistoSetBase::HistGroup::GenInfo, "ResponseMatrix", tXbin, tYbin, fMCWeight * fRecoWeight);
  }

  if (!tPassingReco) {

    FillHisto(HistoSetBase::HistGroup::GenInfo, "ResponseMatrix", -1, tYbin, fMCWeight);
  }

  if (tPassingReco && fRecoWeight != 1) {

    FillHisto(HistoSetBase::HistGroup::GenInfo, "ResponseMatrix", -1, tYbin, fMCWeight * (1. - fRecoWeight));
  }
}


std::string HistoSetMUMU::GetMassBin(double fDimuonMass) {
  for (int i = 0; i < fMassBins.size() - 1; i++) {
    if (fDimuonMass >= fMassBins[i] && fDimuonMass < fMassBins[i+1]) {
      return "m" + std::to_string((int)fMassBins[i]) + "_" + std::to_string((int)fMassBins[i+1]);
    }
  }
  return "";
}

std::string HistoSetMUMU::GetJetBin(double fNJet) {

  if (fNJet == 0) return "0J";
  else if (fNJet == 1) return "1J";
  else if (fNJet >= 2) return "mt1J";
  else return "";
}

std::string HistoSetMUMU::GetBJetBin(double fNBJet) {

  if (fNBJet == 0) return "0BJ";
  else if (fNBJet == 1) return "1BJ";
  else if (fNBJet >= 2) return "mt1BJ";
  else return "";
}

std::string HistoSetMUMU::GetbVetoJetBin(double fNJet) {
  
  if (fNJet == 0) return "bVeto_0J";
  else if (fNJet == 1) return "bVeto_1J";
  else if (fNJet >= 2) return "bVeto_mt1J";
  else return "";
}

double HistoSetMUMU::SetPtOverflow(double fPt) {
  if (fPt > 1500) return 1510;
  else return fPt;
}
double HistoSetMUMU::SetMassOverflow(double fMass) {
  if (fMass < 0) return -0.5;
  if (fMass < 200) return 199.5;
  if (fMass >= 4000) return 4000.5;
  else return fMass;
}
