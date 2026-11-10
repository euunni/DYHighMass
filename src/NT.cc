#include "NT.h"
#include "muon.h"

#include <exception>

bool NT::PassinNoiseFilter() {

  // check https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2#UL_data

  if (fEra == "2016_preVFP" || fEra == "2016_postVFP") {
    
    return **Flag_goodVertices &&
    **Flag_globalSuperTightHalo2016Filter &&
    **Flag_HBHENoiseFilter &&
    **Flag_HBHENoiseIsoFilter &&
    **Flag_EcalDeadCellTriggerPrimitiveFilter &&
    **Flag_BadPFMuonFilter &&
    **Flag_BadPFMuonDzFilter &&
    **Flag_hfNoisyHitsFilter &&
    **Flag_eeBadScFilter &&
    **Flag_ecalBadCalibFilter;
  } else if (fEra == "2017" || fEra == "2018") {

    return **Flag_goodVertices &&
    **Flag_globalSuperTightHalo2016Filter &&
    **Flag_HBHENoiseFilter &&
    **Flag_HBHENoiseIsoFilter &&  
    **Flag_EcalDeadCellTriggerPrimitiveFilter &&
    **Flag_BadPFMuonFilter &&
    **Flag_BadPFMuonDzFilter &&
    **Flag_eeBadScFilter &&
    **Flag_hfNoisyHitsFilter;
  }

  return false;
}

std::vector<std::pair<int, TLorentzVector>> NT::GetGenPart(int tID, int tStatus) {
  std::vector<std::pair<int, TLorentzVector>> returnVec = {};

  for (int i = 0; i < **nGenPart; i++) {
    if (std::abs(std::abs(GenPart_pdgId->At(i))) == tID && GenPart_status->At(i) == tStatus) {
      TLorentzVector tTmpVec;
      tTmpVec.SetPtEtaPhiM(GenPart_pt->At(i), GenPart_eta->At(i), GenPart_phi->At(i), GenPart_mass->At(i));
      int tCharge = GenPart_pdgId->At(i) > 0 ? -1 : 1;
      returnVec.push_back(std::make_pair(tCharge, tTmpVec));
    }
  }

  return returnVec;
}

std::vector<std::pair<int, TLorentzVector>> NT::GetGenDressedLepton(int tID) {
  std::vector<std::pair<int, TLorentzVector>> returnVec = {};

  for (int i = 0; i < **nGenDressedLepton; i++) {
    if (std::abs(std::abs(GenDressedLepton_pdgId->At(i))) != tID)
      continue;

    // if (GenDressedLepton_hasTauAnc->At(i))
    //   continue;

    TLorentzVector tTmpVec;
    tTmpVec.SetPtEtaPhiM(GenDressedLepton_pt->At(i), GenDressedLepton_eta->At(i), GenDressedLepton_phi->At(i), GenDressedLepton_mass->At(i));
    int tCharge = GenDressedLepton_pdgId->At(i) > 0 ? -1 : 1;
    returnVec.push_back(std::make_pair(tCharge, tTmpVec));
  }

  return returnVec;
}

double NT::GetGenTopPtReweightFactor() {

  std::vector<TLorentzVector> tGenTopVec = {};
  double tGenTopPtReweightFactor = 1.;

  for (int i = 0; i < **nGenPart; i++) {
    if (std::abs(GenPart_pdgId->At(i)) == 6 && GenPart_status->At(i) == 22 && GenPart_genPartIdxMother->At(i) == 0) {
      TLorentzVector tTmpVec;
      tTmpVec.SetPtEtaPhiM(GenPart_pt->At(i), GenPart_eta->At(i), GenPart_phi->At(i), GenPart_mass->At(i));
      tGenTopVec.push_back(tTmpVec);
    }
  }

  if (tGenTopVec.size() != 2) {
    
    std::cout << "########################################################" << std::endl;
    std::cout << "  ERROR! Number of gen tops is not 2!" << std::endl;
    std::cout << "########################################################" << std::endl;

    for (int i = 0; i < **nGenPart; i++) {
    
    std::cout << "GetPart " << i << "/" << **nGenPart 
              << " pdgId: " << GenPart_pdgId->At(i)
              << " status: " << GenPart_status->At(i)
              << " statusFlags: " << GenPart_statusFlags->At(i)
              << " mother: " << GenPart_genPartIdxMother->At(i)
              << " pt: " << GenPart_pt->At(i)
              << " eta: " << GenPart_eta->At(i)
              << " phi: " << GenPart_phi->At(i)
              << " mass: " << GenPart_mass->At(i)
              << std::endl;
    
    }

    throw std::runtime_error("NT::GetGenTopPtReweightFactor: Number of gen tops is not 2!");
  }

  for (int i = 0; i < tGenTopVec.size(); i++) {
    if (tGenTopVec.at(i).Pt() >500) tGenTopPtReweightFactor *= fTopPtReweighter->Eval(500);
    else tGenTopPtReweightFactor *= fTopPtReweighter->Eval(tGenTopVec.at(i).Pt());
  }

  return std::sqrt(tGenTopPtReweightFactor);
}

std::vector<TLorentzVector> NT::GetLHE(int fPID) {

  std::vector<TLorentzVector> returnVec = {};

  for (int i = 0; i < **nLHEPart; i++) {

    if (std::fabs(LHEPart_pdgId->At(i)) == fPID) {

      TLorentzVector lvec;
      lvec.SetPtEtaPhiM(LHEPart_pt->At(i), LHEPart_eta->At(i), LHEPart_phi->At(i), LHEPart_mass->At(i));
      returnVec.push_back(lvec);
    }
  }

  return returnVec;
}

const std::vector<TLorentzVector> NT::GetGenPartWithFlag(int tID, int tStatus) const {
  std::vector<TLorentzVector> tReturnVec = {};

  float tMass = 0.;
  if (tID == 13) tMass = 0.1056583755; // Muon mass in PDG
  if (tID == 11) tMass = 0.00051099895; // Electron mass in PDG

  for (int i = 0; i < **nGenPart; i++) {
    if (std::abs(std::abs(GenPart_pdgId->At(i))) == tID && GenPart_status->At(i) == tStatus) {

      int tStatusFlags = GenPart_statusFlags->At(i);

      bool tIsPrompt = (tStatusFlags & 1) == 1;
      bool tFromHardProcess = (tStatusFlags & (1 << 8)) == 256;

      if (!(tIsPrompt && tFromHardProcess))
        continue;

      TLorentzVector tTmpVec;
      tTmpVec.SetPtEtaPhiM(GenPart_pt->At(i), GenPart_eta->At(i), GenPart_phi->At(i), tMass);
      tReturnVec.push_back(tTmpVec);
    }
  }

  return tReturnVec;
}


