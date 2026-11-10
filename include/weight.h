#ifndef WEIGHT_h
#define WEIGHT_h 1

#include <string>
#include <map>
#include <iostream>

class WeightContents {

public:
  WeightContents();
  ~WeightContents();

  void Clear() { 
    fWeightMap.clear(); 
    fTotalWeight = 1.0;
    fMCWeight = 1.0;
    fRecoWeight = 1.0;
  }
  void AddWeight(std::string fName, double fWeight);
  const std::map<std::string, double>& GetWeightMap() const { return fWeightMap; }


  double GetWeight(std::string fName) { return fWeightMap[fName]; }

  double GetMCWeight() { return fMCWeight; }
  double GetRecoWeight() { return fRecoWeight; }
  double GetTotalWeight() { return fTotalWeight; }

private:
  std::map<std::string, double> fWeightMap;
  double fMCWeight;
  double fRecoWeight;
  double fTotalWeight;
};

inline WeightContents::WeightContents() {
  fWeightMap = {};
  fTotalWeight = 1.0;
  fMCWeight = 1.0;
  fRecoWeight = 1.0;
}

inline WeightContents::~WeightContents() {
  fWeightMap.clear();
}

inline void WeightContents::AddWeight(std::string fName, double fWeight) {
  fTotalWeight *= fWeight;
  if (fName == "GenWeight") fMCWeight *= fWeight;
  else fRecoWeight *= fWeight;

  if (fWeightMap.find(fName) == fWeightMap.end()) {
    fWeightMap[fName] = fWeight;
  } else {
    fWeightMap[fName] *= fWeight;
  }
}

class WeigthEnvelope {
public:
  WeigthEnvelope();
  ~WeigthEnvelope();

  void Clear() { 
    if (fDebug) Print();
    fWeightMap.at("OS").Clear();
    fWeightMap.at("SS").Clear();
    fWeightMap.at("OS_inverted").Clear();
    fWeightMap.at("SS_inverted").Clear();
  }
  void AddType(std::string fType) { fWeightMap[fType] = WeightContents(); }

  void AddWeight(std::string fType, std::string fName, double fWeight);
  void AddWeightToAll(std::string fName, double fWeight);
  void Print();
  void SetDebug() { fDebug = true; }

  double GetWeight(std::string fType, std::string fName) { return fWeightMap[fType].GetWeight(fName); }

  double GetMCWeight(std::string fType) { return fWeightMap[fType].GetMCWeight(); }
  double GetRecoWeight(std::string fType) { return fWeightMap[fType].GetRecoWeight(); }
  double GetTotalWeight(std::string fType) { return fWeightMap[fType].GetTotalWeight(); }

private:
  std::map<std::string, WeightContents> fWeightMap;
  bool fDebug;
};

inline WeigthEnvelope::WeigthEnvelope() {
  fDebug = false;
  fWeightMap = {};
}

inline WeigthEnvelope::~WeigthEnvelope() {
  fWeightMap.clear();
}

inline void WeigthEnvelope::AddWeight(std::string fType, std::string fName, double fWeight) {

  if (fWeightMap.find(fType) == fWeightMap.end()) {
    fWeightMap[fType] = WeightContents();
  }

  fWeightMap[fType].AddWeight(fName, fWeight);
}

inline void WeigthEnvelope::AddWeightToAll(std::string fName, double fWeight) {
  for (auto& [fType, fWeightContents] : fWeightMap) {
    fWeightContents.AddWeight(fName, fWeight);
  }
}

inline void WeigthEnvelope::Print() {

  std::cout << "######################################################################" << std::endl;
  std::cout << "                            Weight setting                            " << std::endl;
  std::cout << "----------------------------------------------------------------------" << std::endl;
    
  for (auto& [fType, fWeightContents] : fWeightMap) {
    std::cout << "Type: " << fType << std::endl;
    for (auto& [fName, fWeight] : fWeightContents.GetWeightMap()) {
      std::cout << "    Name: " << fName << " Weight: " << fWeight << std::endl;
    }
    std::cout << "  Total Weight: " << fWeightContents.GetTotalWeight() << std::endl;
    std::cout << "  MC Weight: " << fWeightContents.GetMCWeight() << std::endl;
    std::cout << "  Reco Weight: " << fWeightContents.GetRecoWeight() << std::endl;
  }
  std::cout << "######################################################################" << std::endl;
  std::cout << " " << std::endl;
}

#endif