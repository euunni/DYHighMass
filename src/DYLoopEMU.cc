#include <iostream>
#include <vector>
#include <chrono>

#include "DYLoopEMU.h"
#include "emu.h"
#include "jet.h"
#include "NT.h"

#include "TH1.h"

std::string DYLoopEMU::getCurrentTimeString() {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
  
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
  ss << "." << std::setfill('0') << std::setw(3) << ms.count();
  return ss.str();
}

std::string DYLoopEMU::formatDuration(const std::chrono::high_resolution_clock::duration& duration) {
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

void DYLoopEMU::SetNT() {

}

void DYLoopEMU::Loop() {

  std::chrono::time_point tTimeBegin = std::chrono::system_clock::now();

  double tMaxLoop = 0;
  double tTotalGenWeight = 0;
  while(fNtuples->GetNext()) { // Event loop starts here
    tMaxLoop++;

    if (tMaxLoop == 10000) break;

    fWeightEnvelope.Clear();
    
    if (static_cast<int>(tMaxLoop) % 10000 == 0 ) {
      auto tCurrentTime = std::chrono::system_clock::now();
      auto tElapsed = tCurrentTime - tTimeBegin;
      double tProgressPercent = 100. * tMaxLoop / fMaxEntries;
      
      // 총 예상 시간 계산: 현재 걸린 시간 * 100 / 진행률
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

    fHistoSet->FillHisto((std::string)"h_EventInfo", 1, 1);
    fHistoSet->FillHisto((std::string)"h_EventInfo", 4, tEventGenWeight);

    if ( !(fNtuples->PassinNoiseFilter()) )
      continue;

    if ( !(fNtuples->PassingTrigger()) )
      continue;

    if (fIsMC && fSampleName.Contains("TTTo2L2Nu") && fDoTopPtReweighing)
      fWeightEnvelope.AddWeightToAll("TopPtReweighing", fNtuples->GetGenTopPtReweightFactor());

    if (fIsMC && fDoPU)
      fWeightEnvelope.AddWeightToAll("PUReweighing", fPuReweighting->weight(**(fNtuples->Pileup_nTrueInt)));
    
    if (fIsMC && fDoL1Pre)
      fWeightEnvelope.AddWeightToAll("L1PreFiring", **(fNtuples->L1PreFiringWeight_Nom));

    if ( !(fEMU->PrepareEMUPair()) )
      continue;

    if ( !(fJets->PrepareJet()) )
      continue;

    auto vJets = fJets->GetJets();
    auto vBJets = fJets->GetBJets();

    int nJets = vJets.size();
    int nBJets = vBJets.size();


    if (fIsMC && fDoJetPUID)
      fWeightEnvelope.AddWeightToAll("PUJetIDEff", fJets->GetPUIDSF());

    if (fIsMC && fDoBTag)
      fWeightEnvelope.AddWeightToAll("bTaggingEff", fJets->GetBTagSF());

    if (fEMU->HasPair_OS()) {
      auto tPair = fEMU->GetPair_OS();
      auto tEMuPair = tPair.first.fVec + tPair.second.fVec;

      if (fIsMC && fDoReco)            fWeightEnvelope.AddWeight("OS", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoID)              fWeightEnvelope.AddWeight("OS", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoISO)             fWeightEnvelope.AddWeight("OS", "IsoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IsoEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoTRIGG)           fWeightEnvelope.AddWeight("OS", "SingleTriggerEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["SingleTriggerEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoElecReco)        fWeightEnvelope.AddWeight("OS", "ElecRecoEff", std::get<FuncSingleElecCorrection>(fCorrectionFuncs["ElecRecoEff"])(tPair.second.fVec, tPair.second.fSCEta)); 
      if (fIsMC && fDoElecID)          fWeightEnvelope.AddWeight("OS", "ElecIDEff", std::get<FuncSingleElecCorrection>(fCorrectionFuncs["ElecIDEff"])(tPair.second.fVec, tPair.second.fSCEta)); 
    
      fHistoSet->FillEMUPair(tPair.first.fVec, tPair.second.fVec, nJets, nBJets, fWeightEnvelope.GetTotalWeight("OS"), "OS");
      if (tEMuPair.M() > 40) fHistoSet->FillJet(vJets, vBJets, tEMuPair.M(), fWeightEnvelope.GetTotalWeight("OS"), "OS");  
    } 

    if (fEMU->HasPair_SS()) {
      auto tPair = fEMU->GetPair_SS();
      auto tEMuPair = tPair.first.fVec + tPair.second.fVec;

      if (fIsMC && fDoReco)            fWeightEnvelope.AddWeight("SS", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoID)              fWeightEnvelope.AddWeight("SS", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoISO)             fWeightEnvelope.AddWeight("SS", "IsoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IsoEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoTRIGG)           fWeightEnvelope.AddWeight("SS", "SingleTriggerEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["SingleTriggerEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoElecReco)        fWeightEnvelope.AddWeight("SS", "ElecRecoEff", std::get<FuncSingleElecCorrection>(fCorrectionFuncs["ElecRecoEff"])(tPair.second.fVec, tPair.second.fSCEta)); 
      if (fIsMC && fDoElecID)          fWeightEnvelope.AddWeight("SS", "ElecIDEff", std::get<FuncSingleElecCorrection>(fCorrectionFuncs["ElecIDEff"])(tPair.second.fVec, tPair.second.fSCEta)); 
    
      fHistoSet->FillEMUPair(tPair.first.fVec, tPair.second.fVec, nJets, nBJets, fWeightEnvelope.GetTotalWeight("SS"), "SS");
      if (tEMuPair.M() > 40) fHistoSet->FillJet(vJets, vBJets, tEMuPair.M(), fWeightEnvelope.GetTotalWeight("SS"), "SS");  
    } 

    if (fEMU->HasPair_OS_inverted()) {
      auto tPair = fEMU->GetPair_OS_inverted();
      auto tEMuPair = tPair.first.fVec + tPair.second.fVec;

      if (fIsMC && fDoReco)            fWeightEnvelope.AddWeight("OS_inverted", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoID)              fWeightEnvelope.AddWeight("OS_inverted", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoElecReco)        fWeightEnvelope.AddWeight("OS_inverted", "ElecRecoEff", std::get<FuncSingleElecCorrection>(fCorrectionFuncs["ElecRecoEff"])(tPair.second.fVec, tPair.second.fSCEta)); 

      fHistoSet->FillEMUPair(tPair.first.fVec, tPair.second.fVec, nJets, nBJets, fWeightEnvelope.GetTotalWeight("OS_inverted"), "OS_inverted");
      if (tEMuPair.M() > 40) fHistoSet->FillJet(vJets, vBJets, tEMuPair.M(), fWeightEnvelope.GetTotalWeight("OS_inverted"), "OS_inverted");  
    } 

    if (fEMU->HasPair_SS_inverted()) {
      auto tPair = fEMU->GetPair_SS_inverted();
      auto tEMuPair = tPair.first.fVec + tPair.second.fVec;

      if (fIsMC && fDoReco)            fWeightEnvelope.AddWeight("SS_inverted", "RecoEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["RecoEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoID)              fWeightEnvelope.AddWeight("SS_inverted", "IDEff", std::get<FuncSingleMuonCorrection>(fCorrectionFuncs["IDEff"])(tPair.first.fVecRaw));
      if (fIsMC && fDoElecReco)        fWeightEnvelope.AddWeight("SS_inverted", "ElecRecoEff", std::get<FuncSingleElecCorrection>(fCorrectionFuncs["ElecRecoEff"])(tPair.second.fVec, tPair.second.fSCEta));     

      fHistoSet->FillEMUPair(tPair.first.fVec, tPair.second.fVec, nJets, nBJets, fWeightEnvelope.GetTotalWeight("SS_inverted"), "SS_inverted");
      if (tEMuPair.M() > 40) fHistoSet->FillJet(vJets, vBJets, tEMuPair.M(), fWeightEnvelope.GetTotalWeight("SS_inverted"), "SS_inverted");  
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

void DYLoopEMU::EndOfJob() {

  fHistoSet->WriteHisto(fEra, fSampleName, fOutputDir + "/output_" + fEra + "_" +  fSampleName + "_" + std::to_string(fJobID) + ".root", !fIsMC);
}