const std::vector<TLorentzVector> NT::GetGenJet(const float& fJetPt, const std::vector<TLorentzVector>& tGenLep) const {
  std::vector<TLorentzVector> returnVec = {};

  for (int i = 0; i < **nGenJet; i++) {

    if (std::abs(GenJet_eta->At(i)) > 2.5)
      continue;

    if (GenJet_pt->At(i) < fJetPt)
      continue;

    TLorentzVector tTmpVec;
    tTmpVec.SetPtEtaPhiM(GenJet_pt->At(i), GenJet_eta->At(i), GenJet_phi->At(i), GenJet_mass->At(i));

    bool tIsOverlap = false;
    for (const auto& lep: tGenLep) {
      if (tTmpVec.DeltaR(lep) < 0.4) {
        tIsOverlap = true;
        break;
      }
    }

    if (tIsOverlap)
      continue;

    returnVec.push_back(tTmpVec);
  }

  return returnVec;
}

void NT::init_LHE() {

  LHE_HT = new TTreeReaderValue<float>(*fTreeReader, "LHE_HT");
  LHE_HTIncoming = new TTreeReaderValue<float>(*fTreeReader, "LHE_HTIncoming");
  LHE_Vpt = new TTreeReaderValue<float>(*fTreeReader, "LHE_Vpt");
  LHE_AlphaS = new TTreeReaderValue<float>(*fTreeReader, "LHE_AlphaS");
  LHE_Njets = new TTreeReaderValue<unsigned char>(*fTreeReader, "LHE_Njets");
  LHE_Nb = new TTreeReaderValue<unsigned char>(*fTreeReader, "LHE_Nb");
  LHE_Nc = new TTreeReaderValue<unsigned char>(*fTreeReader, "LHE_Nc");
  LHE_Nuds = new TTreeReaderValue<unsigned char>(*fTreeReader, "LHE_Nuds");
  LHE_Nglu = new TTreeReaderValue<unsigned char>(*fTreeReader, "LHE_Nglu");
  LHE_NpNLO = new TTreeReaderValue<unsigned char>(*fTreeReader, "LHE_NpNLO");
  LHE_NpLO = new TTreeReaderValue<unsigned char>(*fTreeReader, "LHE_NpLO");
  nLHEPart = new TTreeReaderValue<unsigned int>(*fTreeReader, "nLHEPart");
  LHEPart_pt = new TTreeReaderArray<float>(*fTreeReader, "LHEPart_pt");
  LHEPart_eta = new TTreeReaderArray<float>(*fTreeReader, "LHEPart_eta");
  LHEPart_phi = new TTreeReaderArray<float>(*fTreeReader, "LHEPart_phi");
  LHEPart_mass = new TTreeReaderArray<float>(*fTreeReader, "LHEPart_mass");
  LHEPart_incomingpz = new TTreeReaderArray<float>(*fTreeReader, "LHEPart_incomingpz");
  LHEPart_pdgId = new TTreeReaderArray<int>(*fTreeReader, "LHEPart_pdgId");
  LHEPart_status = new TTreeReaderArray<int>(*fTreeReader, "LHEPart_status");
  LHEPart_spin = new TTreeReaderArray<int>(*fTreeReader, "LHEPart_spin");
  LHEWeight_originalXWGTUP = new TTreeReaderValue<float>(*fTreeReader, "LHEWeight_originalXWGTUP");
  nLHEPdfWeight = new TTreeReaderValue<unsigned int>(*fTreeReader, "nLHEPdfWeight");
  LHEPdfWeight = new TTreeReaderArray<float>(*fTreeReader, "LHEPdfWeight");
  nLHEReweightingWeight = new TTreeReaderValue<unsigned int>(*fTreeReader, "nLHEReweightingWeight");
  LHEReweightingWeight = new TTreeReaderArray<float>(*fTreeReader, "LHEReweightingWeight");
  nLHEScaleWeight = new TTreeReaderValue<unsigned int>(*fTreeReader, "nLHEScaleWeight");
  LHEScaleWeight = new TTreeReaderArray<float>(*fTreeReader, "LHEScaleWeight");

}

