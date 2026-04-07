#include "options.h"

#include <iostream>
#include <map>
#include <string>
#include <typeinfo>
#include <filesystem>
#include <cstdlib>
#include <stdexcept>
#include <array>
#include <fstream>

#include "TFile.h"
#include "TH1D.h"

#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

static std::map<std::string, std::map<std::string, std::map<std::string, std::vector<std::string>>>> InputMap = {
  {
    {
      "MUMU", {
        {
          "2016_preVFP", 
          {
            {"Data", {
              "Run2016B_SingleMuon_v2",
              "Run2016C_SingleMuon",
              "Run2016D_SingleMuon",
              "Run2016E_SingleMuon",
              "Run2016F_SingleMuon"
            }},
            {"MC", {
              "NNLO_MUMU_10to50",
              "NNLO_MUMU_inc",
              "NNLO_MUMU_100to200",
              "NNLO_MUMU_200to400",
              "NNLO_MUMU_400to500",
              "NNLO_MUMU_500to700",
              "NNLO_MUMU_700to800",
              "NNLO_MUMU_800to1000",
              "NNLO_MUMU_1000to1500",
              "NNLO_MUMU_1500to2000",
              "NNLO_MUMU_2000toInf",
              "NNLO_tautau",
              "ST_s",
              "ST_t_AntiTop",
              "ST_t_Top",
              "ST_tW_AntiTop",
              "ST_tW_Top",
              "TTTo2L2Nu",
              // "WJetsToLNu",
              "WW",
              "WZ",
              "ZZ",
              "GGToMuMu_10to30_ElEl",
              "GGToMuMu_10to30_InelElElInel",
              "GGToMuMu_10to30_InelInel",
              "GGToMuMu_30to50_ElEl",
              "GGToMuMu_30to50_InelElElInel",
              "GGToMuMu_30to50_InelInel",
              "GGToMuMu_1500toInf_ElEl",
              "GGToMuMu_1500toInf_InelElElInel",
              "GGToMuMu_1500toInf_InelInel",
              "GGToMuMu_200to1500_ElEl",
              "GGToMuMu_200to1500_InelElElInel",
              "GGToMuMu_200to1500_InelInel",
              "GGToMuMu_50to200_ElEl",
              "GGToMuMu_50to200_InelElElInel",
              "GGToMuMu_50to200_InelInel",
            }}
          }
        },
        {
          "2016_postVFP",
          {
            {"Data", {
              "Run2016F_SingleMuon",
              "Run2016G_SingleMuon",
              "Run2016H_SingleMuon"
            }},
            {"MC", {
              "NNLO_MUMU_10to50",
              "NNLO_MUMU_inc",
              "NNLO_MUMU_100to200",
              "NNLO_MUMU_200to400",
              "NNLO_MUMU_400to500",
              "NNLO_MUMU_500to700",
              "NNLO_MUMU_700to800",
              "NNLO_MUMU_800to1000",
              "NNLO_MUMU_1000to1500",
              "NNLO_MUMU_1500to2000",
              "NNLO_MUMU_2000toInf",
              "NNLO_tautau",
              "ST_s",
              "ST_t_AntiTop",
              "ST_t_Top",
              "ST_tW_AntiTop",
              "ST_tW_Top",
              "TTTo2L2Nu",
              // "WJetsToLNu",
              "WW",
              "WZ",
              "ZZ",
              "GGToMuMu_10to30_ElEl",
              "GGToMuMu_10to30_InelElElInel",
              "GGToMuMu_10to30_InelInel",
              "GGToMuMu_30to50_ElEl",
              "GGToMuMu_30to50_InelElElInel",
              "GGToMuMu_30to50_InelInel",
              "GGToMuMu_1500toInf_ElEl",
              "GGToMuMu_1500toInf_InelElElInel",
              "GGToMuMu_1500toInf_InelInel",
              "GGToMuMu_200to1500_ElEl",
              "GGToMuMu_200to1500_InelElElInel",
              "GGToMuMu_200to1500_InelInel",
              "GGToMuMu_50to200_ElEl",
              "GGToMuMu_50to200_InelElElInel",
              "GGToMuMu_50to200_InelInel",
            }}
          }
        },
        {
          "2017",
          {
            {"Data", {
              "Run2017B_SingleMuon",
              "Run2017C_SingleMuon",
              "Run2017D_SingleMuon",
              "Run2017E_SingleMuon",
              "Run2017F_SingleMuon"
            }},
            {"MC", {
              "NNLO_MUMU_10to50",
              "NNLO_MUMU_inc",
              "NNLO_MUMU_100to200",
              "NNLO_MUMU_200to400",
              "NNLO_MUMU_400to500",
              "NNLO_MUMU_500to700",
              "NNLO_MUMU_700to800",
              "NNLO_MUMU_800to1000",
              "NNLO_MUMU_1000to1500",
              "NNLO_MUMU_1500to2000",
              "NNLO_MUMU_2000toInf",
              "NNLO_tautau",
              "ST_s",
              "ST_t_AntiTop",
              "ST_t_Top",
              "ST_tW_AntiTop",
              "ST_tW_Top",
              "TTTo2L2Nu",
              // "WJetsToLNu",
              "WW",
              "WZ",
              "ZZ",
              "GGToMuMu_10to30_ElEl",
              "GGToMuMu_10to30_InelElElInel",
              "GGToMuMu_10to30_InelInel",
              "GGToMuMu_30to50_ElEl",
              "GGToMuMu_30to50_InelElElInel",
              "GGToMuMu_30to50_InelInel",
              "GGToMuMu_1500toInf_ElEl",
              "GGToMuMu_1500toInf_InelElElInel",
              "GGToMuMu_1500toInf_InelInel",
              "GGToMuMu_200to1500_ElEl",
              "GGToMuMu_200to1500_InelElElInel",
              "GGToMuMu_200to1500_InelInel",
              "GGToMuMu_50to200_ElEl",
              "GGToMuMu_50to200_InelElElInel",
              "GGToMuMu_50to200_InelInel",
            }}
          }
        },
        {
          "2018",
          {
            {"Data", {
              "Run2018A_SingleMuon",
              "Run2018B_SingleMuon",
              "Run2018C_SingleMuon",
              "Run2018D_SingleMuon"
            }},
            {"MC", {
              "NNLO_MUMU_10to50",
              "NNLO_MUMU_inc",
              "NNLO_MUMU_100to200",
              "NNLO_MUMU_200to400",
              "NNLO_MUMU_400to500",
              "NNLO_MUMU_500to700",
              "NNLO_MUMU_700to800",
              "NNLO_MUMU_800to1000",
              "NNLO_MUMU_1000to1500",
              "NNLO_MUMU_1500to2000",
              "NNLO_MUMU_2000toInf",
              "NNLO_tautau",
              "ST_s",
              "ST_t_AntiTop",
              "ST_t_Top",
              "ST_tW_AntiTop",
              "ST_tW_Top",
              "TTTo2L2Nu",
              // "WJetsToLNu",
              "WW",
              "WZ",
              "ZZ",
              "GGToMuMu_10to30_ElEl",
              "GGToMuMu_10to30_InelElElInel",
              "GGToMuMu_10to30_InelInel",
              "GGToMuMu_30to50_ElEl",
              "GGToMuMu_30to50_InelElElInel",
              "GGToMuMu_30to50_InelInel",
              "GGToMuMu_1500toInf_ElEl",
              "GGToMuMu_1500toInf_InelElElInel",
              "GGToMuMu_1500toInf_InelInel",
              "GGToMuMu_200to1500_ElEl",
              "GGToMuMu_200to1500_InelElElInel",
              "GGToMuMu_200to1500_InelInel",
              "GGToMuMu_50to200_ElEl",
              "GGToMuMu_50to200_InelElElInel",
              "GGToMuMu_50to200_InelInel",
            }}
          }
        }
      }
    },
    {
      "EE", {
        {
          "2016_preVFP", 
          {
            {"Data", {
              "Run2016B_DoubleEG_v2",
              "Run2016C_DoubleEG",
              "Run2016D_DoubleEG",
              "Run2016E_DoubleEG",
              "Run2016F_DoubleEG",
            }},
            {"MC", {
              "NNLO_EE_10to50",
              "NNLO_EE_inc",
              "NNLO_EE_100to200",
              "NNLO_EE_200to400",
              "NNLO_EE_400to500",
              "NNLO_EE_500to700",
              "NNLO_EE_700to800",
              "NNLO_EE_800to1000",
              "NNLO_EE_1000to1500",
              "NNLO_EE_1500to2000",
              "NNLO_EE_2000toInf",
              "NNLO_tautau",
              "ST_s",
              "ST_t_AntiTop",
              "ST_t_Top",
              "ST_tW_AntiTop",
              "ST_tW_Top",
              "TTTo2L2Nu",
              // "WJetsToLNu",
              "WW",
              "WZ",
              "ZZ",
              "GGToEE_10to30_ElEl",
              "GGToEE_10to30_InelElElInel",
              "GGToEE_10to30_InelInel",
              "GGToEE_30to50_ElEl",
              "GGToEE_30to50_InelElElInel",
              "GGToEE_30to50_InelInel",
              "GGToEE_1500toInf_ElEl",
              "GGToEE_1500toInf_InelElElInel",
              "GGToEE_1500toInf_InelInel",
              "GGToEE_200to1500_ElEl",
              "GGToEE_200to1500_InelElElInel",
              "GGToEE_200to1500_InelInel",
              "GGToEE_50to200_InelElElInel",
              "GGToEE_50to200_InelInel",
              "GGToEE_50to200_ElEl",
            }}
          }
        },
        {
          "2016_postVFP", 
          {
            {"Data", {
              "Run2016F_DoubleEG",
              "Run2016G_DoubleEG",
              "Run2016H_DoubleEG",
            }},
            {"MC", {
              "NNLO_EE_10to50",
              "NNLO_EE_inc",
              "NNLO_EE_100to200",
              "NNLO_EE_200to400",
              "NNLO_EE_400to500",
              "NNLO_EE_500to700",
              "NNLO_EE_700to800",
              "NNLO_EE_800to1000",
              "NNLO_EE_1000to1500",
              "NNLO_EE_1500to2000",
              "NNLO_EE_2000toInf",
              "NNLO_tautau",
              "ST_s",
              "ST_t_AntiTop",
              "ST_t_Top",
              "ST_tW_AntiTop",
              "ST_tW_Top",
              "TTTo2L2Nu",
              // "WJetsToLNu",
              "WW",
              "WZ",
              "ZZ",
              "GGToEE_10to30_ElEl",
              "GGToEE_10to30_InelElElInel",
              "GGToEE_10to30_InelInel",
              "GGToEE_30to50_ElEl",
              "GGToEE_30to50_InelElElInel",
              "GGToEE_30to50_InelInel",
              "GGToEE_1500toInf_ElEl",
              "GGToEE_1500toInf_InelElElInel",
              "GGToEE_1500toInf_InelInel",
              "GGToEE_200to1500_ElEl",
              "GGToEE_200to1500_InelElElInel",
              "GGToEE_200to1500_InelInel",
              "GGToEE_50to200_InelElElInel",
              "GGToEE_50to200_InelInel",
              "GGToEE_50to200_ElEl",
            }}
          }
        },
        {
          "2017", 
          {
            {"Data", {
              "Run2017B_DoubleEG",
              "Run2017C_DoubleEG",
              "Run2017D_DoubleEG",
              "Run2017E_DoubleEG",
              "Run2017F_DoubleEG",
            }},
            {"MC", {
              "NNLO_EE_10to50",
              "NNLO_EE_inc",
              "NNLO_EE_100to200",
              "NNLO_EE_200to400",
              "NNLO_EE_400to500",
              "NNLO_EE_500to700",
              "NNLO_EE_700to800",
              "NNLO_EE_800to1000",
              "NNLO_EE_1000to1500",
              "NNLO_EE_1500to2000",
              "NNLO_EE_2000toInf",
              "NNLO_tautau",
              "ST_s",
              "ST_t_AntiTop",
              "ST_t_Top",
              "ST_tW_AntiTop",
              "ST_tW_Top",
              "TTTo2L2Nu",
              // "WJetsToLNu",
              "WW",
              "WZ",
              "ZZ",
              "GGToEE_10to30_ElEl",
              "GGToEE_10to30_InelElElInel",
              "GGToEE_10to30_InelInel",
              "GGToEE_30to50_ElEl",
              "GGToEE_30to50_InelElElInel",
              "GGToEE_30to50_InelInel",
              "GGToEE_1500toInf_ElEl",
              "GGToEE_1500toInf_InelElElInel",
              "GGToEE_1500toInf_InelInel",
              "GGToEE_200to1500_ElEl",
              "GGToEE_200to1500_InelElElInel",
              "GGToEE_200to1500_InelInel",
              "GGToEE_50to200_InelElElInel",
              "GGToEE_50to200_InelInel",
              "GGToEE_50to200_ElEl",
            }}
          }
        },
        {
          "2018", 
          {
            {"Data", {
              "Run2018A_EGamma",
              "Run2018B_EGamma",
              "Run2018C_EGamma",
              "Run2018D_EGamma"
            }},
            {"MC", {
              "NNLO_EE_10to50",
              "NNLO_EE_inc",
              "NNLO_EE_100to200",
              "NNLO_EE_200to400",
              "NNLO_EE_400to500",
              "NNLO_EE_500to700",
              "NNLO_EE_700to800",
              "NNLO_EE_800to1000",
              "NNLO_EE_1000to1500",
              "NNLO_EE_1500to2000",
              "NNLO_EE_2000toInf",
              "NNLO_tautau",
              "ST_s",
              "ST_t_AntiTop",
              "ST_t_Top",
              "ST_tW_AntiTop",
              "ST_tW_Top",
              "TTTo2L2Nu",
              // "WJetsToLNu",
              "WW",
              "WZ",
              "ZZ",
              "GGToEE_10to30_ElEl",
              "GGToEE_10to30_InelElElInel",
              "GGToEE_10to30_InelInel",
              "GGToEE_30to50_ElEl",
              "GGToEE_30to50_InelElElInel",
              "GGToEE_30to50_InelInel",
              "GGToEE_1500toInf_ElEl",
              "GGToEE_1500toInf_InelElElInel",
              "GGToEE_1500toInf_InelInel",
              "GGToEE_200to1500_ElEl",
              "GGToEE_200to1500_InelElElInel",
              "GGToEE_200to1500_InelInel",
              "GGToEE_50to200_InelElElInel",
              "GGToEE_50to200_InelInel",
              "GGToEE_50to200_ElEl",
            }}
          }
        }
      }
    }
  }
};

