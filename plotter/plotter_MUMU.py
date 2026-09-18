#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine_MUMU as plotterEngine


def main():

    # eras = ["2016_preVFP"]
    eras = ["2018", "2017", "2016_postVFP", "2016_preVFP", "merged"]

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
        #                                 rootPath = f"./Bck_260512/ROOT/MUMU_OS.root", 
        #                                 outputPath = f"./plots_260514/MUMU_Fake/plots" + era + "/",
        #                                 channel = "MUMU", 
        #                                 region = "OS")

        type_list = ["OS", "OS_inverted", "SS_inverted", "SS"]
        # type_list = ["OS"]

        # input_file = "MUMU_BothInverted"
        # input_file = "MUMU_OneInverted"
        input_file = "./Bck/root/260908_MUMU_Update_merged"

        for type in type_list:
            plotter = plotterEngine.Plotter(era, 
                                            rootPath = f"{input_file}.root", 
                                            outputPath = f"./plots_260917/MUMU_{type}/plots_{era}/",
                                            channel = "MUMU", 
                                            region = f"{type}")

            # plotter.SetBackground(rootPath = "./Bck_260706/EMU_FAKE.root", mcList = ["TOP"])
            # plotter.SetFakes(rootPath = "./Bck/MUMU_FAKE.root")
            hasBack = False

            # plotter.Plot("h_nJet",  "", "", xTitle = "N_{jet}", xmin = 0, xmax = 14)
            # plotter.Plot("h_nBJet", "", "", xTitle = "N_{b-jet}", xmin = 0, xmax = 14)

            for case in cases:
                if type == "OS": plotter.Plot("dimuonMass", case, "", xTitle = "M(#mu#mu) [GeV]", xmin = 200, xmax = 4000, yrmin = yrmin_vec[case], yrmax = yrmax_vec[case], logy = True, logx = True)
                else: plotter.Plot("dimuonMass", case, "", xTitle = "M(#mu#mu) [GeV]", xmin = 200, xmax = 4000, logy = True, logx = True)

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
