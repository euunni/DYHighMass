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
}

bool ELEC::PrepareElec() {

  fFVecElecs.clear();

  for (int i = 0; i < **nElectron; i++) {

    if (!(Electron_pt->At(i) > fSubLeadingPt))
      continue;

    float eSCEta = Electron_eta->At(i) + Electron_deltaEtaSC->At(i);
    float eEta = Electron_eta->At(i);

    if (!(std::abs(eSCEta) < fEta))
      continue;

    if (std::abs(eSCEta) > 1.4442 && std::abs(eSCEta) < 1.5660)
      continue;

    if (!Electron_cutBased_HEEP->At(i)) continue;
    // if (Electron_cutBased->At(i) < fID) continue;

    TLorentzVector elecs;
    elecs.SetPtEtaPhiM(Electron_pt->At(i), eEta, Electron_phi->At(i), Electron_mass->At(i));

    fFVecElecs.push_back(StdElec(elecs, eSCEta, Electron_charge->At(i)));
  }

  std::sort(fFVecElecs.begin(), fFVecElecs.end(), [](const StdElec &lhs, const StdElec &rhs) {
    return lhs.fVec.Pt() > rhs.fVec.Pt();
  });

  if (fFVecElecs.size() < 2)
    return false;

  if (fFVecElecs.at(0).fVec.Pt() < fLeadingPt)
    return false;
  
  int tLeadingIdx = -1;
  int tSubLeadingIdx = -1;

  int tChargeSelection = 1;
  if (!fOppositeCharge) tChargeSelection = -1;

  for (int i = 0; i < fFVecElecs.size(); i++) {
    for (int j = i + 1; j < fFVecElecs.size(); j++) {
      if (tChargeSelection * (fFVecElecs.at(i).fCharge * fFVecElecs.at(j).fCharge) > 0)
        continue;

      if (fFVecElecs.at(i).fVec.Pt() < fLeadingPt && fFVecElecs.at(j).fVec.Pt() < fLeadingPt)
        continue;

      auto tDiElecVec = fFVecElecs.at(i).fVec + fFVecElecs.at(j).fVec;

      if (tDiElecVec.M() < fZMassCut)
        continue;

      tLeadingIdx = i;
      tSubLeadingIdx = j;

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

bool ELEC::PrepareElecNoMassCut() {

  fFVecElecs.clear();

  for (int i = 0; i < **nElectron; i++) {

    if (!(Electron_pt->At(i) > fSubLeadingPt))
      continue;

    float eSCEta = Electron_eta->At(i) + Electron_deltaEtaSC->At(i);
    float eEta = Electron_eta->At(i);

    if (!(std::abs(eSCEta) < fEta))
      continue;

    if (std::abs(eSCEta) > 1.4442 && std::abs(eSCEta) < 1.5660)
      continue;

    if (!Electron_cutBased_HEEP->At(i)) continue;
    // if (Electron_cutBased->At(i) < fID) continue;

    TLorentzVector elecs;
    elecs.SetPtEtaPhiM(Electron_pt->At(i), eEta, Electron_phi->At(i), Electron_mass->At(i));

    fFVecElecs.push_back(StdElec(elecs, eSCEta, Electron_charge->At(i)));
  }

  std::sort(fFVecElecs.begin(), fFVecElecs.end(), [](const StdElec &lhs, const StdElec &rhs) {
    return lhs.fVec.Pt() > rhs.fVec.Pt();
  });

  if (fFVecElecs.size() < 2)
    return false;

  if (fFVecElecs.at(0).fVec.Pt() < fLeadingPt)
    return false;
  
  int tLeadingIdx = -1;
  int tSubLeadingIdx = -1;

  int tChargeSelection = 1;
  if (!fOppositeCharge) tChargeSelection = -1;

  for (int i = 0; i < fFVecElecs.size(); i++) {
    for (int j = i + 1; j < fFVecElecs.size(); j++) {
      if (tChargeSelection * (fFVecElecs.at(i).fCharge * fFVecElecs.at(j).fCharge) > 0)
        continue;

      if (fFVecElecs.at(i).fVec.Pt() < fLeadingPt && fFVecElecs.at(j).fVec.Pt() < fLeadingPt)
        continue;

      tLeadingIdx = i;
      tSubLeadingIdx = j;

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
