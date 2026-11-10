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
              "NNLO_MUMU_10to50_v2",
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
              "WJetsToLNu",
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
              "QCD_Pt-15To20_MuEnrichedPt5",
              "QCD_Pt-20To30_MuEnrichedPt5",
              "QCD_Pt-30To50_MuEnrichedPt5",
              "QCD_Pt-50To80_MuEnrichedPt5",
              "QCD_Pt-80To120_MuEnrichedPt5",
              "QCD_Pt-120To170_MuEnrichedPt5",
              "QCD_Pt-170To300_MuEnrichedPt5",
              "QCD_Pt-300To470_MuEnrichedPt5",
              "QCD_Pt-470To600_MuEnrichedPt5",
              "QCD_Pt-600To800_MuEnrichedPt5",
              "QCD_Pt-800To1000_MuEnrichedPt5",
              "QCD_Pt-1000_MuEnrichedPt5",
              "QCD_Pt-15to20_EMEnriched",
              "QCD_Pt-20to30_EMEnriched",
              "QCD_Pt-30to50_EMEnriched",
              "QCD_Pt-50to80_EMEnriched",
              "QCD_Pt-80to120_EMEnriched",
              "QCD_Pt-120to170_EMEnriched",
              "QCD_Pt-170to300_EMEnriched",
              "QCD_Pt-300toInf_EMEnriched",
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
              "NNLO_MUMU_10to50_v2",
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
              "WJetsToLNu",
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
              "QCD_Pt-15To20_MuEnrichedPt5",
              "QCD_Pt-20To30_MuEnrichedPt5",
              "QCD_Pt-30To50_MuEnrichedPt5",
              "QCD_Pt-50To80_MuEnrichedPt5",
              "QCD_Pt-80To120_MuEnrichedPt5",
              "QCD_Pt-120To170_MuEnrichedPt5",
              "QCD_Pt-170To300_MuEnrichedPt5",
              "QCD_Pt-300To470_MuEnrichedPt5",
              "QCD_Pt-470To600_MuEnrichedPt5",
              "QCD_Pt-600To800_MuEnrichedPt5",
              "QCD_Pt-800To1000_MuEnrichedPt5",
              "QCD_Pt-1000_MuEnrichedPt5",
              "QCD_Pt-15to20_EMEnriched",
              "QCD_Pt-20to30_EMEnriched",
              "QCD_Pt-30to50_EMEnriched",
              "QCD_Pt-50to80_EMEnriched",
              "QCD_Pt-80to120_EMEnriched",
              "QCD_Pt-120to170_EMEnriched",
              "QCD_Pt-170to300_EMEnriched",
              "QCD_Pt-300toInf_EMEnriched",
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
              "NNLO_MUMU_10to50_v2",
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
              "WJetsToLNu",
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
              "QCD_Pt-15To20_MuEnrichedPt5",
              "QCD_Pt-20To30_MuEnrichedPt5",
              "QCD_Pt-30To50_MuEnrichedPt5",
              "QCD_Pt-50To80_MuEnrichedPt5",
              "QCD_Pt-80To120_MuEnrichedPt5",
              "QCD_Pt-120To170_MuEnrichedPt5",
              "QCD_Pt-170To300_MuEnrichedPt5",
              "QCD_Pt-300To470_MuEnrichedPt5",
              "QCD_Pt-470To600_MuEnrichedPt5",
              "QCD_Pt-600To800_MuEnrichedPt5",
              "QCD_Pt-800To1000_MuEnrichedPt5",
              "QCD_Pt-1000_MuEnrichedPt5",
              "QCD_Pt-15to20_EMEnriched",
              "QCD_Pt-20to30_EMEnriched",
              "QCD_Pt-30to50_EMEnriched",
              "QCD_Pt-50to80_EMEnriched",
              "QCD_Pt-80to120_EMEnriched",
              "QCD_Pt-120to170_EMEnriched",
              "QCD_Pt-170to300_EMEnriched",
              "QCD_Pt-300toInf_EMEnriched",
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
              "NNLO_MUMU_10to50_v2",
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
              "WJetsToLNu",
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
              "QCD_Pt-15To20_MuEnrichedPt5",
              "QCD_Pt-20To30_MuEnrichedPt5",
              "QCD_Pt-30To50_MuEnrichedPt5",
              "QCD_Pt-50To80_MuEnrichedPt5",
              "QCD_Pt-80To120_MuEnrichedPt5",
              "QCD_Pt-120To170_MuEnrichedPt5",
              "QCD_Pt-170To300_MuEnrichedPt5",
              "QCD_Pt-300To470_MuEnrichedPt5",
              "QCD_Pt-470To600_MuEnrichedPt5",
              "QCD_Pt-600To800_MuEnrichedPt5",
              "QCD_Pt-800To1000_MuEnrichedPt5",
              "QCD_Pt-1000_MuEnrichedPt5",
              "QCD_Pt-15to20_EMEnriched",
              "QCD_Pt-20to30_EMEnriched",
              "QCD_Pt-30to50_EMEnriched",
              "QCD_Pt-50to80_EMEnriched",
              "QCD_Pt-80to120_EMEnriched",
              "QCD_Pt-120to170_EMEnriched",
              "QCD_Pt-170to300_EMEnriched",
              "QCD_Pt-300toInf_EMEnriched",
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
              "NNLO_EE_10to50_v2",
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
              "NNLO_EE_10to50_v2",
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
              "NNLO_EE_10to50_v2",
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
              "NNLO_EE_10to50_v2",
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

  std::string fBaseDir = "";
  fOpt->GetVariable("base", &fBaseDir);
  if (fBaseDir == "") {
    std::cout << "Base directory is not set" << std::endl;
    return -1;
  }
  
  std::string fChannel = "";
  fOpt->GetVariable("channel", &fChannel);
  if (fChannel == "") {
    std::cout << "Channel is not set: EMU, MUMU, EE" << std::endl;
    return -1;
  }

  std::string fChannelTemp = fChannel;
  if (fChannelTemp == "EMU") fChannelTemp = "MUMU";

  std::string fSuffix = "";
  fOpt->GetVariable("suffix", &fSuffix);
  if (fSuffix == "null") fSuffix = "";
  else fSuffix = "/" + fSuffix;

  const char* fWorkspaceBatch = getenv("DY_HIGHMASS_WORKSPACE_BATCH");
  if (fWorkspaceBatch == nullptr) {
    std::cout << "ENV set is needed before running the script" << std::endl;
    return -1;
  }
  std::string fWorkspaceBatchStr = std::string(fWorkspaceBatch);

  const char* fWorkspace = getenv("DY_HIGHMASS_WORKSPACE");
  if (fWorkspace == nullptr) {
    std::cout << "ENV set is needed before running the script" << std::endl;
    return -1;
  }
  std::string fWorkspaceStr = std::string(fWorkspace);

  std::string fBaseDirStr = fWorkspaceBatchStr + "/" + std::string(fBaseDir);
  fs::path fBaseDirFS(fBaseDirStr.data());
  if( !(fs::exists(fBaseDirFS)) ) fs::create_directory(fBaseDirFS);

  if (fChannelTemp == "MUMU") {
    std::string fBaseEOS = "/pnfs/knu.ac.kr/data/cms/store/user/khwang/CMS/HighMassDY/" + fBaseDir;
    fs::path fBaseEOSFS(fBaseEOS.data());
    if( !(fs::exists(fBaseEOSFS)) ) fs::create_directory(fBaseEOSFS);
  }

  std::string fOutputDirStr = fBaseDirStr + "/ROOT";
  fs::path fOutputDirFS(fOutputDirStr.data());
  if( !(fs::exists(fOutputDirFS)) ) fs::create_directory(fOutputDirFS);

  std::string fLogDirStr = fBaseDirStr + "/log";
  fs::path fLogDirFS(fLogDirStr.data());
  if( !(fs::exists(fLogDirFS)) ) fs::create_directory(fLogDirFS);

  std::vector<std::string> fErrorSampleVec;
  fOpt->GetVector("error-sample", &fErrorSampleVec);

  std::string fJobList = "";
  auto fChannelMap = InputMap[fChannelTemp];
  YAML::Node fConfig = YAML::LoadFile(std::string("../input/dataset.yml"));

  if (fErrorSampleVec.size() > 0) {
    for (int i = 0; i < fErrorSampleVec.size(); i++) {
      
      size_t tLint = fErrorSampleVec.at(i).find('-');
      std::string tEra = fErrorSampleVec.at(i).substr(0, tLint);
      std::string tSample = fErrorSampleVec.at(i).substr(tLint + 1); 

      int nList = fConfig[tEra][tSample]["nList"].as<int>();
      for (int l = 0; l < nList; l++) {
        fJobList += R"(../../config/)" + fChannel + fSuffix + R"(/UL)" + tEra + R"(.yml )" + tEra + R"( )" + tSample + R"( )" + std::to_string(l + 1) + "\n";
      }
    }
    
    std::string fJobListStr = fBaseDirStr + "/joblist.txt";
    std::ofstream fJobListStream(fJobListStr);
    if (fJobListStream.is_open()) {
      fJobListStream << fJobList;
      fJobListStream.close();
    } else {
      std::cout << "Failed to create config file: " << fJobListStr << std::endl;
      return -1;
    }

    return 1;
  }

  std::string fCondorSubmit = "";

  if (fChannelTemp == "MUMU") fCondorSubmit = R"(universe              = vanilla
