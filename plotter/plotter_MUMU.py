#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine_MUMU as plotterEngine

INPUT_PATH = "./ROOT/MUMU/output.root"
OUTPUT_PATH = "./plots/MUMU"


def main():

    eras = ["2018", "2017", "2016_postVFP", "2016_preVFP"]

    cases = ["", "_0BJ"]
    massBins = [""] 

    addon_hook = {
        "": "",
        "_0BJ": "b-veto",
    }

    yrmax_vec = {
        "": 1 + 0.18,
        "_0BJ": 1 + 0.18,
    }

    yrmin_vec = {
        "": 1 - 0.18,
        "_0BJ": 1 - 0.18,
    }
 
    for era in eras:

        # plotter = plotterEngine.Plotter(era, 
        #                                 rootPath = "./Bck/ROOT/MUMU_OS.root", 
        #                                 outputPath = "./plots/MUMU_OS/plots" + era + "/",
        #                                 channel = "MUMU", 
        #                                 region = "OS")

        # plotter = plotterEngine.Plotter(era, 
        #                                 rootPath = "./valid/subleading_muon_pt/mmos_sl20.root", 
        #                                 outputPath = "./plots/sl20/plots" + era + "/",
        #                                 channel = "MUMU", 
        #                                 region = "OS")

        # plotter = plotterEngine.Plotter(era, 
        #                                 rootPath = f"./Bck/ROOT/MUMU_OS.root", 
        #                                 outputPath = f"./plots/MUMU_Fake/plots" + era + "/",
        #                                 channel = "MUMU", 
        #                                 region = "OS")

        # type_list = ["OS", "OS_inverted", "SS_inverted", "SS"]
        type_list = ["OS"]

        for type in type_list:
            plotter = plotterEngine.Plotter(era, 
                                            rootPath = INPUT_PATH,
                                            outputPath = f"{OUTPUT_PATH}/{type}/era_{era}",
                                            channel = "MUMU", 
                                            region = f"{type}")

            # plotter.SetBackground(rootPath = "./Bck/EMU_FAKE.root", mcList = ["TOP"])
            # plotter.SetFakes(rootPath = "./Bck/MUMU_FAKE.root")
            hasBack = False

            # plotter.Plot("h_nJet",  "", "", xTitle = "N_{jet}", xmin = 0, xmax = 14)
            # plotter.Plot("h_nBJet", "", "", xTitle = "N_{b-jet}", xmin = 0, xmax = 14)

            for case in cases:
                if type == "OS": plotter.Plot("dimuonMass", case, "", xTitle = "M(#mu#mu) [GeV]", xmin = 40, xmax = 3000, yrmin = yrmin_vec[case], yrmax = yrmax_vec[case], logy = True, logx = True)
                else: plotter.Plot("dimuonMass", case, "", xTitle = "M(#mu#mu) [GeV]", xmin = 40, xmax = 3000, logy = True, logx = True)

                # plotter.Plot("nJet",  case, "", xTitle = "N_{jet}", xmin = 0, xmax = 14)
                # plotter.Plot("nBJet", case, "", xTitle = "N_{b-jet}", xmin = 0, xmax = 14)

                # if type == "OS" and not hasBack:
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
