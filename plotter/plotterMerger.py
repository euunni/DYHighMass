#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array

import plotterEngine_MUMU as plotterEngine

parser = argparse.ArgumentParser()

parser.add_argument(
    '--channel',
    default = "MUMU",
    help = ' : channel to merge'
)

parser.add_argument(
    '--input',
    default = "output.root",
    help=' : input root file'
)

args = parser.parse_args()

class Merger:
    def __init__(self, plot_list, case_list, type_list, input_path = "output.root"):

        baseDir = "/pnfs/knu.ac.kr/data/cms/store/user/khwang/CMS/HighMassDY/"
        self.inputFile = baseDir + input_path

        output_path = "./" + input_path.replace(".root", "_merged.root")
        
        os.system(f"cp {self.inputFile} {output_path}")

        self.plot_list = plot_list
        self.case_list = case_list
        self.type_list = type_list
        self.merge_list = plotterEngine.TotalMCList.copy()
        self.merge_list.append("Data")

        self.merge_file = ROOT.TFile(output_path, "UPDATE")

    def SetGenInfo(self, gen_list):
        self.GenInfo = gen_list
    
    def Merge(self):
        self.merge_file.mkdir("merged")
        for sample in self.merge_list:
            self.merge_file.mkdir(f"merged/{sample}")
            
            for atype in self.type_list:
                self.merge_file.mkdir(f"merged/{sample}/{atype}")

                self.p2016_preVFP = plotterEngine.Plotter("2016_preVFP", region = atype, rootPath = self.inputFile)
                self.p2016_postVFP = plotterEngine.Plotter("2016_postVFP", region = atype, rootPath = self.inputFile)
                self.p2017 = plotterEngine.Plotter("2017", region = atype, rootPath = self.inputFile)
                self.p2018 = plotterEngine.Plotter("2018", region = atype, rootPath = self.inputFile)

                for case in self.case_list:

                    self.merge_file.mkdir(f"merged/{sample}/{atype}/{case}")
                    self.merge_file.mkdir(f"merged/{sample}/{atype}/{case}/inc")
                
                    for plot in self.plot_list:

                        if plot == "dimuonMassFailGen" and sample == "Data":
                            continue

                        hist_p2016_preVFP = self.p2016_preVFP.GetSingleHist(plot, sample, case, "inc")
                        hist_p2016_postVFP = self.p2016_postVFP.GetSingleHist(plot, sample, case, "inc")
                        hist_p2017 = self.p2017.GetSingleHist(plot, sample, case, "inc")
                        hist_p2018 = self.p2018.GetSingleHist(plot, sample, case, "inc")

                        hist_merged = hist_p2016_preVFP.Clone(plot + case)
                        hist_merged.SetDirectory(0)
                        hist_merged.Add(hist_p2016_postVFP)
                        hist_merged.Add(hist_p2017)
                        hist_merged.Add(hist_p2018)
                        hist_merged.SetName(f"h_{plot}")

                        self.merge_file.cd(f"merged/{sample}/{atype}/{case}/inc")
                        hist_merged.Write()
                        del hist_merged

                    # if atype == "OS" and sample.contains("NNLO_MUMU"):
                    #     self.merge_file.mkdir(f"merged/{sample}/{case}/GenInfo")

            if "NNLO_MUMU" in sample and args.channel == "MUMU":
                gen_cases = ["", "_0J", "_1J", "_mt1J"]

                self.merge_file.mkdir(f"merged/{sample}/GenInfo")
                for gen_case in gen_cases:
                    for aPlot in self.GenInfo:

                        if gen_case != "" and "Response" in aPlot:
                            continue

                        hist_name = aPlot + gen_case

                        hist_p2016_preVFP = self.p2016_preVFP.GetSingleGenHist(hist_name, sample)
                        hist_p2016_postVFP = self.p2016_postVFP.GetSingleGenHist(hist_name, sample)
                        hist_p2017 = self.p2017.GetSingleGenHist(hist_name, sample)
                        hist_p2018 = self.p2018.GetSingleGenHist(hist_name, sample)

                        hist_merged = hist_p2016_preVFP.Clone(hist_name)
                        hist_merged.Add(hist_p2016_postVFP)
                        hist_merged.Add(hist_p2017)
                        hist_merged.Add(hist_p2018)
                        hist_merged.SetName(f"{aPlot}{gen_case}")

                        self.merge_file.cd(f"merged/{sample}/GenInfo")

                        hist_merged.Write()

        self.merge_file.Close()

def main(args):

    case_list = ["inc", "bVeto", "bVeto_0J", "bVeto_1J", "bVeto_mt1J"]
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

    plot_list_mumu_Gen = [
        "h_ResponseMatrix", 
        "h_nJet", 
        "h_LeadingMuonPt", 
        "h_LeadingMuonEta", 
        "h_LeadingMuonPhi", 
        "h_SubleadingMuonPt", 
        "h_SubleadingMuonEta", 
        "h_SubleadingMuonPhi", 
        "h_MuonPt", 
        "h_MuonEta", 
        "h_MuonPhi", 
        "h_dimuonMass", 
        "h_dimuonPt", 
        "h_dimuonRap", 
    ]

    input_name = args.input

    if args.channel == "MUMU":
        merger = Merger(plot_list_mumu, case_list, type_list, input_path = input_name)
        merger.SetGenInfo(plot_list_mumu_Gen)
    elif args.channel == "EMU":
        merger = Merger(plot_list_emu, case_list, type_list, input_path = input_name)
    else:
        print("Invalid channel")
        return

    merger.Merge()


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    ROOT.TH1.SetDefaultSumw2()

    main(args)
