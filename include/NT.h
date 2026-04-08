#ifndef NT_h
#define NT_h 1

#include <iostream>
#include <map>
#include <string>

#include "TriggerEngine.h"

#include "TFile.h"
#include "TROOT.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TStopwatch.h"
#include "TChain.h"
#include "TLorentzVector.h"
#include "TF1.h"

#include "yaml-cpp/yaml.h"

class NT
{
public:

  NT(YAML::Node fConfig_, bool fIsMC_)
  : fConfig(fConfig_), fIsMC(fIsMC_) {

    fMaxEvent = 0;
    fMaxEventVec = {};

    fChain = new TChain("Events");
    fEra = fConfig["Info"]["Era"].as<std::string>();
    fChannel = fConfig["Info"]["Channel"].as<std::string>();
    if (fChannel == "EMU") fChannel = "MUMU";

    fTopPtReweighter = new TF1("fTopPtReweighter", "exp(0.0615 - 0.0005 * x)", 0, 600);
  }

  ~NT() {

    delete fChain;
  }

  void SetSampleName(TString fSampleName_) { fSampleName = fSampleName_; }
  void SetEra(TString fEra_) { fEra = fEra_; }

  void AddChain(TString fSample, int fJobID) {

    fID =fJobID;
    
    YAML::Node fConfigSample = YAML::LoadFile(std::string("../../input/dataset.yml"));

    std::string fListPath =
        "../../input/" + (std::string)(fEra.Data()) + "/" + fConfigSample[(std::string)(fEra.Data())][(std::string)(fSample.Data())]["Name"].as<std::string>() +
        "/input_" + std::to_string(fJobID) + ".list";

    std::cout << "######################################################################" << std::endl;
    std::cout << "                             Merging list                             " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << fListPath << std::endl;


    FILE *fList;
    char fFile[1000];
    fList = fopen(fListPath.c_str(),"r");

    while (fscanf(fList, "%s", fFile) != EOF)
    {
      std::string fFileUpdated = (std::string)fFile;
      std::cout << " " << fFileUpdated << std::endl;
      
      fChain->Add((TString)fFileUpdated);
      double tMaxEvent = fChain->GetEntries();
      fMaxEvent = tMaxEvent;
      fMaxEventVec.push_back(tMaxEvent);
    }

    std::cout << "######################################################################" << std::endl;
    std::cout << " " << std::endl;
  }

  void AddChain(TString fFileName) {
    fChain->Add(fFileName);

    fMaxEvent = fChain->GetEntries();
  }

  void AddChain(std::vector<TString> fFileName) {
    for (int i = 0; i < fFileName.size(); i++)
      fChain->Add(fFileName.at(i));

    fMaxEvent = fChain->GetEntries();
  }

  void init();
  void init_MC();
  void init_LHE();

  TTreeReader* GetTreeReader() { return fTreeReader; }

  bool GetNext() { return fTreeReader->Next(); }

  double GetEntries() {

    return fMaxEvent;
  }

  std::vector<double> GetEntriesVec() {

     return fMaxEventVec;
  }

  /**
    * \brief Reweighing the event weight regarding gen-lv top pT
    *
    * Gen-lv top has status == 22 and abs(pdgId) == 6. And there mother index should be 0.
    * 
    * If enabled in the configuratio file, the reweighing factor is calculated
    * as sqrt(SF(pT_1) * SF(pT_2)), where SF(pT) is exp(0.0416 - 0.0003 * pT).
    *
    * It is the user responsibility to load them. 
    *
    * \param none no parameters reqruies for this function
    *
    * \throws No exception is thrown for this function
    */
  double GetGenTopPtReweightFactor();

  /**
    * \brief Gethering gen-lv particles with given pdgId and status
    * 
    * Mostly, this function is used for electron charge mis-id correction.
    *
    * \param tID pdg id in absolute value (e.g. 11 for electron and positron)
    * \param tStatus status of the particle (e.g. 1 for stable final state particle)
    *
    * \throws TTbar should have 2 gen tops, if not, throw an exception
    */
  std::vector<std::pair<int, TLorentzVector>> GetGenPart(int tID, int tStatus);
  // Returns (charge, p4, motherPdgId) for each matched hard-process particle.
  std::vector<std::tuple<int, TLorentzVector, int>> GetHardGenPart(int tID, int tStatus);
  std::vector<std::pair<int, TLorentzVector>> GetGenDressedLepton(int tID);
  std::vector<TLorentzVector> GetGenJet(float fJetPt, float fJetEta, std::vector<TLorentzVector> tGenLep);

