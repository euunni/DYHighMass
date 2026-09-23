#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import shutil
from pathlib import Path

import plotterEngine_MUMU as plotterEngine

parser = argparse.ArgumentParser()
parser.add_argument('--channel', required=True, choices=['MUMU', 'EMU'], help='Channel to merge')
parser.add_argument('--input', default='output.root', help='Input ROOT filename inside Batch/<channel>')
args = parser.parse_args()

class Merger:
    def __init__(self, plot_list, case_list, type_list, input_path):
        
        input_file = Path(input_path)
        output_path = input_file.with_name(input_file.stem + "_merged.root")
        shutil.copy2(input_file, output_path)

        self.plot_list = plot_list
        self.case_list = case_list
        self.type_list = type_list
        self.merge_list = plotterEngine.TotalMCList.copy()
        self.merge_list.append("Data")
    
        self.p2016_preVFP = plotterEngine.Plotter("2016_preVFP", rootPath = str(input_file))
        self.p2016_postVFP = plotterEngine.Plotter("2016_postVFP", rootPath = str(input_file))
        self.p2017 = plotterEngine.Plotter("2017", rootPath = str(input_file))
        self.p2018 = plotterEngine.Plotter("2018", rootPath = str(input_file))

        self.merge_file = ROOT.TFile(str(output_path), "UPDATE")

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

    case_list = ["", "_0BJ"]
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

    workspace_batch = os.environ.get("DY_HIGHMASS_WORKSPACE_BATCH")
    if workspace_batch is None:
        workspace_batch = Path(__file__).resolve().parents[1] / "Batch"

    input_path = Path(workspace_batch) / args.channel / args.input
    if not input_path.is_file():
        raise FileNotFoundError(f"Input ROOT file not found: {input_path}")

    if args.channel == "MUMU":
        merger = Merger(plot_list_mumu, case_list, type_list, input_path)
    elif args.channel == "EMU":
        merger = Merger(plot_list_emu, case_list, type_list, input_path)
    else:
        print("Invalid channel")
        return

    merger.Merge()


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    ROOT.TH1.SetDefaultSumw2()

    main(args)
