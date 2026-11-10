#!/usr/bin/env python3

import os, ROOT, sys, pickle, argparse
import uuid
import numpy as np
import cmsstyle as CMS
import array

ROOT.gROOT.SetBatch(ROOT.kTRUE)

TotalMCList = [
    "NNLO_MUMU_10to50",
    "NNLO_MUMU_inc",
    "NNLO_MUMU_100to200",
    "NNLO_MUMU_200to400",
    "NNLO_MUMU_400to500",
    "NNLO_MUMU_500to700",
    "NNLO_MUMU_700to800",
    "NNLO_MUMU_800to1000",
    "NNLO_MUMU_1000to1500",
    "NNLO_MUMU_1500to2000",
    "NNLO_MUMU_2000toInf",

    "NNLO_tautau",

    "ST_s",
    "ST_t_AntiTop",
    "ST_t_Top",
    "ST_tW_AntiTop",
    "ST_tW_Top",

    "TTTo2L2Nu",
    
    "WW",
    "WZ",
    "ZZ",

    "GGToMuMu_10to30_ElEl",
    "GGToMuMu_30to50_ElEl",
    "GGToMuMu_50to200_ElEl",
    "GGToMuMu_200to1500_ElEl",
    "GGToMuMu_1500toInf_ElEl",

    "GGToMuMu_10to30_InelElElInel",
    "GGToMuMu_30to50_InelElElInel",
    "GGToMuMu_50to200_InelElElInel",
    "GGToMuMu_200to1500_InelElElInel",
    "GGToMuMu_1500toInf_InelElElInel",

    "GGToMuMu_10to30_InelInel",
    "GGToMuMu_30to50_InelInel",
    "GGToMuMu_50to200_InelInel",
    "GGToMuMu_200to1500_InelInel",
    "GGToMuMu_1500toInf_InelInel",
]

TWMCList = [
    "TTTo2L2Nu",
    "ST_tW_AntiTop",
    "ST_tW_Top",
    "WW"
]

BkgMCList = [
    "NNLO_tautau",

    "ST_s",
    "ST_t_AntiTop",
    "ST_t_Top",
    "ST_tW_AntiTop",
    "ST_tW_Top",

    "TTTo2L2Nu",
    
    "WW",
    "WZ",
    "ZZ",

    "GGToMuMu_10to30_ElEl",
    "GGToMuMu_30to50_ElEl",
    "GGToMuMu_50to200_ElEl",
    "GGToMuMu_200to1500_ElEl",
    "GGToMuMu_1500toInf_ElEl",

    "GGToMuMu_10to30_InelElElInel",
    "GGToMuMu_30to50_InelElElInel",
    "GGToMuMu_50to200_InelElElInel",
    "GGToMuMu_200to1500_InelElElInel",
    "GGToMuMu_1500toInf_InelElElInel",

    "GGToMuMu_10to30_InelInel",
    "GGToMuMu_30to50_InelInel",
    "GGToMuMu_50to200_InelInel",
    "GGToMuMu_200to1500_InelInel",
    "GGToMuMu_1500toInf_InelInel",
]

DYMCList = [
    "NNLO_MUMU_10to50",
    "NNLO_MUMU_inc",
    "NNLO_MUMU_100to200",
    "NNLO_MUMU_200to400",
    "NNLO_MUMU_400to500",
    "NNLO_MUMU_500to700",
    "NNLO_MUMU_700to800",
    "NNLO_MUMU_800to1000",
    "NNLO_MUMU_1000to1500",
    "NNLO_MUMU_1500to2000",
    "NNLO_MUMU_2000toInf",
]

GGList = [
    "GGToMuMu_10to30_ElEl",
    "GGToMuMu_30to50_ElEl",
    "GGToMuMu_50to200_ElEl",
    "GGToMuMu_200to1500_ElEl",
    "GGToMuMu_1500toInf_ElEl",

    "GGToMuMu_10to30_InelElElInel",
    "GGToMuMu_30to50_InelElElInel",
    "GGToMuMu_50to200_InelElElInel",
    "GGToMuMu_200to1500_InelElElInel",
    "GGToMuMu_1500toInf_InelElElInel",

    "GGToMuMu_10to30_InelInel",
    "GGToMuMu_30to50_InelInel",
    "GGToMuMu_50to200_InelInel",
    "GGToMuMu_200to1500_InelInel",
    "GGToMuMu_1500toInf_InelInel",
]

