#include <iostream>
#include <vector>

#include "emu.h"

#include "TChain.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TRandom.h"

void EMU::init(TTreeReader* fTreeReader) {

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

  nElectron = new TTreeReaderValue<unsigned int>(*fTreeReader, "nElectron");
  Electron_pt = new TTreeReaderArray<float>(*fTreeReader, "Electron_pt");
  Electron_eta = new TTreeReaderArray<float>(*fTreeReader, "Electron_eta");
  Electron_deltaEtaSC = new TTreeReaderArray<float>(*fTreeReader, "Electron_deltaEtaSC");
  Electron_phi = new TTreeReaderArray<float>(*fTreeReader, "Electron_phi");
  Electron_charge = new TTreeReaderArray<int>(*fTreeReader, "Electron_charge");
  Electron_mass = new TTreeReaderArray<float>(*fTreeReader, "Electron_mass");
  Electron_cutBased = new TTreeReaderArray<int>(*fTreeReader, "Electron_cutBased");
}

// void EMU::PrepareGenMuon() {

//   fFVecGenMuons.clear();

//   for (int i  = 0; i < **nGenPart; i++) {

//     if ( !(std::abs(GenPart_pdgId->At(i)) == 13) )
//       continue;

//     TLorentzVector mu;
//     mu.SetPtEtaPhiM(GenPart_pt->At(i), GenPart_eta->At(i), GenPart_phi->At(i), GenPart_mass->At(i));

//     EMU_MUON mu_std = EMU_MUON(mu, mu, (-1) * (GenPart_pdgId->At(i) / std::abs(GenPart_pdgId->At(i))));
//     fFVecGenMuons.push_back(mu_std);
//   }

//   std::sort(fFVecGenMuons.begin(), fFVecGenMuons.end(), [](const EMU_MUON &lhs, const EMU_MUON &rhs) {
//     return lhs.fVec.Pt() > rhs.fVec.Pt();
//   });
// }

TLorentzVector EMU::GetMuonMCSmearing (TLorentzVector fMu) {

  if (std::abs(fMu.Eta()) <= 1.2 && fMuonSmearingEngine->DoBarrel()) { // barrel

    double fMomentum = fMu.P();
    double tSmearingFactor = 1 + gRandom->Gaus(0, fMuonSmearingEngine->GetBarrelSmearingFactor() * fMuonSmearingEngine->GetBarrelSigma(fMomentum));

    TLorentzVector fMuReturn;
    fMuReturn.SetPtEtaPhiM(tSmearingFactor * fMu.Pt(), fMu.Eta(), fMu.Phi(), fMu.M());
    return fMuReturn;

  } else if (std::abs(fMu.Eta()) > 1.2 && std::abs(fMu.Eta()) < 2.4 && fMuonSmearingEngine->DoEndcap()) { // endcap
    
    double fMomentum = fMu.P();
    double tSmearingFactor = 1 + gRandom->Gaus(0, fMuonSmearingEngine->GetEndcapSmearingFactor() * fMuonSmearingEngine->GetEndcapSigma(fMomentum));

    TLorentzVector fMuReturn;
    fMuReturn.SetPtEtaPhiM(tSmearingFactor * fMu.Pt(), fMu.Eta(), fMu.Phi(), fMu.M());
    return fMuReturn;
    
  } else {

    return fMu;
  }
}

