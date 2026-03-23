#ifndef HistoSetEE_h
#define HistoSetEE_h 1

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


class HistoSetEE
{
public:
  HistoSetEE() {
    Init();
  }

  void Init();

  void FillElec(TLorentzVector& fLeadingElec, TLorentzVector& fSubleadingElec, int nJet, int nBJet, double weight = 1.);
  void FillJet(std::vector<JET::StdJet>* fJet, std::vector<JET::StdJet>* fBJet, double fDielecMass, double weight = 1.);

  void FillHisto(std::string name, double value, double weight = 1.);
  void FillHisto(std::string name, float value, double weight = 1.);
  void FillHisto(std::string name, int value, double weight = 1.);
  void Fill2DHisto(std::string name, double value1, double value2, double weight = 1.) {}

  void SetHisto(std::string name, std::vector<double> bins);
  void SetHisto(std::string name, std::vector<double> bins1, std::vector<double> bins2);

  void SetHisto(std::string name);
  void SetHisto(std::string name, std::string binning);
  void SetHisto(std::string name, std::string binning1, std::string binning2);
  void SetHisto(std::string name, std::string binning1, std::string binning2, std::string binning3);

  std::string GetMassBin(double fDimuonMass);
  std::string GetJetBin(double fNJet);
  std::string GetBJetBin(double fNBJet);
  std::string GetbVetoJetBin(double fNJet);

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
  std::vector<double> fMassBinsHisto;
  std::vector<double> fDeltaRBins;
  std::vector<double> fNJetBins;

};

#endif