void NT::init_MC() {
  nGenJetAK8 = new TTreeReaderValue<unsigned int>(*fTreeReader, "nGenJetAK8");
  GenJetAK8_eta = new TTreeReaderArray<float>(*fTreeReader, "GenJetAK8_eta");
  GenJetAK8_mass = new TTreeReaderArray<float>(*fTreeReader, "GenJetAK8_mass");
  GenJetAK8_phi = new TTreeReaderArray<float>(*fTreeReader, "GenJetAK8_phi");
  GenJetAK8_pt = new TTreeReaderArray<float>(*fTreeReader, "GenJetAK8_pt");
  nGenJet = new TTreeReaderValue<unsigned int>(*fTreeReader, "nGenJet");
  GenJet_eta = new TTreeReaderArray<float>(*fTreeReader, "GenJet_eta");
  GenJet_mass = new TTreeReaderArray<float>(*fTreeReader, "GenJet_mass");
  GenJet_phi = new TTreeReaderArray<float>(*fTreeReader, "GenJet_phi");
  GenJet_pt = new TTreeReaderArray<float>(*fTreeReader, "GenJet_pt");
  nGenPart = new TTreeReaderValue<unsigned int>(*fTreeReader, "nGenPart");
  GenPart_eta = new TTreeReaderArray<float>(*fTreeReader, "GenPart_eta");
  GenPart_mass = new TTreeReaderArray<float>(*fTreeReader, "GenPart_mass");
  GenPart_phi = new TTreeReaderArray<float>(*fTreeReader, "GenPart_phi");
  GenPart_pt = new TTreeReaderArray<float>(*fTreeReader, "GenPart_pt");
  GenPart_genPartIdxMother = new TTreeReaderArray<int>(*fTreeReader, "GenPart_genPartIdxMother");
  GenPart_pdgId = new TTreeReaderArray<int>(*fTreeReader, "GenPart_pdgId");
  GenPart_status = new TTreeReaderArray<int>(*fTreeReader, "GenPart_status");
  GenPart_statusFlags = new TTreeReaderArray<int>(*fTreeReader, "GenPart_statusFlags");
  Generator_binvar = new TTreeReaderValue<float>(*fTreeReader, "Generator_binvar");
  Generator_scalePDF = new TTreeReaderValue<float>(*fTreeReader, "Generator_scalePDF");
  Generator_weight = new TTreeReaderValue<float>(*fTreeReader, "Generator_weight");
  Generator_x1 = new TTreeReaderValue<float>(*fTreeReader, "Generator_x1");
  Generator_x2 = new TTreeReaderValue<float>(*fTreeReader, "Generator_x2");
  Generator_xpdf1 = new TTreeReaderValue<float>(*fTreeReader, "Generator_xpdf1");
  Generator_xpdf2 = new TTreeReaderValue<float>(*fTreeReader, "Generator_xpdf2");
  Generator_id1 = new TTreeReaderValue<int>(*fTreeReader, "Generator_id1");
  Generator_id2 = new TTreeReaderValue<int>(*fTreeReader, "Generator_id2");
  GenVtx_x = new TTreeReaderValue<float>(*fTreeReader, "GenVtx_x");
  GenVtx_y = new TTreeReaderValue<float>(*fTreeReader, "GenVtx_y");
  GenVtx_z = new TTreeReaderValue<float>(*fTreeReader, "GenVtx_z");
  genWeight = new TTreeReaderValue<float>(*fTreeReader, "genWeight");
  GenMET_phi = new TTreeReaderValue<float>(*fTreeReader, "GenMET_phi");
  GenMET_pt = new TTreeReaderValue<float>(*fTreeReader, "GenMET_pt");
  nGenDressedLepton = new TTreeReaderValue<unsigned int>(*fTreeReader, "nGenDressedLepton");
  GenDressedLepton_eta = new TTreeReaderArray<float>(*fTreeReader, "GenDressedLepton_eta");
  GenDressedLepton_mass = new TTreeReaderArray<float>(*fTreeReader, "GenDressedLepton_mass");
  GenDressedLepton_phi = new TTreeReaderArray<float>(*fTreeReader, "GenDressedLepton_phi");
  GenDressedLepton_pt = new TTreeReaderArray<float>(*fTreeReader, "GenDressedLepton_pt");
  GenDressedLepton_pdgId = new TTreeReaderArray<int>(*fTreeReader, "GenDressedLepton_pdgId");
  GenDressedLepton_hasTauAnc = new TTreeReaderArray<bool>(*fTreeReader, "GenDressedLepton_hasTauAnc");
  genTtbarId = new TTreeReaderValue<int>(*fTreeReader, "genTtbarId");
  nPSWeight = new TTreeReaderValue<unsigned int>(*fTreeReader, "nPSWeight");
  PSWeight = new TTreeReaderArray<float>(*fTreeReader, "PSWeight");
  Pileup_nTrueInt = new TTreeReaderValue<float>(*fTreeReader, "Pileup_nTrueInt");
  Pileup_pudensity = new TTreeReaderValue<float>(*fTreeReader, "Pileup_pudensity");
  Pileup_gpudensity = new TTreeReaderValue<float>(*fTreeReader, "Pileup_gpudensity");
  Pileup_nPU = new TTreeReaderValue<int>(*fTreeReader, "Pileup_nPU");
  Pileup_sumEOOT = new TTreeReaderValue<int>(*fTreeReader, "Pileup_sumEOOT");
  Pileup_sumLOOT = new TTreeReaderValue<int>(*fTreeReader, "Pileup_sumLOOT");
}


