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
        "inc": 1 + 0.18,
        "0J": 1 + 0.18,
        "1J": 1 + 0.24,
        "mtJ": 1 + 0.48,
        "0BJ": 1 + 0.18,
        "1BJ": 1 + 0.48,
        "mt1BJ": 1 + 0.48,
        "bVeto": 1 + 0.18,
        "bVeto_0J": 1 + 0.18,
        "bVeto_1J": 1 + 0.24,
        "bVeto_mt1J": 1 + 0.48 
    }

    yrmin_vec = {
        "inc": 1 - 0.18,
        "0J": 1 - 0.18,
        "1J": 1 - 0.24,
        "mt1J": 1 - 0.48,
        "0BJ": 1 - 0.18,
        "1BJ": 1 - 0.48,
        "mt1BJ": 1 - 0.48,
        "bVeto": 1 - 0.18,
        "bVeto_0J": 1 - 0.18,
        "bVeto_1J": 1 - 0.24,
        "bVeto_mt1J": 1 - 0.48,
    }
 
    for era in eras:

        type_list = ["OS", "OS_inverted", "SS_inverted", "SS"]

        input_file = "./root/261002_MUMU_merged.root"

        for type in type_list:
            plotter = plotterEngine.Plotter(era, 
                                            rootPath = f"{input_file}", 
                                            outputPath = f"./plots_261006/MUMU/{type}/plots_{era}/",
                                            channel = "MUMU", 
                                            region = f"{type}")

            # plotter.SetBackground(rootPath = "./Bck/EMU_FAKE.root", mcList = ["TOP"])
            # plotter.SetFakes(rootPath = "./Bck/MUMU_FAKE.root")

            for case in cases:
                if type == "OS": plotter.Plot("dimuonMass", case, "inc", xTitle = "M(#mu#mu) [GeV]", xmin = 200, xmax = 4000, yrmin = yrmin_vec[case], yrmax = yrmax_vec[case], logy = True, logx = True)
                else: plotter.Plot("dimuonMass", case, "inc", xTitle = "M(#mu#mu) [GeV]", xmin = 200, xmax = 4000, logy = True, logx = True)

                # plotter.Plot("nJet",  case, "", xTitle = "N_{jet}", xmin = 0, xmax = 14)
                # plotter.Plot("nBJet", case, "", xTitle = "N_{b-jet}", xmin = 0, xmax = 14)

                # if type == "OS" and not plotter.HasFakes and not plotter.HasTopBkg:
                #     for massbin in massBins:

                #         plotter.Plot("JetPt", case, massbin               , xTitle = "pT(jet) [GeV]"          ,xmin = 0, xmax = 500, logy = True)
                #         plotter.Plot("JetEta", case, massbin              , xTitle = "#eta(jet)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                #         plotter.Plot("JetPhi", case, massbin              , xTitle = "#phi(jet)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                #         plotter.Plot("BJetPt", case, massbin              , xTitle = "pT(b-jet) [GeV]"          ,xmin = 0, xmax = 500, logy = True)
                #         plotter.Plot("BJetEta", case, massbin             , xTitle = "#eta(b-jet)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                #         plotter.Plot("BJetPhi", case, massbin             , xTitle = "#phi(b-jet)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                #         plotter.Plot("LeadingMuonPt", case, massbin       , xTitle = "pT(#mu) [GeV]"          ,xmin = 15, xmax = 1520, logy = True, logx = True)
                #         plotter.Plot("LeadingMuonEta", case, massbin      , xTitle = "#eta(#mu)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                #         plotter.Plot("LeadingMuonPhi", case, massbin      , xTitle = "#phi(#mu)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                #         plotter.Plot("SubleadingMuonPt", case, massbin    , xTitle = "pT(#mu) [GeV]"          ,xmin = 15, xmax = 1520, logy = True, logx = True)
                #         plotter.Plot("SubleadingMuonEta", case, massbin   , xTitle = "#eta(#mu)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                #         plotter.Plot("SubleadingMuonPhi", case, massbin   , xTitle = "#phi(#mu)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                #         plotter.Plot("MuonPt", case, massbin              , xTitle = "pT(#mu) [GeV]"          ,xmin = 15, xmax = 1520, logy = True, logx = True)
                #         plotter.Plot("MuonEta", case, massbin             , xTitle = "#eta(#mu)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                #         plotter.Plot("MuonPhi", case, massbin             , xTitle = "#phi(#mu)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                #         plotter.Plot("dimuonPt", case, massbin            , xTitle = "pT(#mu#mu) [GeV]" ,xmin = 0, xmax = 500, logy = True)
                #         plotter.Plot("dimuonRap", case, massbin           , xTitle = "rapidity(#mu#mu)"      ,xmin = -2.8, xmax = 2.8, logy = True)


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    ROOT.TH1.SetDefaultSumw2()
    
    main()
