#ifndef ELECTRON_h
#define ELECTRON_h 1

#include <iostream>
#include <vector>
#include <string>
#include <limits>

#include "RoccoR.h"

#include "yaml-cpp/yaml.h"

#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"
#include "TLorentzVector.h"


class ELEC
{
public:
  ELEC(YAML::Node fConfig) {

    YAML::Node fElecConf = fConfig["Electron"];
    YAML::Node fZConf = fConfig["Z"];

    fZMassCut = fZConf["MassCut"].as<float>();

    fLeadingPt = fElecConf["LeadingPt"].as<float>();
    fSubLeadingPt = fElecConf["SubLeadingPt"].as<float>();
    fEta = fElecConf["Eta"].as<float>();
    // Cut-based ID (default to Medium=3)
    fID = 3;
    if (fElecConf["ID"]) fID = fElecConf["ID"].as<int>();
    fOppositeCharge = true;
    if (fElecConf["Charge"].as<std::string>() == "same")
      fOppositeCharge = false;


    std::cout << "######################################################################" << std::endl;
    std::cout << "                          Electron selection                          " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " Electron LeadingPt: " << fLeadingPt << std::endl;
    std::cout << " Electron SubLeadingPt: " << fSubLeadingPt << std::endl;
    std::cout << " Electron eta: " << fEta << std::endl;
    // std::cout << " Electron ID: HEEP ID (not in config!)" << std::endl;
    std::cout << " Electron ID: " << fID << std::endl;
    std::cout << " OppositeCharge: " << fOppositeCharge << std::endl;
    std::cout << "######################################################################" << std::endl;
    std::cout << " " << std::endl;
  }
  ~ELEC() {}

  struct StdElec {
    TLorentzVector fVec;
    float fSCEta;
    int fCharge;

    StdElec(TLorentzVector fVec_, float fSCEta_, int fCharge_)
    : fVec(fVec_), fSCEta(fSCEta_), fCharge(fCharge_)
    { };

    float SCEta() const { return fSCEta; }
  };

  void IsMC(bool fIsMC_) { fIsMC = fIsMC_; }

  void init(TTreeReader* fTreeReader);

  bool PrepareElec();

  std::vector<StdElec> GetElecs() { return fFVecElecs; }
  StdElec GetLeadingElec() { return fFVecElecs.at(fLeadingIdx); }
  StdElec GetSubLeadingElec() { return fFVecElecs.at(fSubLeadingIdx); }
  int GetID() const { return fID; }
  std::string GetIDWP() const {
    // NanoAOD Electron_cutBased: 1(Veto), 2(Loose), 3(Medium), 4(Tight)
    switch (fID) {
      case 1: return "Veto";
      case 2: return "Loose";
      case 3: return "Medium";
      case 4: return "Tight";
      default: return "Medium";
    }
  }

  TTreeReaderValue<unsigned int>* nElectron;
  TTreeReaderArray<float>* Electron_pt;
  TTreeReaderArray<float>* Electron_eta;
  TTreeReaderArray<float>* Electron_deltaEtaSC;
  TTreeReaderArray<float>* Electron_phi;
  TTreeReaderArray<float>* Electron_mass;
  TTreeReaderArray<bool>* Electron_cutBased_HEEP;
  TTreeReaderArray<int>* Electron_cutBased;
  TTreeReaderArray<int>* Electron_charge;

  // TTreeReaderArray<float>* Electron_pfRelIso03_all;
  // TTreeReaderArray<float>* Electron_miniPFRelIso_all;
  // TTreeReaderArray<float>* Electron_jetRelIso;
  // TTreeReaderArray<float>* Electron_jetPtRelv2;
  // TTreeReaderArray<int>* Electron_jetIdx;
  // TTreeReaderArray<bool>* Electron_convVeto;
  // TTreeReaderArray<unsigned char>* Electron_lostHits;
  // TTreeReaderArray<float>* Electron_dxy;
  // TTreeReaderArray<float>* Electron_sip3d;
  // TTreeReaderArray<float>* Electron_mvaFall17V2Iso;
  // TTreeReaderArray<float>* Electron_mvaFall17V2noIso;
  // TTreeReaderArray<float>* Electron_hoe;
  // TTreeReaderArray<float>* Electron_sieie;
  // TTreeReaderArray<float>* Electron_eInvMinusPInv;
  // TTreeReaderArray<float>* Electron_r9;
  // TTreeReaderArray<int>* Electron_vidNestedWPBitmap;
  // TTreeReaderArray<float>* Electron_dr03TkSumPt;
  // TTreeReaderArray<float>* Electron_dr03HcalDepth1TowerSumEt;
  // TTreeReaderArray<float>* Electron_dr03EcalRecHitSumEt;
  // TTreeReaderArray<float>* Electron_dr03TkSumPtHEEP;

private:

  std::vector<StdElec> fFVecElecs;

  float fZMassCut;
  
  float fLeadingPt;
  float fSubLeadingPt;
  float fEta;
  int fID;
  bool fOppositeCharge;

  int fSubLeadingIdx;
  int fLeadingIdx;

  bool fIsMC;

};

#endif