void NT::init() {

  fTreeReader = new TTreeReader(fChain);

  run = new TTreeReaderValue<unsigned int>(*fTreeReader, "run");
  luminosityBlock = new TTreeReaderValue<unsigned int>(*fTreeReader, "luminosityBlock");
  event = new TTreeReaderValue<unsigned long long>(*fTreeReader, "event");
  // btagWeight_CSVV2 = new TTreeReaderValue<float>(*fTreeReader, "btagWeight_CSVV2");
  // btagWeight_DeepCSVB = new TTreeReaderValue<float>(*fTreeReader, "btagWeight_DeepCSVB");
  // CaloMET_phi = new TTreeReaderValue<float>(*fTreeReader, "CaloMET_phi");
  // CaloMET_pt = new TTreeReaderValue<float>(*fTreeReader, "CaloMET_pt");
  // CaloMET_sumEt = new TTreeReaderValue<float>(*fTreeReader, "CaloMET_sumEt");
  // ChsMET_phi = new TTreeReaderValue<float>(*fTreeReader, "ChsMET_phi");
  // ChsMET_pt = new TTreeReaderValue<float>(*fTreeReader, "ChsMET_pt");
  // ChsMET_sumEt = new TTreeReaderValue<float>(*fTreeReader, "ChsMET_sumEt");
  // nCorrT1METJet = new TTreeReaderValue<unsigned int>(*fTreeReader, "nCorrT1METJet");
  // CorrT1METJet_area = new TTreeReaderArray<float>(*fTreeReader, "CorrT1METJet_area");
  // CorrT1METJet_eta = new TTreeReaderArray<float>(*fTreeReader, "CorrT1METJet_eta");
  // CorrT1METJet_muonSubtrFactor = new TTreeReaderArray<float>(*fTreeReader, "CorrT1METJet_muonSubtrFactor");
  // CorrT1METJet_phi = new TTreeReaderArray<float>(*fTreeReader, "CorrT1METJet_phi");
  // CorrT1METJet_rawPt = new TTreeReaderArray<float>(*fTreeReader, "CorrT1METJet_rawPt");
  // DeepMETResolutionTune_phi = new TTreeReaderValue<float>(*fTreeReader, "DeepMETResolutionTune_phi");
  // DeepMETResolutionTune_pt = new TTreeReaderValue<float>(*fTreeReader, "DeepMETResolutionTune_pt");
  // DeepMETResponseTune_phi = new TTreeReaderValue<float>(*fTreeReader, "DeepMETResponseTune_phi");
  // DeepMETResponseTune_pt = new TTreeReaderValue<float>(*fTreeReader, "DeepMETResponseTune_pt");
  nElectron = new TTreeReaderValue<unsigned int>(*fTreeReader, "nElectron");
  Electron_dEscaleDown = new TTreeReaderArray<float>(*fTreeReader, "Electron_dEscaleDown");
  Electron_dEscaleUp = new TTreeReaderArray<float>(*fTreeReader, "Electron_dEscaleUp");
  Electron_dEsigmaDown = new TTreeReaderArray<float>(*fTreeReader, "Electron_dEsigmaDown");
  Electron_dEsigmaUp = new TTreeReaderArray<float>(*fTreeReader, "Electron_dEsigmaUp");
  Electron_deltaEtaSC = new TTreeReaderArray<float>(*fTreeReader, "Electron_deltaEtaSC");
  Electron_dr03EcalRecHitSumEt = new TTreeReaderArray<float>(*fTreeReader, "Electron_dr03EcalRecHitSumEt");
  Electron_dr03HcalDepth1TowerSumEt = new TTreeReaderArray<float>(*fTreeReader, "Electron_dr03HcalDepth1TowerSumEt");
  Electron_dr03TkSumPt = new TTreeReaderArray<float>(*fTreeReader, "Electron_dr03TkSumPt");
  Electron_dr03TkSumPtHEEP = new TTreeReaderArray<float>(*fTreeReader, "Electron_dr03TkSumPtHEEP");
  Electron_dxy = new TTreeReaderArray<float>(*fTreeReader, "Electron_dxy");
  Electron_dxyErr = new TTreeReaderArray<float>(*fTreeReader, "Electron_dxyErr");
  Electron_dz = new TTreeReaderArray<float>(*fTreeReader, "Electron_dz");
  Electron_dzErr = new TTreeReaderArray<float>(*fTreeReader, "Electron_dzErr");
  Electron_eCorr = new TTreeReaderArray<float>(*fTreeReader, "Electron_eCorr");
  Electron_eInvMinusPInv = new TTreeReaderArray<float>(*fTreeReader, "Electron_eInvMinusPInv");
  Electron_energyErr = new TTreeReaderArray<float>(*fTreeReader, "Electron_energyErr");
  Electron_eta = new TTreeReaderArray<float>(*fTreeReader, "Electron_eta");
  Electron_hoe = new TTreeReaderArray<float>(*fTreeReader, "Electron_hoe");
  Electron_ip3d = new TTreeReaderArray<float>(*fTreeReader, "Electron_ip3d");
  Electron_jetPtRelv2 = new TTreeReaderArray<float>(*fTreeReader, "Electron_jetPtRelv2");
  Electron_jetRelIso = new TTreeReaderArray<float>(*fTreeReader, "Electron_jetRelIso");
  Electron_mass = new TTreeReaderArray<float>(*fTreeReader, "Electron_mass");
  Electron_miniPFRelIso_all = new TTreeReaderArray<float>(*fTreeReader, "Electron_miniPFRelIso_all");
  Electron_miniPFRelIso_chg = new TTreeReaderArray<float>(*fTreeReader, "Electron_miniPFRelIso_chg");
  Electron_mvaFall17V2Iso = new TTreeReaderArray<float>(*fTreeReader, "Electron_mvaFall17V2Iso");
  Electron_mvaFall17V2noIso = new TTreeReaderArray<float>(*fTreeReader, "Electron_mvaFall17V2noIso");
  Electron_pfRelIso03_all = new TTreeReaderArray<float>(*fTreeReader, "Electron_pfRelIso03_all");
  Electron_pfRelIso03_chg = new TTreeReaderArray<float>(*fTreeReader, "Electron_pfRelIso03_chg");
  Electron_phi = new TTreeReaderArray<float>(*fTreeReader, "Electron_phi");
  Electron_pt = new TTreeReaderArray<float>(*fTreeReader, "Electron_pt");
  Electron_r9 = new TTreeReaderArray<float>(*fTreeReader, "Electron_r9");
  Electron_scEtOverPt = new TTreeReaderArray<float>(*fTreeReader, "Electron_scEtOverPt");
  Electron_sieie = new TTreeReaderArray<float>(*fTreeReader, "Electron_sieie");
  Electron_sip3d = new TTreeReaderArray<float>(*fTreeReader, "Electron_sip3d");
  Electron_mvaTTH = new TTreeReaderArray<float>(*fTreeReader, "Electron_mvaTTH");
  Electron_charge = new TTreeReaderArray<int>(*fTreeReader, "Electron_charge");
  Electron_cutBased = new TTreeReaderArray<int>(*fTreeReader, "Electron_cutBased");
  Electron_jetIdx = new TTreeReaderArray<int>(*fTreeReader, "Electron_jetIdx");
  Electron_pdgId = new TTreeReaderArray<int>(*fTreeReader, "Electron_pdgId");
  Electron_photonIdx = new TTreeReaderArray<int>(*fTreeReader, "Electron_photonIdx");
  Electron_tightCharge = new TTreeReaderArray<int>(*fTreeReader, "Electron_tightCharge");
  Electron_vidNestedWPBitmap = new TTreeReaderArray<int>(*fTreeReader, "Electron_vidNestedWPBitmap");
  Electron_vidNestedWPBitmapHEEP = new TTreeReaderArray<int>(*fTreeReader, "Electron_vidNestedWPBitmapHEEP");
  Electron_convVeto = new TTreeReaderArray<bool>(*fTreeReader, "Electron_convVeto");
  Electron_cutBased_HEEP = new TTreeReaderArray<bool>(*fTreeReader, "Electron_cutBased_HEEP");
  Electron_isPFcand = new TTreeReaderArray<bool>(*fTreeReader, "Electron_isPFcand");
  Electron_jetNDauCharged = new TTreeReaderArray<unsigned char>(*fTreeReader, "Electron_jetNDauCharged");
  Electron_lostHits = new TTreeReaderArray<unsigned char>(*fTreeReader, "Electron_lostHits");
  Electron_mvaFall17V2Iso_WP80 = new TTreeReaderArray<bool>(*fTreeReader, "Electron_mvaFall17V2Iso_WP80");
  Electron_mvaFall17V2Iso_WP90 = new TTreeReaderArray<bool>(*fTreeReader, "Electron_mvaFall17V2Iso_WP90");
  Electron_mvaFall17V2Iso_WPL = new TTreeReaderArray<bool>(*fTreeReader, "Electron_mvaFall17V2Iso_WPL");
  Electron_mvaFall17V2noIso_WP80 = new TTreeReaderArray<bool>(*fTreeReader, "Electron_mvaFall17V2noIso_WP80");
  Electron_mvaFall17V2noIso_WP90 = new TTreeReaderArray<bool>(*fTreeReader, "Electron_mvaFall17V2noIso_WP90");
  Electron_mvaFall17V2noIso_WPL = new TTreeReaderArray<bool>(*fTreeReader, "Electron_mvaFall17V2noIso_WPL");
  Electron_seedGain = new TTreeReaderArray<unsigned char>(*fTreeReader, "Electron_seedGain");
  // nFatJet = new TTreeReaderValue<unsigned int>(*fTreeReader, "nFatJet");
  // FatJet_area = new TTreeReaderArray<float>(*fTreeReader, "FatJet_area");
  // FatJet_btagCSVV2 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_btagCSVV2");
  // FatJet_btagDDBvLV2 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_btagDDBvLV2");
  // FatJet_btagDDCvBV2 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_btagDDCvBV2");
  // FatJet_btagDDCvLV2 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_btagDDCvLV2");
  // FatJet_btagDeepB = new TTreeReaderArray<float>(*fTreeReader, "FatJet_btagDeepB");
  // FatJet_btagHbb = new TTreeReaderArray<float>(*fTreeReader, "FatJet_btagHbb");
  // FatJet_deepTagMD_H4qvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTagMD_H4qvsQCD");
  // FatJet_deepTagMD_HbbvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTagMD_HbbvsQCD");
  // FatJet_deepTagMD_TvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTagMD_TvsQCD");
  // FatJet_deepTagMD_WvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTagMD_WvsQCD");
  // FatJet_deepTagMD_ZHbbvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTagMD_ZHbbvsQCD");
  // FatJet_deepTagMD_ZHccvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTagMD_ZHccvsQCD");
  // FatJet_deepTagMD_ZbbvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTagMD_ZbbvsQCD");
  // FatJet_deepTagMD_ZvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTagMD_ZvsQCD");
  // FatJet_deepTagMD_bbvsLight = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTagMD_bbvsLight");
  // FatJet_deepTagMD_ccvsLight = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTagMD_ccvsLight");
  // FatJet_deepTag_H = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTag_H");
  // FatJet_deepTag_QCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTag_QCD");
  // FatJet_deepTag_QCDothers = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTag_QCDothers");
  // FatJet_deepTag_TvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTag_TvsQCD");
  // FatJet_deepTag_WvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTag_WvsQCD");
  // FatJet_deepTag_ZvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_deepTag_ZvsQCD");
  // FatJet_eta = new TTreeReaderArray<float>(*fTreeReader, "FatJet_eta");
  // FatJet_mass = new TTreeReaderArray<float>(*fTreeReader, "FatJet_mass");
  // FatJet_msoftdrop = new TTreeReaderArray<float>(*fTreeReader, "FatJet_msoftdrop");
  // FatJet_n2b1 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_n2b1");
  // FatJet_n3b1 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_n3b1");
  // FatJet_particleNetMD_QCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNetMD_QCD");
  // FatJet_particleNetMD_Xbb = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNetMD_Xbb");
  // FatJet_particleNetMD_Xcc = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNetMD_Xcc");
  // FatJet_particleNetMD_Xqq = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNetMD_Xqq");
  // FatJet_particleNet_H4qvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNet_H4qvsQCD");
  // FatJet_particleNet_HbbvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNet_HbbvsQCD");
  // FatJet_particleNet_HccvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNet_HccvsQCD");
  // FatJet_particleNet_QCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNet_QCD");
  // FatJet_particleNet_TvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNet_TvsQCD");
  // FatJet_particleNet_WvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNet_WvsQCD");
  // FatJet_particleNet_ZvsQCD = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNet_ZvsQCD");
  // FatJet_particleNet_mass = new TTreeReaderArray<float>(*fTreeReader, "FatJet_particleNet_mass");
  // FatJet_phi = new TTreeReaderArray<float>(*fTreeReader, "FatJet_phi");
  // FatJet_pt = new TTreeReaderArray<float>(*fTreeReader, "FatJet_pt");
  // FatJet_rawFactor = new TTreeReaderArray<float>(*fTreeReader, "FatJet_rawFactor");
  // FatJet_tau1 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_tau1");
  // FatJet_tau2 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_tau2");
  // FatJet_tau3 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_tau3");
  // FatJet_tau4 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_tau4");
  // FatJet_lsf3 = new TTreeReaderArray<float>(*fTreeReader, "FatJet_lsf3");
  // FatJet_jetId = new TTreeReaderArray<int>(*fTreeReader, "FatJet_jetId");
  // FatJet_subJetIdx1 = new TTreeReaderArray<int>(*fTreeReader, "FatJet_subJetIdx1");
  // FatJet_subJetIdx2 = new TTreeReaderArray<int>(*fTreeReader, "FatJet_subJetIdx2");
  // FatJet_electronIdx3SJ = new TTreeReaderArray<int>(*fTreeReader, "FatJet_electronIdx3SJ");
  // FatJet_muonIdx3SJ = new TTreeReaderArray<int>(*fTreeReader, "FatJet_muonIdx3SJ");
  // FatJet_nConstituents = new TTreeReaderArray<unsigned char>(*fTreeReader, "FatJet_nConstituents");
  // nFsrPhoton = new TTreeReaderValue<unsigned int>(*fTreeReader, "nFsrPhoton");
  // FsrPhoton_dROverEt2 = new TTreeReaderArray<float>(*fTreeReader, "FsrPhoton_dROverEt2");
  // FsrPhoton_eta = new TTreeReaderArray<float>(*fTreeReader, "FsrPhoton_eta");
  // FsrPhoton_phi = new TTreeReaderArray<float>(*fTreeReader, "FsrPhoton_phi");
  // FsrPhoton_pt = new TTreeReaderArray<float>(*fTreeReader, "FsrPhoton_pt");
  // FsrPhoton_relIso03 = new TTreeReaderArray<float>(*fTreeReader, "FsrPhoton_relIso03");
  // FsrPhoton_muonIdx = new TTreeReaderArray<int>(*fTreeReader, "FsrPhoton_muonIdx");
  // nJet = new TTreeReaderValue<unsigned int>(*fTreeReader, "nJet");
  // Jet_area = new TTreeReaderArray<float>(*fTreeReader, "Jet_area");
  // Jet_btagCSVV2 = new TTreeReaderArray<float>(*fTreeReader, "Jet_btagCSVV2");
  // Jet_btagDeepB = new TTreeReaderArray<float>(*fTreeReader, "Jet_btagDeepB");
  // Jet_btagDeepCvB = new TTreeReaderArray<float>(*fTreeReader, "Jet_btagDeepCvB");
  // Jet_btagDeepCvL = new TTreeReaderArray<float>(*fTreeReader, "Jet_btagDeepCvL");
  // Jet_btagDeepFlavB = new TTreeReaderArray<float>(*fTreeReader, "Jet_btagDeepFlavB");
  // Jet_btagDeepFlavCvB = new TTreeReaderArray<float>(*fTreeReader, "Jet_btagDeepFlavCvB");
  // Jet_btagDeepFlavCvL = new TTreeReaderArray<float>(*fTreeReader, "Jet_btagDeepFlavCvL");
  // Jet_btagDeepFlavQG = new TTreeReaderArray<float>(*fTreeReader, "Jet_btagDeepFlavQG");
  // Jet_chEmEF = new TTreeReaderArray<float>(*fTreeReader, "Jet_chEmEF");
  // Jet_chFPV0EF = new TTreeReaderArray<float>(*fTreeReader, "Jet_chFPV0EF");
  // Jet_chHEF = new TTreeReaderArray<float>(*fTreeReader, "Jet_chHEF");
  // Jet_eta = new TTreeReaderArray<float>(*fTreeReader, "Jet_eta");
  // Jet_hfsigmaEtaEta = new TTreeReaderArray<float>(*fTreeReader, "Jet_hfsigmaEtaEta");
  // Jet_hfsigmaPhiPhi = new TTreeReaderArray<float>(*fTreeReader, "Jet_hfsigmaPhiPhi");
  // Jet_mass = new TTreeReaderArray<float>(*fTreeReader, "Jet_mass");
  // Jet_muEF = new TTreeReaderArray<float>(*fTreeReader, "Jet_muEF");
  // Jet_muonSubtrFactor = new TTreeReaderArray<float>(*fTreeReader, "Jet_muonSubtrFactor");
  // Jet_neEmEF = new TTreeReaderArray<float>(*fTreeReader, "Jet_neEmEF");
  // Jet_neHEF = new TTreeReaderArray<float>(*fTreeReader, "Jet_neHEF");
  // Jet_phi = new TTreeReaderArray<float>(*fTreeReader, "Jet_phi");
  // Jet_pt = new TTreeReaderArray<float>(*fTreeReader, "Jet_pt");
  // Jet_puIdDisc = new TTreeReaderArray<float>(*fTreeReader, "Jet_puIdDisc");
  // Jet_qgl = new TTreeReaderArray<float>(*fTreeReader, "Jet_qgl");
  // Jet_rawFactor = new TTreeReaderArray<float>(*fTreeReader, "Jet_rawFactor");
  // Jet_bRegCorr = new TTreeReaderArray<float>(*fTreeReader, "Jet_bRegCorr");
  // Jet_bRegRes = new TTreeReaderArray<float>(*fTreeReader, "Jet_bRegRes");
  // Jet_cRegCorr = new TTreeReaderArray<float>(*fTreeReader, "Jet_cRegCorr");
  // Jet_cRegRes = new TTreeReaderArray<float>(*fTreeReader, "Jet_cRegRes");
  // Jet_electronIdx1 = new TTreeReaderArray<int>(*fTreeReader, "Jet_electronIdx1");
  // Jet_electronIdx2 = new TTreeReaderArray<int>(*fTreeReader, "Jet_electronIdx2");
  // Jet_hfadjacentEtaStripsSize = new TTreeReaderArray<int>(*fTreeReader, "Jet_hfadjacentEtaStripsSize");
  // Jet_hfcentralEtaStripSize = new TTreeReaderArray<int>(*fTreeReader, "Jet_hfcentralEtaStripSize");
  // Jet_jetId = new TTreeReaderArray<int>(*fTreeReader, "Jet_jetId");
  // Jet_muonIdx1 = new TTreeReaderArray<int>(*fTreeReader, "Jet_muonIdx1");
  // Jet_muonIdx2 = new TTreeReaderArray<int>(*fTreeReader, "Jet_muonIdx2");
  // Jet_nElectrons = new TTreeReaderArray<int>(*fTreeReader, "Jet_nElectrons");
  // Jet_nMuons = new TTreeReaderArray<int>(*fTreeReader, "Jet_nMuons");
  // Jet_puId = new TTreeReaderArray<int>(*fTreeReader, "Jet_puId");
  // Jet_nConstituents = new TTreeReaderArray<unsigned char>(*fTreeReader, "Jet_nConstituents");
  L1PreFiringWeight_Dn = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_Dn");
  L1PreFiringWeight_ECAL_Dn = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_ECAL_Dn");
  L1PreFiringWeight_ECAL_Nom = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_ECAL_Nom");
  L1PreFiringWeight_ECAL_Up = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_ECAL_Up");
  L1PreFiringWeight_Muon_Nom = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_Muon_Nom");
  L1PreFiringWeight_Muon_StatDn = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_Muon_StatDn");
  L1PreFiringWeight_Muon_StatUp = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_Muon_StatUp");
  L1PreFiringWeight_Muon_SystDn = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_Muon_SystDn");
  L1PreFiringWeight_Muon_SystUp = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_Muon_SystUp");
  L1PreFiringWeight_Nom = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_Nom");
  L1PreFiringWeight_Up = new TTreeReaderValue<float>(*fTreeReader, "L1PreFiringWeight_Up");
  // MET_MetUnclustEnUpDeltaX = new TTreeReaderValue<float>(*fTreeReader, "MET_MetUnclustEnUpDeltaX");
  // MET_MetUnclustEnUpDeltaY = new TTreeReaderValue<float>(*fTreeReader, "MET_MetUnclustEnUpDeltaY");
  // MET_covXX = new TTreeReaderValue<float>(*fTreeReader, "MET_covXX");
  // MET_covXY = new TTreeReaderValue<float>(*fTreeReader, "MET_covXY");
  // MET_covYY = new TTreeReaderValue<float>(*fTreeReader, "MET_covYY");
  // MET_phi = new TTreeReaderValue<float>(*fTreeReader, "MET_phi");
  // MET_pt = new TTreeReaderValue<float>(*fTreeReader, "MET_pt");
  // MET_significance = new TTreeReaderValue<float>(*fTreeReader, "MET_significance");
  // MET_sumEt = new TTreeReaderValue<float>(*fTreeReader, "MET_sumEt");
  // MET_sumPtUnclustered = new TTreeReaderValue<float>(*fTreeReader, "MET_sumPtUnclustered");
  // nMuon = new TTreeReaderValue<unsigned int>(*fTreeReader, "nMuon");
  // Muon_dxy = new TTreeReaderArray<float>(*fTreeReader, "Muon_dxy");
  // Muon_dxyErr = new TTreeReaderArray<float>(*fTreeReader, "Muon_dxyErr");
  // Muon_dxybs = new TTreeReaderArray<float>(*fTreeReader, "Muon_dxybs");
  // Muon_dz = new TTreeReaderArray<float>(*fTreeReader, "Muon_dz");
  // Muon_dzErr = new TTreeReaderArray<float>(*fTreeReader, "Muon_dzErr");
  // Muon_eta = new TTreeReaderArray<float>(*fTreeReader, "Muon_eta");
  // Muon_ip3d = new TTreeReaderArray<float>(*fTreeReader, "Muon_ip3d");
  // Muon_jetPtRelv2 = new TTreeReaderArray<float>(*fTreeReader, "Muon_jetPtRelv2");
  // Muon_jetRelIso = new TTreeReaderArray<float>(*fTreeReader, "Muon_jetRelIso");
  // Muon_mass = new TTreeReaderArray<float>(*fTreeReader, "Muon_mass");
  // Muon_miniPFRelIso_all = new TTreeReaderArray<float>(*fTreeReader, "Muon_miniPFRelIso_all");
  // Muon_miniPFRelIso_chg = new TTreeReaderArray<float>(*fTreeReader, "Muon_miniPFRelIso_chg");
  // Muon_pfRelIso03_all = new TTreeReaderArray<float>(*fTreeReader, "Muon_pfRelIso03_all");
  // Muon_pfRelIso03_chg = new TTreeReaderArray<float>(*fTreeReader, "Muon_pfRelIso03_chg");
  // Muon_pfRelIso04_all = new TTreeReaderArray<float>(*fTreeReader, "Muon_pfRelIso04_all");
  // Muon_phi = new TTreeReaderArray<float>(*fTreeReader, "Muon_phi");
  // Muon_pt = new TTreeReaderArray<float>(*fTreeReader, "Muon_pt");
  // Muon_ptErr = new TTreeReaderArray<float>(*fTreeReader, "Muon_ptErr");
  // Muon_segmentComp = new TTreeReaderArray<float>(*fTreeReader, "Muon_segmentComp");
  // Muon_sip3d = new TTreeReaderArray<float>(*fTreeReader, "Muon_sip3d");
  // Muon_softMva = new TTreeReaderArray<float>(*fTreeReader, "Muon_softMva");
  // Muon_tkRelIso = new TTreeReaderArray<float>(*fTreeReader, "Muon_tkRelIso");
  // Muon_tunepRelPt = new TTreeReaderArray<float>(*fTreeReader, "Muon_tunepRelPt");
  // Muon_mvaLowPt = new TTreeReaderArray<float>(*fTreeReader, "Muon_mvaLowPt");
  // Muon_mvaTTH = new TTreeReaderArray<float>(*fTreeReader, "Muon_mvaTTH");
  // Muon_charge = new TTreeReaderArray<int>(*fTreeReader, "Muon_charge");
  // Muon_jetIdx = new TTreeReaderArray<int>(*fTreeReader, "Muon_jetIdx");
  // Muon_nStations = new TTreeReaderArray<int>(*fTreeReader, "Muon_nStations");
  // Muon_nTrackerLayers = new TTreeReaderArray<int>(*fTreeReader, "Muon_nTrackerLayers");
  // Muon_pdgId = new TTreeReaderArray<int>(*fTreeReader, "Muon_pdgId");
  // Muon_tightCharge = new TTreeReaderArray<int>(*fTreeReader, "Muon_tightCharge");
  // Muon_fsrPhotonIdx = new TTreeReaderArray<int>(*fTreeReader, "Muon_fsrPhotonIdx");
  // Muon_highPtId = new TTreeReaderArray<unsigned char>(*fTreeReader, "Muon_highPtId");
  // Muon_highPurity = new TTreeReaderArray<bool>(*fTreeReader, "Muon_highPurity");
  // Muon_inTimeMuon = new TTreeReaderArray<bool>(*fTreeReader, "Muon_inTimeMuon");
  // Muon_isGlobal = new TTreeReaderArray<bool>(*fTreeReader, "Muon_isGlobal");
  // Muon_isPFcand = new TTreeReaderArray<bool>(*fTreeReader, "Muon_isPFcand");
  // Muon_isStandalone = new TTreeReaderArray<bool>(*fTreeReader, "Muon_isStandalone");
  // Muon_isTracker = new TTreeReaderArray<bool>(*fTreeReader, "Muon_isTracker");
  // Muon_jetNDauCharged = new TTreeReaderArray<unsigned char>(*fTreeReader, "Muon_jetNDauCharged");
  // Muon_looseId = new TTreeReaderArray<bool>(*fTreeReader, "Muon_looseId");
  // Muon_mediumId = new TTreeReaderArray<bool>(*fTreeReader, "Muon_mediumId");
  // Muon_mediumPromptId = new TTreeReaderArray<bool>(*fTreeReader, "Muon_mediumPromptId");
  // Muon_miniIsoId = new TTreeReaderArray<unsigned char>(*fTreeReader, "Muon_miniIsoId");
  // Muon_multiIsoId = new TTreeReaderArray<unsigned char>(*fTreeReader, "Muon_multiIsoId");
  // Muon_mvaId = new TTreeReaderArray<unsigned char>(*fTreeReader, "Muon_mvaId");
  // Muon_mvaLowPtId = new TTreeReaderArray<unsigned char>(*fTreeReader, "Muon_mvaLowPtId");
  // Muon_pfIsoId = new TTreeReaderArray<unsigned char>(*fTreeReader, "Muon_pfIsoId");
  // Muon_puppiIsoId = new TTreeReaderArray<unsigned char>(*fTreeReader, "Muon_puppiIsoId");
  // Muon_softId = new TTreeReaderArray<bool>(*fTreeReader, "Muon_softId");
  // Muon_softMvaId = new TTreeReaderArray<bool>(*fTreeReader, "Muon_softMvaId");
  // Muon_tightId = new TTreeReaderArray<bool>(*fTreeReader, "Muon_tightId");
  // Muon_tkIsoId = new TTreeReaderArray<unsigned char>(*fTreeReader, "Muon_tkIsoId");
  // Muon_triggerIdLoose = new TTreeReaderArray<bool>(*fTreeReader, "Muon_triggerIdLoose");
  TkMET_phi = new TTreeReaderValue<float>(*fTreeReader, "TkMET_phi");
  TkMET_pt = new TTreeReaderValue<float>(*fTreeReader, "TkMET_pt");
  TkMET_sumEt = new TTreeReaderValue<float>(*fTreeReader, "TkMET_sumEt");
  nTrigObj = new TTreeReaderValue<unsigned int>(*fTreeReader, "nTrigObj");
  TrigObj_pt = new TTreeReaderArray<float>(*fTreeReader, "TrigObj_pt");
  TrigObj_eta = new TTreeReaderArray<float>(*fTreeReader, "TrigObj_eta");
  TrigObj_phi = new TTreeReaderArray<float>(*fTreeReader, "TrigObj_phi");
  TrigObj_l1pt = new TTreeReaderArray<float>(*fTreeReader, "TrigObj_l1pt");
  TrigObj_l1pt_2 = new TTreeReaderArray<float>(*fTreeReader, "TrigObj_l1pt_2");
  TrigObj_l2pt = new TTreeReaderArray<float>(*fTreeReader, "TrigObj_l2pt");
  TrigObj_id = new TTreeReaderArray<int>(*fTreeReader, "TrigObj_id");
  TrigObj_l1iso = new TTreeReaderArray<int>(*fTreeReader, "TrigObj_l1iso");
  TrigObj_l1charge = new TTreeReaderArray<int>(*fTreeReader, "TrigObj_l1charge");
  TrigObj_filterBits = new TTreeReaderArray<int>(*fTreeReader, "TrigObj_filterBits");
  // nOtherPV = new TTreeReaderValue<unsigned int>(*fTreeReader, "nOtherPV");
  // OtherPV_z = new TTreeReaderArray<float>(*fTreeReader, "OtherPV_z");
  // PV_ndof = new TTreeReaderValue<float>(*fTreeReader, "PV_ndof");
  // PV_x = new TTreeReaderValue<float>(*fTreeReader, "PV_x");
  // PV_y = new TTreeReaderValue<float>(*fTreeReader, "PV_y");
  // PV_z = new TTreeReaderValue<float>(*fTreeReader, "PV_z");
  // PV_chi2 = new TTreeReaderValue<float>(*fTreeReader, "PV_chi2");
  // PV_score = new TTreeReaderValue<float>(*fTreeReader, "PV_score");
  PV_npvs = new TTreeReaderValue<int>(*fTreeReader, "PV_npvs");
  PV_npvsGood = new TTreeReaderValue<int>(*fTreeReader, "PV_npvsGood");
  Flag_HBHENoiseFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_HBHENoiseFilter");
  Flag_HBHENoiseIsoFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_HBHENoiseIsoFilter");
  Flag_CSCTightHaloFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_CSCTightHaloFilter");
  Flag_CSCTightHaloTrkMuUnvetoFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_CSCTightHaloTrkMuUnvetoFilter");
  Flag_CSCTightHalo2015Filter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_CSCTightHalo2015Filter");
  Flag_globalTightHalo2016Filter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_globalTightHalo2016Filter");
  Flag_globalSuperTightHalo2016Filter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_globalSuperTightHalo2016Filter");
  Flag_HcalStripHaloFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_HcalStripHaloFilter");
  Flag_hcalLaserEventFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_hcalLaserEventFilter");
  Flag_EcalDeadCellTriggerPrimitiveFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_EcalDeadCellTriggerPrimitiveFilter");
  Flag_EcalDeadCellBoundaryEnergyFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_EcalDeadCellBoundaryEnergyFilter");
  Flag_ecalBadCalibFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_ecalBadCalibFilter");
  Flag_goodVertices = new TTreeReaderValue<bool>(*fTreeReader, "Flag_goodVertices");
  Flag_eeBadScFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_eeBadScFilter");
  Flag_ecalLaserCorrFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_ecalLaserCorrFilter");
  Flag_trkPOGFilters = new TTreeReaderValue<bool>(*fTreeReader, "Flag_trkPOGFilters");
  Flag_chargedHadronTrackResolutionFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_chargedHadronTrackResolutionFilter");
  Flag_muonBadTrackFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_muonBadTrackFilter");
  Flag_BadChargedCandidateFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_BadChargedCandidateFilter");
  Flag_BadPFMuonFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_BadPFMuonFilter");
  Flag_BadPFMuonDzFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_BadPFMuonDzFilter");
  Flag_hfNoisyHitsFilter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_hfNoisyHitsFilter");
  Flag_BadChargedCandidateSummer16Filter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_BadChargedCandidateSummer16Filter");
  Flag_BadPFMuonSummer16Filter = new TTreeReaderValue<bool>(*fTreeReader, "Flag_BadPFMuonSummer16Filter");
  Flag_trkPOG_manystripclus53X = new TTreeReaderValue<bool>(*fTreeReader, "Flag_trkPOG_manystripclus53X");
  Flag_trkPOG_toomanystripclus53X = new TTreeReaderValue<bool>(*fTreeReader, "Flag_trkPOG_toomanystripclus53X");
  Flag_trkPOG_logErrorTooManyClusters = new TTreeReaderValue<bool>(*fTreeReader, "Flag_trkPOG_logErrorTooManyClusters");
  Flag_METFilters = new TTreeReaderValue<bool>(*fTreeReader, "Flag_METFilters");
  
  if (fIsMC) init_MC();
  if (fIsMC && fSampleName.Contains("NNLO")) init_LHE();
  
  fTrigger = TriggerFactory::createTrigger(fEra, fChannel, fSampleName, fID, fTreeReader);
}