refLumi = {
    "2016_preVFP": 19.5,
    "2016_postVFP": 16.8,
    "2017": 42.12,
    "2018": 59.45,
    "merged": 137.88
}

xSec = {
    "NNLO_10to50": 7012.53,
    "NNLO_inc": 1925.65,
    "NNLO_100to200": 77.95,
    "NNLO_200to400": 2.78,
    "NNLO_400to500": 0.15,
    "NNLO_500to700": 0.084,
    "NNLO_700to800": 0.013,
    "NNLO_800to1000": 0.011,
    "NNLO_1000to1500": 0.006,
    "NNLO_1500to2000": 0.00081,
    "NNLO_2000toInf": 0.0002,

    "NNLO_MUMU_10to50": 7012.53,
    "NNLO_MUMU_inc": 1925.65,
    "NNLO_MUMU_100to200": 77.95,
    "NNLO_MUMU_200to400": 2.78,
    "NNLO_MUMU_400to500": 0.15,
    "NNLO_MUMU_500to700": 0.084,
    "NNLO_MUMU_700to800": 0.013,
    "NNLO_MUMU_800to1000": 0.011,
    "NNLO_MUMU_1000to1500": 0.006,
    "NNLO_MUMU_1500to2000": 0.00081,
    "NNLO_MUMU_2000toInf": 0.0002,

    "NNLO_EE_10to50": 7012.53,
    "NNLO_EE_inc": 1925.65,
    "NNLO_EE_100to200": 77.95,
    "NNLO_EE_200to400": 2.78,
    "NNLO_EE_400to500": 0.15,
    "NNLO_EE_500to700": 0.084,
    "NNLO_EE_700to800": 0.013,
    "NNLO_EE_800to1000": 0.011,
    "NNLO_EE_1000to1500": 0.006,
    "NNLO_EE_1500to2000": 0.00081,
    "NNLO_EE_2000toInf": 0.0002,
    
    "NNLO_tautau": 1164.31,

    "ST_s": 10.32,
    "ST_t_AntiTop": 80.0,
    "ST_t_Top": 134.2,
    "ST_tW_AntiTop": 39.65,
    "ST_tW_Top": 39.65,
    
    "TTTo2L2Nu": 88.51,
    
    "WJetsToLNu": 61526.7,
    
    "WW": 11.09,
    "WZ": 27.59,
    "ZZ": 12.17,

    "GGToMuMu_10to30_ElEl": 11.39,
    "GGToMuMu_30to50_ElEl": 0.6716,
    "GGToMuMu_50to200_ElEl": 0.2473,
    "GGToMuMu_200to1500_ElEl": 0.005637,
    "GGToMuMu_1500toInf_ElEl": 0.000003813,

    "GGToMuMu_10to30_InelElElInel": 10.52,
    "GGToMuMu_30to50_InelElElInel": 0.8958,
    "GGToMuMu_50to200_InelElElInel": 0.3694,
    "GGToMuMu_200to1500_InelElElInel": 0.0111,
    "GGToMuMu_1500toInf_InelElElInel": 0.000008786,

    "GGToMuMu_10to30_InelInel": 10.40,
    "GGToMuMu_30to50_InelInel": 1.171,
    "GGToMuMu_50to200_InelInel": 0.5615,
    "GGToMuMu_200to1500_InelInel": 0.02173,
    "GGToMuMu_1500toInf_InelInel": 0.00002023,

    "GGToEE_10to30_ElEl": 11.39,
    "GGToEE_30to50_ElEl": 0.6716,
    "GGToEE_50to200_ElEl": 0.2473,
    "GGToEE_200to1500_ElEl": 0.005637,
    "GGToEE_1500toInf_ElEl": 0.000003813,

    "GGToEE_10to30_InelElElInel": 10.52,
    "GGToEE_30to50_InelElElInel": 0.8958,
    "GGToEE_50to200_InelElElInel": 0.3694,
    "GGToEE_200to1500_InelElElInel": 0.0111,
    "GGToEE_1500toInf_InelElElInel": 0.000008786,

    "GGToEE_10to30_InelInel": 10.40,
    "GGToEE_30to50_InelInel": 1.171,
    "GGToEE_50to200_InelInel": 0.5615,
    "GGToEE_200to1500_InelInel": 0.02173,
    "GGToEE_1500toInf_InelInel": 0.00002023,
}


