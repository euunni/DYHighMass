#ifndef DYLoopMUMU_h
#define DYLoopMUMU_h 1

#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

#include "muon.h"
#include "jet.h"
#include "NT.h"
#include "options.h"
#include "LumiReWeighting.h"
#include "EfficiencyTable.h"
#include "HistoSetMUMU.h"

#include "TFile.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TChain.h"
#include "TLorentzVector.h"
#include "TH1.h"

#include "correction.h"

#include "yaml-cpp/yaml.h"

class DYLoopMUMU
{
public:

  DYLoopMUMU(options* fOpt) {
    fOpt->GetVariable("config", &fConfigPath);
    fConfig = YAML::LoadFile(std::string(fConfigPath));

    fOpt->GetVariable("id", &fJobID);
    fOpt->GetVariable("era", &fEra);
    fOpt->GetVariable("sample", &fSampleName);


    YAML::Node fConfigSample = YAML::LoadFile(std::string("../../input/dataset.yml"));
    fIsMC = false;
    fIsMC = fConfigSample[std::string(fEra)][std::string(fSampleName)]["IsMC"].as<bool>();

    fIsInverted = false;
    fIsInverted = fConfig["Muon"]["ISOinverted"].as<bool>();

    fDoL1Pre = false;
    fDoL1Pre = fConfig["Correction"]["L1PreFiring"].as<bool>();

    fDoReco = false;
    fDoReco = fConfig["Correction"]["Reco"].as<bool>(); 
    fReco_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Reco"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["Reco"]["Name"].as<std::string>());

    fDoID = false;
    fDoID = fConfig["Correction"]["ID"].as<bool>();
    fID_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["ID"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["ID"]["Name"].as<std::string>());

    fDoISO = false;
    fDoISO = fConfig["Correction"]["ISO"].as<bool>();
    fISO_SF = correction::CorrectionSet::from_file(fConfig["Efficiency"]["ISO"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["ISO"]["Name"].as<std::string>());

    fDoTRIGG = false;
    fDoTRIGG = fConfig["Correction"]["Trigger"].as<bool>();
    fTRIG_Eff_MC = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["Trigger"]["MC"].as<std::string>());
    fTRIG_Eff_Data = correction::CorrectionSet::from_file(fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>())->at(fConfig["Efficiency"]["Trigger"]["Data"].as<std::string>());
    
    fDoPU = false;
    fDoPU = fConfig["Correction"]["PileUp"].as<bool>();
    fPuReweighting = new LumiReWeighting(
      "../../PileupInfo/" + fConfig["Pileup"]["MC"].as<std::string>(),
      "../../PileupInfo/" + fConfig["Pileup"]["Data"].as<std::string>(),
      "pileup",
      "pileup"
    );

    fDoTopPtReweighing = false;
    fDoTopPtReweighing = fConfig["Correction"]["TopPtReweighing"].as<bool>();

    fDoJetPUID = false;
    fDoJetPUID = fConfig["Correction"]["JetPU"].as<bool>();

    fDoBTag = false;
    fDoBTag = fConfig["Correction"]["BTag"].as<bool>();

    Print();

    fHistoSet = new HistoSetMUMU();

    fNtuples = new NT(fConfig, fIsMC);
    fNtuples->SetSampleName(fSampleName);
    fNtuples->SetEra(fEra);
    fNtuples->AddChain(fSampleName, fJobID);

    std::cout << " " << std::endl;

    fMaxEntries = fNtuples->GetEntries();
    fNtuples->init();

    std::cout << " " << std::endl;

    fMuons = new MUON(fConfig);
    fMuons->IsMC(fIsMC);
    fMuons->init(fNtuples->GetTreeReader());

    fJets = new JET(fConfig);
    fJets->IsMC(fIsMC);
    fJets->init(fNtuples->GetTreeReader());

    std::cout << std::fixed;
  }

