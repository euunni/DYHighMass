#include <iostream>
#include <vector>

#include "muon.h"

#include "TChain.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TRandom.h"

void MUON::init(TTreeReader* fTreeReader) {

  nMuon = new TTreeReaderValue<unsigned int>(*fTreeReader, "nMuon");
  Muon_pt = new TTreeReaderArray<float>(*fTreeReader, "Muon_pt");
  Muon_eta = new TTreeReaderArray<float>(*fTreeReader, "Muon_eta");
  Muon_phi = new TTreeReaderArray<float>(*fTreeReader, "Muon_phi");
  Muon_charge = new TTreeReaderArray<int>(*fTreeReader, "Muon_charge");
  Muon_mass = new TTreeReaderArray<float>(*fTreeReader, "Muon_mass");
  Muon_tightId = new TTreeReaderArray<bool>(*fTreeReader, "Muon_tightId");
  Muon_pfRelIso04_all = new TTreeReaderArray<float>(*fTreeReader, "Muon_pfRelIso04_all");
  Muon_nTrackerLayers = new TTreeReaderArray<int>(*fTreeReader, "Muon_nTrackerLayers");

  if (fIsMC && fTreeReader->GetTree() && fTreeReader->GetTree()->GetBranch("Muon_genPartIdx"))
    Muon_genPartIdx = new TTreeReaderArray<int>(*fTreeReader, "Muon_genPartIdx");

  if (fIsMC && fTreeReader->GetTree() && fTreeReader->GetTree()->GetBranch("GenPart_pt"))
    GenPart_pt = new TTreeReaderArray<float>(*fTreeReader, "GenPart_pt");
}

TLorentzVector MUON::GetRochesterCorrectedMuon(TLorentzVector fMu, int fIndex) {

  if (!fRoccoR)
    return fMu;

  const int tCharge = Muon_charge->At(fIndex);
  double tCorrectionFactor = 1.;

  if (fIsMC) {
    const bool tHasGenMatch = (Muon_genPartIdx != nullptr && GenPart_pt != nullptr);
    const int tGenIndex = tHasGenMatch ? Muon_genPartIdx->At(fIndex) : -1;

    if (tGenIndex >= 0 && tGenIndex < GenPart_pt->GetSize()) {
      tCorrectionFactor = fRoccoR->kSpreadMC(
        tCharge,
        fMu.Pt(),
        fMu.Eta(),
        fMu.Phi(),
        GenPart_pt->At(tGenIndex),
        5,
        0
      );
    } else {
      tCorrectionFactor = fRoccoR->kSmearMC(
        tCharge,
        fMu.Pt(),
        fMu.Eta(),
        fMu.Phi(),
        Muon_nTrackerLayers->At(fIndex),
        gRandom->Rndm(),
        5,
        0
      );
    }
  } else {
    tCorrectionFactor = fRoccoR->kScaleDT(
      tCharge,
      fMu.Pt(),
      fMu.Eta(),
      fMu.Phi(),
      5,
      0
    );
  }

  TLorentzVector tCorrectedMuon = fMu;
  tCorrectedMuon *= tCorrectionFactor;
  return tCorrectedMuon;
}

TLorentzVector MUON::GetMCSmearing (TLorentzVector fMu) {

  if (std::abs(fMu.Eta()) <= 1.2 && fSmearingEngine->DoBarrel()) { // barrel

    double fMomentum = fMu.P();
    double tSmearingFactor = 1 + gRandom->Gaus(0, fSmearingEngine->GetBarrelSmearingFactor() * fSmearingEngine->GetBarrelSigma(fMomentum));

    TLorentzVector fMuReturn;
    fMuReturn.SetPtEtaPhiM(tSmearingFactor * fMu.Pt(), fMu.Eta(), fMu.Phi(), fMu.M());
    return fMuReturn;

  } else if (std::abs(fMu.Eta()) > 1.2 && std::abs(fMu.Eta()) < 2.4 && fSmearingEngine->DoEndcap()) { // endcap
    
    double fMomentum = fMu.P();
    double tSmearingFactor = 1 + gRandom->Gaus(0, fSmearingEngine->GetEndcapSmearingFactor() * fSmearingEngine->GetEndcapSigma(fMomentum));

    TLorentzVector fMuReturn;
    fMuReturn.SetPtEtaPhiM(tSmearingFactor * fMu.Pt(), fMu.Eta(), fMu.Phi(), fMu.M());
    return fMuReturn;
    
  } else {

    return fMu;
  }
}

