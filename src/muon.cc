#include <iostream>
#include <vector>

#include "muon.h"

#include "TChain.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TRandom.h"

void MUON::init(TTreeReader* fTreeReader) {

  // if (fIsMC) {

  //   nGenPart = new TTreeReaderValue<unsigned int>(*fTreeReader, "nGenPart");
  //   GenPart_pt = new TTreeReaderArray<float>(*fTreeReader, "GenPart_pt");
  //   GenPart_eta = new TTreeReaderArray<float>(*fTreeReader, "GenPart_eta");
  //   GenPart_phi = new TTreeReaderArray<float>(*fTreeReader, "GenPart_phi");
  //   GenPart_mass = new TTreeReaderArray<float>(*fTreeReader, "GenPart_mass");
  //   GenPart_pdgId = new TTreeReaderArray<int>(*fTreeReader, "GenPart_pdgId");
  // }

  nMuon = new TTreeReaderValue<unsigned int>(*fTreeReader, "nMuon");
  Muon_pt = new TTreeReaderArray<float>(*fTreeReader, "Muon_pt");
  Muon_tunepRelPt = new TTreeReaderArray<float>(*fTreeReader, "Muon_tunepRelPt");
  Muon_eta = new TTreeReaderArray<float>(*fTreeReader, "Muon_eta");
  Muon_phi = new TTreeReaderArray<float>(*fTreeReader, "Muon_phi");
  Muon_charge = new TTreeReaderArray<int>(*fTreeReader, "Muon_charge");
  Muon_mass = new TTreeReaderArray<float>(*fTreeReader, "Muon_mass");
  Muon_highPtId = new TTreeReaderArray<unsigned char>(*fTreeReader, "Muon_highPtId");
  Muon_tkRelIso = new TTreeReaderArray<float>(*fTreeReader, "Muon_tkRelIso");
  Muon_nTrackerLayers = new TTreeReaderArray<int>(*fTreeReader, "Muon_nTrackerLayers");
  Muon_highPurity = new TTreeReaderArray<bool>(*fTreeReader, "Muon_highPurity");
}

// void MUON::PrepareGenMuon() {

//   fFVecGenMuons.clear();

//   for (int i  = 0; i < **nGenPart; i++) {

//     if ( !(std::abs(GenPart_pdgId->At(i)) == 13) )
//       continue;

//     TLorentzVector mu;
//     mu.SetPtEtaPhiM(GenPart_pt->At(i), GenPart_eta->At(i), GenPart_phi->At(i), GenPart_mass->At(i));

//     StdMuon mu_std = StdMuon(mu, mu, (-1) * (GenPart_pdgId->At(i) / std::abs(GenPart_pdgId->At(i))));
//     fFVecGenMuons.push_back(mu_std);
//   }

//   std::sort(fFVecGenMuons.begin(), fFVecGenMuons.end(), [](const StdMuon &lhs, const StdMuon &rhs) {
//     return lhs.fVec.Pt() > rhs.fVec.Pt();
//   });
// }

// TLorentzVector MUON::GetRochesterCorrectedMuon (TLorentzVector fMu, int fMuCharge, int nTkLayers) {

//   double tCorrectionFactor = 1.;
//   if (!fIsMC) {
//     tCorrectionFactor = fRoccoR->kScaleDT(
//       fMuCharge,
//       fMu.Pt(),
//       fMu.Eta(),
//       fMu.Phi(),
//       5,
//       0
//     );
//   } else {

//     double drmin = 999.;
//     bool match = false;
//     int genMuonIdx = 0;

//     for (int j = 0; j < fFVecGenMuons.size(); j++) {
//       if (fMu.DeltaR(fFVecGenMuons.at(j).fVec) < 0.1 &&
//           fMu.DeltaR(fFVecGenMuons.at(j).fVec) < drmin) {
//         match = true;
//         genMuonIdx = j;
//         drmin = fMu.DeltaR(fFVecGenMuons.at(j).fVec);
//       }
//     }

//     if (match) {
//       tCorrectionFactor = fRoccoR->kSpreadMC(
//         fMuCharge,
//         fMu.Pt(),
//         fMu.Eta(),
//         fMu.Phi(),
//         fFVecGenMuons.at(genMuonIdx).fVec.Pt(),
//         5,
//         0
//       );
//     } else {
//       double rndm = gRandom->Rndm();
//       tCorrectionFactor = fRoccoR->kSmearMC(
//         fMuCharge,
//         fMu.Pt(),
//         fMu.Eta(),
//         fMu.Phi(),
//         nTkLayers,
//         rndm,
//         5,
//         0
//       );
//     }
//   }
  
