#include <iostream>
#include <vector>
#include <chrono>

#include "DYLoopMUMU.h"
#include "muon.h"
#include "jet.h"
#include "electron.h"
#include "muon.h"
#include "NT.h"

#include "TH1.h"

std::string DYLoopMUMU::getCurrentTimeString() {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
  
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
  ss << "." << std::setfill('0') << std::setw(3) << ms.count();
  return ss.str();
}

std::string DYLoopMUMU::formatDuration(const std::chrono::high_resolution_clock::duration& duration) {
  auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  auto minutes = total_ms / 60000;
  auto seconds = (total_ms % 60000) / 1000;
  auto milliseconds = total_ms % 1000;
  
  std::stringstream ss;
  if (minutes > 0)
    ss << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
  else
    ss << "0:" << std::setfill('0') << std::setw(2) << seconds;

  return ss.str();
}

void DYLoopMUMU::SetNT() {

}

void DYLoopMUMU::Loop() {

  std::chrono::time_point tTimeBegin = std::chrono::system_clock::now();

  double tMaxLoop = 0;
  double tTotalGenWeight = 0;
  while(fNtuples->GetNext()) { // Event loop starts here
    tMaxLoop++;
    
    fWeightEnvelope.Clear();

    // print progress
    if (static_cast<int>(tMaxLoop) % 10000 == 0 ) {
      auto tCurrentTime = std::chrono::system_clock::now();
      auto tElapsed = tCurrentTime - tTimeBegin;
      double tProgressPercent = 100. * tMaxLoop / fMaxEntries;
      
      auto tEstimatedTotal = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(
          tElapsed * (100.0 / tProgressPercent)
      );
      
      std::string tElapsedStr = formatDuration(std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(tElapsed));
      std::string tEstimatedStr = formatDuration(tEstimatedTotal);
      
      std::string tProgress = Form("Loop: %.0f / %.0f | %.2f %% | %s / %s", 
                                   tMaxLoop, fMaxEntries, tProgressPercent,
                                   tElapsedStr.c_str(), tEstimatedStr.c_str());
      std::cout << tProgress << std::endl;
    }

    // for the PU reweighting
    if (fIsMC) {
      fHistoSet->FillHisto((std::string)"h_PileUp_Count_Interaction_before", **(fNtuples->Pileup_nTrueInt), 1.);
      fHistoSet->FillHisto((std::string)"h_PileUp_Count_Interaction_after", **(fNtuples->Pileup_nTrueInt), fPuReweighting->weight(**(fNtuples->Pileup_nTrueInt)));
    }

    // for the gen weight
    double tEventGenWeight = 1.;
    if (fIsMC) {
      tEventGenWeight = **(fNtuples->genWeight);

      if (fSampleName.Contains("NNLO")) {
        if (tEventGenWeight > 0) tEventGenWeight = 1;
        else                     tEventGenWeight = -1;
      }

      fHistoSet->FillHisto((std::string)"h_GenWeight", tEventGenWeight, 1.);
      fWeightEnvelope.AddWeightToAll("GenWeight", tEventGenWeight);
    }

    // for the LHE mass, in case of inclusive NNLO DY sample, we need to cut on the LHE mass at 100 GeV
    if (fIsMC && fSampleName.Contains("NNLO")) {
      auto tLHEMuons = fNtuples->GetLHE(13);

      fHistoSet->FillHisto((std::string)"h_LHEnMuon", static_cast<int>(tLHEMuons.size()));


      double tDiMuonMassLHE = 0;

      if (tLHEMuons.size() == 2) {
        auto tDiMuonLHE = tLHEMuons.at(0) + tLHEMuons.at(1);
        tDiMuonMassLHE = tDiMuonLHE.M();
      }

      if (fSampleName.Contains("NNLO") && fSampleName.Contains("inc") && tDiMuonMassLHE > 100 )
        continue;

      fHistoSet->FillHisto((std::string)"h_LHEDimuonMass", tDiMuonMassLHE, tEventGenWeight);
    }
    
    // for the event info
    fHistoSet->FillHisto((std::string)"h_EventInfo", 1, 1);
    fHistoSet->FillHisto((std::string)"h_EventInfo", 4, tEventGenWeight);


    std::vector<std::pair<int, TLorentzVector>> tDreessedOrigin = {};
    std::vector<TLorentzVector> tDressedLeptons = {};
    std::vector<TLorentzVector> tGenJets = {};
    int tGenMass = -9999;
    bool tHasGen = true;

    if (fSampleName.Contains("NNLO_MUMU")) {
      tHasGen = false;
    
      tDreessedOrigin = fNtuples->GetGenDressedLepton(13);
      tDressedLeptons = fMuons->GetFiducialMuons(tDreessedOrigin);

      if (tDressedLeptons.size() == 2) tHasGen = true;

      tGenJets = fNtuples->GetGenJet(30., tDressedLeptons);
    }

    if (tDressedLeptons.size() == 2 && fSampleName.Contains("NNLO_MUMU")) {
      tGenMass = (tDressedLeptons.at(0) + tDressedLeptons.at(1)).M();
      
      fHistoSet->FillGenInfoIndependently(
        tDressedLeptons,
        tGenJets.size(),
        fWeightEnvelope.GetMCWeight("OS")
      );
    }

    bool tPassingEventOfflineSelection = true; 
    if ( !(fNtuples->PassinNoiseFilter()) )
      tPassingEventOfflineSelection = false;
    
    if ( !(fNtuples->PassingTrigger()) )
      tPassingEventOfflineSelection = false;

    if (tPassingEventOfflineSelection) {
      if (fIsMC && fSampleName.Contains("TTTo2L2Nu") && fDoTopPtReweighing)
        fWeightEnvelope.AddWeightToAll("TopPtReweighing", fNtuples->GetGenTopPtReweightFactor());

      if (fIsMC && fDoPU)
        fWeightEnvelope.AddWeightToAll("PUReweighing", fPuReweighting->weight(**(fNtuples->Pileup_nTrueInt)));
      
      if (fIsMC && fDoL1Pre)
        fWeightEnvelope.AddWeightToAll("L1PreFiring", **(fNtuples->L1PreFiringWeight_Nom));
    }

    bool tPassingMuonOfflineSelection = false;
    if (fMuons->PrepareMuon())
      tPassingMuonOfflineSelection = true;

    bool tPassingJetSelection = fJets->PrepareJet();

    auto vJets = fJets->GetJets();
    auto vBJets = fJets->GetBJets();

    int nJets = static_cast<int>(vJets.size());
    int nBJets = static_cast<int>(vBJets.size());

    const auto& tMuon_OS = fMuons->GetMuons("OS");
    const auto& tMuon_SS = fMuons->GetMuons("SS");
    const auto& tMuon_OSinverted = fMuons->GetMuons("OS_inverted");
    const auto& tMuon_SSinverted = fMuons->GetMuons("SS_inverted");

    bool tPassingSignalRegionSeletion = false;

    if (tMuon_OS.size() == 2 && tPassingEventOfflineSelection) {
      tPassingSignalRegionSeletion = true;

      if (fIsMC && fDoReco) {
        fWeightEnvelope.AddWeight("OS", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tMuon_OS.at(0).fVecRaw));
        fWeightEnvelope.AddWeight("OS", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tMuon_OS.at(1).fVecRaw));
      }
      if (fIsMC && fDoID) {
        fWeightEnvelope.AddWeight("OS", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tMuon_OS.at(0).fVecRaw));
        fWeightEnvelope.AddWeight("OS", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tMuon_OS.at(1).fVecRaw));
      }
      if (fIsMC && fDoISO) {
        fWeightEnvelope.AddWeight("OS", "IsoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IsoEff"])(tMuon_OS.at(0).fVecRaw));
        fWeightEnvelope.AddWeight("OS", "IsoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IsoEff"])(tMuon_OS.at(1).fVecRaw));
      }
      if (fIsMC && fDoTRIGG) {
        fWeightEnvelope.AddWeight("OS", "EventTriggerEff", std::get<FuncDiMuonCorrection>(fCorrectionFuncs["EventTriggerEff"])(tMuon_OS.at(0).fVecRaw, tMuon_OS.at(1).fVecRaw));
      }
      if (fIsMC && fDoJetPUID) {
        fWeightEnvelope.AddWeight("OS", "PUJetIDEff", std::get<FuncEmptyInput>(fCorrectionFuncs["PUJetIDEff"])());
      }
      if (fIsMC && fDoBTag) {
        fWeightEnvelope.AddWeight("OS", "bTaggingEff", std::get<FuncEmptyInput>(fCorrectionFuncs["bTaggingEff"])());
      }

      fHistoSet->FillMuon(tMuon_OS.at(0).fVec, tMuon_OS.at(1).fVec, tMuon_OS.at(0).fCharge * tMuon_OS.at(1).fCharge, nJets, nBJets, fWeightEnvelope.GetTotalWeight("OS"), "OS", tHasGen);
      fHistoSet->FillJet(vJets, vBJets, (tMuon_OS.at(0).fVec + tMuon_OS.at(1).fVec).M(), fWeightEnvelope.GetTotalWeight("OS"), "OS");

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                             OS debugging                             " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << "tMaxLoop: " << tMaxLoop << std::endl;
      // std::cout << "tMuon_OS.size(): " << tMuon_OS.size() << std::endl;
      // std::cout << "tMuon_OS.at(0).fVec: " << tMuon_OS.at(0).fVec.Pt() << " " << tMuon_OS.at(0).fVec.Eta() << " " << tMuon_OS.at(0).fVec.Phi() << std::endl;
      // std::cout << "tMuon_OS.at(1).fVec: " << tMuon_OS.at(1).fVec.Pt() << " " << tMuon_OS.at(1).fVec.Eta() << " " << tMuon_OS.at(1).fVec.Phi() << std::endl;
      // std::cout << "nJets: " << nJets << std::endl;
      // std::cout << "nBJets: " << nBJets << std::endl;
      // std::cout << "fWeightEnvelope.GetTotalWeight(\"OS\"): " << fWeightEnvelope.GetTotalWeight("OS") << std::endl;
      // std::cout << "fWeightEnvelope.GetMCWeight(\"OS\"): " << fWeightEnvelope.GetMCWeight("OS") << std::endl;
      // std::cout << "fWeightEnvelope.GetRecoWeight(\"OS\"): " << fWeightEnvelope.GetRecoWeight("OS") << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;

      if (fSampleName.Contains("NNLO_MUMU") && nBJets == 0 && tDressedLeptons.size() == 2) {
        fHistoSet->FillResponseMatrix_v2(tGenMass, tGenJets.size(), (tMuon_OS.at(0).fVec + tMuon_OS.at(1).fVec).M(), nJets, fWeightEnvelope.GetMCWeight("OS"), fWeightEnvelope.GetRecoWeight("OS"), true);
        fHistoSet->FillGenInfo(
          tDressedLeptons,
          tGenJets.size(),
          tMuon_OS,
          nJets,
          nBJets,
          fWeightEnvelope.GetMCWeight("OS"),
          fWeightEnvelope.GetRecoWeight("OS")
        );
      }
    }
    
    if (fSampleName.Contains("NNLO_MUMU") && tDressedLeptons.size() == 2 && (tMuon_OS.size() != 2 || nBJets != 0 || !tPassingEventOfflineSelection)) {
      if (tGenMass < 0) {
        std::cout << "tGenMass: " << tGenMass << std::endl;
        std::cout << "tGenJets.size(): " << tGenJets.size() << std::endl;
        std::cout << "(tDressedLeptons.at(0) + tDressedLeptons.at(1)).M(): " << (tDressedLeptons.at(0) + tDressedLeptons.at(1)).M() << std::endl;
      }
      
      fHistoSet->FillResponseMatrix_v2(tGenMass, tGenJets.size(), -9999, 0, fWeightEnvelope.GetMCWeight("OS"), fWeightEnvelope.GetRecoWeight("OS"), false);
    }
    

    if (tMuon_SS.size() == 2 && tPassingEventOfflineSelection) {
      if (fIsMC && fDoReco) {
        fWeightEnvelope.AddWeight("SS", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tMuon_SS.at(0).fVecRaw));
        fWeightEnvelope.AddWeight("SS", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tMuon_SS.at(1).fVecRaw));
      }
      if (fIsMC && fDoID) {
        fWeightEnvelope.AddWeight("SS", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tMuon_SS.at(0).fVecRaw));
        fWeightEnvelope.AddWeight("SS", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tMuon_SS.at(1).fVecRaw));
      }
      if (fIsMC && fDoISO) {
        fWeightEnvelope.AddWeight("SS", "IsoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IsoEff"])(tMuon_SS.at(0).fVecRaw));
        fWeightEnvelope.AddWeight("SS", "IsoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IsoEff"])(tMuon_SS.at(1).fVecRaw));
      }
      if (fIsMC && fDoTRIGG) {
        fWeightEnvelope.AddWeight("SS", "EventTriggerEff", std::get<FuncDiMuonCorrection>(fCorrectionFuncs["EventTriggerEff"])(tMuon_SS.at(0).fVecRaw, tMuon_SS.at(1).fVecRaw));
      }
      if (fIsMC && fDoJetPUID) {
        fWeightEnvelope.AddWeight("SS", "PUJetIDEff", std::get<FuncEmptyInput>(fCorrectionFuncs["PUJetIDEff"])());
      }
      if (fIsMC && fDoBTag) {
        fWeightEnvelope.AddWeight("SS", "bTaggingEff", std::get<FuncEmptyInput>(fCorrectionFuncs["bTaggingEff"])());
      }

      fHistoSet->FillMuon(tMuon_SS.at(0).fVec, tMuon_SS.at(1).fVec, tMuon_SS.at(0).fCharge * tMuon_SS.at(1).fCharge, nJets, nBJets, fWeightEnvelope.GetTotalWeight("SS"), "SS", true);
      fHistoSet->FillJet(vJets, vBJets, (tMuon_SS.at(0).fVec + tMuon_SS.at(1).fVec).M(), fWeightEnvelope.GetTotalWeight("SS"), "SS");

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                             SS debugging                             " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << "tMaxLoop: " << tMaxLoop << std::endl;
      // std::cout << "tMuon_SS.size(): " << tMuon_SS.size() << std::endl;
      // std::cout << "tMuon_SS.at(0).fVec: " << tMuon_SS.at(0).fVec.Pt() << " " << tMuon_SS.at(0).fVec.Eta() << " " << tMuon_SS.at(0).fVec.Phi() << std::endl;
      // std::cout << "tMuon_SS.at(1).fVec: " << tMuon_SS.at(1).fVec.Pt() << " " << tMuon_SS.at(1).fVec.Eta() << " " << tMuon_SS.at(1).fVec.Phi() << std::endl;
      // std::cout << "nJets: " << nJets << std::endl;
      // std::cout << "nBJets: " << nBJets << std::endl;
      // std::cout << "fWeightEnvelope.GetTotalWeight(\"SS\"): " << fWeightEnvelope.GetTotalWeight("SS") << std::endl;
      // std::cout << "fWeightEnvelope.GetMCWeight(\"SS\"): " << fWeightEnvelope.GetMCWeight("SS") << std::endl;
      // std::cout << "fWeightEnvelope.GetRecoWeight(\"SS\"): " << fWeightEnvelope.GetRecoWeight("SS") << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;
    }

    if (tMuon_OSinverted.size() == 2 && tPassingEventOfflineSelection) {
      if (fIsMC && fDoReco) {
        fWeightEnvelope.AddWeight("OS_inverted", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tMuon_OSinverted.at(0).fVecRaw));
        fWeightEnvelope.AddWeight("OS_inverted", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tMuon_OSinverted.at(1).fVecRaw));
      }
      if (fIsMC && fDoID) {
        fWeightEnvelope.AddWeight("OS_inverted", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tMuon_OSinverted.at(0).fVecRaw));
        fWeightEnvelope.AddWeight("OS_inverted", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tMuon_OSinverted.at(1).fVecRaw));
      }
      if (fIsMC && fDoJetPUID) {
        fWeightEnvelope.AddWeight("OS_inverted", "PUJetIDEff", std::get<FuncEmptyInput>(fCorrectionFuncs["PUJetIDEff"])());
      }
      if (fIsMC && fDoBTag) {
        fWeightEnvelope.AddWeight("OS_inverted", "bTaggingEff", std::get<FuncEmptyInput>(fCorrectionFuncs["bTaggingEff"])());
      }

      fHistoSet->FillMuon(tMuon_OSinverted.at(0).fVec, tMuon_OSinverted.at(1).fVec, tMuon_OSinverted.at(0).fCharge * tMuon_OSinverted.at(1).fCharge, nJets, nBJets, fWeightEnvelope.GetTotalWeight("OS_inverted"), "OS_inverted", true);
      fHistoSet->FillJet(vJets, vBJets, (tMuon_OSinverted.at(0).fVec + tMuon_OSinverted.at(1).fVec).M(), fWeightEnvelope.GetTotalWeight("OS_inverted"), "OS_inverted");

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                             OS_inverted debugging                             " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << "tMaxLoop: " << tMaxLoop << std::endl;
      // std::cout << "tMuon_OSinverted.size(): " << tMuon_OSinverted.size() << std::endl;
      // std::cout << "tMuon_OSinverted.at(0).fVec: " << tMuon_OSinverted.at(0).fVec.Pt() << " " << tMuon_OSinverted.at(0).fVec.Eta() << " " << tMuon_OSinverted.at(0).fVec.Phi() << std::endl;
      // std::cout << "tMuon_OSinverted.at(1).fVec: " << tMuon_OSinverted.at(1).fVec.Pt() << " " << tMuon_OSinverted.at(1).fVec.Eta() << " " << tMuon_OSinverted.at(1).fVec.Phi() << std::endl;
      // std::cout << "nJets: " << nJets << std::endl;
      // std::cout << "nBJets: " << nBJets << std::endl;
      // std::cout << "fWeightEnvelope.GetTotalWeight(\"OS_inverted\"): " << fWeightEnvelope.GetTotalWeight("OS_inverted") << std::endl;
      // std::cout << "fWeightEnvelope.GetMCWeight(\"OS_inverted\"): " << fWeightEnvelope.GetMCWeight("OS_inverted") << std::endl;
      // std::cout << "fWeightEnvelope.GetRecoWeight(\"OS_inverted\"): " << fWeightEnvelope.GetRecoWeight("OS_inverted") << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;
    }

    if (tMuon_SSinverted.size() == 2 && tPassingEventOfflineSelection) {
      if (fIsMC && fDoReco) {
        fWeightEnvelope.AddWeight("SS_inverted", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tMuon_SSinverted.at(0).fVecRaw));
        fWeightEnvelope.AddWeight("SS_inverted", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tMuon_SSinverted.at(1).fVecRaw));
      }
      if (fIsMC && fDoID) {
        fWeightEnvelope.AddWeight("SS_inverted", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tMuon_SSinverted.at(0).fVecRaw));
        fWeightEnvelope.AddWeight("SS_inverted", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tMuon_SSinverted.at(1).fVecRaw));
      }
      if (fIsMC && fDoJetPUID) {
        fWeightEnvelope.AddWeight("SS_inverted", "PUJetIDEff", std::get<FuncEmptyInput>(fCorrectionFuncs["PUJetIDEff"])());
      }
      if (fIsMC && fDoBTag) {
        fWeightEnvelope.AddWeight("SS_inverted", "bTaggingEff", std::get<FuncEmptyInput>(fCorrectionFuncs["bTaggingEff"])());
      }

      fHistoSet->FillMuon(tMuon_SSinverted.at(0).fVec, tMuon_SSinverted.at(1).fVec, tMuon_SSinverted.at(0).fCharge * tMuon_SSinverted.at(1).fCharge, nJets, nBJets, fWeightEnvelope.GetTotalWeight("SS_inverted"), "SS_inverted", true);
      fHistoSet->FillJet(vJets, vBJets, (tMuon_SSinverted.at(0).fVec + tMuon_SSinverted.at(1).fVec).M(), fWeightEnvelope.GetTotalWeight("SS_inverted"), "SS_inverted");

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                             SS_inverted debugging                             " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << "tMaxLoop: " << tMaxLoop << std::endl;
      // std::cout << "tMuon_SSinverted.size(): " << tMuon_SSinverted.size() << std::endl;
      // std::cout << "tMuon_SSinverted.at(0).fVec: " << tMuon_SSinverted.at(0).fVec.Pt() << " " << tMuon_SSinverted.at(0).fVec.Eta() << " " << tMuon_SSinverted.at(0).fVec.Phi() << std::endl;
      // std::cout << "tMuon_SSinverted.at(1).fVec: " << tMuon_SSinverted.at(1).fVec.Pt() << " " << tMuon_SSinverted.at(1).fVec.Eta() << " " << tMuon_SSinverted.at(1).fVec.Phi() << std::endl;
      // std::cout << "nJets: " << nJets << std::endl;
      // std::cout << "nBJets: " << nBJets << std::endl;
      // std::cout << "fWeightEnvelope.GetTotalWeight(\"SS_inverted\"): " << fWeightEnvelope.GetTotalWeight("SS_inverted") << std::endl;
      // std::cout << "fWeightEnvelope.GetMCWeight(\"SS_inverted\"): " << fWeightEnvelope.GetMCWeight("SS_inverted") << std::endl;
      // std::cout << "fWeightEnvelope.GetRecoWeight(\"SS_inverted\"): " << fWeightEnvelope.GetRecoWeight("SS_inverted") << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;
    }

  } // End of event loop

  fHistoSet->FillHisto((std::string)"h_EventInfo", 5, tTotalGenWeight);
  fHistoSet->FillHisto((std::string)"h_EventInfo", 2, tMaxLoop);
  fHistoSet->FillHisto((std::string)"h_EventInfo", 3, fMaxEntries);

  std::chrono::duration tTimeTaken = std::chrono::system_clock::now() - tTimeBegin;
  std::chrono::minutes tTimeMin = std::chrono::duration_cast<std::chrono::minutes>(tTimeTaken);
  std::chrono::seconds tTimeSec = std::chrono::duration_cast<std::chrono::seconds>(tTimeTaken - tTimeMin);

  std::cout << " " << std::endl;
  std::cout << "######################################################################" << std::endl;
  std::cout << "                             Loop summary                             " << std::endl;
  std::cout << "----------------------------------------------------------------------" << std::endl;
  std::cout << " Entries: " << fMaxEntries << std::endl;
  std::cout << " nLoop: " << tMaxLoop << std::endl;
  std::cout << " GenWeight: " << tTotalGenWeight << std::endl;
  std::cout << " Time taken: " << tTimeMin.count() << "min " << int(tTimeSec.count()) << "sec" << std::endl;
  std::cout << "######################################################################" << std::endl;
  std::cout << " " << std::endl;

  EndOfJob();
}

void DYLoopMUMU::EndOfJob() {

  TString fOutputFileName = fOutputDir + "/output_" + fEra + "_" +  fSampleName + "_" + std::to_string(fJobID) + ".root";

  TFile* fOutputFile = new TFile(fOutputFileName, "RECREATE");

  fHistoSet->WriteHisto(fEra, fSampleName, fOutputFile, !fIsMC);
  
  if (fSampleName.Contains("NNLO_MUMU"))
    fHistoSet->WriteGenHisto(fEra, fSampleName, fOutputFile);

  fOutputFile->Close();
}
