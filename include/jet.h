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

    fEra = fConfig["Info"]["Era"].as<std::string>();

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

    fJetBTagEff = correction::CorrectionSet::from_file(fConfig["Efficiency"]["BTagEff"]["Path"].as<std::string>())->at("BTagEff");

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
  void SetSampleName(std::string fSampleName_) { 

    if (fSampleName_.find("NNLO") != std::string::npos && fSampleName_.find("tau") == std::string::npos)
      fSampleName = "DY";
    else if (fSampleName_ == "TTTo2L2Nu")
      fSampleName = fSampleName_;
    else if (fSampleName_.find("GG") != std::string::npos)
      fSampleName = "GG";
    else
      fSampleName = "Merged";
  }

  bool PrepareJet();

  const std::vector<StdJet>& GetJets() const { return fFVecJets; }
  const std::vector<StdJet>& GetBJets() const { return fFVecBJets; }

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

  std::string fEra;
  std::string fSampleName;

  std::shared_ptr<const correction::Correction> fJetPUIDSF;
  
  std::shared_ptr<const correction::Correction> fJetBTagEff;

  std::shared_ptr<const correction::Correction> fBTagMuJets;
  std::shared_ptr<const correction::Correction> fBTagIncl;
};

#endif