bool MUON::PrepareMuon() {

  fFVecMuons.clear();

  fFVecOSMuons.clear();
  fFVecSSMuons.clear();
  fFVecOSinvertedMuons.clear();
  fFVecSSinvertedMuons.clear();

  for (int i = 0; i < **nMuon; i++) {
    
    if ( !Muon_tightId->At(i) )
      continue;

    if (std::abs(Muon_eta->At(i)) > fEta)
      continue;

    TLorentzVector mu;
    mu.SetPtEtaPhiM(Muon_pt->At(i), Muon_eta->At(i), Muon_phi->At(i), Muon_mass->At(i));

    TLorentzVector mu_corr = mu;

    if (fApplyRoccoR)
      mu_corr = GetRochesterCorrectedMuon(mu, i);
    else if (fDoMCSmearing)
      mu_corr = GetMCSmearing(mu);
    
    if ( !(mu_corr.Pt() > fSubLeadingMuonPt) )
      continue;

    int tIso = 1;
    if (Muon_pfRelIso04_all->At(i) >= fISO) tIso = -1;

    StdMuon mu_std = StdMuon(mu_corr, mu, Muon_charge->At(i), tIso);
    fFVecMuons.push_back(mu_std);
  }

  std::sort(fFVecMuons.begin(), fFVecMuons.end(), [](const StdMuon &lhs, const StdMuon &rhs) {
    return lhs.fVec.Pt() > rhs.fVec.Pt();
  });

  if (fFVecMuons.size() < 2)
    return false;

  for (int i = 0; i < fFVecMuons.size(); i++) {
    for (int j = i + 1; j < fFVecMuons.size(); j++) {

      if (fFVecOSMuons.size() == 0
          && fFVecMuons.at(i).fCharge * fFVecMuons.at(j).fCharge < 0
          && fFVecMuons.at(i).fISO == 1
          && fFVecMuons.at(j).fISO == 1
          && ( (fFVecMuons.at(i).fVec.Pt() > fLeadingMuonPt && fFVecMuons.at(j).fVec.Pt() > fSubLeadingMuonPt) ||
               (fFVecMuons.at(i).fVec.Pt() > fSubLeadingMuonPt && fFVecMuons.at(j).fVec.Pt() > fLeadingMuonPt) ) 
         ) {

        const auto tDiMuon = fFVecMuons.at(i).fVec + fFVecMuons.at(j).fVec;
        if (tDiMuon.M() < fZMassCut)
          continue;
        if (tDiMuon.M() / fFVecMuons.at(j).fVec.Pt() >= 10.)
          continue;

        fFVecOSMuons.push_back(fFVecMuons.at(i));
        fFVecOSMuons.push_back(fFVecMuons.at(j));

        continue;
      }

      if (fFVecSSMuons.size() == 0
          && fFVecMuons.at(i).fCharge * fFVecMuons.at(j).fCharge > 0
          && fFVecMuons.at(i).fISO == 1
          && fFVecMuons.at(j).fISO == 1
          && ( (fFVecMuons.at(i).fVec.Pt() > fLeadingMuonPt && fFVecMuons.at(j).fVec.Pt() > fSubLeadingMuonPt) ||
               (fFVecMuons.at(i).fVec.Pt() > fSubLeadingMuonPt && fFVecMuons.at(j).fVec.Pt() > fLeadingMuonPt) ) 
         ) {

        const auto tDiMuon = fFVecMuons.at(i).fVec + fFVecMuons.at(j).fVec;
        if (tDiMuon.M() < fZMassCut)
          continue;
        if (tDiMuon.M() / fFVecMuons.at(j).fVec.Pt() >= 10.)
          continue;

        fFVecSSMuons.push_back(fFVecMuons.at(i));
        fFVecSSMuons.push_back(fFVecMuons.at(j));

        continue;
      }

      if (fFVecOSinvertedMuons.size() == 0
          && fFVecMuons.at(i).fCharge * fFVecMuons.at(j).fCharge < 0
          && (fFVecMuons.at(i).fISO + fFVecMuons.at(j).fISO <= 0)
          && ( (fFVecMuons.at(i).fVec.Pt() > fLeadingMuonPt && fFVecMuons.at(j).fVec.Pt() > fSubLeadingMuonPt) ||
               (fFVecMuons.at(i).fVec.Pt() > fSubLeadingMuonPt && fFVecMuons.at(j).fVec.Pt() > fLeadingMuonPt) ) 
         ) {

        const auto tDiMuon = fFVecMuons.at(i).fVec + fFVecMuons.at(j).fVec;
        if (tDiMuon.M() < fZMassCut)
          continue;
        if (tDiMuon.M() / fFVecMuons.at(j).fVec.Pt() >= 10.)
          continue;

        fFVecOSinvertedMuons.push_back(fFVecMuons.at(i));
        fFVecOSinvertedMuons.push_back(fFVecMuons.at(j));

        continue;
      }

      if (fFVecSSinvertedMuons.size() == 0
          && fFVecMuons.at(i).fCharge * fFVecMuons.at(j).fCharge > 0
          && (fFVecMuons.at(i).fISO + fFVecMuons.at(j).fISO <= 0)
          && ( (fFVecMuons.at(i).fVec.Pt() > fLeadingMuonPt && fFVecMuons.at(j).fVec.Pt() > fSubLeadingMuonPt) ||
              (fFVecMuons.at(i).fVec.Pt() > fSubLeadingMuonPt && fFVecMuons.at(j).fVec.Pt() > fLeadingMuonPt) ) 
        ) {
      
        const auto tDiMuon = fFVecMuons.at(i).fVec + fFVecMuons.at(j).fVec;
        if (tDiMuon.M() < fZMassCut)
          continue;
        if (tDiMuon.M() / fFVecMuons.at(j).fVec.Pt() >= 10.)
          continue;

        fFVecSSinvertedMuons.push_back(fFVecMuons.at(j));
        fFVecSSinvertedMuons.push_back(fFVecMuons.at(i));

        continue;
      }
    }
  }

  // std::cout << " " << std::endl;
  // std::cout << "######################################################################" << std::endl;
  // std::cout << "                              Muon debug                              " << std::endl;
  // std::cout << "----------------------------------------------------------------------" << std::endl;
  // if (fFVecOSMuons.size() == 2) {
  //   std::cout << " OS: " << std::endl;
  //   std::cout << "    1 - " << fFVecOSMuons.at(0).fVec.Pt() << " " << fFVecOSMuons.at(0).fVec.Eta() << " " << fFVecOSMuons.at(0).fVec.Phi() << " " << fFVecOSMuons.at(0).fCharge << " " << fFVecOSMuons.at(0).fISO << std::endl;
  //   std::cout << "    1 - " << fFVecOSMuons.at(0).fVecRaw.Pt() << " " << fFVecOSMuons.at(0).fVecRaw.Eta() << " " << fFVecOSMuons.at(0).fVecRaw.Phi() << " " << fFVecOSMuons.at(0).fCharge << " " << fFVecOSMuons.at(0).fISO << std::endl;
  //   std::cout << "    2 - " << fFVecOSMuons.at(1).fVec.Pt() << " " << fFVecOSMuons.at(1).fVec.Eta() << " " << fFVecOSMuons.at(1).fVec.Phi() << " " << fFVecOSMuons.at(1).fCharge << " " << fFVecOSMuons.at(1).fISO << std::endl;
  //   std::cout << "    2 - " << fFVecOSMuons.at(1).fVecRaw.Pt() << " " << fFVecOSMuons.at(1).fVecRaw.Eta() << " " << fFVecOSMuons.at(1).fVecRaw.Phi() << " " << fFVecOSMuons.at(1).fCharge << " " << fFVecOSMuons.at(1).fISO << std::endl;
  // }
  // if (fFVecSSMuons.size() == 2) {
  //   std::cout << " SS: " << std::endl;
  //   std::cout << "    1 - " << fFVecSSMuons.at(0).fVec.Pt() << " " << fFVecSSMuons.at(0).fVec.Eta() << " " << fFVecSSMuons.at(0).fVec.Phi() << " " << fFVecSSMuons.at(0).fCharge << " " << fFVecSSMuons.at(0).fISO << std::endl;
  //   std::cout << "    1 - " << fFVecSSMuons.at(0).fVecRaw.Pt() << " " << fFVecSSMuons.at(0).fVecRaw.Eta() << " " << fFVecSSMuons.at(0).fVecRaw.Phi() << " " << fFVecSSMuons.at(0).fCharge << " " << fFVecSSMuons.at(0).fISO << std::endl;
  //   std::cout << "    2 - " << fFVecSSMuons.at(1).fVec.Pt() << " " << fFVecSSMuons.at(1).fVec.Eta() << " " << fFVecSSMuons.at(1).fVec.Phi() << " " << fFVecSSMuons.at(1).fCharge << " " << fFVecSSMuons.at(1).fISO << std::endl;
  //   std::cout << "    2 - " << fFVecSSMuons.at(1).fVecRaw.Pt() << " " << fFVecSSMuons.at(1).fVecRaw.Eta() << " " << fFVecSSMuons.at(1).fVecRaw.Phi() << " " << fFVecSSMuons.at(1).fCharge << " " << fFVecSSMuons.at(1).fISO << std::endl;
  // }
  // if (fFVecOSinvertedMuons.size() == 2) {
  //   std::cout << " OS_inverted: " << std::endl;
  //   std::cout << "    1 - " << fFVecOSinvertedMuons.at(0).fVec.Pt() << " " << fFVecOSinvertedMuons.at(0).fVec.Eta() << " " << fFVecOSinvertedMuons.at(0).fVec.Phi() << " " << fFVecOSinvertedMuons.at(0).fCharge << " " << fFVecOSinvertedMuons.at(0).fISO << std::endl;
  //   std::cout << "    1 - " << fFVecOSinvertedMuons.at(0).fVecRaw.Pt() << " " << fFVecOSinvertedMuons.at(0).fVecRaw.Eta() << " " << fFVecOSinvertedMuons.at(0).fVecRaw.Phi() << " " << fFVecOSinvertedMuons.at(0).fCharge << " " << fFVecOSinvertedMuons.at(0).fISO << std::endl;
  //   std::cout << "    2 - " << fFVecOSinvertedMuons.at(1).fVec.Pt() << " " << fFVecOSinvertedMuons.at(1).fVec.Eta() << " " << fFVecOSinvertedMuons.at(1).fVec.Phi() << " " << fFVecOSinvertedMuons.at(1).fCharge << " " << fFVecOSinvertedMuons.at(1).fISO << std::endl;
  //   std::cout << "    2 - " << fFVecOSinvertedMuons.at(1).fVecRaw.Pt() << " " << fFVecOSinvertedMuons.at(1).fVecRaw.Eta() << " " << fFVecOSinvertedMuons.at(1).fVecRaw.Phi() << " " << fFVecOSinvertedMuons.at(1).fCharge << " " << fFVecOSinvertedMuons.at(1).fISO << std::endl;
  // }
  // if (fFVecSSinvertedMuons.size() == 2) {
  //   std::cout << " SS_inverted: " << std::endl;
  //   std::cout << "    1 - " << fFVecSSinvertedMuons.at(0).fVec.Pt() << " " << fFVecSSinvertedMuons.at(0).fVec.Eta() << " " << fFVecSSinvertedMuons.at(0).fVec.Phi() << " " << fFVecSSinvertedMuons.at(0).fCharge << " " << fFVecSSinvertedMuons.at(0).fISO << std::endl;
  //   std::cout << "    1 - " << fFVecSSinvertedMuons.at(0).fVecRaw.Pt() << " " << fFVecSSinvertedMuons.at(0).fVecRaw.Eta() << " " << fFVecSSinvertedMuons.at(0).fVecRaw.Phi() << " " << fFVecSSinvertedMuons.at(0).fCharge << " " << fFVecSSinvertedMuons.at(0).fISO << std::endl;
  //   std::cout << "    2 - " << fFVecSSinvertedMuons.at(1).fVec.Pt() << " " << fFVecSSinvertedMuons.at(1).fVec.Eta() << " " << fFVecSSinvertedMuons.at(1).fVec.Phi() << " " << fFVecSSinvertedMuons.at(1).fCharge << " " << fFVecSSinvertedMuons.at(1).fISO << std::endl;
  //   std::cout << "    2 - " << fFVecSSinvertedMuons.at(1).fVecRaw.Pt() << " " << fFVecSSinvertedMuons.at(1).fVecRaw.Eta() << " " << fFVecSSinvertedMuons.at(1).fVecRaw.Phi() << " " << fFVecSSinvertedMuons.at(1).fCharge << " " << fFVecSSinvertedMuons.at(1).fISO << std::endl;
  // }
  // std::cout << "######################################################################" << std::endl;
  // std::cout << " " << std::endl;

  return (fFVecOSMuons.size() == 2 || fFVecSSMuons.size() == 2 || fFVecOSinvertedMuons.size() == 2 || fFVecSSinvertedMuons.size() == 2);
}