executable            = condor_wrapper.sh
getenv                = True
arguments             = analysis_)" + fChannel + R"( --config $(config) --era $(era) --sample $(sample) --id $(id) --OutputDir $(base)

request_memory        = 500 MB
should_transfer_files = YES
transfer_input_files = )" + fWorkspaceStr + R"(/envset.sh, \
                       )" + fWorkspaceStr + R"(/install/lib, \
                       )" + fWorkspaceStr + R"(/install/bin

JobBatchName          = )" + fBaseDir + R"(
+JobType = "long"

output                = log/$(era)_$(sample)_$(id).out
error                 = log/$(era)_$(sample)_$(id).err
log                   = log/$(era)_$(sample)_$(id).log

queue config,era,sample,id,base from joblist.txt
)"; 
  else fCondorSubmit = R"(universe              = vanilla
  executable            = condor_wrapper.sh
  getenv                = True
  arguments             = analysis_)" + fChannel + R"( --config $(config) --era $(era) --sample $(sample) --id $(id)
  
  request_memory        = 500 MB
  should_transfer_files = YES
  transfer_input_files = )" + fWorkspaceStr + R"(/envset.sh, \
                         )" + fWorkspaceStr + R"(/install/lib, \
                         )" + fWorkspaceStr + R"(/install/bin
  
  JobBatchName          = )" + fBaseDir + R"(
  +JobType = "heavy"
  
  output                = log/$(era)_$(sample)_$(id).out
  error                 = log/$(era)_$(sample)_$(id).err
  log                   = log/$(era)_$(sample)_$(id).log
  
  queue config,era,sample,id from joblist.txt
  )";

  std::string fCondorSubmitStr = fBaseDirStr + "/condor_submit.sub";
  std::ofstream fCondorSubmitStream(fCondorSubmitStr);
  if (fCondorSubmitStream.is_open()) {
    fCondorSubmitStream << fCondorSubmit;
    fCondorSubmitStream.close();
  } else {
    std::cout << "Failed to create config file: " << fCondorSubmitStr << std::endl;
    return -1;
  }

  std::string fHaddSubmit = R"(universe              = vanilla
