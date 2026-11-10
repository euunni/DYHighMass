#ifndef EMU_h
#define EMU_h 1

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

#include "RoccoR.h"
#include "NT.h"

#include "yaml-cpp/yaml.h"

#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"
#include "TLorentzVector.h"
#include "TF1.h"
#include "TRandom3.h"

class SmearingEngineEMU {
public:
  SmearingEngineEMU(YAML::Node fConfig) {

    fBarrelSmearingFactor = fConfig["barrel"]["smearing"].as<double>();
    fEndcapSmearingFactor = fConfig["endcap"]["smearing"].as<double>();

    fDoBarrel = true;
    if (fBarrelSmearingFactor == -1)
      fDoBarrel = false;

    fDoEndcap = true;
    if (fEndcapSmearingFactor == -1)
      fDoEndcap = false;

    std::string tBarrelSigma = fConfig["barrel"]["sigma"].as<std::string>();
    fBarrelSmearing = new TF1("fBarrelSmearing", (TString)tBarrelSigma, 0, 5000);

    std::string tEndcapSigma = fConfig["endcap"]["sigma"].as<std::string>();
    fEndcapSmearing = new TF1("fEndcapSmearing", (TString)tEndcapSigma, 0, 5000);

    std::cout << "######################################################################" << std::endl;
    std::cout << "                         Muon smearing setting                        " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " fBarrelSmearingFactor: " << fBarrelSmearingFactor << std::endl;
    std::cout << " tBarrelSigma: " << tBarrelSigma << std::endl;
    std::cout << " fDoBarrel: " << fDoBarrel << std::endl;
    std::cout << " fEndcapSmearingFactor: " << fEndcapSmearingFactor << std::endl;
    std::cout << " tEndcapSigma: " << tEndcapSigma << std::endl;
    std::cout << " fDoEndcap: " << fDoEndcap << std::endl;
    std::cout << "######################################################################" << std::endl;
    std::cout << " " << std::endl;
  }
  ~SmearingEngineEMU() {}

  bool DoBarrel() { return fDoBarrel; }
  bool DoEndcap() { return fDoEndcap; }
  double GetBarrelSigma(double fP) { return fBarrelSmearing->Eval(fP); }
  double GetEndcapSigma(double fP) { return fEndcapSmearing->Eval(fP); }
  double GetBarrelSmearingFactor() { return fBarrelSmearingFactor; }
  double GetEndcapSmearingFactor() { return fEndcapSmearingFactor; }

  private:

    TF1* fBarrelSmearing;
    TF1* fEndcapSmearing;

    bool fDoBarrel;
    bool fDoEndcap;

    double fBarrelSmearingFactor;
    double fEndcapSmearingFactor;

};

class EMU
{
public:
  EMU(YAML::Node fConfig) {

    YAML::Node fMuonConf = fConfig["Muon"];

    fMuonPt = fMuonConf["Pt"].as<float>();
    fMuonEta = fMuonConf["Eta"].as<float>();

    std::string tIDString = fMuonConf["ID"].as<std::string>();
    if (tIDString == "global") fMuonID = (UChar_t)(2);
    else if (tIDString == "tracker") fMuonID = (UChar_t)(1);
    else throw std::runtime_error("Wrong definitions for HighPtID, allowed optsions: global, tracker");

    fMuonISO = fMuonConf["ISO"].as<float>();
    fDoMuonMCSmearing = fMuonConf["doMCSmearing"].as<bool>();
    
    YAML::Node fElecConf = fConfig["Electron"];

    fElecPt = fElecConf["Pt"].as<float>();
    fElecEta = fElecConf["Eta"].as<float>();
    fElecID = fElecConf["ID"].as<int>();

    YAML::Node fZConf = fConfig["Z"];
    fMassCut = fZConf["MassCut"].as<float>();

    std::cout << "######################################################################" << std::endl;
    std::cout << "                            Muon selection                            " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " Pt: " << fMuonPt << std::endl;
    std::cout << " Eta: " << fMuonEta << std::endl;
    std::cout << " ID: " << fMuonConf["ID"].as<std::string>() << " " << fMuonID << std::endl;
    std::cout << " ISO: " << fMuonISO << std::endl;
    std::cout << " doMCSmearing: " << fDoMuonMCSmearing << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << "                            Elec selection                            " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " Pt: " << fElecPt << std::endl;
    std::cout << " Eta: " << fElecEta << std::endl;
    std::cout << " ID: " << fElecID << std::endl;
    std::cout << " ID: HEEP ID (not in config!)" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << "                           EMU Pair selection                         " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " MassCut: " << fMassCut << std::endl;
    std::cout << "######################################################################" << std::endl;
    std::cout << " " << std::endl;


    fMuonSmearingEngine = new SmearingEngineEMU(fMuonConf["MCSmearing"]);
  }
  ~EMU() {}

