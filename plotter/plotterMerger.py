#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array

import plotterEngine_MUMU as plotterEngine

parser = argparse.ArgumentParser()
parser.add_argument('--channel', help=' : channel to merge')
args = parser.parse_args()

class Merger:
    def __init__(self, plot_list, case_list, type_list, input_path = "output.root"):

        baseDir = "/pnfs/knu.ac.kr/data/cms/store/user/khwang/CMS/HighMassDY/"
        inputFile = baseDir + input_path

        output_path = "./" + input_path.replace(".root", "_merged.root")
        
        os.system(f"cp {inputFile} {output_path}")

        self.plot_list = plot_list
        self.case_list = case_list
        self.type_list = type_list
        self.merge_list = plotterEngine.TotalMCList.copy()
        self.merge_list.append("Data")
    
        self.p2016_preVFP = plotterEngine.Plotter("2016_preVFP", rootPath = inputFile)
        self.p2016_postVFP = plotterEngine.Plotter("2016_postVFP", rootPath = inputFile)
        self.p2017 = plotterEngine.Plotter("2017", rootPath = inputFile)
        self.p2018 = plotterEngine.Plotter("2018", rootPath = inputFile)

        self.merge_file = ROOT.TFile(output_path, "UPDATE")

    def Merge(self):
        self.merge_file.mkdir("merged")
        for sample in self.merge_list:
            self.merge_file.mkdir(f"merged/{sample}")
            
            for atype in self.type_list:
                for case in self.case_list:

                    if case != "":
                        if atype == "OS":
                             self.merge_file.mkdir(f"merged/{sample}/{case}")
                    
                    for plot in self.plot_list:

                        if plot == "dimuonMassFailGen" and sample == "Data":
                            continue

                        histname = case + "/h_" + atype + "_" + plot + case
                        if case == "": 
                            histname = "h_" + atype + "_" + plot

                        hist_p2016_preVFP = self.p2016_preVFP.GetSingleHist(histname, sample)
                        hist_p2016_postVFP = self.p2016_postVFP.GetSingleHist(histname, sample)
                        hist_p2017 = self.p2017.GetSingleHist(histname, sample)
                        hist_p2018 = self.p2018.GetSingleHist(histname, sample)

                        hist_merged = hist_p2016_preVFP.Clone(plot + case)
                        hist_merged.Add(hist_p2016_postVFP)
                        hist_merged.Add(hist_p2017)
                        hist_merged.Add(hist_p2018)
                        hist_merged.SetName(f"h_{atype}_{plot}{case}")

                        self.merge_file.cd(f"merged/{sample}/{case}")
                        hist_merged.Write()

        self.merge_file.Close()

def main(args):

    case_list = ["", "_0BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]
    type_list = ["OS", "SS", "OS_inverted", "SS_inverted"]
    
    plot_list_mumu = [
        "JetPt",
        "JetEta",
        "JetPhi",
        "BJetPt",
        "BJetEta",
        "BJetPhi",
        "LeadingMuonPt",
        "LeadingMuonEta",
        "LeadingMuonPhi",
        "SubleadingMuonPt",
        "SubleadingMuonEta",
        "SubleadingMuonPhi",
        "MuonPt",
        "MuonEta",
        "MuonPhi",
        "dimuonMassFailGen",
        "dimuonMass",
        "dimuonPt",
        "dimuonRap",
    ]

    plot_list_emu = [
        "JetPt",
        "JetEta",
        "JetPhi",
        "BJetPt",
        "BJetEta",
        "BJetPhi",
        "ElecPt",
        "ElecEta",
        "ElecPhi",
        "MuonPt",
        "MuonEta",
        "MuonPhi",
        "PairMass",
        "PairPt",
        "PairRap"
    ]

    # input_name = "260824_MUMU_bothInverted.root"
    # input_name = "260901_MUMU_OneInverted.root"
    input_name = "260911_MUMU_GenUpdate.root"

    if args.channel == "MUMU":
        merger = Merger(plot_list_mumu, case_list, type_list, input_path = input_name)
    elif args.channel == "EMU":
        merger = Merger(plot_list_emu, case_list, type_list)
    else:
        print("Invalid channel")
        return

    merger.Merge()


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    ROOT.TH1.SetDefaultSumw2()

    main(args)