  void SetMC() { fIsMC = true; }

  bool PassingTrigger() {
    return fTrigger->PassingTrigger();
  }

  bool PassinNoiseFilter();

  std::vector<TLorentzVector> GetLHE(int fPID);

  TTreeReaderValue<unsigned int>* run;
  TTreeReaderValue<unsigned int>* luminosityBlock;
  TTreeReaderValue<unsigned long long>* event;
  // TTreeReaderValue<float>* btagWeight_CSVV2;
  // TTreeReaderValue<float>* btagWeight_DeepCSVB;
  // TTreeReaderValue<float>* CaloMET_phi;
  // TTreeReaderValue<float>* CaloMET_pt;
  // TTreeReaderValue<float>* CaloMET_sumEt;
  // TTreeReaderValue<float>* ChsMET_phi;
  // TTreeReaderValue<float>* ChsMET_pt;
  // TTreeReaderValue<float>* ChsMET_sumEt;
  // TTreeReaderValue<unsigned int>* nCorrT1METJet;
  // TTreeReaderArray<float>* CorrT1METJet_area;
  // TTreeReaderArray<float>* CorrT1METJet_eta;
  // TTreeReaderArray<float>* CorrT1METJet_muonSubtrFactor;
  // TTreeReaderArray<float>* CorrT1METJet_phi;
  // TTreeReaderArray<float>* CorrT1METJet_rawPt;
  // TTreeReaderValue<float>* DeepMETResolutionTune_phi;
  // TTreeReaderValue<float>* DeepMETResolutionTune_pt;
  // TTreeReaderValue<float>* DeepMETResponseTune_phi;
  // TTreeReaderValue<float>* DeepMETResponseTune_pt;
  TTreeReaderValue<unsigned int>* nElectron;
  TTreeReaderArray<float>* Electron_dEscaleDown;
  TTreeReaderArray<float>* Electron_dEscaleUp;
  TTreeReaderArray<float>* Electron_dEsigmaDown;
  TTreeReaderArray<float>* Electron_dEsigmaUp;
  TTreeReaderArray<float>* Electron_deltaEtaSC;
  TTreeReaderArray<float>* Electron_dr03EcalRecHitSumEt;
  TTreeReaderArray<float>* Electron_dr03HcalDepth1TowerSumEt;
  TTreeReaderArray<float>* Electron_dr03TkSumPt;
  TTreeReaderArray<float>* Electron_dr03TkSumPtHEEP;
  TTreeReaderArray<float>* Electron_dxy;
  TTreeReaderArray<float>* Electron_dxyErr;
  TTreeReaderArray<float>* Electron_dz;
  TTreeReaderArray<float>* Electron_dzErr;
  TTreeReaderArray<float>* Electron_eCorr;
  TTreeReaderArray<float>* Electron_eInvMinusPInv;
  TTreeReaderArray<float>* Electron_energyErr;
  TTreeReaderArray<float>* Electron_eta;
  TTreeReaderArray<float>* Electron_hoe;
  TTreeReaderArray<float>* Electron_ip3d;
  TTreeReaderArray<float>* Electron_jetPtRelv2;
  TTreeReaderArray<float>* Electron_jetRelIso;
  TTreeReaderArray<float>* Electron_mass;
  TTreeReaderArray<float>* Electron_miniPFRelIso_all;
  TTreeReaderArray<float>* Electron_miniPFRelIso_chg;
  TTreeReaderArray<float>* Electron_mvaFall17V2Iso;
  TTreeReaderArray<float>* Electron_mvaFall17V2noIso;
  TTreeReaderArray<float>* Electron_pfRelIso03_all;
  TTreeReaderArray<float>* Electron_pfRelIso03_chg;
  TTreeReaderArray<float>* Electron_phi;
  TTreeReaderArray<float>* Electron_pt;
  TTreeReaderArray<float>* Electron_r9;
  TTreeReaderArray<float>* Electron_scEtOverPt;
  TTreeReaderArray<float>* Electron_sieie;
  TTreeReaderArray<float>* Electron_sip3d;
  TTreeReaderArray<float>* Electron_mvaTTH;
  TTreeReaderArray<int>* Electron_charge;
  TTreeReaderArray<int>* Electron_cutBased;
  TTreeReaderArray<int>* Electron_jetIdx;
  TTreeReaderArray<int>* Electron_pdgId;
  TTreeReaderArray<int>* Electron_photonIdx;
  TTreeReaderArray<int>* Electron_tightCharge;
  TTreeReaderArray<int>* Electron_vidNestedWPBitmap;
  TTreeReaderArray<int>* Electron_vidNestedWPBitmapHEEP;
  TTreeReaderArray<bool>* Electron_convVeto;
  TTreeReaderArray<bool>* Electron_cutBased_HEEP;
  TTreeReaderArray<bool>* Electron_isPFcand;
  TTreeReaderArray<unsigned char>* Electron_jetNDauCharged;
  TTreeReaderArray<unsigned char>* Electron_lostHits;
  TTreeReaderArray<bool>* Electron_mvaFall17V2Iso_WP80;
  TTreeReaderArray<bool>* Electron_mvaFall17V2Iso_WP90;
  TTreeReaderArray<bool>* Electron_mvaFall17V2Iso_WPL;
  TTreeReaderArray<bool>* Electron_mvaFall17V2noIso_WP80;
  TTreeReaderArray<bool>* Electron_mvaFall17V2noIso_WP90;
  TTreeReaderArray<bool>* Electron_mvaFall17V2noIso_WPL;
  TTreeReaderArray<unsigned char>* Electron_seedGain;
  // TTreeReaderValue<unsigned int>* nFatJet;
  // TTreeReaderArray<float>* FatJet_area;
  // TTreeReaderArray<float>* FatJet_btagCSVV2;
  // TTreeReaderArray<float>* FatJet_btagDDBvLV2;
  // TTreeReaderArray<float>* FatJet_btagDDCvBV2;
  // TTreeReaderArray<float>* FatJet_btagDDCvLV2;
  // TTreeReaderArray<float>* FatJet_btagDeepB;
  // TTreeReaderArray<float>* FatJet_btagHbb;
  // TTreeReaderArray<float>* FatJet_deepTagMD_H4qvsQCD;
  // TTreeReaderArray<float>* FatJet_deepTagMD_HbbvsQCD;
  // TTreeReaderArray<float>* FatJet_deepTagMD_TvsQCD;
  // TTreeReaderArray<float>* FatJet_deepTagMD_WvsQCD;
  // TTreeReaderArray<float>* FatJet_deepTagMD_ZHbbvsQCD;
  // TTreeReaderArray<float>* FatJet_deepTagMD_ZHccvsQCD;
  // TTreeReaderArray<float>* FatJet_deepTagMD_ZbbvsQCD;
  // TTreeReaderArray<float>* FatJet_deepTagMD_ZvsQCD;
  // TTreeReaderArray<float>* FatJet_deepTagMD_bbvsLight;
  // TTreeReaderArray<float>* FatJet_deepTagMD_ccvsLight;
  // TTreeReaderArray<float>* FatJet_deepTag_H;
  // TTreeReaderArray<float>* FatJet_deepTag_QCD;
  // TTreeReaderArray<float>* FatJet_deepTag_QCDothers;
  // TTreeReaderArray<float>* FatJet_deepTag_TvsQCD;
  // TTreeReaderArray<float>* FatJet_deepTag_WvsQCD;
  // TTreeReaderArray<float>* FatJet_deepTag_ZvsQCD;
  // TTreeReaderArray<float>* FatJet_eta;
  // TTreeReaderArray<float>* FatJet_mass;
  // TTreeReaderArray<float>* FatJet_msoftdrop;
  // TTreeReaderArray<float>* FatJet_n2b1;
  // TTreeReaderArray<float>* FatJet_n3b1;
  // TTreeReaderArray<float>* FatJet_particleNetMD_QCD;
  // TTreeReaderArray<float>* FatJet_particleNetMD_Xbb;
  // TTreeReaderArray<float>* FatJet_particleNetMD_Xcc;
  // TTreeReaderArray<float>* FatJet_particleNetMD_Xqq;
  // TTreeReaderArray<float>* FatJet_particleNet_H4qvsQCD;
  // TTreeReaderArray<float>* FatJet_particleNet_HbbvsQCD;
  // TTreeReaderArray<float>* FatJet_particleNet_HccvsQCD;
  // TTreeReaderArray<float>* FatJet_particleNet_QCD;
  // TTreeReaderArray<float>* FatJet_particleNet_TvsQCD;
  // TTreeReaderArray<float>* FatJet_particleNet_WvsQCD;
  // TTreeReaderArray<float>* FatJet_particleNet_ZvsQCD;
  // TTreeReaderArray<float>* FatJet_particleNet_mass;
  // TTreeReaderArray<float>* FatJet_phi;
  // TTreeReaderArray<float>* FatJet_pt;
  // TTreeReaderArray<float>* FatJet_rawFactor;
  // TTreeReaderArray<float>* FatJet_tau1;
  // TTreeReaderArray<float>* FatJet_tau2;
  // TTreeReaderArray<float>* FatJet_tau3;
  // TTreeReaderArray<float>* FatJet_tau4;
  // TTreeReaderArray<float>* FatJet_lsf3;
  // TTreeReaderArray<int>* FatJet_jetId;
  // TTreeReaderArray<int>* FatJet_subJetIdx1;
  // TTreeReaderArray<int>* FatJet_subJetIdx2;
  // TTreeReaderArray<int>* FatJet_electronIdx3SJ;
  // TTreeReaderArray<int>* FatJet_muonIdx3SJ;
  // TTreeReaderArray<unsigned char>* FatJet_nConstituents;
  // TTreeReaderValue<unsigned int>* nFsrPhoton;
  // TTreeReaderArray<float>* FsrPhoton_dROverEt2;
  // TTreeReaderArray<float>* FsrPhoton_eta;
  // TTreeReaderArray<float>* FsrPhoton_phi;
  // TTreeReaderArray<float>* FsrPhoton_pt;
  // TTreeReaderArray<float>* FsrPhoton_relIso03;
  // TTreeReaderArray<int>* FsrPhoton_muonIdx;
  TTreeReaderValue<unsigned int>* nGenJetAK8;
  TTreeReaderArray<float>* GenJetAK8_eta;
  TTreeReaderArray<float>* GenJetAK8_mass;
  TTreeReaderArray<float>* GenJetAK8_phi;
  TTreeReaderArray<float>* GenJetAK8_pt;
  TTreeReaderValue<unsigned int>* nGenJet;
  TTreeReaderArray<float>* GenJet_eta;
  TTreeReaderArray<float>* GenJet_mass;
  TTreeReaderArray<float>* GenJet_phi;
  TTreeReaderArray<float>* GenJet_pt;
  TTreeReaderValue<unsigned int>* nGenPart;
  TTreeReaderArray<float>* GenPart_eta;
  TTreeReaderArray<float>* GenPart_mass;
  TTreeReaderArray<float>* GenPart_phi;
  TTreeReaderArray<float>* GenPart_pt;
  TTreeReaderArray<int>* GenPart_genPartIdxMother;
  TTreeReaderArray<int>* GenPart_pdgId;
  TTreeReaderArray<int>* GenPart_status;
  TTreeReaderArray<int>* GenPart_statusFlags;
  TTreeReaderValue<float>* Generator_binvar;
  TTreeReaderValue<float>* Generator_scalePDF;
  TTreeReaderValue<float>* Generator_weight;
  TTreeReaderValue<float>* Generator_x1;
  TTreeReaderValue<float>* Generator_x2;
  TTreeReaderValue<float>* Generator_xpdf1;
  TTreeReaderValue<float>* Generator_xpdf2;
  TTreeReaderValue<int>* Generator_id1;
  TTreeReaderValue<int>* Generator_id2;
  TTreeReaderValue<float>* GenVtx_x;
  TTreeReaderValue<float>* GenVtx_y;
  TTreeReaderValue<float>* GenVtx_z;
  TTreeReaderValue<float>* genWeight;
  TTreeReaderValue<float>* LHEWeight_originalXWGTUP;
  TTreeReaderValue<unsigned int>* nLHEPdfWeight;
  TTreeReaderArray<float>* LHEPdfWeight;
  TTreeReaderValue<unsigned int>* nLHEReweightingWeight;
  TTreeReaderArray<float>* LHEReweightingWeight;
  TTreeReaderValue<unsigned int>* nLHEScaleWeight;
  TTreeReaderArray<float>* LHEScaleWeight;
  TTreeReaderValue<unsigned int>* nPSWeight;
  TTreeReaderArray<float>* PSWeight;
  // TTreeReaderValue<unsigned int>* nJet;
  // TTreeReaderArray<float>* Jet_area;
  // TTreeReaderArray<float>* Jet_btagCSVV2;
  // TTreeReaderArray<float>* Jet_btagDeepB;
  // TTreeReaderArray<float>* Jet_btagDeepCvB;
  // TTreeReaderArray<float>* Jet_btagDeepCvL;
  // TTreeReaderArray<float>* Jet_btagDeepFlavB;
  // TTreeReaderArray<float>* Jet_btagDeepFlavCvB;
  // TTreeReaderArray<float>* Jet_btagDeepFlavCvL;
  // TTreeReaderArray<float>* Jet_btagDeepFlavQG;
  // TTreeReaderArray<float>* Jet_chEmEF;
  // TTreeReaderArray<float>* Jet_chFPV0EF;
  // TTreeReaderArray<float>* Jet_chHEF;
  // TTreeReaderArray<float>* Jet_eta;
  // TTreeReaderArray<float>* Jet_hfsigmaEtaEta;
  // TTreeReaderArray<float>* Jet_hfsigmaPhiPhi;
  // TTreeReaderArray<float>* Jet_mass;
  // TTreeReaderArray<float>* Jet_muEF;
  // TTreeReaderArray<float>* Jet_muonSubtrFactor;
  // TTreeReaderArray<float>* Jet_neEmEF;
  // TTreeReaderArray<float>* Jet_neHEF;
  // TTreeReaderArray<float>* Jet_phi;
  // TTreeReaderArray<float>* Jet_pt;
  // TTreeReaderArray<float>* Jet_puIdDisc;
  // TTreeReaderArray<float>* Jet_qgl;
  // TTreeReaderArray<float>* Jet_rawFactor;
  // TTreeReaderArray<float>* Jet_bRegCorr;
  // TTreeReaderArray<float>* Jet_bRegRes;
  // TTreeReaderArray<float>* Jet_cRegCorr;
  // TTreeReaderArray<float>* Jet_cRegRes;
  // TTreeReaderArray<int>* Jet_electronIdx1;
  // TTreeReaderArray<int>* Jet_electronIdx2;
  // TTreeReaderArray<int>* Jet_hfadjacentEtaStripsSize;
  // TTreeReaderArray<int>* Jet_hfcentralEtaStripSize;
  // TTreeReaderArray<int>* Jet_jetId;
  // TTreeReaderArray<int>* Jet_muonIdx1;
  // TTreeReaderArray<int>* Jet_muonIdx2;
  // TTreeReaderArray<int>* Jet_nElectrons;
  // TTreeReaderArray<int>* Jet_nMuons;
  // TTreeReaderArray<int>* Jet_puId;
  // TTreeReaderArray<unsigned char>* Jet_nConstituents;
  TTreeReaderValue<float>* L1PreFiringWeight_Dn;
  TTreeReaderValue<float>* L1PreFiringWeight_ECAL_Dn;
  TTreeReaderValue<float>* L1PreFiringWeight_ECAL_Nom;
  TTreeReaderValue<float>* L1PreFiringWeight_ECAL_Up;
  TTreeReaderValue<float>* L1PreFiringWeight_Muon_Nom;
  TTreeReaderValue<float>* L1PreFiringWeight_Muon_StatDn;
  TTreeReaderValue<float>* L1PreFiringWeight_Muon_StatUp;
  TTreeReaderValue<float>* L1PreFiringWeight_Muon_SystDn;
  TTreeReaderValue<float>* L1PreFiringWeight_Muon_SystUp;
  TTreeReaderValue<float>* L1PreFiringWeight_Nom;
  TTreeReaderValue<float>* L1PreFiringWeight_Up;
  TTreeReaderValue<float>* LHE_HT;
  TTreeReaderValue<float>* LHE_HTIncoming;
  TTreeReaderValue<float>* LHE_Vpt;
  TTreeReaderValue<float>* LHE_AlphaS;
  TTreeReaderValue<unsigned char>* LHE_Njets;
  TTreeReaderValue<unsigned char>* LHE_Nb;
  TTreeReaderValue<unsigned char>* LHE_Nc;
  TTreeReaderValue<unsigned char>* LHE_Nuds;
  TTreeReaderValue<unsigned char>* LHE_Nglu;
  TTreeReaderValue<unsigned char>* LHE_NpNLO;
  TTreeReaderValue<unsigned char>* LHE_NpLO;
  TTreeReaderValue<unsigned int>* nLHEPart;
  TTreeReaderArray<float>* LHEPart_pt;
  TTreeReaderArray<float>* LHEPart_eta;
  TTreeReaderArray<float>* LHEPart_phi;
  TTreeReaderArray<float>* LHEPart_mass;
  TTreeReaderArray<float>* LHEPart_incomingpz;
  TTreeReaderArray<int>* LHEPart_pdgId;
  TTreeReaderArray<int>* LHEPart_status;
  TTreeReaderArray<int>* LHEPart_spin;
  TTreeReaderValue<float>* GenMET_phi;
  TTreeReaderValue<float>* GenMET_pt;
  // TTreeReaderValue<float>* MET_MetUnclustEnUpDeltaX;
  // TTreeReaderValue<float>* MET_MetUnclustEnUpDeltaY;
  // TTreeReaderValue<float>* MET_covXX;
  // TTreeReaderValue<float>* MET_covXY;
  // TTreeReaderValue<float>* MET_covYY;
  // TTreeReaderValue<float>* MET_phi;
  // TTreeReaderValue<float>* MET_pt;
  // TTreeReaderValue<float>* MET_significance;
  // TTreeReaderValue<float>* MET_sumEt;
  // TTreeReaderValue<float>* MET_sumPtUnclustered;
  // TTreeReaderValue<unsigned int>* nMuon;
  // TTreeReaderArray<float>* Muon_dxy;
  // TTreeReaderArray<float>* Muon_dxyErr;
  // TTreeReaderArray<float>* Muon_dxybs;
  // TTreeReaderArray<float>* Muon_dz;
  // TTreeReaderArray<float>* Muon_dzErr;
  // TTreeReaderArray<float>* Muon_eta;
  // TTreeReaderArray<float>* Muon_ip3d;
  // TTreeReaderArray<float>* Muon_jetPtRelv2;
  // TTreeReaderArray<float>* Muon_jetRelIso;
  // TTreeReaderArray<float>* Muon_mass;
  // TTreeReaderArray<float>* Muon_miniPFRelIso_all;
  // TTreeReaderArray<float>* Muon_miniPFRelIso_chg;
  // TTreeReaderArray<float>* Muon_pfRelIso03_all;
  // TTreeReaderArray<float>* Muon_pfRelIso03_chg;
  // TTreeReaderArray<float>* Muon_pfRelIso04_all;
  // TTreeReaderArray<float>* Muon_phi;
  // TTreeReaderArray<float>* Muon_pt;
  // TTreeReaderArray<float>* Muon_ptErr;
  // TTreeReaderArray<float>* Muon_segmentComp;
  // TTreeReaderArray<float>* Muon_sip3d;
  // TTreeReaderArray<float>* Muon_softMva;
  // TTreeReaderArray<float>* Muon_tkRelIso;
  // TTreeReaderArray<float>* Muon_tunepRelPt;
  // TTreeReaderArray<float>* Muon_mvaLowPt;
  // TTreeReaderArray<float>* Muon_mvaTTH;
  // TTreeReaderArray<int>* Muon_charge;
  // TTreeReaderArray<int>* Muon_jetIdx;
  // TTreeReaderArray<int>* Muon_nStations;
  // TTreeReaderArray<int>* Muon_nTrackerLayers;
  // TTreeReaderArray<int>* Muon_pdgId;
  // TTreeReaderArray<int>* Muon_tightCharge;
  // TTreeReaderArray<int>* Muon_fsrPhotonIdx;
  // TTreeReaderArray<unsigned char>* Muon_highPtId;
  // TTreeReaderArray<bool>* Muon_highPurity;
  // TTreeReaderArray<bool>* Muon_inTimeMuon;
  // TTreeReaderArray<bool>* Muon_isGlobal;
  // TTreeReaderArray<bool>* Muon_isPFcand;
  // TTreeReaderArray<bool>* Muon_isStandalone;
  // TTreeReaderArray<bool>* Muon_isTracker;
  // TTreeReaderArray<unsigned char>* Muon_jetNDauCharged;
  // TTreeReaderArray<bool>* Muon_looseId;
  // TTreeReaderArray<bool>* Muon_mediumId;
  // TTreeReaderArray<bool>* Muon_mediumPromptId;
  // TTreeReaderArray<unsigned char>* Muon_miniIsoId;
  // TTreeReaderArray<unsigned char>* Muon_multiIsoId;
  // TTreeReaderArray<unsigned char>* Muon_mvaId;
  // TTreeReaderArray<unsigned char>* Muon_mvaLowPtId;
  // TTreeReaderArray<unsigned char>* Muon_pfIsoId;
  // TTreeReaderArray<unsigned char>* Muon_puppiIsoId;
  // TTreeReaderArray<bool>* Muon_softId;
  // TTreeReaderArray<bool>* Muon_softMvaId;
  // TTreeReaderArray<bool>* Muon_tightId;
  // TTreeReaderArray<unsigned char>* Muon_tkIsoId;
  // TTreeReaderArray<bool>* Muon_triggerIdLoose;
  // TTreeReaderValue<unsigned int>* nPhoton;
  // TTreeReaderArray<float>* Photon_dEscaleDown;
  // TTreeReaderArray<float>* Photon_dEscaleUp;
  // TTreeReaderArray<float>* Photon_dEsigmaDown;
  // TTreeReaderArray<float>* Photon_dEsigmaUp;
  // TTreeReaderArray<float>* Photon_eCorr;
  // TTreeReaderArray<float>* Photon_energyErr;
  // TTreeReaderArray<float>* Photon_eta;
  // TTreeReaderArray<float>* Photon_hoe;
  // TTreeReaderArray<float>* Photon_mass;
  // TTreeReaderArray<float>* Photon_mvaID;
  // TTreeReaderArray<float>* Photon_mvaID_Fall17V1p1;
  // TTreeReaderArray<float>* Photon_pfRelIso03_all;
  // TTreeReaderArray<float>* Photon_pfRelIso03_chg;
  // TTreeReaderArray<float>* Photon_phi;
  // TTreeReaderArray<float>* Photon_pt;
  // TTreeReaderArray<float>* Photon_r9;
  // TTreeReaderArray<float>* Photon_sieie;
  // TTreeReaderArray<int>* Photon_charge;
  // TTreeReaderArray<int>* Photon_cutBased;
  // TTreeReaderArray<int>* Photon_cutBased_Fall17V1Bitmap;
  // TTreeReaderArray<int>* Photon_electronIdx;
  // TTreeReaderArray<int>* Photon_jetIdx;
  // TTreeReaderArray<int>* Photon_pdgId;
  // TTreeReaderArray<int>* Photon_vidNestedWPBitmap;
  // TTreeReaderArray<bool>* Photon_electronVeto;
  // TTreeReaderArray<bool>* Photon_isScEtaEB;
  // TTreeReaderArray<bool>* Photon_isScEtaEE;
  // TTreeReaderArray<bool>* Photon_mvaID_WP80;
  // TTreeReaderArray<bool>* Photon_mvaID_WP90;
  // TTreeReaderArray<bool>* Photon_pixelSeed;
  // TTreeReaderArray<unsigned char>* Photon_seedGain;
  TTreeReaderValue<float>* Pileup_nTrueInt;
  TTreeReaderValue<float>* Pileup_pudensity;
  TTreeReaderValue<float>* Pileup_gpudensity;
  TTreeReaderValue<int>* Pileup_nPU;
  TTreeReaderValue<int>* Pileup_sumEOOT;
  TTreeReaderValue<int>* Pileup_sumLOOT;
  TTreeReaderValue<unsigned int>* nGenDressedLepton;
  TTreeReaderArray<float>* GenDressedLepton_eta;
  TTreeReaderArray<float>* GenDressedLepton_mass;
  TTreeReaderArray<float>* GenDressedLepton_phi;
  TTreeReaderArray<float>* GenDressedLepton_pt;
  TTreeReaderArray<int>* GenDressedLepton_pdgId;
  TTreeReaderArray<bool>* GenDressedLepton_hasTauAnc;
  TTreeReaderValue<float>* TkMET_phi;
  TTreeReaderValue<float>* TkMET_pt;
  TTreeReaderValue<float>* TkMET_sumEt;
  TTreeReaderValue<unsigned int>* nTrigObj;
  TTreeReaderArray<float>* TrigObj_pt;
  TTreeReaderArray<float>* TrigObj_eta;
  TTreeReaderArray<float>* TrigObj_phi;
  TTreeReaderArray<float>* TrigObj_l1pt;
  TTreeReaderArray<float>* TrigObj_l1pt_2;
  TTreeReaderArray<float>* TrigObj_l2pt;
  TTreeReaderArray<int>* TrigObj_id;
  TTreeReaderArray<int>* TrigObj_l1iso;
  TTreeReaderArray<int>* TrigObj_l1charge;
  TTreeReaderArray<int>* TrigObj_filterBits;
  TTreeReaderValue<int>* genTtbarId;
  TTreeReaderValue<int>* PV_npvs;
  TTreeReaderValue<int>* PV_npvsGood;
  TTreeReaderValue<bool>* Flag_HBHENoiseFilter;
  TTreeReaderValue<bool>* Flag_HBHENoiseIsoFilter;
  TTreeReaderValue<bool>* Flag_CSCTightHaloFilter;
  TTreeReaderValue<bool>* Flag_CSCTightHaloTrkMuUnvetoFilter;
  TTreeReaderValue<bool>* Flag_CSCTightHalo2015Filter;
  TTreeReaderValue<bool>* Flag_globalTightHalo2016Filter;
  TTreeReaderValue<bool>* Flag_globalSuperTightHalo2016Filter;
  TTreeReaderValue<bool>* Flag_HcalStripHaloFilter;
  TTreeReaderValue<bool>* Flag_hcalLaserEventFilter;
  TTreeReaderValue<bool>* Flag_EcalDeadCellTriggerPrimitiveFilter;
  TTreeReaderValue<bool>* Flag_EcalDeadCellBoundaryEnergyFilter;
  TTreeReaderValue<bool>* Flag_ecalBadCalibFilter;
  TTreeReaderValue<bool>* Flag_goodVertices;
  TTreeReaderValue<bool>* Flag_eeBadScFilter;
  TTreeReaderValue<bool>* Flag_ecalLaserCorrFilter;
  TTreeReaderValue<bool>* Flag_trkPOGFilters;
  TTreeReaderValue<bool>* Flag_chargedHadronTrackResolutionFilter;
  TTreeReaderValue<bool>* Flag_muonBadTrackFilter;
  TTreeReaderValue<bool>* Flag_BadChargedCandidateFilter;
  TTreeReaderValue<bool>* Flag_BadPFMuonFilter;
  TTreeReaderValue<bool>* Flag_BadPFMuonDzFilter;
  TTreeReaderValue<bool>* Flag_hfNoisyHitsFilter;
  TTreeReaderValue<bool>* Flag_BadChargedCandidateSummer16Filter;
  TTreeReaderValue<bool>* Flag_BadPFMuonSummer16Filter;
  TTreeReaderValue<bool>* Flag_trkPOG_manystripclus53X;
  TTreeReaderValue<bool>* Flag_trkPOG_toomanystripclus53X;
  TTreeReaderValue<bool>* Flag_trkPOG_logErrorTooManyClusters;
  TTreeReaderValue<bool>* Flag_METFilters;

private:

  YAML::Node fConfig;
  TChain* fChain;
  TTreeReader* fTreeReader;

  bool fIsMC;
  TString fEra;
  TString fChannel;
  TString fSampleName;
  int fID;
  TF1* fTopPtReweighter;

  std::unique_ptr<TriggerBase> fTrigger;

  double fMaxEvent;
  std::vector<double> fMaxEventVec;
};

#endif
