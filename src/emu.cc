#include <iostream>
#include <vector>

#include "emu.h"

#include "TChain.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

void EMU::init(TTreeReader* fTreeReader) {

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
  Muon_mediumId = new TTreeReaderArray<bool>(*fTreeReader, "Muon_mediumId");

  nElectron = new TTreeReaderValue<unsigned int>(*fTreeReader, "nElectron");
  Electron_pt = new TTreeReaderArray<float>(*fTreeReader, "Electron_pt");
  Electron_eta = new TTreeReaderArray<float>(*fTreeReader, "Electron_eta");
  Electron_deltaEtaSC = new TTreeReaderArray<float>(*fTreeReader, "Electron_deltaEtaSC");
  Electron_phi = new TTreeReaderArray<float>(*fTreeReader, "Electron_phi");
  Electron_charge = new TTreeReaderArray<int>(*fTreeReader, "Electron_charge");
  Electron_mass = new TTreeReaderArray<float>(*fTreeReader, "Electron_mass");
  Electron_cutBased = new TTreeReaderArray<int>(*fTreeReader, "Electron_cutBased");
}

TLorentzVector EMU::GetMuonMCSmearing (TLorentzVector fMu) {

  if (std::abs(fMu.Eta()) <= 1.2 && fMuonSmearingEngine->DoBarrel()) { // barrel

    double fMomentum = fMu.P();
    double tSmearingFactor = 1 + fMuonRandom.Gaus(0, fMuonSmearingEngine->GetBarrelSmearingFactor() * fMuonSmearingEngine->GetBarrelSigma(fMomentum));

    TLorentzVector fMuReturn;
    fMuReturn.SetPtEtaPhiM(tSmearingFactor * fMu.Pt(), fMu.Eta(), fMu.Phi(), fMu.M());
    return fMuReturn;

  } else if (std::abs(fMu.Eta()) > 1.2 && std::abs(fMu.Eta()) < 2.4 && fMuonSmearingEngine->DoEndcap()) { // endcap
    
    double fMomentum = fMu.P();
    double tSmearingFactor = 1 + fMuonRandom.Gaus(0, fMuonSmearingEngine->GetEndcapSmearingFactor() * fMuonSmearingEngine->GetEndcapSigma(fMomentum));

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

  fFVecPair_OS.clear();
  fFVecPair_SS.clear();
  fFVecPair_OS_inverted.clear();
  fFVecPair_SS_inverted.clear();

  for (int i = 0; i < **nMuon; i++) {
    if ( !(Muon_highPtId->At(i) == fMuonID) )
      continue;

    bool tIso = false;
    if (Muon_tkRelIso->At(i) < fMuonISO)
      tIso = true;

    if (std::abs(Muon_eta->At(i)) > fMuonEta)
      continue;

    if (!Muon_highPurity->At(i))
      continue;

    TLorentzVector mu;
    mu.SetPtEtaPhiM(Muon_pt->At(i) * Muon_tunepRelPt->At(i), Muon_eta->At(i), Muon_phi->At(i), Muon_mass->At(i));

    TLorentzVector mu_corr;
    if (fIsMC && fDoMuonMCSmearing) mu_corr = GetMuonMCSmearing(mu);
    else mu_corr = mu;

    if ( !(mu_corr.Pt() > fMuonPt) )
      continue;

    EMU_MUON mu_std = EMU_MUON(mu_corr, mu, Muon_charge->At(i), tIso);
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

    bool tID = false;
    if (Electron_cutBased->At(i) >= fElecID)
      tID = true;

    TLorentzVector elecs;
    elecs.SetPtEtaPhiM(Electron_pt->At(i), Electron_eta->At(i), Electron_phi->At(i), Electron_mass->At(i));

    fFVecElecs.push_back(EMU_ELEC(elecs, eSCEta, Electron_charge->At(i), tID));
  }

  std::sort(fFVecElecs.begin(), fFVecElecs.end(), [](const EMU_ELEC &lhs, const EMU_ELEC &rhs) {
    return lhs.fVec.Pt() > rhs.fVec.Pt();
  });

  for (int i = 0; i < fFVecMuons.size(); i++) {
    for (int j = 0; j < fFVecElecs.size(); j++) {

      if (fFVecPair_OS.size() == 0 &&
        fFVecMuons.at(i).fCharge * fFVecElecs.at(j).fCharge < 0 &&
        fFVecMuons.at(i).fIso &&
        fFVecElecs.at(j).fID
      ) {
        fFVecPair_OS.push_back(std::make_pair(fFVecMuons.at(i), fFVecElecs.at(j)));
        continue;
      }

      if (fFVecPair_SS.size() == 0 &&
        fFVecMuons.at(i).fCharge * fFVecElecs.at(j).fCharge > 0 &&
        fFVecMuons.at(i).fIso &&
        fFVecElecs.at(j).fID
      ) {
        fFVecPair_SS.push_back(std::make_pair(fFVecMuons.at(i), fFVecElecs.at(j)));
        continue;
      }

      if (fFVecPair_OS_inverted.size() == 0 &&
        fFVecMuons.at(i).fCharge * fFVecElecs.at(j).fCharge < 0 &&
        !(fFVecMuons.at(i).fIso && fFVecElecs.at(j).fID)
      ) {
        fFVecPair_OS_inverted.push_back(std::make_pair(fFVecMuons.at(i), fFVecElecs.at(j)));
        continue;
      }

      if (fFVecPair_SS_inverted.size() == 0 &&
        fFVecMuons.at(i).fCharge * fFVecElecs.at(j).fCharge > 0 &&
        !(fFVecMuons.at(i).fIso && fFVecElecs.at(j).fID)
      ) {
        fFVecPair_SS_inverted.push_back(std::make_pair(fFVecMuons.at(i), fFVecElecs.at(j)));
        continue;
      }

    }
  }

  // if (fFVecPair_OS.size() == 1 || fFVecPair_SS.size() == 1 || fFVecPair_OS_inverted.size() == 1 || fFVecPair_SS_inverted.size() == 1) {
  //   std::cout << "######################################################################" << std::endl;
  //   std::cout << "                       Debug EMu Pair Selection                       " << std::endl;
  //   std::cout << "----------------------------------------------------------------------" << std::endl;
  //   if (fFVecPair_OS.size() == 1) {
  //     std::cout << "  OS Pair: " << std::endl;
  //     std::cout << "    Muon: " << fFVecPair_OS.at(0).first.fVec.Pt() << " " << fFVecPair_OS.at(0).first.fVec.Eta() << " " << fFVecPair_OS.at(0).first.fCharge << " " << fFVecPair_OS.at(0).first.fIso << std::endl;
  //     std::cout << "    Elec: " << fFVecPair_OS.at(0).second.fVec.Pt() << " " << fFVecPair_OS.at(0).second.fSCEta << " " << fFVecPair_OS.at(0).second.fCharge << " " << fFVecPair_OS.at(0).second.fID << std::endl;
  //   }
  //   if (fFVecPair_SS.size() == 1) {
  //     std::cout << "  SS Pair: " << std::endl;
  //     std::cout << "    Muon: " << fFVecPair_SS.at(0).first.fVec.Pt() << " " << fFVecPair_SS.at(0).first.fVec.Eta() << " " << fFVecPair_SS.at(0).first.fCharge << " " << fFVecPair_SS.at(0).first.fIso << std::endl;
  //     std::cout << "    Elec: " << fFVecPair_SS.at(0).second.fVec.Pt() << " " << fFVecPair_SS.at(0).second.fSCEta << " " << fFVecPair_SS.at(0).second.fCharge << " " << fFVecPair_SS.at(0).second.fID << std::endl;
  //   }
  //   if (fFVecPair_OS_inverted.size() == 1) {
  //     std::cout << "  OS Inverted Pair: " << std::endl;
  //     std::cout << "    Muon: " << fFVecPair_OS_inverted.at(0).first.fVec.Pt() << " " << fFVecPair_OS_inverted.at(0).first.fVec.Eta() << " " << fFVecPair_OS_inverted.at(0).first.fCharge << " " << fFVecPair_OS_inverted.at(0).first.fIso << std::endl;
  //     std::cout << "    Elec: " << fFVecPair_OS_inverted.at(0).second.fVec.Pt() << " " << fFVecPair_OS_inverted.at(0).second.fSCEta << " " << fFVecPair_OS_inverted.at(0).second.fCharge << " " << fFVecPair_OS_inverted.at(0).second.fID << std::endl;
  //   }
  //   if (fFVecPair_SS_inverted.size() == 1) {
  //     std::cout << "  SS Inverted Pair: " << std::endl;
  //     std::cout << "    Muon: " << fFVecPair_SS_inverted.at(0).first.fVec.Pt() << " " << fFVecPair_SS_inverted.at(0).first.fVec.Eta() << " " << fFVecPair_SS_inverted.at(0).first.fCharge << " " << fFVecPair_SS_inverted.at(0).first.fIso << std::endl;
  //     std::cout << "    Elec: " << fFVecPair_SS_inverted.at(0).second.fVec.Pt() << " " << fFVecPair_SS_inverted.at(0).second.fSCEta << " " << fFVecPair_SS_inverted.at(0).second.fCharge << " " << fFVecPair_SS_inverted.at(0).second.fID << std::endl;
  //   }
  //   std::cout << "######################################################################" << std::endl;
  // }

  return fFVecPair_OS.size() == 1 || fFVecPair_SS.size() == 1 || fFVecPair_OS_inverted.size() == 1 || fFVecPair_SS_inverted.size() == 1;
}