int main(int argc, char* argv[]) {

  options* fOpt = new options(argc, argv);

  std::string fChannel = "";
  fOpt->GetVariable("channel", &fChannel);
  if (fChannel == "") {
    std::cout << "Channel is not set: EMU, MUMU, EE" << std::endl;
    return -1;
  }
  std::string fChannelTemp = fChannel;
  if (fChannel == "EMU") fChannelTemp = "MUMU";

  std::string fInput = "";
  fOpt->GetVariable("input", &fInput);
  if (fInput == "null") {
    fInput = "output.root";
  }

  // bool fErrorCorrection = false;
  // fOpt->GetVariable("error", &fErrorCorrection);

  TFile* fFile = new TFile(fInput.c_str(), "READ");

  auto fChannelMap = InputMap[fChannelTemp];
  std::vector<std::string> fEraVec = {"2016_preVFP", "2016_postVFP", "2017", "2018"};
  std::vector<std::string> fTierVec = {"Data", "MC"};
  YAML::Node fConfig = YAML::LoadFile(std::string("../../input/dataset.yml"));

  std::vector<std::string> fErrorEra;
  std::vector<std::string> fErrorSample;
  std::vector<double> fErrorRef;
  std::vector<double> fErrorTarget;

  for (int i = 0; i < fChannelMap.size(); i++) {

    auto fEraMap = fChannelMap[fEraVec[i]];
    for (int j = 0; j < fTierVec.size(); j++) {

      auto fSampleMap = fEraMap[fTierVec[j]];
      for (int k = 0; k < fSampleMap.size(); k++) {
        // std::cout << "Processing: " << fEraVec[i] << " " << fSampleMap[k] << std::endl;

        double nEventRef    = fConfig[fEraVec[i]][fSampleMap[k]]["nEvent"].as<double>();
        if (fSampleMap[k] == "NNLO_EE_10to50") nEventRef += fConfig[fEraVec[i]]["NNLO_EE_10to50_v2"]["nEvent"].as<double>();
        if (fSampleMap[k] == "NNLO_MUMU_10to50") nEventRef += fConfig[fEraVec[i]]["NNLO_MUMU_10to50_v2"]["nEvent"].as<double>();
        auto fEventHist = (TH1D*)fFile->Get((fEraVec[i] + "/" + fSampleMap[k] + "/h_EventInfo").c_str());
        
        double nEventTarget = 0;
        if (fEventHist == nullptr) {
          fErrorEra.push_back(fEraVec[i]);
          fErrorSample.push_back(fSampleMap[k]);
          fErrorRef.push_back(-1);
          fErrorTarget.push_back(-1);

          continue;
        } else {
          nEventTarget = ((TH1D*)fFile->Get((fEraVec[i] + "/" + fSampleMap[k] + "/h_EventInfo").c_str()))->GetBinContent(2);
        }

        if (nEventRef != nEventTarget) {
          // std::cout << "ERROR: " << fEraVec[i] << " " << fSampleMap[k] << " ref: " << nEventRef << " target: " << nEventTarget << std::endl;

          fErrorEra.push_back(fEraVec[i]);
          fErrorSample.push_back(fSampleMap[k]);
          fErrorRef.push_back(nEventRef);
          fErrorTarget.push_back(nEventTarget);
        } else {
          // std::cout << "OK: " << fEraVec[i] << " " << fSampleMap[k] << " ref: " << nEventRef << " target: " << nEventTarget << std::endl;
        }

      }
    }
  }

  std::vector<std::pair<std::string, std::string>> fErrorList;
  std::cout << "ERROR SAMPLES:" << std::endl;
  for (int i = 0; i < fErrorEra.size(); i++) {
    std::cout << "    " << fErrorEra[i] << " " << fErrorSample[i] << " ref: " << fErrorRef[i] << " target: " << fErrorTarget[i] << std::endl;
    fErrorList.push_back(std::make_pair(fErrorEra[i], fErrorSample[i]));
  }

  return 1;
}