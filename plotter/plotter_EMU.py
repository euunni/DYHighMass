#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import cmsstyle as CMS
import array
import plotterEngine_MUMU as plotterEngine

INPUT_PATH = "./ROOT/EMU/output.root"
FAKE_INPUT_PATH = "./Bck/EMU_FAKE.root"
OUTPUT_PATH = "./plots/EMU"


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
        #                                 rootPath = <path_to_root_file>,
        #                                 outputPath = <path_to_output_pdf>,
        #                                 channel = <channel>, "EMU" or "MUMU"
        #                                 region = <region>) "OS", "SS", "OS_inverted", "SS_inverted"

        # plotter = plotterEngine.Plotter(era, 
        #                                 rootPath = f"./Bck_260512/ROOT/EMU_OS.root", 
        #                                 outputPath = f"./plots_260514/EMU_OSwithFake/plots" + era + "/",
        #                                 channel = "EMU", 
        #                                 region = "OS")

        # type_list = ["OS", "SS", "OS_inverted", "SS_inverted"]
        type_list = ["OS"]

        for type in type_list:
            plotter = plotterEngine.Plotter(era, 
                                            rootPath = INPUT_PATH,
                                            outputPath = f"{OUTPUT_PATH}/{type}/era_{era}",
                                            channel = "EMU", 
                                            region = f"{type}")

            plotter.SetFakes(rootPath = FAKE_INPUT_PATH)

            for case in cases:
                plotter.Plot("PairMass", case, "", xTitle = "M(e#mu) [GeV]", xmin = 40, xmax = 3000, logy = True, logx = True)
                
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