std::vector<TLorentzVector> MUON::GetFiducialMuons(const std::vector<std::pair<int, TLorentzVector>>& fMuons) const {
  
  std::vector<TLorentzVector> tReturnVec = {};
  bool tIsFiducial = false;

  for (int i = 0; i < fMuons.size(); i++) {
    
    if (std::abs(fMuons.at(i).second.Eta()) > fEta)
      continue;

    if (fMuons.at(i).second.Pt() < fSubLeadingMuonPt)
      continue;
    
    for (int j = i + 1; j < fMuons.size(); j++) {
      
      if (std::abs(fMuons.at(j).second.Eta()) > fEta)
        continue;

      if (fMuons.at(j).second.Pt() < fSubLeadingMuonPt)
        continue;

      if (
        fMuons.at(i).first * fMuons.at(j).first < 0 &&
        ((fMuons.at(i).second.Pt() > fLeadingMuonPt && fMuons.at(j).second.Pt() > fSubLeadingMuonPt) ||
         (fMuons.at(i).second.Pt() > fSubLeadingMuonPt && fMuons.at(j).second.Pt() > fLeadingMuonPt))
      ) {

        if (fMuons.at(i).second.Pt() > fMuons.at(j).second.Pt()) {
          tReturnVec.push_back(fMuons.at(i).second);
          tReturnVec.push_back(fMuons.at(j).second);
        } else {
          tReturnVec.push_back(fMuons.at(j).second);
          tReturnVec.push_back(fMuons.at(i).second);
        }

        tIsFiducial = true;
      }

      if (tIsFiducial) break;
    }

    if (tIsFiducial) break;
  }
  
  return tReturnVec;
}
