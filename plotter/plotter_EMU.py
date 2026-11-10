#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine_MUMU as plotterEngine


def main():

    # eras = ["merged"]
    eras = ["2018", "2017", "2016_postVFP", "2016_preVFP", "merged"]

    # cases = ["inc", "bVeto", "bVeto_0J", "bVeto_1J", "bVeto_mt1J", "0J", "1J", "mtJ", "0BJ", "1BJ", "mt1BJ"]
    cases = ["inc", "bVeto", "bVeto_0J", "bVeto_1J", "bVeto_mt1J"]

    # massBins = ["inc", "m200_220", "m220_243", "m243_273", "m273_320", "m320_380", "m380_440", "m440_510", "m510_600", "m600_700", "m700_830", "m830_1000", "m1000_1500", "m1500_4000"]
    massBins = ["inc"] 

    yrmax_vec = {
        "": 1 + 0.18,
        "0J": 1 + 0.18,
        "1J": 1 + 0.24,
        "mtJ": 1 + 0.48,
        "0BJ": 1 + 0.18,
        "1BJ": 1 + 0.48,
        "mt1BJ": 1 + 0.48,
        "bVeto": 1 + 0.18,
        "bVeto_0J": 1 + 0.48,
        "bVeto_1J": 1 + 0.48,
        "bVeto_mt1J": 1 + 0.48 
    }

    yrmin_vec = {
        "": 1 - 0.18,
        "0J": 1 - 0.18,
        "1J": 1 - 0.24,
        "mt1J": 1 - 0.48,
        "0BJ": 1 - 0.18,
        "1BJ": 1 - 0.48,
        "mt1BJ": 1 - 0.48,
        "bVeto": 1 - 0.18,
        "bVeto_0J": 1 - 0.48,
        "bVeto_1J": 1 - 0.48,
        "bVeto_mt1J": 1 - 0.48,
    }
 
    for era in eras:

        type_list = ["OS", "SS", "OS_inverted", "SS_inverted"]
        # type_list = ["OS"]

        input_file = "./root/261002_EMU_merged.root"

        for type in type_list:
            plotter = plotterEngine.Plotter(era, 
                                            rootPath = f"{input_file}", 
                                            outputPath = f"./plots_261006/EMU/{type}/plots_" + era + "/",
                                            channel = "EMU", 
                                            region = f"{type}")

            # plotter.SetFakes(rootPath = "./Bck/EMU_FAKE.root")

            for case in cases:
                # plotter.Plot("h_PairMass", case, "inc", xTitle = "M(e#mu) [GeV]", xmin = 200, xmax = 4000, yrmin = yrmin_vec[case], yrmax = yrmax_vec[case], logy = True, logx = True)
                plotter.Plot("PairMass", case, "inc", xTitle = "M(e#mu) [GeV]", xmin = 200, xmax = 4000, logy = True, logx = True)
                
                # if not plotter.HasFakes:
                #     for massbin in massBins:

                #         plotter.Plot("JetPt", case, massbin                 , xTitle = "pT(jet) [GeV]"          ,xmin = 0, xmax = 500, logy = True)
                #         plotter.Plot("JetEta", case, massbin                , xTitle = "#eta(jet)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                #         plotter.Plot("JetPhi", case, massbin                , xTitle = "#phi(jet)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                #         plotter.Plot("BJetPt", case, massbin                , xTitle = "pT(b-jet) [GeV]"          ,xmin = 0, xmax = 500, logy = True)
                #         plotter.Plot("BJetEta", case, massbin               , xTitle = "#eta(b-jet)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                #         plotter.Plot("BJetPhi", case, massbin               , xTitle = "#phi(b-jet)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                #         plotter.Plot("MuonPt", case, massbin                , xTitle = "pT(#mu) [GeV]"          ,xmin = 15, xmax = 1520, logy = True, logx = True)
                #         plotter.Plot("MuonEta", case, massbin               , xTitle = "#eta(#mu)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                #         plotter.Plot("MuonPhi", case, massbin               , xTitle = "#phi(#mu)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                #         plotter.Plot("ElecPt", case, massbin                , xTitle = "pT(e) [GeV]"          ,xmin = 15, xmax = 1520, logy = True, logx = True)
                #         plotter.Plot("ElecEta", case, massbin               , xTitle = "#eta(e)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                #         plotter.Plot("ElecPhi", case, massbin               , xTitle = "#phi(e)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                #         plotter.Plot("PairPt", case, massbin              , xTitle = "pT(e#mu) [GeV]" ,xmin = 0, xmax = 500, logy = True)
                #         plotter.Plot("PairRap", case, massbin             , xTitle = "rapidity(e#mu)"      ,xmin = -2.8, xmax = 2.8, logy = True)


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    main()
