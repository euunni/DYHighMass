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

    // if (tMaxLoop == 100) break;
    

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

    double tEventGenWeight = 1.;

    if (fIsMC) {
      tEventGenWeight = **(fNtuples->genWeight);

      if (fSampleName.Contains("NNLO")) {
        if (tEventGenWeight > 0) tEventGenWeight = 1;
        else                     tEventGenWeight = -1;
      }

      fHistoSet->FillHisto((std::string)"h_GenWeight", tEventGenWeight, 1.);
    }

    if (fIsMC && fSampleName.Contains("TTTo2L2Nu") && fDoTopPtReweighing)
      tEventGenWeight *= fNtuples->GetGenTopPtReweightFactor();

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
    
    fHistoSet->FillHisto((std::string)"h_EventInfo", 1, 1);
    fHistoSet->FillHisto((std::string)"h_EventInfo", 4, tEventGenWeight);

    float tPUReweightingFactor = 1;
    if (fIsMC && fDoPU) {

      tPUReweightingFactor = fPuReweighting->weight(**(fNtuples->Pileup_nTrueInt));
      tEventGenWeight *= tPUReweightingFactor;
    }

    if (fIsMC && fDoL1Pre) {
      tEventGenWeight *= **(fNtuples->L1PreFiringWeight_Nom);
    }

    if ( !(fNtuples->PassinNoiseFilter()) )
      continue;

    if ( !(fNtuples->PassingTrigger()) )
      continue;

    if ( !(fMuons->PrepareMuon()) )
      continue;

    if ( !(fJets->PrepareJet()) )
      continue;

    auto vJets = fJets->GetJets();
    auto vBJets = fJets->GetBJets();

    int nJets = vJets.size();
    int nBJets = vBJets.size();

    auto tLeadingMuon = fMuons->GetLeadingMuon();
    auto tFVecLeadingMuon = tLeadingMuon.fVec;
    auto tFVecRawLeadingMuon = tLeadingMuon.fVecRaw;
    float tChargeLeadingMuon = tLeadingMuon.fCharge;

    auto tSubLeadingMuon = fMuons->GetSubLeadingMuon();
    auto tFVecSubLeadingMuon = tSubLeadingMuon.fVec;
    auto tFVecRawSubLeadingMuon = tSubLeadingMuon.fVecRaw;
    float tChargeSubLeadingMuon = tSubLeadingMuon.fCharge;

    // std::cout << "######################################################################" << std::endl;
    // std::cout << "                      dimuon selection debugging                      " << std::endl;
    // std::cout << "----------------------------------------------------------------------" << std::endl;
    // std::cout << " LEADING: " << std::endl;
    // std::cout << "   pT: " << tFVecLeadingMuon.Pt() << std::endl;
    // std::cout << "   eta: " << tFVecLeadingMuon.Eta() << std::endl;
    // std::cout << "   charge: " << tChargeLeadingMuon << std::endl;
    // std::cout << "   ISO: " << tLeadingMuon.fISO << std::endl;
    // std::cout << " SUB-LLEADING: " << std::endl;
    // std::cout << "   pT: " << tFVecRawSubLeadingMuon.P() << std::endl;
    // std::cout << "   eta: " << tFVecRawSubLeadingMuon.Eta() << std::endl;
    // std::cout << "   charge: " << tChargeSubLeadingMuon << std::endl;
    // std::cout << "   ISO: " << tSubLeadingMuon.fISO << std::endl;
    // std::cout << "######################################################################" << std::endl;
    // std::cout << " " << std::endl;

    float tCharge = tChargeLeadingMuon * tChargeSubLeadingMuon;
    if (tCharge < 0) tCharge = -0.5;
    else             tCharge = 0.5;
    
    auto tDiMuon = tFVecLeadingMuon + tFVecSubLeadingMuon;

    if (fIsMC && fDoReco) {

      double tRecoEffSFLeading = 0;

      if (tFVecRawLeadingMuon.P() < 15.) tRecoEffSFLeading = 0;
      else                               tRecoEffSFLeading = fReco_SF->evaluate({std::abs(tFVecRawLeadingMuon.Eta()), tFVecRawLeadingMuon.P(), "nominal"});

      tEventGenWeight *= tRecoEffSFLeading;


      double tRecoEffSFSubleading = 0;

      if (tFVecRawSubLeadingMuon.P() < 15.) tRecoEffSFSubleading = 0;
      else                                  tRecoEffSFSubleading = fReco_SF->evaluate({std::abs(tFVecRawSubLeadingMuon.Eta()), tFVecRawSubLeadingMuon.P(), "nominal"});

      tEventGenWeight *= tRecoEffSFSubleading;

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                       Reco efficiency debugging                      " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << " LEADING: " << tFVecRawLeadingMuon.P() << " " << tFVecRawLeadingMuon.Eta() << " " << tRecoEffSFLeading << std::endl;
      // std::cout << " SUB-LLEADING: " << tFVecRawSubLeadingMuon.P() << " " << tFVecRawSubLeadingMuon.Eta() << " " << tRecoEffSFSubleading << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;

    }

    if (fIsMC && fDoID) {

      double tIDEffSFLeading = 0;

      if (tFVecRawLeadingMuon.Pt() < 15.) tIDEffSFLeading = 0;
      else                                tIDEffSFLeading = fID_SF->evaluate({std::abs(tFVecRawLeadingMuon.Eta()), tFVecRawLeadingMuon.Pt(), "nominal"});

      tEventGenWeight *= tIDEffSFLeading;


      double tIDEffSFSubleading = 0;

      if (tFVecRawSubLeadingMuon.Pt() < 15.) tIDEffSFSubleading = 0;
      else                                   tIDEffSFSubleading = fID_SF->evaluate({std::abs(tFVecRawSubLeadingMuon.Eta()), tFVecRawSubLeadingMuon.Pt(), "nominal"});

      tEventGenWeight *= tIDEffSFSubleading;

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                        ID efficiency debugging                       " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << " LEADING: " << tFVecRawLeadingMuon.Pt() << " " << tFVecRawLeadingMuon.Eta() << " " << tIDEffSFLeading << std::endl;
      // std::cout << " SUB-LLEADING: " << tFVecRawSubLeadingMuon.Pt() << " " << tFVecRawSubLeadingMuon.Eta() << " " << tIDEffSFSubleading << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;

    }

    if (fIsMC && fDoISO) {

      double tISOEffSFLeading = 0;
      double tISOEffSFSubleading = 0;

      if (tFVecRawLeadingMuon.Pt() < 15.) tISOEffSFLeading = 0;
      else                                tISOEffSFLeading = fISO_SF->evaluate({std::abs(tFVecRawLeadingMuon.Eta()), tFVecRawLeadingMuon.Pt(), "nominal"});
      tEventGenWeight *= tISOEffSFLeading;

      if (!fIsInverted) {

        if (tFVecRawSubLeadingMuon.Pt() < 15.) tISOEffSFSubleading = 0;
        else                                   tISOEffSFSubleading = fISO_SF->evaluate({std::abs(tFVecRawSubLeadingMuon.Eta()), tFVecRawSubLeadingMuon.Pt(), "nominal"});
        tEventGenWeight *= tISOEffSFSubleading;
      }

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                        ISO efficiency debugging                      " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << " LEADING: " << tFVecRawLeadingMuon.Pt() << " " << tFVecRawLeadingMuon.Eta() << " " << tISOEffSFLeading << std::endl;
      // if (!fIsInverted) std::cout << " SUB-LEADING: " << tFVecRawSubLeadingMuon.Pt() << " " << tFVecRawSubLeadingMuon.Eta() << " " << tISOEffSFSubleading << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;
    }

    if (fIsMC && fDoTRIGG) {
      
      if (!fIsInverted) {

        double mu_1_data = 0;
        double mu_2_data = 0;

        double mu_1_mc = 0;
        double mu_2_mc = 0;

        if (tFVecRawLeadingMuon.Pt() < 52.) {
          mu_1_data = 0.;
          mu_1_mc = 0.;
        } else {
          mu_1_data = fTRIG_Eff_Data->evaluate({std::abs(tFVecRawLeadingMuon.Eta()), tFVecRawLeadingMuon.Pt(), "nominal"});
          mu_1_mc = fTRIG_Eff_MC->evaluate({std::abs(tFVecRawLeadingMuon.Eta()), tFVecRawLeadingMuon.Pt(), "nominal"});
        }

        if (tFVecRawSubLeadingMuon.Pt() < 52.) {
          mu_2_data = 0.;
          mu_2_mc = 0.;
        } else {
          mu_2_data = fTRIG_Eff_Data->evaluate({std::abs(tFVecRawSubLeadingMuon.Eta()), tFVecRawSubLeadingMuon.Pt(), "nominal"});
          mu_2_mc = fTRIG_Eff_MC->evaluate({std::abs(tFVecRawSubLeadingMuon.Eta()), tFVecRawSubLeadingMuon.Pt(), "nominal"});
        }

        double data_tot = 1. - (1. - mu_1_data) * (1. - mu_2_data);
        double mc_tot = 1. - (1. - mu_1_mc) * (1. - mu_2_mc);

        double eventTriggerEffSF = 0.;
        if ( mc_tot != 0 )
          eventTriggerEffSF = data_tot / mc_tot;

        tEventGenWeight *= eventTriggerEffSF;

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                       TRIGG efficiency debugging                     " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " LEADING: " << tFVecRawLeadingMuon.Pt() << " " << tFVecRawLeadingMuon.Eta() << " " << mu_1_data << " " << mu_1_mc << std::endl;
        // std::cout << " SUB-LEADING: " << tFVecRawSubLeadingMuon.Pt() << " " << tFVecRawSubLeadingMuon.Eta() << " " << mu_2_data << " " << mu_2_mc << std::endl;
        // std::cout << "(1 - (1 - " <<  mu_1_data << ") * (1 - " << mu_2_data << ")) / (1 - (1 - " << mu_1_mc << ") * (1 - " << mu_2_mc << ")) = " << eventTriggerEffSF << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;
      } else {

        double mu_1_data = 0;
        double mu_1_mc = 0;

        if (tFVecRawLeadingMuon.Pt() < 52.) {
          mu_1_data = 0.;
          mu_1_mc = 0.;
        } else {
          mu_1_data = fTRIG_Eff_Data->evaluate({std::abs(tFVecRawLeadingMuon.Eta()), tFVecRawLeadingMuon.Pt(), "nominal"});
          mu_1_mc = fTRIG_Eff_MC->evaluate({std::abs(tFVecRawLeadingMuon.Eta()), tFVecRawLeadingMuon.Pt(), "nominal"});
        }

        double eventTriggerEffSF = 0.;
        if ( mu_1_mc != 0 )
          eventTriggerEffSF = mu_1_data / mu_1_mc;

        tEventGenWeight *= eventTriggerEffSF;

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                       TRIGG efficiency debugging                     " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " LEADING: " << tFVecRawLeadingMuon.Pt() << " " << tFVecRawLeadingMuon.Eta() << " " << mu_1_data << " " << mu_1_mc << std::endl;
        // std::cout << mu_1_data << " / " << mu_1_mc << " = " << eventTriggerEffSF << std::endl;
        // std::cout << "######################################################################" << std::endl;
        // std::cout << " " << std::endl;
      }
    }

    if (fIsMC && fDoJetPUID) {
      tEventGenWeight *= fJets->GetPUIDSF();
    }

    if (fIsMC && fDoBTag) {
      double bTagWeight = fJets->GetBTagSF();
      tEventGenWeight *= bTagWeight;
    }

    tTotalGenWeight += tEventGenWeight;

    fHistoSet->FillHisto((std::string)"h_nPVGood_Count", **(fNtuples->PV_npvsGood), tEventGenWeight);
    if (nJets == 0) fHistoSet->FillMuon(tFVecLeadingMuon, tFVecSubLeadingMuon, tCharge, nJets, nBJets, tDiMuon, tEventGenWeight);
    else            fHistoSet->FillMuon(tFVecLeadingMuon, tFVecSubLeadingMuon, tCharge, nJets, nBJets, vJets.at(0).fVec, tEventGenWeight);
    fHistoSet->FillJet(&vJets, &vBJets, tDiMuon.M(), tEventGenWeight);

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

  fHistoSet->WriteHisto(fEra, fSampleName, "./ROOT/output_" + fEra + "_" +  fSampleName + "_" + std::to_string(fJobID) + ".root", !fIsMC);
}
