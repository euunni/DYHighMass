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

class SmearingEngineEMU {
public:
  SmearingEngineEMU(YAML::Node fConfig) {

    YAML::Node fMCSmearingConf = fConfig["MCSmearing"];

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

    fMuonISOinverted = fMuonConf["ISOinverted"].as<bool>();

    fMuonISO = fMuonConf["ISO"].as<float>();
    
    fDoMuonMCSmearing = true;
    if (!fMuonConf["doMCSmearing"].as<bool>())
      fDoMuonMCSmearing = false;

    
    YAML::Node fElecConf = fConfig["Electron"];

    fElecPt = fElecConf["Pt"].as<float>();
    fElecEta = fElecConf["Eta"].as<float>();
    fElecID = fElecConf["ID"].as<int>();
    fElecIDinverted = fElecConf["IDinverted"].as<bool>();

    fIsOppositeCharge = true;
    if (fConfig["Pair"]["Charge"].as<std::string>() == "same")
      fIsOppositeCharge = false;

    YAML::Node fZConf = fConfig["Z"];
    fMassCut = fZConf["MassCut"].as<float>();

    std::cout << "######################################################################" << std::endl;
    std::cout << "                            Muon selection                            " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " Pt: " << fMuonPt << std::endl;
    std::cout << " Eta: " << fMuonEta << std::endl;
    std::cout << " ID: " << fMuonConf["ID"].as<std::string>() << " " << fMuonID << std::endl;
    std::cout << " ISOinverted: " << fMuonISOinverted << std::endl;
    std::cout << " ISO: " << fMuonISO << std::endl;
    std::cout << " doMCSmearing: " << fDoMuonMCSmearing << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << "                            Elec selection                            " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " Pt: " << fElecPt << std::endl;
    std::cout << " Eta: " << fElecEta << std::endl;
    std::cout << " ID: " << fElecID << std::endl;
    std::cout << " IDinverted: " << fElecIDinverted << std::endl;
    std::cout << " ID: HEEP ID (not in config!)" << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << "                           EMU Pair selection                         " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " MassCut: " << fMassCut << std::endl;
    std::cout << " Charge: " << (fIsOppositeCharge ? "Opposite" : "Same") << std::endl;
    std::cout << "######################################################################" << std::endl;
    std::cout << " " << std::endl;


    fMuonSmearingEngine = new SmearingEngineEMU(fMuonConf["MCSmearing"]);
  }
  ~EMU() {}

  struct EMU_MUON {
    TLorentzVector fVec;
    TLorentzVector fVecRaw;
    int fCharge;

    EMU_MUON(TLorentzVector fVec_, TLorentzVector fVecRaw_, int fCharge_)
    : fVec(fVec_), fVecRaw(fVecRaw_), fCharge(fCharge_)
    { };
  };

  struct EMU_ELEC {
    TLorentzVector fVec;
    float fSCEta;
    int fCharge;

    EMU_ELEC(TLorentzVector fVec_, float fSCEta_,int fCharge_)
    : fVec(fVec_), fSCEta(fSCEta_), fCharge(fCharge_)
    { };

    float SCEta() { return fSCEta; }
  };

  void init(TTreeReader* fTreeReader);

  void IsMC(bool fIsMC_) { fIsMC = fIsMC_; }

  bool PrepareEMUPair();
  // void PrepareGenMuon();

  TLorentzVector GetMuonMCSmearing(TLorentzVector fMu);

  std::vector<EMU_MUON> GetMuons() { return fFVecMuons; }
  std::vector<EMU_MUON> GetGenMuons() { return fFVecGenMuons; }
  std::vector<EMU_ELEC> GetElecs() { return fFVecElecs; }

  EMU_MUON GetMuon() { return fFVecMuons.at(fSelectedMuonIdx); }
  EMU_ELEC GetElec() { return fFVecElecs.at(fSelectedElecIdx); }

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

  TTreeReaderValue<unsigned int>* nElectron;
  TTreeReaderArray<float>* Electron_pt;
  TTreeReaderArray<float>* Electron_eta;
  TTreeReaderArray<float>* Electron_deltaEtaSC;
  TTreeReaderArray<float>* Electron_phi;
  TTreeReaderArray<float>* Electron_mass;
  TTreeReaderArray<int>* Electron_charge;
  TTreeReaderArray<int>* Electron_cutBased;

  // TTreeReaderValue<unsigned int>* nGenPart;
  // TTreeReaderArray<float>* GenPart_pt;
  // TTreeReaderArray<float>* GenPart_eta;
  // TTreeReaderArray<float>* GenPart_phi;
  // TTreeReaderArray<float>* GenPart_mass;
  // TTreeReaderArray<int>* GenPart_pdgId;

private:

  std::vector<EMU_MUON> fFVecMuons;
  std::vector<EMU_MUON> fFVecGenMuons;
  
  std::vector<EMU_ELEC> fFVecElecs;
  std::vector<EMU_ELEC> fFvecGenElecs;
  
  int fSelectedMuonIdx;
  int fSelectedElecIdx;

  bool fIsMC;

  float fMuonPt;
  float fMuonEta;
  UChar_t fMuonID;
  bool fMuonISOinverted;
  float fMuonISO;
  SmearingEngineEMU* fMuonSmearingEngine;
  bool fDoMuonMCSmearing;

  float fElecPt;
  float fElecEta;
  int fElecID;
  bool fElecIDinverted;

  bool fIsOppositeCharge;
  
  float fMassCut;
};

#endif