addon_hook_jet = {
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

class Plotter:
    def __init__(self, era, rootPath = "output.root", 
                outputPath = "./plots/plot_temp/", 
                channel = "MUMU", 
                region = "OS"):

        self.rootPath = rootPath
        self.era = era
        self.outputPath = outputPath
        self.channel = channel

        self.HasTopBkg = False
        self.HasFakes = False
        self.IsEMUSignalRegion = False

        os.makedirs(self.outputPath, exist_ok=True)

        self.lumi = refLumi[self.era]
        CMS.SetLumi(self.lumi)

        self.region = region
        if self.region == "OS":
            self.IsSignalRegion = True
        else:
            self.IsSignalRegion = False

        if self.channel == "EMU":
            self.IsSignalRegion = False

        if self.channel == "EMU" and self.region == "OS":
            self.IsEMUSignalRegion = True

        self.plot_addon = [
            self.era + ", #mu#mu channel",
            "p_{T}(#mu) > 52 (50) GeV, |#eta(#mu)| < 2.4",
            "M_{#mu#mu} > 200 GeV",
            "",
        ]

        if self.era == "merged":
            self.plot_addon = [
                "Run2, #mu#mu channel",
                "p_{T}(#mu) > 52 (50) GeV, |#eta(#mu)| < 2.4",
                "M_{#mu#mu} > 200 GeV",
                "",
            ]

        if self.channel == "EMU":
            self.plot_addon[0] = self.era + ", e#mu channel"
            if self.era == "merged":
                self.plot_addon[0] = "Run2, e#mu channel"
            self.plot_addon[1] = "p_{T}(#mu(e)) > 52 (50) GeV, |#eta(#mu(e))| < 2.4 (2.5)"
            self.plot_addon[2] = "M_{e#mu} > 200 GeV"

        if self.region == "OS":          self.plot_addon[0] = self.plot_addon[0] + ", OS"
        if self.region == "OS_inverted": self.plot_addon[0] = self.plot_addon[0] + ", OS, Fake CR region"
        if self.region == "SS":          self.plot_addon[0] = self.plot_addon[0] + ", SS"
        if self.region == "SS_inverted": self.plot_addon[0] = self.plot_addon[0] + ", SS, Fake CR region"

        self.PrepareFiles()

        self.normFactor = {}
        if self.era != "merged":
            self.PrepareNorm()


    def PrepareFiles(self):
        self.fileSet = ROOT.TFile(self.rootPath, "READ");

    def PrepareNorm(self):
        for mcSet in TotalMCList:
            nEvent = self.fileSet.Get(self.era + "/" + mcSet + "/h_EventInfo").GetBinContent(4)
            self.normFactor[mcSet] = (1000. * self.lumi * xSec[mcSet]) / nEvent;

    def SetBackground(self, rootPath, mcList):
        self.bkgRootPath = rootPath
        self.BkgFile = ROOT.TFile(self.bkgRootPath, "READ")
        for mc in mcList:
            if mc == "TOP":
                self.HasTopBkg = True

    def SetFakes(self, rootPath):
        self.fakeRootPath = rootPath
        self.FakeFile = ROOT.TFile(self.fakeRootPath, "READ")
        self.HasFakes = True

    def CheckSanity(self, hist):

        hist_clone = hist.Clone(f"{hist.GetName()}_{uuid.uuid4()}_cl")
        for i in range(1, hist_clone.GetNbinsX() + 1):
            if hist_clone.GetBinContent(i) < 0:
                hist_clone.SetBinContent(i, 0)
                hist_clone.SetBinError(i, 0)
            
        return hist_clone

    def CheckSanity2D(self, hist):

        hist_clone = hist.Clone(f"{hist.GetName()}_{uuid.uuid4()}_cl")
        for i in range(1, hist_clone.GetNbinsX() + 1):
            for j in range(1, hist_clone.GetNbinsY() + 1):
                if hist_clone.GetBinContent(i, j) < 0:
                    hist_clone.SetBinContent(i, j, 0)
                    hist_clone.SetBinError(i, j, 0)
            
        return hist_clone

    def GetMCHist(self, histName, list):
        
        histSet = {}
        for mc in list:
            hist = self.fileSet.Get(self.era + "/" + mc + "/" + histName).Clone(f"{histName}_{uuid.uuid4()}")
            hist.SetDirectory(0)
            hist.SetStats(0);
            # hist.Sumw2();
            if (self.era != "merged"):
                hist.Scale(self.normFactor[mc]);
            hist = self.CheckSanity(hist)

            histSet[mc] = hist

        return_hist = histSet[list[0]].Clone(f"{histName}_{uuid.uuid4()}")
        for mc in list[1:]:
            return_hist.Add(histSet[mc])
        
        return return_hist

    def GetMCHist2D(self, histName, list):
        
        histSet = {}
        for mc in list:
            hist = self.fileSet.Get(self.era + "/" + mc + "/" + histName).Clone(f"{histName}_{uuid.uuid4()}")
            hist.SetDirectory(0)
            hist.SetStats(0);
            # hist.Sumw2();
            if (self.era != "merged"):
                hist.Scale(self.normFactor[mc]);
            hist = self.CheckSanity2D(hist)

            histSet[mc] = hist

        return_hist = histSet[list[0]].Clone(f"{histName}_{uuid.uuid4()}")
        for mc in list[1:]:
            return_hist.Add(histSet[mc])
        
        return return_hist

    def GetSingleHist(self, histname, sample):

        hist = self.fileSet.Get(self.era + "/" + sample + "/" + histname).Clone(f"{histname}_{uuid.uuid4()}")
        hist.SetDirectory(0)
        # hist.Sumw2()
        if sample != "Data":
            hist.Scale(self.normFactor[sample]);
        hist = self.CheckSanity(hist)

        return hist

    def GetDataHist(self, histName):

        hist = self.fileSet.Get(self.era + "/Data/" + histName).Clone(f"{histName}_{uuid.uuid4()}")
        hist.SetDirectory(0)
        hist.SetStats(0);
        
        return hist

    def GetDYGenInfo(self, histName):

        histSet = {}
        for mc in DYMCList:
            hist = self.fileSet.Get(self.era + "/" + mc + "/GenInfo/" + histName).Clone(f"{histName}_{uuid.uuid4()}")
            hist.SetDirectory(0)
            hist.SetStats(0);
            # hist.Sumw2();
            if (self.era != "merged"):
                hist.Scale(self.normFactor[mc]);
            hist = self.CheckSanity(hist)

            histSet[mc] = hist

        return_hist = histSet[list[0]].Clone(f"{histName}_{uuid.uuid4()}")
        for mc in list[1:]:
            return_hist.Add(histSet[mc])
        
        return return_hist

    def GetDataRelStat(self, hist):
        return_hist = ROOT.TGraphErrors()
        for i in range (1, hist.GetNbinsX() + 1):
            if hist.GetBinContent(i) != 0: 
                return_hist.SetPoint(i - 1, hist.GetBinCenter(i), 1.)
                return_hist.SetPointError(i - 1, hist.GetBinWidth(i) / 2., 1. / np.sqrt(hist.GetBinContent(i)))

        return return_hist

    def GetRatioHistNoError(self, num, den):
        ratio = num.Clone(f"ratio_{uuid.uuid4()}")
        ratio.Divide(den)

        for i in range(1, ratio.GetNbinsX() + 1):
            ratio.SetBinError(i, 1e-7)

        return ratio

    def GetRatioHist(self, num, den):
        ratio = num.Clone(f"ratio_{uuid.uuid4()}")
        ratio.Divide(den)

        return ratio

    def GetSignalFraction(self, histName):

        data = self.GetDataHist(histName)
        totalBKG = self.GetMCHist(histName, BkgMCList)

        data_signalOnly = data.Clone(f"{histName}_signalOnly_{uuid.uuid4()}")
        data_signalOnly.Add(totalBKG, -1)

        signalFraction = data_signalOnly.Clone(f"{histName}_signalFraction_{uuid.uuid4()}")
        signalFraction.Divide(data)

        return data_signalOnly, signalFraction

    def GetRatioString(self, data, TotalMC):
        dataN = 0
        totalN = 0
        for i in range(1, data.GetNbinsX() + 1):
            if data.GetBinCenter(i) > 200 and data.GetBinCenter(i) < 4000:
                dataN += data.GetBinContent(i)
                totalN += TotalMC.GetBinContent(i)

        total_ratio = 1
        if totalN == 0:
            total_ratio = 0
        else:
            total_ratio = dataN / totalN

        
        return f"data/Pred. = {(total_ratio):.3f}"

    def GetRatioRange(self, hist, xmin, xmax):
        if self.IsSignalRegion or self.IsEMUSignalRegion:
            residual = -999
            for i in range(1, hist.GetNbinsX() + 1):
                if hist.GetBinCenter(i) > xmin and hist.GetBinCenter(i) < xmax:
                    if hist.GetBinContent(i) > 0.2 and hist.GetBinContent(i) < 1.8:
                        if (residual < abs(1 - hist.GetBinContent(i))):
                            residual = abs(1 - hist.GetBinContent(i))

            if residual < 0.05:
                yrmin = 1 - 0.06
                yrmax = 1 + 0.06

            elif residual < 0.1:
                yrmin = 1 - 0.12
                yrmax = 1 + 0.12
            
            elif residual < 0.15:
                yrmin = 1 - 0.18
                yrmax = 1 + 0.18

            elif residual < 0.2:
                yrmin = 1 - 0.24
                yrmax = 1 + 0.24
            
            elif residual < 0.3:
                yrmin = 1 - 0.36
                yrmax = 1 + 0.36

            elif residual < 0.4:
                yrmin = 1 - 0.48
                yrmax = 1 + 0.48

            else:
                yrmin = 0.5
                yrmax = 1.5

            return yrmin, yrmax
        else:
            residual = -999
            for i in range(1, hist.GetNbinsX() + 1):
                if hist.GetBinCenter(i) > xmin and hist.GetBinCenter(i) < xmax:
                    if hist.GetBinContent(i) < 1.0:
                        if (residual < hist.GetBinContent(i)):
                            residual = hist.GetBinContent(i)

            if residual < 0.05:
                yrmax = 0.06

            elif residual < 0.1:
                yrmax = 0.12
            
            elif residual < 0.15:
                yrmax = 0.18

            elif residual < 0.2:
                yrmax = 0.24
            
            elif residual < 0.3:
                yrmax = 0.36

            elif residual < 0.4:
                yrmax = 0.48

            else:
                yrmax = 0.6

            return 0, yrmax



    def Plot(self, histName, case, massbin, xTitle = "temp X title", 
            xmin = -1, xmax = -1, ymin = -1, ymax = -1, yrmin = -1, yrmax = -1, logy = False, logx = False):
        
        CMS.SetExtraText("Private Work")
        CMS.SetEnergy("13")

        addon = self.plot_addon.copy()
        if case != "":
            addon[2] = f"{addon[2]}, {addon_hook_jet[case]}"

        doAutoXrange = False
        doAutoYrange = False
        doAutoYRatiorange = False

        if (xmin == -1 and xmax == -1): doAutoXrange = True
        if (ymin == -1 and ymax == -1): doAutoYrange = True
        if (yrmin == -1 and yrmax == -1): doAutoYRatiorange = True

        self.histName = ""
        if case != "" and massbin != "":
            self.histName += case + massbin + "/" + histName + case + massbin
        if case != "" and massbin == "":
            self.histName += case + "/" + histName + case
        if case == "" and massbin != "":
            self.histName += massBin + "/" + histName + massBin
        if case == "" and massbin == "":
            self.histName = histName

        self.histOutputName = histName + case + massbin
        canvasName = self.era + "_" + histName

        data = self.GetDataHist(self.histName)
        DY = self.GetMCHist(self.histName, DYMCList)
        TT = self.GetMCHist(self.histName, TWMCList)
        DY_tau = self.GetMCHist(self.histName, ["NNLO_tautau"])
        EW = self.GetMCHist(self.histName, ["WZ", "ZZ"])
        GG = self.GetMCHist(self.histName, GGList)
        Fake = None;

        if self.HasTopBkg:
            TT = self.BkgFile.Get(self.era + "/TOP_MUMU/MUMU_OS_TOP_DataDriven" + case)

        if self.HasFakes:
            if self.channel == "MUMU":
                Fake = self.FakeFile.Get(self.era + "/FAKE_MUMU_OS/MUMU_OS_FAKE_DataDriven" + case)
            if self.channel == "EMU":
                Fake = self.FakeFile.Get(self.era + "/FAKE_EMU_OS/EMU_OS_FAKE_DataDriven" + case)

        TotalMC = DY_tau.Clone(f"{self.histName}_TotalMC_{uuid.uuid4()}")
        TotalMC.Add(GG)
        TotalMC.Add(EW)
        TotalMC.Add(TT)

        if self.IsSignalRegion:
            TotalMC.Add(DY)
        
        if self.HasFakes:
            TotalMC.Add(Fake)

        dataOmc = self.GetRatioHistNoError(data, TotalMC)

        if (not self.IsSignalRegion):
            dataOmc = self.GetRatioHist(TotalMC, data)

        if self.IsEMUSignalRegion:
            dataOmc = self.GetRatioHist(data, TotalMC)

        data_relStat = self.GetDataRelStat(data)

        if doAutoXrange: 
            xmin = dataOmc.GetBinLowEdge(1)
            xmax = dataOmc.GetBinLowEdge(dataOmc.GetNbisX()) + dataOmc.GetBinWidth(dataOmc.GetNbinsX())

        if doAutoYrange:
            if logy:
                ymin = 0.01
                ymax = data.GetMaximum() * 1e5
            else:
                ymin = 0
                ymax = data.GetMaximum() * 1.3

        if doAutoYRatiorange:
            yrmin, yrmax = self.GetRatioRange(dataOmc, xmin, xmax)

        ratioTitle = "Data/Pred."
        if not self.IsSignalRegion: 
            ratioTitle = "Pred./Data"

        totalRatioString = "#font[42]{#scale[2]{" + self.GetRatioString(data, TotalMC) + "}" + "}"

        dicanv = CMS.cmsDiCanvas(
            canvasName,
            xmin,
            xmax,
            ymin,
            ymax,
            yrmin,
            yrmax,
            xTitle,
            "nEvents",
            ratioTitle,
            square = CMS.kSquare,
            extraSpace = 0.1,
            iPos = 0,
        )

        dicanv.cd(1)
        if logy: dicanv.cd(1).SetLogy(True)
        if logx: dicanv.cd(1).SetLogx(True)

        stack = ROOT.THStack("stack", "Stacked")

        leg = CMS.cmsLeg(0.70, 0.89 - 0.05 * 6, 0.89, 0.89, textSize=0.03)

        stackSeet = {
            "DY#rightarrow#tau#tau": DY_tau,
            "#gamma#gamma#rightarrow#mu#mu": GG,
            "ZZ + ZW": EW,
            "tt + tW + WW": TT,
            "DY#rightarrow#mu#mu": DY
        }

        if not self.IsSignalRegion:
            stackSeet = {
                "DY#rightarrow#tau#tau": DY_tau,
                "#gamma#gamma#rightarrow#mu#mu": GG,
                "ZZ + ZW": EW,
                "tt + tW + WW": TT,
            }

        if self.HasFakes and self.IsSignalRegion:
            stackSeet = {
                "Fake": Fake,
                "DY#rightarrow#tau#tau": DY_tau,
                "#gamma#gamma#rightarrow#mu#mu": GG,
                "ZZ + ZW": EW,
                "tt + tW + WW": TT,
                "DY#rightarrow#mu#mu": DY
            }

        if self.HasFakes and not self.IsSignalRegion:
            stackSeet = {
                "Fake": Fake,
                "DY#rightarrow#tau#tau": DY_tau,
                "#gamma#gamma#rightarrow#mu#mu": GG,
                "ZZ + ZW": EW,
                "tt + tW + WW": TT,
            }
        
        CMS.cmsDrawStack(stack, leg, stackSeet, data = data)
        dicanv.cd(1).RedrawAxis();
        
        latex = ROOT.TLatex()
        latex.SetTextAlign(14);
        latex.SetTextSize(0.04);
        latex.SetTextFont(42);

        for idx, addon_text in enumerate(addon):
            latex.DrawLatexNDC(0.18, 0.86 - idx * 0.065, addon_text.encode('utf-8'))

        dicanv.cd(2)
        if logx: dicanv.cd(2).SetLogx(True)
        latex.DrawLatexNDC(0.72, 0.83, totalRatioString.encode('utf-8'))


        if self.IsSignalRegion:
            CMS.cmsDraw(data_relStat, "2", fcolor=ROOT.TColor.GetColor(CMS.petroff_6[0]), fstyle=3002)
            dicanv.cd(2).RedrawAxis()
            
            tmpHist = ROOT.TH1F("tmpHist", "tmpHist", 1, xmin, xmax)
            tmpHist.SetFillColor(ROOT.TColor.GetColor(CMS.petroff_6[0]))
            tmpHist.SetFillStyle(3002)

            leg_ratio = CMS.cmsLeg(0.18, 0.85 - 0.05, 0.3, 0.85, textSize=0.06)
            leg_ratio.AddEntry(tmpHist, "data relative stat. unc", "F")
            leg_ratio.Draw()

        CMS.cmsDraw(dataOmc, "P", mcolor=ROOT.kBlack)

        if self.IsSignalRegion:
            ref_line = ROOT.TLine(xmin, 1, xmax, 1)
            CMS.cmsDrawLine(ref_line, lcolor=ROOT.kRed, lstyle=ROOT.kDotted)

        CMS.SaveCanvas(dicanv, os.path.join(self.outputPath, self.era + "_" + self.histOutputName + ".pdf"))





