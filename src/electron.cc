#include <iostream>
#include <vector>

#include "electron.h"

#include "TChain.h"
#include "TTreeReaderArray.h"
#include "TRandom.h"

void ELEC::init(TTreeReader* fTreeReader) {

  nElectron = new TTreeReaderValue<unsigned int>(*fTreeReader, "nElectron");
  Electron_pt = new TTreeReaderArray<float>(*fTreeReader, "Electron_pt");
  Electron_eta = new TTreeReaderArray<float>(*fTreeReader, "Electron_eta");
  Electron_deltaEtaSC = new TTreeReaderArray<float>(*fTreeReader, "Electron_deltaEtaSC");
  Electron_phi = new TTreeReaderArray<float>(*fTreeReader, "Electron_phi");
  Electron_mass = new TTreeReaderArray<float>(*fTreeReader, "Electron_mass");
  Electron_cutBased_HEEP = new TTreeReaderArray<bool>(*fTreeReader, "Electron_cutBased_HEEP");
  Electron_cutBased = new TTreeReaderArray<int>(*fTreeReader, "Electron_cutBased");
  Electron_charge = new TTreeReaderArray<int>(*fTreeReader, "Electron_charge");

  // For low-pT electron debugging
  // Electron_pfRelIso03_all = new TTreeReaderArray<float>(*fTreeReader, "Electron_pfRelIso03_all");
  // Electron_miniPFRelIso_all = new TTreeReaderArray<float>(*fTreeReader, "Electron_miniPFRelIso_all");
  // Electron_jetRelIso = new TTreeReaderArray<float>(*fTreeReader, "Electron_jetRelIso");
  // Electron_jetPtRelv2 = new TTreeReaderArray<float>(*fTreeReader, "Electron_jetPtRelv2");
  // Electron_jetIdx = new TTreeReaderArray<int>(*fTreeReader, "Electron_jetIdx");
  // Electron_convVeto = new TTreeReaderArray<bool>(*fTreeReader, "Electron_convVeto");
  // Electron_lostHits = new TTreeReaderArray<unsigned char>(*fTreeReader, "Electron_lostHits");
  // Electron_dxy = new TTreeReaderArray<float>(*fTreeReader, "Electron_dxy");
  // Electron_sip3d = new TTreeReaderArray<float>(*fTreeReader, "Electron_sip3d");
  // Electron_mvaFall17V2Iso = new TTreeReaderArray<float>(*fTreeReader, "Electron_mvaFall17V2Iso");
  // Electron_mvaFall17V2noIso = new TTreeReaderArray<float>(*fTreeReader, "Electron_mvaFall17V2noIso");
  // Electron_hoe = new TTreeReaderArray<float>(*fTreeReader, "Electron_hoe");
  // Electron_sieie = new TTreeReaderArray<float>(*fTreeReader, "Electron_sieie");
  // Electron_eInvMinusPInv = new TTreeReaderArray<float>(*fTreeReader, "Electron_eInvMinusPInv");
  // Electron_r9 = new TTreeReaderArray<float>(*fTreeReader, "Electron_r9");
  // Electron_vidNestedWPBitmap = new TTreeReaderArray<int>(*fTreeReader, "Electron_vidNestedWPBitmap");
  // Electron_dr03TkSumPt = new TTreeReaderArray<float>(*fTreeReader, "Electron_dr03TkSumPt");
  // Electron_dr03HcalDepth1TowerSumEt = new TTreeReaderArray<float>(*fTreeReader, "Electron_dr03HcalDepth1TowerSumEt");
  // Electron_dr03EcalRecHitSumEt = new TTreeReaderArray<float>(*fTreeReader, "Electron_dr03EcalRecHitSumEt");
  // Electron_dr03TkSumPtHEEP = new TTreeReaderArray<float>(*fTreeReader, "Electron_dr03TkSumPtHEEP");
}

bool ELEC::PrepareElec() {

  fFVecElecs.clear();

  for (int i = 0; i < (int)**nElectron; i++) {

    if (!(Electron_pt->At(i) > fSubLeadingPt))
      continue;

    float eSCEta = Electron_eta->At(i) + Electron_deltaEtaSC->At(i);
    float eEta = Electron_eta->At(i);

    if (!(std::abs(eSCEta) < fEta))
      continue;

    if (std::abs(eSCEta) > 1.4442 && std::abs(eSCEta) < 1.5660)
      continue;

    // if (!Electron_cutBased_HEEP->At(i)) continue;
    bool passID = (Electron_cutBased->At(i) >= fID);
    if (!fInvertedID && !passID) continue;

    TLorentzVector elecs;
    elecs.SetPtEtaPhiM(Electron_pt->At(i), eEta, Electron_phi->At(i), Electron_mass->At(i));

    fFVecElecs.push_back(StdElec(elecs, eSCEta, Electron_charge->At(i), passID));
  }

  std::sort(fFVecElecs.begin(), fFVecElecs.end(), [](const StdElec &lhs, const StdElec &rhs) {
    return lhs.fVec.Pt() > rhs.fVec.Pt();
  });

  if (fFVecElecs.size() < 2)
    return false;

  if (fFVecElecs.at(0).fVec.Pt() < fLeadingPt)
    return false;

  int tChargeSelection = 1;
  if (!fOppositeCharge) tChargeSelection = -1;

  // Force the two highest-pT electrons as the dielectron pair
  const StdElec &tLeadElec = fFVecElecs.at(0);
  const StdElec &tSubElec  = fFVecElecs.at(1);

  if (tChargeSelection * (tLeadElec.fCharge * tSubElec.fCharge) > 0)
    return false;

  if (fInvertedID && (tLeadElec.fPassID == tSubElec.fPassID))
    return false;  // exactly one fails ID

  if ((tLeadElec.fVec + tSubElec.fVec).M() < fZMassCut)
    return false;

  fLeadingIdx = 0;
  fSubLeadingIdx = 1;

  return true;
}