  void Print() {

    std::cout << "######################################################################" << std::endl;
    std::cout << "                             Loop setting                             " << std::endl;
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << " fDoReco    : " << fDoReco << " " << fConfig["Efficiency"]["Reco"]["Path"].as<std::string>() << std::endl;
    std::cout << "              " << fDoReco << " " << fConfig["Efficiency"]["Reco"]["Name"].as<std::string>() << std::endl;
    std::cout << " fDoID      : " << fDoID << " " << fConfig["Efficiency"]["ID"]["Path"].as<std::string>() << std::endl;
    std::cout << "              " << fDoID << " " << fConfig["Efficiency"]["ID"]["Name"].as<std::string>() << std::endl;
    std::cout << " fDoISO     : " << fDoISO << " " << fConfig["Efficiency"]["ISO"]["Path"].as<std::string>() << std::endl;
    std::cout << "              " << fDoISO << " " << fConfig["Efficiency"]["ISO"]["Name"].as<std::string>() << std::endl;
    std::cout << " fDoTRIGG   : " << fDoTRIGG << " " << fConfig["Efficiency"]["Trigger"]["Path"].as<std::string>() << std::endl;
    std::cout << "              " << fDoTRIGG << " " << fConfig["Efficiency"]["Trigger"]["Data"].as<std::string>() << std::endl;
    std::cout << "              " << fDoTRIGG << " " << fConfig["Efficiency"]["Trigger"]["MC"].as<std::string>() << std::endl;
    std::cout << " fDoPU      : " << fDoPU << " " << fConfig["Pileup"]["Data"].as<std::string>() << std::endl;
    std::cout << "              " << fConfig["Pileup"]["MC"].as<std::string>() << std::endl;
    std::cout << " fDoL1Pre   : " << fDoL1Pre << " " << std::endl;
    std::cout << " fDoJetPUID : " << fDoJetPUID << " " << fConfig["Efficiency"]["JetPU"]["Path"].as<std::string>() << std::endl;
    std::cout << " fDoBTag    : " << fDoBTag << " " << fConfig["Efficiency"]["BTag"]["Path"].as<std::string>() << std::endl;
    std::cout << "              " << fConfig["Efficiency"]["BTagEff"]["bQuark"].as<std::string>() << std::endl;
    std::cout << "              " << fConfig["Efficiency"]["BTagEff"]["cQuark"].as<std::string>() << std::endl;
    std::cout << "              " << fConfig["Efficiency"]["BTagEff"]["lQuark"].as<std::string>() << std::endl;
    std::cout << " fDoTopPtReweighing : " << fDoTopPtReweighing << std::endl;
    std::cout << "######################################################################" << std::endl;
    std::cout << " " << std::endl;
  }

  TH1D* GetHist(TString name, TString title, int nBin, float xlow, float xhigh) {

    float binWidth = (xhigh - xlow) / (float)nBin;

    return new TH1D(name, title, nBin + 2, xlow - binWidth, xhigh + binWidth);
  }

  void FillHisto(TH1D* hist, double value, double weight) {

    float xMin = hist->GetBinLowEdge(1);
    float xMax = hist->GetBinLowEdge(hist->GetNbinsX()) + hist->GetBinWidth(hist->GetNbinsX());

    if (value < xMin) hist->Fill(hist->GetBinCenter(1), weight);
    else if (value > xMax) hist->Fill(hist->GetBinCenter(hist->GetNbinsX()), weight);
    else hist->Fill(value, weight);
  }

  std::string getCurrentTimeString();
  std::string formatDuration(const std::chrono::high_resolution_clock::duration& duration); 

  void SetEra(TString fEra_) { fEra = fEra_; }
  void SetSample(TString fSampleName_) { fSampleName = fSampleName_; }
  void SetJobID(int fJobID_) { fJobID = fJobID_; }
  void SetMC(bool fIsMC_) { fIsMC = fIsMC_; }
  void SetNT();

  void PrepareHist();
  void EndOfJob();

  void Loop();


private:
  TString fConfigPath;
  TString fEra;
  TString fSampleName;
  int fJobID;
  bool fIsMC;

  LumiReWeighting* fPuReweighting;
  std::shared_ptr<const correction::Correction> fReco_SF;
  std::shared_ptr<const correction::Correction> fID_SF;
  std::shared_ptr<const correction::Correction> fISO_SF;
  std::shared_ptr<const correction::Correction> fTRIG_Eff_Data;
  std::shared_ptr<const correction::Correction> fTRIG_Eff_MC;

  bool fDoReco;
  bool fDoID;
  bool fDoISO;
  bool fDoTRIGG;
  bool fDoPU;
  bool fDoL1Pre;
  bool fDoJetPUID;
  bool fDoBTag;
  bool fDoTopPtReweighing;
  bool fIsInverted;

  NT* fNtuples;
  YAML::Node fConfig;

  MUON* fMuons;
  JET* fJets;

  double fMaxEntries;

  HistoSetMUMU* fHistoSet;
};

#endif
