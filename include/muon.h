#ifndef MUON_h
#define MUON_h 1

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

class SmearingEngine {
public:
  SmearingEngine(YAML::Node fConfig) {

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
  ~SmearingEngine() {}

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

class MUON
{
public:
  MUON(YAML::Node fConfig) {

    YAML::Node fMuonConf = fConfig["Muon"];
    YAML::Node fZConf = fConfig["Z"];

    fLeadingMuonPt = fMuonConf["LeadingMuonPt"].as<float>();
    fSubLeadingMuonPt = fMuonConf["SubLeadingMuonPt"].as<float>();
    fEta = fMuonConf["Eta"].as<float>();
    fZMassCut = fZConf["MassCut"].as<float>();

    std::string id_string = fMuonConf["ID"].as<std::string>();
    if (id_string == "global") fID = (UChar_t)(2);
    else if (id_string == "tracker") fID = (UChar_t)(1);
    else throw std::runtime_error("Wrong definitions for HighPtID, allowed optsions: global, tracker");

    fISO = fMuonConf["ISO"].as<float>();
    fISOinverted = fMuonConf["ISOinverted"].as<bool>();

    fOppositeCharge = true;
    if (fMuonConf["Charge"].as<std::string>() == "same")
      fOppositeCharge = false;

    fDoRoccoR = true;
    if (!fMuonConf["doRoccoR"].as<bool>())
      fDoRoccoR = false;

    fDoMCSmearing = true;
    if (!fMuonConf["doMCSmearing"].as<bool>())
      fDoMCSmearing = false;

    fRoccoR = new RoccoR(fMuonConf["RoccoR"].as<std::string>());

    std::cout << "######################################################################" << std::endl;
    std::cout << "                            Muon selection                            " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " LeadingMuonPt: " << fLeadingMuonPt << std::endl;
    std::cout << " SubleadingMuonPt: " << fSubLeadingMuonPt << std::endl;
    std::cout << " Eta: " << fEta << std::endl;
    std::cout << " ID: " << fMuonConf["ID"].as<std::string>() << " " << fID << std::endl;
    std::cout << " ISO: " << fISO << std::endl;
    std::cout << " ISOinverted: " << fISOinverted << std::endl;
    std::cout << " MassCut: " << fZMassCut << std::endl;
    std::cout << " OppositeCharge: " << fOppositeCharge << std::endl;
    std::cout << " doRoccoR: " << fDoRoccoR << std::endl;
    std::cout << " doMCSmearing: " << fDoMCSmearing << std::endl;
    std::cout << "######################################################################" << std::endl;
    std::cout << " " << std::endl;

    if (fDoRoccoR && fDoMCSmearing) {
      std::cout << "Error: doRoccoR and doMCSmearing cannot be true at the same time" << std::endl;
      exit(1);
    }

    fSmearingEngine = new SmearingEngine(fMuonConf["MCSmearing"]);
  }
  ~MUON() {}

  struct StdMuon {
    TLorentzVector fVec;
    TLorentzVector fVecRaw;
    int fCharge;
    bool fPassingID;
    int fISO;

    StdMuon(TLorentzVector fVec_, TLorentzVector fVecRaw_, int fCharge_, int fISO_)
    : fVec(fVec_), fVecRaw(fVecRaw_), fCharge(fCharge_), fISO(fISO_)
    { };
  };

  void init(TTreeReader* fTreeReader);

  void IsMC(bool fIsMC_) { fIsMC = fIsMC_; }

  bool PrepareMuon();
  // void PrepareGenMuon();

  // TLorentzVector GetRochesterCorrectedMuon(TLorentzVector fMu, int fMuCharge, int nTkLayers);
  TLorentzVector GetMCSmearing(TLorentzVector fMu);

  std::vector<StdMuon> GetMuons() { return fFVecMuons; }
  std::vector<StdMuon> GetGenMuons() { return fFVecGenMuons; }
  StdMuon GetLeadingMuon() { return fFVecMuons.at(fLeadingIdx); }
  StdMuon GetSubLeadingMuon() { return fFVecMuons.at(fSubLeadingIdx); }

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

  // TTreeReaderValue<unsigned int>* nGenPart;
  // TTreeReaderArray<float>* GenPart_pt;
  // TTreeReaderArray<float>* GenPart_eta;
  // TTreeReaderArray<float>* GenPart_phi;
  // TTreeReaderArray<float>* GenPart_mass;
  // TTreeReaderArray<int>* GenPart_pdgId;

private:

  std::vector<StdMuon> fFVecMuons;
  std::vector<StdMuon> fFVecGenMuons;

  float fLeadingMuonPt;
  float fSubLeadingMuonPt;
  float fEta;
  bool fOppositeCharge;
  float fZMassCut;
  UChar_t fID;
  float fISO;
  bool fISOinverted;

  RoccoR* fRoccoR;
  SmearingEngine* fSmearingEngine;
  bool fDoRoccoR;
  bool fDoMCSmearing;

  int fLeadingIdx;
  int fSubLeadingIdx;

  bool fIsMC;

};

#endif