bool EMU::PrepareEMUPair() {

  fFVecMuons.clear();
  fFVecElecs.clear();

  fSelectedMuonIdx = -1;
  fSelectedElecIdx = -1;

  for (int i = 0; i < **nMuon; i++) {
    if ( !(Muon_highPtId->At(i) == fMuonID) )
      continue;

    if ( !fMuonISOinverted && !(Muon_tkRelIso->At(i) < fMuonISO) )
      continue;

    if ( fMuonISOinverted && Muon_tkRelIso->At(i) < fMuonISO )
      continue;

    if (std::abs(Muon_eta->At(i)) > fMuonEta)
      continue;

    if (!Muon_highPurity->At(i))
      continue;

    TLorentzVector mu;
    mu.SetPtEtaPhiM(Muon_pt->At(i) * Muon_tunepRelPt->At(i), Muon_eta->At(i), Muon_phi->At(i), Muon_mass->At(i));

    TLorentzVector mu_corr;
    if (fDoMuonMCSmearing) mu_corr = GetMuonMCSmearing(mu);
    else mu_corr = mu;

    if ( !(mu_corr.Pt() > fMuonPt) )
      continue;

    EMU_MUON mu_std = EMU_MUON(mu_corr, mu, Muon_charge->At(i));
    fFVecMuons.push_back(mu_std);
  }

  std::sort(fFVecMuons.begin(), fFVecMuons.end(), [](const EMU_MUON &lhs, const EMU_MUON &rhs) {
    return lhs.fVec.Pt() > rhs.fVec.Pt();
  });

  for (int i = 0; i < **nElectron; i++) {

    if (!(Electron_pt->At(i) > fElecPt))
      continue;

    float eSCEta = Electron_eta->At(i) + Electron_deltaEtaSC->At(i);
    if (!(std::abs(eSCEta) < fElecEta))
      continue;

    if (std::abs(eSCEta) > 1.4442 && std::abs(eSCEta) < 1.5660)
      continue;

    if (!fElecIDinverted && Electron_cutBased->At(i) < fElecID)
      continue;

    if (fElecIDinverted && Electron_cutBased->At(i) >= fElecID)
      continue;


    TLorentzVector elecs;
    elecs.SetPtEtaPhiM(Electron_pt->At(i), Electron_eta->At(i), Electron_phi->At(i), Electron_mass->At(i));

    fFVecElecs.push_back(EMU_ELEC(elecs, eSCEta, Electron_charge->At(i)));
  }

  std::sort(fFVecElecs.begin(), fFVecElecs.end(), [](const EMU_ELEC &lhs, const EMU_ELEC &rhs) {
    return lhs.fVec.Pt() > rhs.fVec.Pt();
  });

  float tChargeSelection = 1;
  if (!fIsOppositeCharge) tChargeSelection = -1;

  for (int i = 0; i < fFVecMuons.size(); i++) {
    for (int j = 0; j < fFVecElecs.size(); j++) {
      if (tChargeSelection * (fFVecMuons.at(i).fCharge * fFVecElecs.at(j).fCharge) > 0)
        continue;

      fSelectedMuonIdx = i;
      fSelectedElecIdx = j;
      break;
    }
  }

  // std::cout << "######################################################################" << std::endl;
  // std::cout << "                       Debug EMu Pair Selection                       " << std::endl;
  // std::cout << "----------------------------------------------------------------------" << std::endl;

  // for (int i = 0; i < fFVecMuons.size(); i++)
  //   std::cout << "Muon " << i << " Pt: " << fFVecMuons.at(i).fVec.Pt() << " Eta: " << fFVecMuons.at(i).fVec.Eta() << " Phi: " << fFVecMuons.at(i).fVec.Phi() << " Charge: " << fFVecMuons.at(i).fCharge << std::endl;
  
  // for (int i = 0; i < fFVecElecs.size(); i++) 
  //   std::cout << "Elec " << i << " Pt: " << fFVecElecs.at(i).fVec.Pt() << " Eta: " << fFVecElecs.at(i).fVec.Eta() << " Phi: " << fFVecElecs.at(i).fVec.Phi() << " Charge: " << fFVecElecs.at(i).fCharge << std::endl;
  
  // std::cout << "----------------------------------------------------------------------" << std::endl;
  // std::cout << "Selected Muon: " << fSelectedMuonIdx << " Selected Elec: " << fSelectedElecIdx << std::endl;

  // if (fSelectedMuonIdx == -1 || fSelectedElecIdx == -1) {
  //   std::cout << "No EMU pair found" << std::endl;
  //   return false;
  // } else {
  //   auto tMuon = fFVecMuons.at(fSelectedMuonIdx).fVec;
  //   auto tElec = fFVecElecs.at(fSelectedElecIdx).fVec;
  //   double tEMUMass = (tMuon + tElec).M();

  //   std::cout << "EMU Pair Mass: " << tEMUMass << std::endl;
  // }

  // std::cout << "######################################################################" << std::endl;

  if (fSelectedMuonIdx == -1 || fSelectedElecIdx == -1)
    return false;

  auto tMuon = fFVecMuons.at(fSelectedMuonIdx).fVec;
  auto tElec = fFVecElecs.at(fSelectedElecIdx).fVec;
  double tEMUMass = (tMuon + tElec).M();

  if (tEMUMass < fMassCut)
    return false;

  return true;
}
