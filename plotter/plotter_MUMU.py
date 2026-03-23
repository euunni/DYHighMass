#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine


def main():

    eras = ["merged", "2018", "2017", "2016_postVFP", "2016_preVFP"]

    # cases = ["", "_0J", "_1J", "_mtJ", "_0BJ", "_1BJ", "_mt1BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]
    cases = ["", "_0BJ", "_bVeto_0J", "_bVeto_1J", "_bVeto_mt1J"]
    # cases = ["_0BJ"]
    # cases = [""]
    # massBins = ["", "_m200_220", "_m220_243", "_m243_273", "_m273_320", "_m320_380", "_m380_440", "_m440_510", "_m510_600", "_m600_700", "_m700_830", "_m830_1000", "_m1000_1500", "_m1500_4000"]
    massBins = [""] 

    addon_hook = {
        "": "",
        "_0J": "N(jet) = 0",
        "_1J": "N(jet) = 1",
        "_mt1J": "N(jet) > 1",
        "_0BJ": "b-veto",
        "_1BJ": "N(b-jet) = 1",
        "_mt1BJ": "N(b-jet) > 1",
        "_bVeto_0J": "b-veto, N(jet) = 0",
        "_bVeto_1J": "b-veto, N(jet) = 1",
        "_bVeto_mt1J": "b-veto, N(jet) > 1",
    }

    yrmax_vec = {
        "": 1 + 0.18,
        "_0J": 1 + 0.18,
        "_1J": 1 + 0.24,
        "_mtJ": 1 + 0.48,
        "_0BJ": 1 + 0.18,
        "_1BJ": 1 + 0.48,
        "_mt1BJ": 1 + 0.48,
        "_bVeto_0J": 1 + 0.18,
        "_bVeto_1J": 1 + 0.24,
        "_bVeto_mt1J": 1 + 0.48 
    }

    yrmin_vec = {
        "": 1 - 0.18,
        "_0J": 1 - 0.18,
        "_1J": 1 - 0.24,
        "_mt1J": 1 - 0.48,
        "_0BJ": 1 - 0.18,
        "_1BJ": 1 - 0.48,
        "_mt1BJ": 1 - 0.48,
        "_bVeto_0J": 1 - 0.18,
        "_bVeto_1J": 1 - 0.24,
        "_bVeto_mt1J": 1 - 0.48,
    }

    addon_hook_mass = {
        "": "M_{#mu#mu} > 200 GeV",
        "_m200_220": "200 < M_{#mu#mu} < 220 GeV",
        "_m220_243": "220 < M_{#mu#mu} < 243 GeV",
        "_m243_273": "243 < M_{#mu#mu} < 273 GeV",
        "_m273_320": "273 < M_{#mu#mu} < 320 GeV",
        "_m320_380": "320 < M_{#mu#mu} < 380 GeV",
        "_m380_440": "380 < M_{#mu#mu} < 440 GeV",
        "_m440_510": "440 < M_{#mu#mu} < 510 GeV",
        "_m510_600": "510 < M_{#mu#mu} < 600 GeV",
        "_m600_700": "600 < M_{#mu#mu} < 700 GeV",
        "_m700_830": "700 < M_{#mu#mu} < 830 GeV",
        "_m830_1000": "830 < M_{#mu#mu} < 1000 GeV",
        "_m1000_1500": "1000 < M_{#mu#mu} < 1500 GeV",
        "_m1500_4000": "1500 < M_{#mu#mu} < 4000 GeV"
    }
 
    for era in eras:
    # def __init__(self, era, rootPath = "output.root", 
    #             outputPath = "./plots/plot_" + era + "/", 
    #             channel = "MUMU", 
    #             region = "OS"):

        # plotter = plotterEngine.Plotter(era, 
        #                                 rootPath = "./Bck/ROOT/MUMU_OS.root", 
        #                                 outputPath = "./plots/MUMU_OS/plots" + era + "/",
        #                                 channel = "MUMU", 
        #                                 region = "OS")

        plotter = plotterEngine.Plotter(era, 
                                        rootPath = "./Bck/ROOT/MUMU_OS.root", 
                                        outputPath = "./plots/MUMU_OS_withFakes/plots" + era + "/",
                                        channel = "MUMU", 
                                        region = "OS")

        # plotter.SetBackground(rootPath = "./Bck/EMU_FAKE.root", mcList = ["TOP"])
        plotter.SetFakes(rootPath = "./Bck/MUMU_FAKE.root")

        for case in cases:
            # plotter.Plot("h_dimuonMass", case, "", xTitle = "M(#mu#mu) [GeV]", xmin = 200, xmax = 4000, yrmin = yrmin_vec[case], yrmax = yrmax_vec[case], logy = True, logx = True)
            plotter.Plot("h_dimuonMass", case, "", xTitle = "M(#mu#mu) [GeV]", xmin = 200, xmax = 4000, yrmin = yrmin_vec[case], yrmax = yrmax_vec[case], logy = True, logx = True)
            
            # for massbin in massBins:

                # plotter.Plot("h_JetPt", case, massbin                 , latex_temp, xTitle = "pT(jet) [GeV]"          ,xmin = 0, xmax = 500, logy = True)
                # plotter.Plot("h_JetEta", case, massbin                , latex_temp, xTitle = "#eta(jet)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                # plotter.Plot("h_JetPhi", case, massbin                , latex_temp, xTitle = "#phi(jet)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                # plotter.Plot("h_BJetPt", case, massbin                , latex_temp, xTitle = "pT(b-jet) [GeV]"          ,xmin = 0, xmax = 500, logy = True)
                # plotter.Plot("h_BJetEta", case, massbin               , latex_temp, xTitle = "#eta(b-jet)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                # plotter.Plot("h_BJetPhi", case, massbin               , latex_temp, xTitle = "#phi(b-jet)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                # plotter.Plot("h_LeadingMuonPt", case, massbin         , latex_temp, xTitle = "pT(#mu) [GeV]"          ,xmin = 15, xmax = 1520, logy = True, logx = True)
                # plotter.Plot("h_LeadingMuonEta", case, massbin        , latex_temp, xTitle = "#eta(#mu)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                # plotter.Plot("h_LeadingMuonPhi", case, massbin        , latex_temp, xTitle = "#phi(#mu)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                # plotter.Plot("h_SubleadingMuonPt", case, massbin      , latex_temp, xTitle = "pT(#mu) [GeV]"          ,xmin = 15, xmax = 1520, logy = True, logx = True)
                # plotter.Plot("h_SubleadingMuonEta", case, massbin     , latex_temp, xTitle = "#eta(#mu)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                # plotter.Plot("h_SubleadingMuonPhi", case, massbin     , latex_temp, xTitle = "#phi(#mu)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)

                # plotter.Plot("h_MuonPt", case, massbin                , latex_temp, xTitle = "pT(#mu) [GeV]"          ,xmin = 15, xmax = 1520, logy = True, logx = True)
                # plotter.Plot("h_MuonEta", case, massbin               , latex_temp, xTitle = "#eta(#mu)"              ,xmin = -2.5, xmax = 2.5, logy = True)
                # plotter.Plot("h_MuonPhi", case, massbin               , latex_temp, xTitle = "#phi(#mu)"              ,xmin = -3.141593, xmax = 3.141593, logy = True)
                # plotter.Plot("h_MuonDeltaR", case, massbin            , latex_temp, xTitle = "#DeltaR(#mu_{1}, #mu_{2})" ,xmin = 0, xmax = 6.4, logy = True)

                # plotter.Plot("h_dimuonPt", case, massbin              , latex_temp, xTitle = "pT(#mu#mu) [GeV]" ,xmin = 0, xmax = 500, logy = True)
                # plotter.Plot("h_dimuonRap", case, massbin             , latex_temp, xTitle = "rapidity(#mu#mu)"      ,xmin = -2.8, xmax = 2.8, logy = True)


if __name__ == "__main__" :
    ROOT.TH1.AddDirectory(False)
    main()
