#ifndef JET_h
#define JET_h 1

#include <iostream>
#include <vector>
#include <string>

#include "RoccoR.h"
#include "muon.h"
#include "electron.h"
#include "EfficiencyTable.h"

#include "yaml-cpp/yaml.h"

#include "correction.h"

#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"
#include "TLorentzVector.h"


class JET
{
public:
  JET(YAML::Node fConfig) {

    YAML::Node fJetConf = fConfig["Jet"];

    fJetPt = fJetConf["Pt"].as<float>();
    fEta = fJetConf["Eta"].as<float>();
    fCleaning = fJetConf["Cleaning"].as<bool>();
    fJetID = fJetConf["ID"].as<int>();
    fJetPUID = fJetConf["PUID"].as<int>();

    fJetPUIDSF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["JetPU"]["Path"].as<std::string>())->at("PUJetID_eff");

    fBTagWP = fJetConf["BTag"].as<std::string>();

    std::shared_ptr<const correction::Correction> fDeepJetWP = correction::CorrectionSet::from_file(fConfig["Efficiency"]["BTag"]["Path"].as<std::string>())->at("deepJet_wp_values");
    fBJetTaggerCut = fDeepJetWP->evaluate({fBTagWP});

    fJetBTagEffB = EffTable(fConfig["Efficiency"]["BTagEff"]["bQuark"].as<std::string>());
    fJetBTagEffC = EffTable(fConfig["Efficiency"]["BTagEff"]["cQuark"].as<std::string>());
    fJetBTagEffL = EffTable(fConfig["Efficiency"]["BTagEff"]["lQuark"].as<std::string>());

    fBTagMuJets = correction::CorrectionSet::from_file(fConfig["Efficiency"]["BTag"]["Path"].as<std::string>())->at("deepJet_mujets");
    fBTagIncl = correction::CorrectionSet::from_file(fConfig["Efficiency"]["BTag"]["Path"].as<std::string>())->at("deepJet_incl");    

    std::cout << "######################################################################" << std::endl;
    std::cout << "                            Jet selection                             " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " jet pT: " << fJetPt << std::endl;
    std::cout << " jet eta: " << fEta << std::endl;
    std::cout << " jet ID: " << fJetID << std::endl;
    std::cout << " jet PU ID: " << fJetPUID << std::endl;
    std::cout << " b-tagger WP: " << fBTagWP << " " << fBJetTaggerCut << std::endl;
    std::cout << " jet cleaning: " << fCleaning << std::endl;
    std::cout << "######################################################################" << std::endl;
    std::cout << " " << std::endl;
  }
  ~JET() {}

  struct StdJet {
    TLorentzVector fVec;
    TLorentzVector fVecRaw;
    bool fPassingBJetTagger;
    int fID;
    int fHadFlav;
    int fGenJetIdx;
    bool fPassingPUID;

    StdJet(TLorentzVector fVec_, TLorentzVector fVecRaw_, bool fPassingBJetTagger_, int fID_, int fHadFlav_, int fGenJetIdx_, bool fPassingPUID_)
    : fVec(fVec_), fVecRaw(fVecRaw_), fPassingBJetTagger(fPassingBJetTagger_), fID(fID_), fHadFlav(fHadFlav_), fGenJetIdx(fGenJetIdx_), fPassingPUID(fPassingPUID_)
    { };
  };

  void init(TTreeReader* fTreeReader);

  void IsMC(bool fIsMC_) { fIsMC = fIsMC_; }

  bool PrepareJet();

  std::vector<StdJet> GetJets() { return fFVecJets; }
  std::vector<StdJet> GetBJets() { return fFVecBJets; }

  double GetPUIDSF();
  double GetBTagSF();

  TTreeReaderValue<unsigned int>* nJet;
  TTreeReaderArray<float>* Jet_pt;
  TTreeReaderArray<float>* Jet_eta;
  TTreeReaderArray<float>* Jet_phi;
  TTreeReaderArray<float>* Jet_mass;
  TTreeReaderArray<int>* Jet_jetId;
  TTreeReaderArray<int>* Jet_puId;
  TTreeReaderArray<float>* Jet_btagDeepFlavB;
  TTreeReaderArray<int>* Jet_hadronFlavour;
  TTreeReaderArray<int>* Jet_genJetIdx;

private:

  std::vector<StdJet> fFVecJets;
  std::vector<StdJet> fFVecBJets;
  std::vector<StdJet> fFVecHSJet;

  float fJetPt;
  float fEta;
  int fJetID;
  int fJetPUID;
  std::string fBTagWP;
  float fBJetTaggerCut;
  bool fCleaning;
  bool fIsMC;

  std::shared_ptr<const correction::Correction> fJetPUIDSF;
  
  EffTable fJetBTagEffB;
  EffTable fJetBTagEffC;
  EffTable fJetBTagEffL;

  std::shared_ptr<const correction::Correction> fBTagMuJets;
  std::shared_ptr<const correction::Correction> fBTagIncl;
};

#endif