executable            = condor_wrapper.sh
getenv                = True
arguments             = hadd -j 15 output.root ./ROOT/*.root

request_memory        = 4 GB
request_disk          = 1 GB
request_cpus          = 15
should_transfer_files = YES
transfer_input_files = )" + fWorkspaceStr + R"(/envset.sh, \
                        )" + fWorkspaceStr + R"(/install/lib, \
                        )" + fWorkspaceStr + R"(/install/bin

JobBatchName          = )" + fBaseDir + R"(_hadd
+JobType = "long"

output                = log/hadd.out
error                 = log/hadd.err
log                   = log/hadd.log

queue 1
)";
  
    std::string fHaddSubmitStr = fBaseDirStr + "/hadd_submit.sub";
    std::ofstream fHaddSubmitStream(fHaddSubmitStr);
    if (fHaddSubmitStream.is_open()) {
      fHaddSubmitStream << fHaddSubmit;
      fHaddSubmitStream.close();
    } else {
      std::cout << "Failed to create config file: " << fHaddSubmitStr << std::endl;
      return -1;
    }


  std::string fCondorWrapper = R"(#!/bin/sh

echo "$@"
cd )" + fWorkspaceStr + R"(

source /cvmfs/sft.cern.ch/lcg/views/LCG_107/x86_64-el9-gcc13-opt/setup.sh
export INSTALL_DIR_PATH=$PWD/install

export PATH=$PATH:$INSTALL_DIR_PATH/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_DIR_PATH/lib
export YAMLPATH=/cvmfs/sft.cern.ch/lcg/releases/yamlcpp/0.6.3-d05b2/x86_64-el9-gcc13-opt/lib
export CORRECTIONLIBPATH=/cvmfs/sft.cern.ch/lcg/releases/correctionlib/2.6.4-b8584/x86_64-el9-gcc13-opt/lib

export DY_HIGHMASS_WORKSPACE=$PWD
export DY_HIGHMASS_WORKSPACE_INSTALL=$PWD/install
export DY_HIGHMASS_WORKSPACE_BATCH=$PWD/Batch

cd )" + fBaseDirStr + R"(

echo $PWD

echo "$@"
eval "$@"
)";

  std::string fConfigWrapperStr = fBaseDirStr + "/condor_wrapper.sh";
  std::ofstream fConfigWrapperStream(fConfigWrapperStr);
  if (fConfigWrapperStream.is_open()) {
    fConfigWrapperStream << fCondorWrapper;
    fConfigWrapperStream.close();
  } else {
    std::cout << "Failed to create config file: " << fConfigWrapperStr << std::endl;
    return -1;
  }

  fJobList = "";
  std::vector<std::string> fEraVec = {"2016_preVFP", "2016_postVFP", "2017", "2018"};
  std::vector<std::string> fTierVec = {"Data", "MC"};

  for (int i = 0; i < fChannelMap.size(); i++) {

    auto fEraMap = fChannelMap[fEraVec[i]];
    for (int j = 0; j < fTierVec.size(); j++) {

      auto fSampleMap = fEraMap[fTierVec[j]];
      for (int k = 0; k < fSampleMap.size(); k++) {

        int nList = fConfig[fEraVec[i]][fSampleMap[k]]["nList"].as<int>();
        for (int l = 0; l < nList; l++) {

          if (fChannelTemp == "MUMU") fJobList += R"(../../config/)" + fChannel + fSuffix + R"(/UL)" + fEraVec[i] + R"(.yml )" + fEraVec[i] + R"( )" + fSampleMap[k] + R"( )" + std::to_string(l + 1) + R"( /pnfs/knu.ac.kr/data/cms/store/user/khwang/CMS/HighMassDY/)" + fBaseDir + "\n";
          else fJobList += R"(../../config/)" + fChannel + fSuffix + R"(/UL)" + fEraVec[i] + R"(.yml )" + fEraVec[i] + R"( )" + fSampleMap[k] + R"( )" + std::to_string(l + 1) + "\n";
        }
      }
    }
  }

  std::string fJobListStr = fBaseDirStr + "/joblist.txt";
  std::ofstream fJobListStream(fJobListStr);
  if (fJobListStream.is_open()) {
    fJobListStream << fJobList;
    fJobListStream.close();
  } else {
    std::cout << "Failed to create config file: " << fJobListStr << std::endl;
    return -1;
  }

  return 1;
}
