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

    if ( !(fEMU->PrepareEMUPair()) )
      continue;

    if ( !(fJets->PrepareJet()) )
      continue;

    auto vJets = fJets->GetJets();
    auto vBJets = fJets->GetBJets();

    int nJets = vJets.size();
    int nBJets = vBJets.size();

    auto tMuon = fEMU->GetMuon();
    auto tFVecMuon = tMuon.fVec;
    auto tFVecRawMuon = tMuon.fVecRaw;

    auto tElec = fEMU->GetElec();
    auto tFVecElec = tElec.fVec;
    auto tSCEtaElec = tElec.SCEta();

    auto tEMuPair = tFVecMuon + tFVecElec;

    if (fIsMC && fDoReco) {

      double tRecoEffSFMuon = 0;

      if (tFVecRawMuon.P() < 52.) tRecoEffSFMuon = 0;
      else                        tRecoEffSFMuon = fReco_SF->evaluate({std::abs(tFVecRawMuon.Eta()), tFVecRawMuon.P(), "nominal"});

      tEventGenWeight *= tRecoEffSFMuon;

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                       Reco efficiency debugging                      " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << " LEADING: " << tFVecRawLeadingMuon.P() << " " << tFVecRawLeadingMuon.Eta() << " " << tRecoEffSFLeading << std::endl;
      // std::cout << " SUB-LLEADING: " << tFVecRawSubLeadingMuon.P() << " " << tFVecRawSubLeadingMuon.Eta() << " " << tRecoEffSFSubleading << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;

    }

    if (fIsMC && fDoID) {

      double tIDEffSFMuon = 0;

      if (tFVecRawMuon.Pt() < 52.) tIDEffSFMuon = 0;
      else                         tIDEffSFMuon = fID_SF->evaluate({std::abs(tFVecRawMuon.Eta()), tFVecRawMuon.Pt(), "nominal"});

      tEventGenWeight *= tIDEffSFMuon;

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                        ID efficiency debugging                       " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << " LEADING: " << tFVecRawLeadingMuon.Pt() << " " << tFVecRawLeadingMuon.Eta() << " " << tIDEffSFLeading << std::endl;
      // std::cout << " SUB-LLEADING: " << tFVecRawSubLeadingMuon.Pt() << " " << tFVecRawSubLeadingMuon.Eta() << " " << tIDEffSFSubleading << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;

    }

    if (fIsMC && fDoISO && !fIsInverted) {

      double tISOEffSFMuon = 0;

      if (tFVecRawMuon.Pt() < 52.) tISOEffSFMuon = 0;
      else                         tISOEffSFMuon = fISO_SF->evaluate({std::abs(tFVecRawMuon.Eta()), tFVecRawMuon.Pt(), "nominal"});

      tEventGenWeight *= tISOEffSFMuon;


      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                        ISO efficiency debugging                      " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << " LEADING: " << tFVecRawLeadingMuon.Pt() << " " << tFVecRawLeadingMuon.Eta() << " " << tISOEffSFLeading << std::endl;
      // std::cout << " SUB-LEADING: " << tFVecRawSubLeadingMuon.Pt() << " " << tFVecRawSubLeadingMuon.Eta() << " " << tISOEffSFSubleading << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;


    }

    if (fIsMC && fDoTRIGG && !fIsInverted) {

      double tTRIGEffSFMuon = 0;

      if (tFVecRawMuon.Pt() < 52.) tTRIGEffSFMuon = 0;
      else                         tTRIGEffSFMuon = fTRIG_SF->evaluate({std::abs(tFVecRawMuon.Eta()), tFVecRawMuon.Pt(), "nominal"});

      tEventGenWeight *= tTRIGEffSFMuon;

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                       TRIGG efficiency debugging                     " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << " LEADING: " << tFVecRawLeadingMuon.Pt() << " " << tFVecRawLeadingMuon.Eta() << " " << mu_1_data << " " << mu_1_mc << std::endl;
      // std::cout << " SUB-LEADING: " << tFVecRawSubLeadingMuon.Pt() << " " << tFVecRawSubLeadingMuon.Eta() << " " << mu_2_data << " " << mu_2_mc << std::endl;
      // std::cout << eventTriggerEffSF << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;
    }

    if (fIsMC && fDoElecReco) {
      double tElecRecoEffSFElec = 0;

      if (tFVecElec.Pt() < 20.) tElecRecoEffSFElec = 0;
      else                      tElecRecoEffSFElec = fElecReco_SF->evaluate({(std::string)(fEra), "sf", "RecoAbove20", tSCEtaElec, tFVecElec.Pt()});

      tEventGenWeight *= tElecRecoEffSFElec;
    }

    if (fIsMC && fDoElecID && !fIsInverted) {
      double tElecIDEffSFElec = 0;

      if (tFVecElec.Pt() < 20.) tElecIDEffSFElec = 0;
      else                      tElecIDEffSFElec = fElecReco_SF->evaluate({(std::string)(fEra), "sf", "Medium", tSCEtaElec, tFVecElec.Pt()});

      tEventGenWeight *= tElecIDEffSFElec;
    }

    // Get gen-lv electrons (status == 1, abs(pdgId) == 11)
    bool tRecoGenMatched = false;
    bool tMisIdAssigned = false;
    double tMatchedDeltaR = 9999;
    double tMatchedRelPt = 9999;

    std::vector<std::pair<int, TLorentzVector>> tGenElecs = {};
    if (fIsMC && fDoElecMisCharge) tGenElecs = fNtuples->GetGenPart(11, 1);
    if (fIsMC && fDoElecMisCharge && tGenElecs.size() > 0) {
      tRecoGenMatched = true;
      int tRecoCharge = tElec.fCharge;

      // Find the closest gen-lv electron
      int tMacthedIndex = -1;
      float tDeltaRmin = 9999;
      for (int i = 0; i < tGenElecs.size(); i++) {
        auto tGenElecVec = tGenElecs.at(i).second;
        float tDeltaR = tFVecElec.DeltaR(tGenElecVec);

        if (tDeltaR < tDeltaRmin) {
          tDeltaRmin = tDeltaR;
          tMacthedIndex = i;
        }
      }

      tMatchedDeltaR = tDeltaRmin;
      tMatchedRelPt = tGenElecs.at(tMacthedIndex).second.Pt() / tFVecElec.Pt();

      // If the reco and gen charges are opposite, apply mischarge correction
      if (tRecoCharge * tGenElecs.at(tMacthedIndex).first < 0) {
        tMisIdAssigned = true;
        int tBinIndexX = fElecMisCharge_SF->GetXaxis()->FindBin(std::abs(tFVecElec.Eta()));
        if (tBinIndexX == 0) tBinIndexX = 1;
        else if (tBinIndexX > fElecMisCharge_SF->GetNbinsX()) tBinIndexX = fElecMisCharge_SF->GetNbinsX();
    
        int tBinIndexY = fElecMisCharge_SF->GetYaxis()->FindBin(tFVecElec.Pt());
        if (tBinIndexY == 0) tBinIndexY = 1;
        else if (tBinIndexY > fElecMisCharge_SF->GetNbinsY()) tBinIndexY = fElecMisCharge_SF->GetNbinsY();
    
        double tElecMisChargeSFWeight = fElecMisCharge_SF->GetBinContent(tBinIndexX, tBinIndexY);
        tEventGenWeight *= tElecMisChargeSFWeight;

        // std::cout << "######################################################################" << std::endl;
        // std::cout << "                      Elec Charge Mis-Id debugging                    " << std::endl;
        // std::cout << "----------------------------------------------------------------------" << std::endl;
        // std::cout << " Reco elec: " << tFVecElec.Pt() << " " << tFVecElec.Eta() << " " << tRecoCharge << std::endl;
        // std::cout << " Selected gen elec: " << tGenElecs.at(tMacthedIndex).second.Pt() << " " << tGenElecs.at(tMacthedIndex).second.Eta() << " " << tGenElecs.at(tMacthedIndex).first << std::endl;

        // std::cout << "tEventGenWeight before: " << tEventGenWeight / tElecMisChargeSFWeight << std::endl;
        // std::cout << "tElecMisChargeSFWeight: " << tElecMisChargeSFWeight << std::endl;
        // std::cout << "tEventGenWeight after: " << tEventGenWeight << std::endl;

        // std::cout << " Gen elec: " << std::endl;
        // for (int i = 0; i < tGenElecs.size(); i++) {
        //   auto tGenElecVec = tGenElecs.at(i).second;
        //   std::cout << tGenElecVec.Pt() << " " << tGenElecVec.Eta() << " " << tFVecElec.DeltaR(tGenElecVec) << " " << tGenElecs.at(i).first << std::endl;
        // }
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

    if (tRecoGenMatched) {
      if (!tMisIdAssigned) {
        fHistoSet->FillHistoSet("h_ElecRecoGen_SameSign_DeltaR", tEMuPair.M(), nJets, nBJets, tMatchedDeltaR, tEventGenWeight);
        fHistoSet->FillHistoSet("h_ElecRecoGen_SameSign_RelPt", tEMuPair.M(), nJets, nBJets, tMatchedRelPt, tEventGenWeight);
      } else {
        fHistoSet->FillHistoSet("h_ElecRecoGen_OppositeSign_DeltaR", tEMuPair.M(), nJets, nBJets, tMatchedDeltaR, tEventGenWeight);
        fHistoSet->FillHistoSet("h_ElecRecoGen_OppositeSign_RelPt", tEMuPair.M(), nJets, nBJets, tMatchedRelPt, tEventGenWeight);
      }
    }

    fHistoSet->FillHisto((std::string)"h_nPVGood_Count", **(fNtuples->PV_npvsGood), tEventGenWeight);
    fHistoSet->FillEMUPair(tFVecMuon, tFVecElec, nJets, nBJets, tEventGenWeight);
    fHistoSet->FillJet(&vJets, &vBJets, tEMuPair.M(), tEventGenWeight);

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

  fHistoSet->WriteHisto(fEra, fSampleName, "./ROOT/output_" + fEra + "_" +  fSampleName + "_" + std::to_string(fJobID) + ".root", !fIsMC);
}
