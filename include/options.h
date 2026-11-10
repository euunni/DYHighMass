#ifndef options_h
#define options_h 1

#include "TString.h"

#include <map>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <cmath>
#include <numeric>
#include <functional>

class options
{
public:
  options(int argc, char* argv[])
  : fArgc(argc)
  {
    for (int i = 1; i < fArgc; i++) {
      fArgv.push_back(std::string(argv[i]));
      // std::cout << "Print " << i - 1 << " " << fArgv.at(i - 1) << std::endl;
    }

    init();

    for (int i = 0; i < fArgv.size(); i++) {

      if (fArgv[i] == "--config") {
        i++;
        AddVariable("config", (TString)(fArgv[i]));
      }

      if (fArgv[i] == "--OutputDir") {
        i++;
        AddVariable("OutputDir", (TString)(fArgv[i]));
      }

      if (fArgv[i] == "--base") {
        i++;
        AddVariable("base", (std::string)(fArgv[i]));
      }

      if (fArgv[i] == "--input") {
        i++;
        AddVariable("input", (std::string)(fArgv[i]));
      }

      if (fArgv[i] == "--era") {
        i++;
        AddVariable("era", (TString)(fArgv[i]));
      }

      if (fArgv[i] == "--sample") {
        i++;
        AddVariable("sample", (TString)(fArgv[i]));
      }

      if (fArgv[i] == "--id") {
        i++;
        AddVariable("id", std::stoi(fArgv[i]));
      }

      if (fArgv[i] == "--output") {
        i++;
        AddVariable("output", (TString)(fArgv[i]));
      }

      if (fArgv[i] == "--input") {
        i++;
        AddVariable("input", (TString)(fArgv[i]));
      }

      if (fArgv[i] == "--suffix") {
        i++;
        AddVariable("suffix", (std::string)(fArgv[i]));
      }

      if (fArgv[i] == "--channel") {
        i++;
        AddVariable("channel", (std::string)(fArgv[i]));
      }

      if (fArgv[i] == "--error") {
        AddVariable("error", true);
      }

      if (fArgv[i] == "--debug") {
        AddVariable("debug", true);
      }

      if (fArgv[i] == "--error-sample") {

        while (1) {
          i++;
          if (i == (fArgc - 1)) break;

          std::string tErrorSample = (std::string)fArgv[i];
          if (tErrorSample.find("--") != std::string::npos) break;

          AddVec("error-sample", tErrorSample);
        }
      }
    }
  }

  ~options() {}

  void init() {

    fVarBool = {};
    fVarInt = {};
    fVarIntVec = {};
    fVarDouble = {};
    fVarDoubleVec = {};
    fVarString = {};
    fVarStringVec = {};
    fVarTString = {};
    fVarTStringVec = {};
  }
  bool Help();

  void AddVariable(std::string key, bool value) { fVarBool[key] = value; }
  void AddVariable(std::string key, int value) { fVarInt[key] = value; }
  void AddVariable(std::string key, double value) { fVarDouble[key] = value; }
  void AddVariable(std::string key, std::string value) { fVarString[key] = value; }
  void AddVariable(std::string key, TString value) { fVarTString[key] = value; }

  void AddVec(std::string key, int value) { fVarIntVec[key].push_back(value); }
  void AddVec(std::string key, double value) { fVarDoubleVec[key].push_back(value); }
  void AddVec(std::string key, std::string value) { fVarStringVec[key].push_back(value); }

  void GetVariable(std::string key, bool* value) {

    if (fVarBool.find(key) == fVarBool.end()) *value = false;
    else                                      *value = fVarBool[key];
  }

  void GetVariable(std::string key, int* value) {

    if (fVarInt.find(key) == fVarInt.end()) *value = -1;
    else                                    *value = fVarInt[key];
  }

  void GetVariable(std::string key, double* value) {

    if (fVarDouble.find(key) == fVarDouble.end()) *value = -1;
    else                                          *value = fVarDouble[key];
  }

  void GetVariable(std::string key, std::string* value) {

    if (fVarString.find(key) == fVarString.end()) *value = "null";
    else                                          *value = fVarString[key];
  }

  void GetVariable(std::string key, TString* value) {

    if (fVarTString.find(key) == fVarTString.end()) *value = "null";
    else                                            *value = fVarTString[key];
  }

  void GetVector(std::string key, std::vector<int>* value) {
    *value = fVarIntVec[key];

    if (fVarIntVec.find(key) == fVarIntVec.end()) *value = std::vector<int>{};
    else                                          *value = fVarIntVec[key];
  }

  void GetVector(std::string key, std::vector<double>* value) {

    if (fVarDoubleVec.find(key) == fVarDoubleVec.end()) *value = std::vector<double>{};
    else                                                *value = fVarDoubleVec[key];
  }

  void GetVector(std::string key, std::vector<std::string>* value) {

    if (fVarStringVec.find(key) == fVarStringVec.end()) *value = std::vector<std::string>{};
    else                                                *value = fVarStringVec[key];
  }

  void Print() {

    for (int i = 0; i < fArgv.size(); i++)
      std::cout << i << " " << fArgv.at(i) << std::endl;

    for (auto varBool : fVarBool)
      std::cout << "fVarBool: " << varBool.first << " " << varBool.second << std::endl;

    for (auto varInt : fVarInt)
      std::cout << "fVarInt: " << varInt.first << " " << varInt.second << std::endl;

    for (auto varDouble : fVarDouble)
      std::cout << "fVarDouble: " << varDouble.first << " " << varDouble.second << std::endl;

    for (auto varStr : fVarString)
      std::cout << "fVarString: " << varStr.first << " " << varStr.second << std::endl;

    for (auto varStr : fVarTString)
      std::cout << "fVarTString: " << varStr.first << " " << varStr.second << std::endl;

    for (auto varInt : fVarIntVec) {
      std::cout << "fVarIntVec: " + varInt.first;
      for (int i = 0; i < varInt.second.size(); i++)
        std::cout << " " << varInt.second.at(i) << " |";
    }
    std::cout << " " << std::endl;

    for (auto varInt : fVarDoubleVec) {
      std::cout << "fVarDoubleVec: " + varInt.first;
      for (int i = 0; i < varInt.second.size(); i++)
        std::cout << " " << varInt.second.at(i) << " |";
    }
    std::cout << " " << std::endl;

    for (auto varInt : fVarStringVec) {
      std::cout << "fVarStringVec: " + varInt.first;
      std::cout << " " + std::to_string(varInt.second.size());
      for (int i = 0; i < varInt.second.size(); i++)
        std::cout << " " << varInt.second.at(i) << " |";
    }
    std::cout << " " << std::endl;
  }


private:

  int fArgc;
  std::vector<std::string> fArgv;

  std::map<std::string, bool> fVarBool;

  std::map<std::string, int> fVarInt;
  std::map<std::string, std::vector<int>> fVarIntVec;

  std::map<std::string, double> fVarDouble;
  std::map<std::string, std::vector<double>> fVarDoubleVec;

  std::map<std::string, std::string> fVarString;
  std::map<std::string, std::vector<std::string>> fVarStringVec;

  std::map<std::string, TString> fVarTString;
  std::map<std::string, std::vector<TString>> fVarTStringVec;
};

#endif
