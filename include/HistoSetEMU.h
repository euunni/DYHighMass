#ifndef HistoSetEMU_h
#define HistoSetEMU_h 1

#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

#include "jet.h"

#include "TFile.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TChain.h"
#include "TLorentzVector.h"
#include "TH1.h"
#include "TH2.h"


class HistoSetEMU
{
public:
  HistoSetEMU() {
    Init();
  }
  
  ~HistoSetEMU() {}

  void Init();

  void FillEMUPair(
    const TLorentzVector& fMuon, 
    const TLorentzVector& fElec, 
    const int& nJet, 
    const int& nBJet, 
    const double& fWeight,
    const std::string& fType
  );

  void FillJet(
    const std::vector<JET::StdJet>& fJet, 
    const std::vector<JET::StdJet>& fBJet, 
    const double& fEMUMass, 
    const double& fWeight,
    const std::string& fType
  );

  void FillHisto(std::string name, double value, double weight = 1.);
  void FillHisto(std::string name, float value, double weight = 1.);
  void FillHisto(std::string name, int value, double weight = 1.);

  void FillHistoSet(std::string name, double mass, int nJet, int nBJet, double value, double weight = 1.);
  void FillHistoSet(std::string name, double mass, int nJet, int nBJet, float value, double weight = 1.);
  void FillHistoSet(std::string name, double mass, int nJet, int nBJet, int value, double weight = 1.);

  void Fill2DHisto(std::string name, double value1, double value2, double weight = 1.) {}

  void SetHisto(std::string name, std::vector<double> bins);
  void SetHisto(std::string name, std::vector<double> bins1, std::vector<double> bins2);

  void SetHisto(std::string name);
  void SetHisto(std::string name, std::string binning);
  void SetHisto(std::string name, std::string binning1, std::string binning2);
  void SetHisto(std::string name, std::string binning1, std::string binning2, std::string binning3);

  std::string GetMassBin(double fDimuonMass);

  double SetPtOverflow(double fPt);
  double SetMassOverflow(double fMass);

  void WriteHisto(TString fEra, TString fSampleName, TString fOutputDir, bool fIsData);

private:
  std::map<std::string, TH1D*> fHistSet;
  std::map<std::string, TH2D*> fHistSet2D;

  std::vector<std::string> fSuffix;

  std::vector<double> fPtBins;
  std::vector<double> fEtaBins;
  std::vector<double> fPhiBins;
  std::vector<double> fMassBins;
  std::vector<double> fDeltaRBins;
  std::vector<double> fNJetBins;

};

#endif
