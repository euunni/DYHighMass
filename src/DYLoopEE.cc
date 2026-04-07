#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>

#include "DYLoopEE.h"
#include "muon.h"
#include "jet.h"
#include "electron.h"
#include "muon.h"
#include "NT.h"

#include "TH1.h"

std::string DYLoopEE::getCurrentTimeString() {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
  
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
  ss << "." << std::setfill('0') << std::setw(3) << ms.count();
  return ss.str();
}

std::string DYLoopEE::formatDuration(const std::chrono::high_resolution_clock::duration& duration) {
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

void DYLoopEE::SetNT() {

}

void DYLoopEE::Loop() {

  std::chrono::time_point tTimeBegin = std::chrono::system_clock::now();

  double tMaxLoop = 0;
  double tTotalGenWeight = 0;
  while(fNtuples->GetNext()) { // Event loop starts here
    tMaxLoop++;

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

    // Acceptance and efficiency
    if (fIsMC) {
      const double leadPtCut = fConfig["Electron"]["LeadingPt"].as<float>();
      const double subPtCut  = fConfig["Electron"]["SubLeadingPt"].as<float>();
      const double etaMax    = fConfig["Electron"]["Eta"].as<float>();
      const double genJetPtCut  = fConfig["Jet"]["Pt"].as<float>();
      const double genJetEtaCut = fConfig["Jet"]["Eta"].as<float>();

      // Get hard gen electrons
      auto tGenElecs = fNtuples->GetHardGenPart(11, 1); // (charge, p4, motherPdgId)
      fHistoSet->FillHisto((std::string)"h_nHardGenElec", static_cast<int>(tGenElecs.size()), tEventGenWeight); // for sanity check

      for (const auto& [charge, p4, motherPdgId] : tGenElecs)
        fHistoSet->FillHisto((std::string)"h_HardGenElecMotherPdgId", std::abs(motherPdgId), tEventGenWeight); // for sanity check

      // Acceptance denominator: Find OS gen pair
      int leadIdx = -1, subIdx = -1;
      if (tGenElecs.size() >= 2) {
        std::sort(tGenElecs.begin(), tGenElecs.end(),
                  [](const auto& a, const auto& b) { return std::get<1>(a).Pt() > std::get<1>(b).Pt(); });

        for (int i = 0; i < (int)tGenElecs.size(); ++i) {
          for (int j = i + 1; j < (int)tGenElecs.size(); ++j) {
            if (std::get<0>(tGenElecs[i]) * std::get<0>(tGenElecs[j]) > 0)
              continue;
            leadIdx = i;
            subIdx = j;
            break;
          }
          if (leadIdx != -1) break;
        }
      }

      if (leadIdx != -1) {
        const auto& leadVec = std::get<1>(tGenElecs[leadIdx]);
        const auto& subVec  = std::get<1>(tGenElecs[subIdx]);

        // Gen jet counting with lepton cleaning using the selected OS pair
        // Denom: no pT/eta cuts (captures jet acceptance effect)
        // Numer: config pT/eta cuts applied
        std::vector<TLorentzVector> tGenLepVec = {leadVec, subVec};
        auto tGenJetsDenom = fNtuples->GetGenJet(0., 9999., tGenLepVec);
        auto tGenJetsNumer = fNtuples->GetGenJet(genJetPtCut, genJetEtaCut, tGenLepVec);
        std::string tGenJetSuffixDenom = fHistoSet->GetJetBin(tGenJetsDenom.size());
        std::string tGenJetSuffixNumer = fHistoSet->GetJetBin(tGenJetsNumer.size());

        double tGenDielecMass = (leadVec + subVec).M();
        tGenDielecMass = fHistoSet->SetMassOverflow(tGenDielecMass);

        fHistoSet->FillHisto((std::string)"h_GenAcc_Denom", tGenDielecMass, tEventGenWeight);
        fHistoSet->FillHisto((std::string)"h_GenAcc_Denom" + tGenJetSuffixDenom, tGenDielecMass, tEventGenWeight);

        // Acceptance numerator
        auto passFid = [&](const TLorentzVector& v, double ptCut) -> bool {
          if (v.Pt() <= ptCut) return false;
          const double aeta = std::abs(v.Eta());
          if (aeta >= etaMax) return false;
          if (aeta > 1.4442 && aeta < 1.566) return false;
          return true;
        };

        if (passFid(leadVec, leadPtCut) && passFid(subVec, subPtCut)) {
          // Inclusive in jets: electrons passing fiducial, no jet condition
          fHistoSet->FillHisto((std::string)"h_GenAcc_Numer", tGenDielecMass, tEventGenWeight); // Inclusive in jets: electrons passing fiducial, no jet condition

          // Jet multiplicity: electrons and jets passing fiducial
          if (tGenJetSuffixNumer == tGenJetSuffixDenom)
            fHistoSet->FillHisto((std::string)"h_GenAcc_Numer" + tGenJetSuffixDenom, tGenDielecMass, tEventGenWeight);

          // Efficiency numerator: trigger + reco electron (no mass cut) selection + reco jet selection amomg those in the denominator.
          if (fNtuples->PassingTrigger()) {
            if (fElecs->PrepareElecNoMassCut()) {
              fJets->PrepareJet();
              int nRecoJets = fJets->GetJets().size();
              std::string tRecoJetSuffix = fHistoSet->GetJetBin(nRecoJets);

              // Inclusive: no reco jet multiplicity requirement
              fHistoSet->FillHisto((std::string)"h_GenEff_Numer", tGenDielecMass, tEventGenWeight);

              // Jet multiplicity: reco jets must match gen jets in the denominator
              if (tRecoJetSuffix == tGenJetSuffixNumer && tGenJetSuffixNumer == tGenJetSuffixDenom)
                fHistoSet->FillHisto((std::string)"h_GenEff_Numer" + tGenJetSuffixNumer, tGenDielecMass, tEventGenWeight);
            }
          }
        }
      }
    }

    if (fIsMC && fSampleName.Contains("NNLO")) {
      auto tLHEElecs = fNtuples->GetLHE(11);

      fHistoSet->FillHisto((std::string)"h_LHEnElec", static_cast<int>(tLHEElecs.size()));

      double tDiElecMassLHE = 0;

      if (tLHEElecs.size() == 2) {
        auto tDiElecLHE = tLHEElecs.at(0) + tLHEElecs.at(1);
        tDiElecMassLHE = tDiElecLHE.M();
      }

      if (fSampleName.Contains("NNLO") && fSampleName.Contains("inc") && tDiElecMassLHE > 100 )
        continue;

      fHistoSet->FillHisto((std::string)"h_LHEDielecMass", tDiElecMassLHE, tEventGenWeight);
    }

    if (fIsMC && fSampleName.Contains("TTTo2L2Nu") && fDoTopPtReweighing)
      tEventGenWeight *= fNtuples->GetGenTopPtReweightFactor();
    
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
    
    if ( !(fElecs->PrepareElec()) )
      continue;

    if ( !(fJets->PrepareJet()) )
      continue;

    auto vJets = fJets->GetJets();
    auto vBJets = fJets->GetBJets();

    int nJets = vJets.size();
    int nBJets = vBJets.size();

    auto tLeadingElec = fElecs->GetLeadingElec();
    auto tFVecLeadingElec = tLeadingElec.fVec;
    // auto tFVecRawLeadingElec = tLeadingElec.fVecRaw;

    auto tSubLeadingElec = fElecs->GetSubLeadingElec();
    auto tFVecSubLeadingElec = tSubLeadingElec.fVec;
    // auto tFVecRawSubLeadingElec = tSubLeadingElec.fVecRaw;

    auto tDiElec = tFVecLeadingElec + tFVecSubLeadingElec;

    if (fIsMC && fDoReco) {

      double tRecoEffSFLeading = 0;

      if (tFVecLeadingElec.Pt() < 20.) tRecoEffSFLeading = 0;
      else                             tRecoEffSFLeading = fReco_SF->evaluate({(std::string)(fEra), "sf", "RecoAbove20", tLeadingElec.SCEta(), tFVecLeadingElec.Pt()});

      tEventGenWeight *= tRecoEffSFLeading;


      double tRecoEffSFSubleading = 0;

      if (tFVecSubLeadingElec.Pt() < 20.) tRecoEffSFSubleading = 0;
      else                                tRecoEffSFSubleading = fReco_SF->evaluate({(std::string)(fEra), "sf", "RecoAbove20", tSubLeadingElec.SCEta(), tFVecSubLeadingElec.Pt()});

      tEventGenWeight *= tRecoEffSFSubleading;

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                       Reco efficiency debugging                      " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << " LEADING: " << tFVecLeadingElec.Pt() << " " << tFVecLeadingElec.Eta() << " " << tRecoEffSFLeading << std::endl;
      // std::cout << " SUB-LLEADING: " << tFVecSubLeadingElec.Pt() << " " << tFVecSubLeadingElec.Eta() << " " << tRecoEffSFSubleading << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;
    }

    if (fIsMC && fDoID) {

      const std::string tElecIDWP = fElecs->GetIDWP();

      double tIDEffSFLeading = 0;

      if (tFVecLeadingElec.Pt() < 20.) tIDEffSFLeading = 0;
      // else                             tIDEffSFLeading = fID_SF->evaluate({(std::string)(fEra), "sf", std::abs(tLeadingElec.SCEta())}); // HEEP ID
      else                             tIDEffSFLeading = fID_SF->evaluate({(std::string)(fEra), "sf", tElecIDWP, tLeadingElec.SCEta(), tFVecLeadingElec.Pt()}); // cutBased ID WP

      tEventGenWeight *= tIDEffSFLeading;

      double tIDEffSFSubleading = 0;

      if (tFVecSubLeadingElec.Pt() < 20.) tIDEffSFSubleading = 0;
      // else                                tIDEffSFSubleading = fID_SF->evaluate({(std::string)(fEra), "sf", std::abs(tSubLeadingElec.SCEta())}); // HEEP ID
      else                                tIDEffSFSubleading = fID_SF->evaluate({(std::string)(fEra), "sf", tElecIDWP, tSubLeadingElec.SCEta(), tFVecSubLeadingElec.Pt()}); // cutBased ID WP

      tEventGenWeight *= tIDEffSFSubleading;

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                        ID efficiency debugging                       " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << " LEADING: " << fEra << " " << tFVecLeadingElec.Eta() << " " << tIDEffSFLeading << std::endl;
      // std::cout << " SUB-LLEADING: " << fEra << " " << tFVecSubLeadingElec.Eta() << " " << tIDEffSFSubleading << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;
    }

    if (fIsMC && fDoTRIGG) {

      // Leading electron -> Leg1 SF, sub-leading electron -> Leg2 SF
      // Event trigger SF = SF1 * SF2 (no trigger matching applied)
      double tTrigEffSFLeading    = fTRIG_SF_Leg1->evaluate({tLeadingElec.SCEta(),    tFVecLeadingElec.Pt(),    "nominal"});
      double tTrigEffSFSubleading = fTRIG_SF_Leg2->evaluate({tSubLeadingElec.SCEta(), tFVecSubLeadingElec.Pt(), "nominal"});

      double tTrigEffSF = tTrigEffSFLeading * tTrigEffSFSubleading;

      tEventGenWeight *= tTrigEffSF;

      // std::cout << "######################################################################" << std::endl;
      // std::cout << "                       TRIGG efficiency debugging                     " << std::endl;
      // std::cout << "----------------------------------------------------------------------" << std::endl;
      // std::cout << " LEADING  SCEta=" << tLeadingElec.SCEta() << " Pt=" << tFVecLeadingElec.Pt() << " SF=" << tTrigEffSFLeading << std::endl;
      // std::cout << " SUB-LEADING SCEta=" << tSubLeadingElec.SCEta() << " Pt=" << tFVecSubLeadingElec.Pt() << " SF=" << tTrigEffSFSubleading << std::endl;
      // std::cout << " tTrigEffSF=" << tTrigEffSF << std::endl;
      // std::cout << "######################################################################" << std::endl;
      // std::cout << " " << std::endl;
    }

    // Get gen-lv electrons (status == 1, abs(pdgId) == 11)
    std::vector<std::pair<int, TLorentzVector>> tGenElecs = {};
    if (fIsMC) tGenElecs = fNtuples->GetGenPart(11, 1);
    if (fIsMC && fDoElecMisCharge && tGenElecs.size() > 0) {
      int tLeadMatchedIndex = -1;
      int tSubMatchedIndex = -1;
      double tLeadMatchedDeltaR = 9999.;
      double tSubMatchedDeltaR = 9999.;

      // Step 1: find the global minimal pair
      double tBestDeltaR = 9999.;
      bool tBestIsLead = true;
      int tBestGenIndex = -1;
      for (int i = 0; i < tGenElecs.size(); i++) {
        double dRlead = tFVecLeadingElec.DeltaR(tGenElecs.at(i).second);
        double dRsub  = tFVecSubLeadingElec.DeltaR(tGenElecs.at(i).second);
        if (dRlead < tBestDeltaR) { tBestDeltaR = dRlead; tBestIsLead = true;  tBestGenIndex = i; }
        if (dRsub  < tBestDeltaR) { tBestDeltaR = dRsub;  tBestIsLead = false; tBestGenIndex = i; }
      }

      if (tBestGenIndex != -1) {
        if (tBestIsLead) {
          tLeadMatchedIndex = tBestGenIndex;
          tLeadMatchedDeltaR = tBestDeltaR;
        } else {
          tSubMatchedIndex = tBestGenIndex;
          tSubMatchedDeltaR = tBestDeltaR;
        }
      }

      // Step 2: match the other reco to the closest among remaining gen
      if (tGenElecs.size() >= 2) {
        if (tLeadMatchedIndex != -1) {
          double tBestSubDeltaR = 9999.;
          int tBestSubIndex = -1;
          for (int i = 0; i < tGenElecs.size(); i++) {
            if (i == tLeadMatchedIndex) continue;
            double dR = tFVecSubLeadingElec.DeltaR(tGenElecs.at(i).second);
            if (dR < tBestSubDeltaR) { tBestSubDeltaR = dR; tBestSubIndex = i; }
          }

          tSubMatchedIndex = tBestSubIndex;
          tSubMatchedDeltaR = tBestSubDeltaR;
        } else if (tSubMatchedIndex != -1) {
          double tBestLeadDeltaR = 9999.;
          int tBestLeadIndex = -1;
          for (int i = 0; i < tGenElecs.size(); i++) {
            if (i == tSubMatchedIndex) continue;
            double dR = tFVecLeadingElec.DeltaR(tGenElecs.at(i).second);
            if (dR < tBestLeadDeltaR) { tBestLeadDeltaR = dR; tBestLeadIndex = i; }
          }
          tLeadMatchedIndex = tBestLeadIndex;
          tLeadMatchedDeltaR = tBestLeadDeltaR;
        }
      } else {
        // Only one gen: whichever reco had the global best gets matched; the other stays unmatched
      }

      // Apply mis-charge SF to the matched reco electrons.
      if (tLeadMatchedIndex != -1) {
        int tBinIndexX = fElecMisCharge_SF->GetXaxis()->FindBin(std::abs(tLeadingElec.SCEta()));
        if (tBinIndexX == 0) tBinIndexX = 1;
        else if (tBinIndexX > fElecMisCharge_SF->GetNbinsX()) tBinIndexX = fElecMisCharge_SF->GetNbinsX();
        
        int tBinIndexY = fElecMisCharge_SF->GetYaxis()->FindBin(tFVecLeadingElec.Pt());
        if (tBinIndexY == 0) tBinIndexY = 1;
        else if (tBinIndexY > fElecMisCharge_SF->GetNbinsY()) tBinIndexY = fElecMisCharge_SF->GetNbinsY();
        
        if (tLeadingElec.fCharge * tGenElecs.at(tLeadMatchedIndex).first < 0) {
          double tElecMisChargeSFWeight = fElecMisCharge_SF->GetBinContent(tBinIndexX, tBinIndexY);
          tEventGenWeight *= tElecMisChargeSFWeight;
        }
      }

      if (tSubMatchedIndex != -1) {
        int tBinIndexX = fElecMisCharge_SF->GetXaxis()->FindBin(std::abs(tSubLeadingElec.SCEta()));
        if (tBinIndexX == 0) tBinIndexX = 1;
        else if (tBinIndexX > fElecMisCharge_SF->GetNbinsX()) tBinIndexX = fElecMisCharge_SF->GetNbinsX();
        
        int tBinIndexY = fElecMisCharge_SF->GetYaxis()->FindBin(tFVecSubLeadingElec.Pt());
        if (tBinIndexY == 0) tBinIndexY = 1;
        else if (tBinIndexY > fElecMisCharge_SF->GetNbinsY()) tBinIndexY = fElecMisCharge_SF->GetNbinsY();
        
        if (tSubLeadingElec.fCharge * tGenElecs.at(tSubMatchedIndex).first < 0) {
          double tElecMisChargeSFWeight = fElecMisCharge_SF->GetBinContent(tBinIndexX, tBinIndexY);
          tEventGenWeight *= tElecMisChargeSFWeight;
        }
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
    fHistoSet->FillElec(tFVecLeadingElec, tFVecSubLeadingElec, nJets, nBJets, tEventGenWeight);
    fHistoSet->FillJet(&vJets, &vBJets, tDiElec.M(), tEventGenWeight);

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

void DYLoopEE::EndOfJob() {

  fHistoSet->WriteHisto(fEra, fSampleName, "./ROOT/output_" + fEra + "_" +  fSampleName + "_" + std::to_string(fJobID) + ".root", !fIsMC);
}