//   if (tCorrectionFactor != 1.) {

//     TLorentzVector fMuReturn;
//     fMuReturn.SetPtEtaPhiM(tCorrectionFactor * fMu.Pt(), fMu.Eta(), fMu.Phi(), fMu.M());
//     return fMuReturn;
//   } else {
  
//     return fMu;
//   }
// }

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

  for (int i = 0; i < **nMuon; i++) {
    if ( !(Muon_highPtId->At(i) == fID) )
      continue;

    if (std::abs(Muon_eta->At(i)) > fEta)
      continue;

    if (!Muon_highPurity->At(i))
      continue;

    TLorentzVector mu;
    mu.SetPtEtaPhiM(Muon_pt->At(i) * Muon_tunepRelPt->At(i), Muon_eta->At(i), Muon_phi->At(i), Muon_mass->At(i));

    TLorentzVector mu_corr;

    // if (fDoRoccoR && !fDoMCSmearing)
      // mu_corr = GetRochesterCorrectedMuon(mu, Muon_charge->At(i), Muon_nTrackerLayers->At(i));

    if (!fDoRoccoR && fDoMCSmearing)
      mu_corr = GetMCSmearing(mu);

    if (!fDoRoccoR && !fDoMCSmearing)
      mu_corr = mu;
    
    if ( !(mu_corr.Pt() > fSubLeadingMuonPt) )
      continue;

    int tIso = 1;
    if (Muon_tkRelIso->At(i) > fISO) tIso = -1;

    if (!fISOinverted && tIso == 1) {
      StdMuon mu_std = StdMuon(mu_corr, mu, Muon_charge->At(i), tIso);
      fFVecMuons.push_back(mu_std);
    } else if (fISOinverted) {
      StdMuon mu_std = StdMuon(mu_corr, mu, Muon_charge->At(i), tIso);
      fFVecMuons.push_back(mu_std);
    }
  }

  std::sort(fFVecMuons.begin(), fFVecMuons.end(), [](const StdMuon &lhs, const StdMuon &rhs) {
    return lhs.fVec.Pt() > rhs.fVec.Pt();
  });

  if (fFVecMuons.size() < 2)
    return false;

  int tLeadingIdx = -1;
  int tSubLeadingIdx = -1;

  int tChargeSelection = 1;
  if (!fOppositeCharge) tChargeSelection = -1;

  for (int i = 0; i < fFVecMuons.size(); i++) {
    for (int j = i + 1; j < fFVecMuons.size(); j++) {
      if (tChargeSelection * (fFVecMuons.at(i).fCharge * fFVecMuons.at(j).fCharge) > 0)
        continue;

      if (!fISOinverted && fFVecMuons.at(i).fVec.Pt() < fLeadingMuonPt && fFVecMuons.at(j).fVec.Pt() < fLeadingMuonPt)
        continue;

      if (fISOinverted && fFVecMuons.at(i).fISO * fFVecMuons.at(j).fISO > 0)
        continue;

      if (fISOinverted && fFVecMuons.at(i).fISO == 1 && !(fFVecMuons.at(i).fVec.Pt() > fLeadingMuonPt)) 
        continue;

      if (fISOinverted && fFVecMuons.at(j).fISO == 1 && !(fFVecMuons.at(j).fVec.Pt() > fLeadingMuonPt)) 
        continue;

      auto tDimuonVec = fFVecMuons.at(i).fVec + fFVecMuons.at(j).fVec;

      if (tDimuonVec.M() < fZMassCut)
        continue;

      tLeadingIdx = i;
      tSubLeadingIdx = j;

      if (fISOinverted) {
        if (fFVecMuons.at(i).fISO == -1 && fFVecMuons.at(j).fISO == 1) {
          tLeadingIdx = j;
          tSubLeadingIdx = i;
        } 
      }

      if (tSubLeadingIdx != -1 && tLeadingIdx != -1)
        break;
    }
    
    if (tSubLeadingIdx != -1 && tLeadingIdx != -1)
      break;
  }

  if (tSubLeadingIdx == -1 || tLeadingIdx == -1)
    return false;

  fLeadingIdx = tLeadingIdx;
  fSubLeadingIdx = tSubLeadingIdx;

  return true;
}
