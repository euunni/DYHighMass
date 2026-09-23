#ifndef TriggerEngine_h
#define TriggerEngine_h 1

#include <iostream>
#include <map>
#include <string>

#include "TString.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

class TriggerBase {
  public:
    virtual ~TriggerBase() = default;
    virtual bool PassingTrigger() = 0;
};

class Trigger_MUMU_2016 : public TriggerBase {
  public:
    Trigger_MUMU_2016(TTreeReader* fTreeReader) {
      HLT_IsoMu24 = new TTreeReaderValue<bool>(*fTreeReader, "HLT_IsoMu24");
      HLT_IsoTkMu24 = new TTreeReaderValue<bool>(*fTreeReader, "HLT_IsoTkMu24");

      std::cout << "######################################################################" << std::endl;
      std::cout << "                            Trigger setting                           " << std::endl;
      std::cout << "----------------------------------------------------------------------" << std::endl;
      std::cout << " MUMU 2016        HLT_IsoMu24 || HLT_IsoTkMu24                        " << std::endl;
      std::cout << "######################################################################" << std::endl;
      std::cout << " " << std::endl;
    }
    
    bool PassingTrigger() override {
      return (**HLT_IsoMu24 || **HLT_IsoTkMu24);
    }

  private:
    TTreeReaderValue<bool>* HLT_IsoMu24;
    TTreeReaderValue<bool>* HLT_IsoTkMu24;
};

class Trigger_MUMU_2017 : public TriggerBase {
  public:
    Trigger_MUMU_2017(TTreeReader* fTreeReader) {
      HLT_IsoMu27 = new TTreeReaderValue<bool>(*fTreeReader, "HLT_IsoMu27");
      
      std::cout << "######################################################################" << std::endl;
      std::cout << "                            Trigger setting                           " << std::endl;
      std::cout << "----------------------------------------------------------------------" << std::endl;
      std::cout << " MUMU 2017                    HLT_IsoMu27                              " << std::endl;
      std::cout << "######################################################################" << std::endl;
      std::cout << " " << std::endl;
    }
    
    bool PassingTrigger() override {
      return **HLT_IsoMu27;
    }

  private:
    TTreeReaderValue<bool>* HLT_IsoMu27;
};

class Trigger_MUMU_2018 : public TriggerBase {
  public:
    Trigger_MUMU_2018(TTreeReader* fTreeReader) {
      HLT_IsoMu24 = new TTreeReaderValue<bool>(*fTreeReader, "HLT_IsoMu24");
    
      std::cout << "######################################################################" << std::endl;
      std::cout << "                            Trigger setting                           " << std::endl;
      std::cout << "----------------------------------------------------------------------" << std::endl;
      std::cout << " MUMU 2018                    HLT_IsoMu24                              " << std::endl;
      std::cout << "######################################################################" << std::endl;
      std::cout << " " << std::endl;
    }
    
    bool PassingTrigger() override {
      return **HLT_IsoMu24;
    }
  
  private:
    TTreeReaderValue<bool>* HLT_IsoMu24;
};

class Trigger_EE_2016 : public TriggerBase {
  public:
    
    Trigger_EE_2016(TTreeReader* fTreeReader) {
      HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ = new TTreeReaderValue<bool>(*fTreeReader, "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ");
    
      std::cout << "######################################################################" << std::endl;
      std::cout << "                            Trigger setting                           " << std::endl;
      std::cout << "----------------------------------------------------------------------" << std::endl;
      std::cout << " Trigger_EE_2016       HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ" << std::endl;
      std::cout << "######################################################################" << std::endl;
      std::cout << " " << std::endl;
    }
    
    bool PassingTrigger() override {
      return **HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ;
    }
  
  private:
    TTreeReaderValue<bool>* HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ;
};

class Trigger_EE_20172018 : public TriggerBase {
  public:
    
    Trigger_EE_20172018(TTreeReader* fTreeReader) {
      HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL = new TTreeReaderValue<bool>(*fTreeReader, "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL");
    
      std::cout << "######################################################################" << std::endl;
      std::cout << "                            Trigger setting                           " << std::endl;
      std::cout << "----------------------------------------------------------------------" << std::endl;
      std::cout << " Trigger_EE_20172018      HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL" << std::endl;
      std::cout << "######################################################################" << std::endl;
      std::cout << " " << std::endl;
    }
      
    bool PassingTrigger() override {
      return **HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL;
    }
    
  private:
    TTreeReaderValue<bool>* HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL;
};

class TriggerFactory {
  public:
    static std::unique_ptr<TriggerBase> createTrigger(TString fEra, TString fChannel, TString fSample, int fID, TTreeReader* fTreeReader) {

      if (fChannel == "MUMU") {
        if (fEra == "2016_preVFP" || fEra == "2016_postVFP") return std::make_unique<Trigger_MUMU_2016>(fTreeReader);
        else if (fEra == "2017")                            return std::make_unique<Trigger_MUMU_2017>(fTreeReader);
        else if (fEra == "2018")                            return std::make_unique<Trigger_MUMU_2018>(fTreeReader);
        else {
          std::cerr << "Unknown era: " << fEra << std::endl;
          return nullptr;
        }
      }
      else if (fChannel == "EE") {
        if (fEra == "2018" || fEra == "2017")                       return std::make_unique<Trigger_EE_20172018>(fTreeReader);
        else if (fEra == "2016_preVFP" || fEra == "2016_postVFP")   return std::make_unique<Trigger_EE_2016>(fTreeReader);
        else {
          std::cerr << "Unknown era: " << fEra << std::endl;
          return nullptr;
        }
      } else {
        std::cerr << "Unknown channel: " << fChannel << std::endl;
        return nullptr;
      }
    }
};

#endif
