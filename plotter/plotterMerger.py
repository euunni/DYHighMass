#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array

import plotterEngine

parser = argparse.ArgumentParser()
parser.add_argument('--channel', help=' : channel to merge')
args = parser.parse_args()

class Merger:
    def __init__(self, plot_list, case_list, input_path = "output.root"):

        output_path = input_path.replace(".root", "_merged.root")
        os.system(f"cp {input_path} {output_path}")

        self.plot_list = plot_list
        self.case_list = case_list
        self.merge_list = plotterEngine.TotalMCList.copy()
        self.merge_list.append("Data")
    
        self.p2016_preVFP = plotterEngine.Plotter("2016_preVFP", rootPath = input_path)
        self.p2016_postVFP = plotterEngine.Plotter("2016_postVFP", rootPath = input_path)
        self.p2017 = plotterEngine.Plotter("2017", rootPath = input_path)
        self.p2018 = plotterEngine.Plotter("2018", rootPath = input_path)

        self.merge_file = ROOT.TFile(output_path, "UPDATE")

    def Merge(self):
        self.merge_file.mkdir("merged")
        for sample in self.merge_list:
            self.merge_file.mkdir(f"merged/{sample}")
            for case in self.case_list:
                if case != "":
                    self.merge_file.mkdir(f"merged/{sample}/{case}")
                for plot in self.plot_list:
                    histname = case + "/" + plot + case
                    if case == "": histname = plot

                    hist_p2016_preVFP = self.p2016_preVFP.GetSingleHist(histname, sample)
                    hist_p2016_postVFP = self.p2016_postVFP.GetSingleHist(histname, sample)
                    hist_p2017 = self.p2017.GetSingleHist(histname, sample)
                    hist_p2018 = self.p2018.GetSingleHist(histname, sample)

                    hist_merged = hist_p2016_preVFP.Clone(plot + case)
                    hist_merged.Add(hist_p2016_postVFP)
                    hist_merged.Add(hist_p2017)
                    hist_merged.Add(hist_p2018)
                    hist_merged.SetName(plot + case)

                    self.merge_file.cd(f"merged/{sample}/{case}")
                    hist_merged.Write()

        self.merge_file.Close()

def main(args):

    case_list = ["", "_0BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]
    
    plot_list_mumu = [
        "h_JetPt",
        "h_JetEta",
        "h_JetPhi",
        "h_BJetPt",
        "h_BJetEta",
        "h_BJetPhi",
        "h_LeadingMuonPt",
        "h_LeadingMuonEta",
        "h_LeadingMuonPhi",
        "h_SubleadingMuonPt",
        "h_SubleadingMuonEta",
        "h_SubleadingMuonPhi",
        "h_MuonPt",
        "h_MuonEta",
        "h_MuonPhi",
        "h_MuonDeltaR",
        "h_dimuonMass",
        "h_dimuonPt",
        "h_dimuonRap",
    ]

    plot_list_emu = [
        "h_JetPt",
        "h_JetEta",
        "h_JetPhi",
        "h_BJetPt",
        "h_BJetEta",
        "h_BJetPhi",
        "h_ElecPt",
        "h_ElecEta",
        "h_ElecPhi",
        "h_MuonPt",
        "h_MuonEta",
        "h_MuonPhi",
        "h_PairMass",
        "h_PairDeltaR",
        "h_PairPt",
        "h_PairRap"
    ]

    if args.channel == "MUMU":
        merger = Merger(plot_list_mumu, case_list)
    elif args.channel == "EMU":
        merger = Merger(plot_list_emu, case_list)
    else:
        print("Invalid channel")
        return

    merger.Merge()


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    ROOT.TH1.SetDefaultSumw2()

    main(args)