  struct EMU_MUON {
    TLorentzVector fVec;
    TLorentzVector fVecRaw;
    int fCharge;
    bool fIso;

    EMU_MUON(TLorentzVector fVec_, TLorentzVector fVecRaw_, int fCharge_, bool fIso_)
    : fVec(fVec_), fVecRaw(fVecRaw_), fCharge(fCharge_), fIso(fIso_)
    { };
  };

  struct EMU_ELEC {
    TLorentzVector fVec;
    float fSCEta;
    int fCharge;
    bool fID;

    EMU_ELEC(TLorentzVector fVec_, float fSCEta_,int fCharge_, bool fID_)
    : fVec(fVec_), fSCEta(fSCEta_), fCharge(fCharge_), fID(fID_)
    { };

    float SCEta() { return fSCEta; }
  };

  void init(TTreeReader* fTreeReader);

  void IsMC(bool fIsMC_) { fIsMC = fIsMC_; }
  void SetMuonSmearingSeed(UInt_t fSeed) { fMuonRandom.SetSeed(fSeed); }

  bool PrepareEMUPair();

  TLorentzVector GetMuonMCSmearing(TLorentzVector fMu);

  std::vector<EMU_MUON> GetMuons() { return fFVecMuons; }
  std::vector<EMU_ELEC> GetElecs() { return fFVecElecs; }

  std::pair<EMU_MUON, EMU_ELEC> GetPair_OS() const { return fFVecPair_OS.at(0); }
  std::pair<EMU_MUON, EMU_ELEC> GetPair_SS() const { return fFVecPair_SS.at(0); }
  std::pair<EMU_MUON, EMU_ELEC> GetPair_OS_inverted() const { return fFVecPair_OS_inverted.at(0); }
  std::pair<EMU_MUON, EMU_ELEC> GetPair_SS_inverted() const { return fFVecPair_SS_inverted.at(0); }

  const bool HasPair_OS() const { return fFVecPair_OS.size() == 1; }
  const bool HasPair_SS() const { return fFVecPair_SS.size() == 1; }
  const bool HasPair_OS_inverted() const { return fFVecPair_OS_inverted.size() == 1; }
  const bool HasPair_SS_inverted() const { return fFVecPair_SS_inverted.size() == 1; }

  TTreeReaderValue<unsigned int>* nMuon;
  TTreeReaderArray<float>* Muon_pt;
  TTreeReaderArray<float>* Muon_tunepRelPt;
  TTreeReaderArray<float>* Muon_eta;
  TTreeReaderArray<float>* Muon_phi;
  TTreeReaderArray<int>* Muon_charge;
  TTreeReaderArray<float>* Muon_mass;
  TTreeReaderArray<unsigned char>* Muon_highPtId;
  TTreeReaderArray<float>* Muon_tkRelIso;
  TTreeReaderArray<int>* Muon_nTrackerLayers;
  TTreeReaderArray<bool>* Muon_highPurity;
  TTreeReaderArray<bool>* Muon_mediumId;

  TTreeReaderValue<unsigned int>* nElectron;
  TTreeReaderArray<float>* Electron_pt;
  TTreeReaderArray<float>* Electron_eta;
  TTreeReaderArray<float>* Electron_deltaEtaSC;
  TTreeReaderArray<float>* Electron_phi;
  TTreeReaderArray<float>* Electron_mass;
  TTreeReaderArray<int>* Electron_charge;
  TTreeReaderArray<int>* Electron_cutBased;

private:

  std::vector<EMU_MUON> fFVecMuons;
  std::vector<EMU_ELEC> fFVecElecs;

  std::vector<std::pair<EMU_MUON, EMU_ELEC>> fFVecPair_OS;
  std::vector<std::pair<EMU_MUON, EMU_ELEC>> fFVecPair_SS;
  std::vector<std::pair<EMU_MUON, EMU_ELEC>> fFVecPair_OS_inverted;
  std::vector<std::pair<EMU_MUON, EMU_ELEC>> fFVecPair_SS_inverted;
  

  bool fIsMC;

  float fMuonPt;
  float fMuonEta;
  UChar_t fMuonID;
  float fMuonISO;
  SmearingEngineEMU* fMuonSmearingEngine;
  TRandom3 fMuonRandom;
  bool fDoMuonMCSmearing;

  float fElecPt;
  float fElecEta;
  int fElecID;
  
  float fMassCut;
};

#endif
