// Script for calculating rate histograms
// Originally from Aaron Bundock
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TChain.h"
#include <iostream>
#include <fstream>
#include <string>
#include "L1Trigger/L1TNtuples/interface/L1AnalysisEventDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisL1UpgradeDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisRecoVertexDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisCaloTPDataFormat.h"
#include "L1Trigger/L1TNtuples/interface/L1AnalysisGeneratorDataFormat.h"

/* TODO: put errors in rates...
creates the the rates and distributions for l1 trigger objects
How to use:
1. input the number of bunches in the run (~line 35)
2. change the variables "newConditionsNtuples" and "oldConditionsNtuples" to ntuple paths
3. If good run JSON is not applied during ntuple production, modify isGoodLumiSection()

Optionally, if you want to rescale to a given instantaneous luminosity:
1. input the instantaneous luminosity of the run (~line 32) [only if we scale to 2016 nominal]
2. select whether you rescale to L=1.5e34 (~line606??...) generally have it setup to rescale
nb: for 2&3 I have provided the info in runInfoForRates.txt
*/

// configurable parameters
double numBunch = 2352; //the number of bunches colliding for the run of interest
double runLum = 0.02; // 0.44: 275783  0.58:  276363 //luminosity of the run of interest (*10^34)
double expectedLum = 1.15; //expected luminosity of 2016 runs (*10^34)

void rates(bool newConditions, const std::string& inputFileDirectory, bool includeHB, bool includeHE1, bool includeHE2, bool includeHF);

int main(int argc, char *argv[])
{
  bool newConditions = true;
  std::string ntuplePath("");

  if (argc != 3) {
    std::cout << "Usage: rates.exe [new/def] [path to ntuples]\n"
	      << "[new/def] indicates new or default (existing) conditions" << std::endl;
    exit(1);
  }
  else {
    std::string par1(argv[1]);
    std::transform(par1.begin(), par1.end(), par1.begin(), ::tolower);
    if(par1.compare("new") == 0) newConditions = true;
    else if(par1.compare("def") == 0) newConditions = false;
    else {
      std::cout << "First parameter must be \"new\" or \"def\"" << std::endl;
      exit(1);
    }
    ntuplePath = argv[2];
  }

  rates(newConditions, ntuplePath, true, true, true, true); // inclusive rates
  rates(newConditions, ntuplePath, true, false, false, false); // HB rates
  rates(newConditions, ntuplePath, false, true, false, false); // eta 1.3 - 2.5 rates
  rates(newConditions, ntuplePath, false, false, true, false); // eta 2.5 - 3.0 rates
  rates(newConditions, ntuplePath, false, false, false, true); // HF rates
  std::cout << "Done with rates " << std::endl;

  return 0;
}

// only need to edit this section if good run JSON
// is not used during ntuple production
bool isGoodLumiSection(int lumiBlock)
{
  if (lumiBlock >= 1
      || lumiBlock <= 10000) {
    return true;
  }

  return false;
}

void rates(bool newConditions, const std::string& inputFileDirectory, bool includeHB, bool includeHE1, bool includeHE2, bool includeHF){
  
  bool hwOn = true;   //are we using data from hardware? (upgrade trigger had to be running!!!)
  bool emuOn = true;  //are we using data from emulator?

  if (hwOn==false && emuOn==false){
    std::cout << "exiting as neither hardware or emulator selected" << std::endl;
    return;
  }

  //PU reweight tree
/*  TFile * f_PUweight = TFile::Open("PUweight.root");
  TH1F * h_PUweight  = (TH1F *) f_PUweight->Get("ratio");
  h_PUweight->SetDirectory(0);
  f_PUweight->Close();*/

  std::string inputFile(inputFileDirectory);
  //inputFile += "/L1rates_*.root";
  std::string outputDirectory = "emu";  //***runNumber, triggerType, version, hw/emu/both***MAKE SURE IT EXISTS
  std::string outputFilename = "rates_def.root";
  std::cout << newConditions << std::endl;
  std::cout << includeHB << "," << includeHE1 << "," << includeHE2 << "," << includeHF << std::endl;
  if(!newConditions && includeHB && !includeHE1 && !includeHE2 && !includeHF){ outputFilename = "rates_def_HB.root";
  }else if(!newConditions && !includeHB && includeHE1 && !includeHE2 && !includeHF){ outputFilename = "rates_def_HE1.root";
  }else if(!newConditions && !includeHB && !includeHE1 && includeHE2 && !includeHF){ outputFilename = "rates_def_HE2.root";
  }else if(!newConditions && !includeHB && !includeHE1 && !includeHE2 && includeHF){ outputFilename = "rates_def_HF.root";}

  if(newConditions && includeHB && includeHE1 && includeHE2 && includeHF){ outputFilename = "rates_new_cond.root";
  }else if(newConditions && includeHB && !includeHE1 && !includeHE2 && !includeHF){ outputFilename = "rates_new_cond_HB.root";
  }else if(newConditions && !includeHB && includeHE1 && !includeHE2 && !includeHF){ outputFilename = "rates_new_cond_HE1.root";
  }else if(newConditions && !includeHB && !includeHE1 && includeHE2 && !includeHF){ outputFilename = "rates_new_cond_HE2.root";
  }else if(newConditions && !includeHB && !includeHE1 && !includeHE2 && includeHF){ outputFilename = "rates_new_cond_HF.root";}
  TFile* kk = TFile::Open( outputFilename.c_str() , "recreate");
  std::cout << outputFilename.c_str() << std::endl;
  // if (kk!=0){
  //   cout << "TERMINATE: not going to overwrite file " << outputFilename << endl;
  //   return;
  // }


  // make trees
  std::cout << "Loading up the TChain..." << std::endl;
  TChain * treeL1emu = new TChain("l1UpgradeEmuTree/L1UpgradeTree");
  if (emuOn){
    treeL1emu->Add(inputFile.c_str());
  }
  TChain * treeL1hw = new TChain("l1UpgradeTree/L1UpgradeTree");
  if (hwOn){
    treeL1hw->Add(inputFile.c_str());
  }
  TChain * eventTree = new TChain("l1EventTree/L1EventTree");
  eventTree->Add(inputFile.c_str());

//  TChain * GeneratorTree = new TChain("l1GeneratorTree/L1GenTree");
//  GeneratorTree->Add(inputFile.c_str());

  // In case you want to include PU info


  TChain * treeL1TPemu = new TChain("l1CaloTowerEmuTree/L1CaloTowerTree");
  if (emuOn){
    treeL1TPemu->Add(inputFile.c_str());
  }

  TChain * treeL1TPhw = new TChain("l1CaloTowerTree/L1CaloTowerTree");
  if (hwOn){
    treeL1TPhw->Add(inputFile.c_str());
  }

  L1Analysis::L1AnalysisL1UpgradeDataFormat    *l1emu_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();
  treeL1emu->SetBranchAddress("L1Upgrade", &l1emu_);
  L1Analysis::L1AnalysisL1UpgradeDataFormat    *l1hw_ = new L1Analysis::L1AnalysisL1UpgradeDataFormat();
  treeL1hw->SetBranchAddress("L1Upgrade", &l1hw_);
  L1Analysis::L1AnalysisEventDataFormat    *event_ = new L1Analysis::L1AnalysisEventDataFormat();
  eventTree->SetBranchAddress("Event", &event_);
//  L1Analysis::L1AnalysisGeneratorDataFormat        *generator_ = new L1Analysis::L1AnalysisGeneratorDataFormat();
//  GeneratorTree->SetBranchAddress("Generator", &generator_);

  L1Analysis::L1AnalysisCaloTPDataFormat    *l1TPemu_ = new L1Analysis::L1AnalysisCaloTPDataFormat();
  treeL1TPemu->SetBranchAddress("CaloTP", &l1TPemu_);
  L1Analysis::L1AnalysisCaloTPDataFormat    *l1TPhw_ = new L1Analysis::L1AnalysisCaloTPDataFormat();
  treeL1TPhw->SetBranchAddress("CaloTP", &l1TPhw_);


  // get number of entries
  Long64_t nentries;
  if (emuOn) nentries = treeL1emu->GetEntries();
  else nentries = treeL1hw->GetEntries();
  int goodLumiEventCount = 0;
  int goodLumiEventCount2 = 0;
  std::cout << "nentries: " << nentries << std::endl;

  std::string outputTxtFilename = "output_rates/" + outputDirectory + "/extraInfo.txt";
  std::ofstream myfile; // save info about the run, including rates for a given lumi section, and number of events we used.
  myfile.open(outputTxtFilename.c_str());
  eventTree->GetEntry(0);
  myfile << "run number = " << event_->run << std::endl;


  // set parameters for histograms
  // jet bins
  //int nJetBins = 199;
  //float jetLo = 1.;
  //float jetHi = 200.;
  //float jetBinWidth = (jetHi-jetLo)/nJetBins;

  // EG bins
  int nEgBins1 = 28;
  Double_t EgEdges1[nEgBins1+1] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 22.0, 24.0, 26.0, 28.0, 30.0, 32.0, 34.0, 36.0, 38.0, 40.0, 45.0, 50.0, 55.0, 60.};
  
  int nEgBins2 = 16;
  Double_t EgEdges2[nEgBins2+1] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 20.0, 30.};
/*  int nEgBins = 160;
  float egLo = 0.;
  float egHi = 60.;
  float egBinWidth = (egHi-egLo)/nEgBins;*/

  // tau bins
  int nTauBins = 79;
  float tauLo = 1.;
  float tauHi = 80.;
  float tauBinWidth = (tauHi-tauLo)/nTauBins;

  // htSum bins
  int nHtSumBins = 200;
  float htSumLo = 0.;
  float htSumHi = 2000.;
  float htSumBinWidth = (htSumHi-htSumLo)/nHtSumBins;

  // mhtSum bins
  int nMhtSumBins = 299;
  float mhtSumLo = 1.;
  float mhtSumHi = 300.;
  float mhtSumBinWidth = (mhtSumHi-mhtSumLo)/nMhtSumBins;

  // etSum bins
  int nEtSumBins = 200;
  float etSumLo = 0.;
  float etSumHi = 1200.;
  float etSumBinWidth = (etSumHi-etSumLo)/nEtSumBins;

//  float metSumBinWidth = (metSumHi-metSumLo)/nMetSumBins;

  // metHFSum bins
  int nMetHFSumBins = 299;
  float metHFSumLo = 1.;
  float metHFSumHi = 300.;
//  float metHFSumBinWidth = (metHFSumHi-metHFSumLo)/nMetHFSumBins;

  //int nJetBins1 = 18;
  //Double_t edges1[nJetBins1+1] = {1.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 50.0, 60.0, 70.0, 200.};
  int nJetBins1 = 30;
  Double_t edges1[nJetBins1+1] = {1.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0, 110.0, 120.0, 130.0, 140.0, 150.0, 160.0, 170.0, 180.0, 190.0, 200.};
/*
  int nJetBins2 = 17;
  Double_t edges2[nJetBins2+1] = {0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 50.0, 60.0, 70.0};
*/
  int nJetBins3 = 23;
  Double_t edges3[nJetBins3+1] = {1.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0, 110.0, 130.0, 200.};

  int nJetBins4 = 18;
  Double_t edges4[nJetBins4+1] = {1.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 50.0, 60., 80., 130.};

  int nMetBins = 32;
  Double_t metEdges1[nMetBins+1] = {0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 45.0, 50.0, 55.0, 60.0, 70.0, 80.0, 90.0, 100.0, 120.0, 140., 160., 180., 200., 220., 240., 260., 280., 300.};

  int nMetHFBins = 32;
  Double_t metEdges2[nMetHFBins+1] = {0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 45.0, 50.0, 55.0, 60.0, 70.0, 80.0, 90.0, 100.0, 120.0, 140., 160., 180., 200., 220., 240., 260., 280., 300.};

  // tp bins
  int nTpBins = 100;
  float tpLo = 0.;
  float tpHi = 100.;

//  float PULo = 30;
//  float PUHi = 80;

  std::string axR = ";Threshold E_{T} (GeV);rate (Hz)";
  std::string axD = ";E_{T} (GeV);events/bin";

//  TH1F* singleJetRates_emu = new TH1F("singleJetRates_emu", axR.c_str(), nJetBins, jetLo, jetHi);
//  TH1F* doubleJetRates_emu = new TH1F("doubleJetRates_emu", axR.c_str(), nJetBins, jetLo, jetHi);
//  TH1F* tripleJetRates_emu = new TH1F("tripleJetRates_emu", axR.c_str(), nJetBins, jetLo, jetHi);
//  TH1F* quadJetRates_emu = new TH1F("quadJetRates_emu", axR.c_str(), nJetBins, jetLo, jetHi);

  TH1F* singleJetRates_emu = new TH1F("singleJetRates_emu", axR.c_str(), nJetBins1, edges1);
  TH1F* doubleJetRates_emu = new TH1F("doubleJetRates_emu", axR.c_str(), nJetBins1, edges1);
  TH1F* tripleJetRates_emu = new TH1F("tripleJetRates_emu", axR.c_str(), nJetBins3, edges3);
  TH1F* quadJetRates_emu = new TH1F("quadJetRates_emu", axR.c_str(), nJetBins4, edges4);

  TH1F* singleJetLLPRates_emu = new TH1F("singleJetLLPRates_emu", axR.c_str(), nJetBins1, edges1);
  TH1F* doubleJetLLPRates_emu = new TH1F("doubleJetLLPRates_emu", axR.c_str(), nJetBins1, edges1);
  TH1F* singleJetLLP_HTT120Rates_emu = new TH1F("singleJetLLP_HTT120Rates_emu", axR.c_str(), nJetBins1, edges1);
  TH1F* singleJetLLP_HTT160Rates_emu = new TH1F("singleJetLLP_HTT160Rates_emu", axR.c_str(), nJetBins1, edges1);
  TH1F* singleJetLLP_HTT200Rates_emu = new TH1F("singleJetLLP_HTT200Rates_emu", axR.c_str(), nJetBins1, edges1);
  TH1F* singleJetLLP_HTT240Rates_emu = new TH1F("singleJetLLP_HTT240Rates_emu", axR.c_str(), nJetBins1, edges1);

  TH1F* singleEgRates_emu = new TH1F("singleEgRates_emu", axR.c_str(), nEgBins1, EgEdges1);
  TH1F* doubleEgRates_emu = new TH1F("doubleEgRates_emu", axR.c_str(), nEgBins2, EgEdges2);
  TH1F* singleTauRates_emu = new TH1F("singleTauRates_emu", axR.c_str(), nTauBins, tauLo, tauHi);
  TH1F* doubleTauRates_emu = new TH1F("doubleTauRates_emu", axR.c_str(), nTauBins, tauLo, tauHi);
  TH1F* singleISOEgRates_emu = new TH1F("singleISOEgRates_emu", axR.c_str(), nEgBins1, EgEdges1);
  TH1F* doubleISOEgRates_emu = new TH1F("doubleISOEgRates_emu", axR.c_str(), nEgBins2, EgEdges2);
  TH1F* singleISOTauRates_emu = new TH1F("singleISOTauRates_emu", axR.c_str(), nTauBins, tauLo, tauHi);
  TH1F* doubleISOTauRates_emu = new TH1F("doubleISOTauRates_emu", axR.c_str(), nTauBins, tauLo, tauHi);
  TH1F* htSumRates_emu = new TH1F("htSumRates_emu",axR.c_str(), nHtSumBins, htSumLo, htSumHi);
  TH1F* htSum_emu = new TH1F("htSum_emu",axR.c_str(), nHtSumBins, htSumLo, htSumHi);
  TH1F* mhtSumRates_emu = new TH1F("mhtSumRates_emu",axR.c_str(), nMhtSumBins, mhtSumLo, mhtSumHi);
  TH1F* etSumRates_emu = new TH1F("etSumRates_emu",axR.c_str(), nEtSumBins, etSumLo, etSumHi);
  TH1F* etSum_emu = new TH1F("etSum_emu",axR.c_str(), nEtSumBins, etSumLo, etSumHi);
//  TH1F* metSumRates_emu = new TH1F("metSumRates_emu",axR.c_str(), nMetSumBins, metSumLo, metSumHi); 
//  TH1F* metHFSumRates_emu = new TH1F("metHFSumRates_emu",axR.c_str(), nMetHFSumBins, metHFSumLo, metHFSumHi); 
  TH1F* nTPs_emu = new TH1F("nTPsRates_emu",";nTPs(E_{T}>=1 GeV); events/bin", 300, 0, 300);
  TH1F* nTPs_hw = new TH1F("nTPsRates_hw",";nTPs(E_{T}>=1 GeV); events/bin", 300, 0, 300);

  TH1F* metSumRates_emu = new TH1F("metSumRates_emu",axR.c_str(), nMetBins, metEdges1); 
  TH1F* metHFSumRates_emu = new TH1F("metHFSumRates_emu",axR.c_str(), nMetHFBins, metEdges2); 


  TH1F* singleJetRates_hw = new TH1F("singleJetRates_hw", axR.c_str(), nJetBins1, edges1);
  TH1F* doubleJetRates_hw = new TH1F("doubleJetRates_hw", axR.c_str(), nJetBins1, edges1);
  TH1F* tripleJetRates_hw = new TH1F("tripleJetRates_hw", axR.c_str(), nJetBins3, edges3);
  TH1F* quadJetRates_hw = new TH1F("quadJetRates_hw", axR.c_str(), nJetBins4, edges4);
  TH1F* singleJetLLPRates_hw = new TH1F("singleJetLLPRates_hw", axR.c_str(), nJetBins1, edges1);
  TH1F* doubleJetLLPRates_hw = new TH1F("doubleJetLLPRates_hw", axR.c_str(), nJetBins1, edges1);
  TH1F* singleJetLLP_HTT120Rates_hw = new TH1F("singleJetLLP_HTT120Rates_hw", axR.c_str(), nJetBins1, edges1);
  TH1F* singleJetLLP_HTT160Rates_hw = new TH1F("singleJetLLP_HTT160Rates_hw", axR.c_str(), nJetBins1, edges1);
  TH1F* singleJetLLP_HTT200Rates_hw = new TH1F("singleJetLLP_HTT200Rates_hw", axR.c_str(), nJetBins1, edges1);
  TH1F* singleJetLLP_HTT240Rates_hw = new TH1F("singleJetLLP_HTT240Rates_hw", axR.c_str(), nJetBins1, edges1);	
  TH1F* singleEgRates_hw = new TH1F("singleEgRates_hw", axR.c_str(), nEgBins1, EgEdges1);
  TH1F* doubleEgRates_hw = new TH1F("doubleEgRates_hw", axR.c_str(), nEgBins2, EgEdges2);
  TH1F* singleTauRates_hw = new TH1F("singleTauRates_hw", axR.c_str(), nTauBins, tauLo, tauHi);
  TH1F* doubleTauRates_hw = new TH1F("doubleTauRates_hw", axR.c_str(), nTauBins, tauLo, tauHi);
  TH1F* singleISOEgRates_hw = new TH1F("singleISOEgRates_hw", axR.c_str(), nEgBins1, EgEdges1);
  TH1F* doubleISOEgRates_hw = new TH1F("doubleISOEgRates_hw", axR.c_str(), nEgBins2, EgEdges2);
  TH1F* singleISOTauRates_hw = new TH1F("singleISOTauRates_hw", axR.c_str(), nTauBins, tauLo, tauHi);
  TH1F* doubleISOTauRates_hw = new TH1F("doubleISOTauRates_hw", axR.c_str(), nTauBins, tauLo, tauHi);
  TH1F* htSumRates_hw = new TH1F("htSumRates_hw",axR.c_str(), nHtSumBins, htSumLo, htSumHi);
  TH1F* mhtSumRates_hw = new TH1F("mhtSumRates_hw",axR.c_str(), nMhtSumBins, mhtSumLo, mhtSumHi);
  TH1F* etSumRates_hw = new TH1F("etSumRates_hw",axR.c_str(), nEtSumBins, etSumLo, etSumHi);
  TH1F* metSumRates_hw = new TH1F("metSumRates_hw",axR.c_str(), nMetHFSumBins, metHFSumLo, metHFSumHi); 
  TH1F* metHFSumRates_hw = new TH1F("metHFSumRates_hw",axR.c_str(), nMetHFSumBins, metHFSumLo, metHFSumHi); 

  TH1F* hcalTP_emu = new TH1F("hcalTP_emu", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi);
  TH1F* ecalTP_emu = new TH1F("ecalTP_emu", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi);

  TH1F* hcalTP_hw = new TH1F("hcalTP_hw", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi);
  TH1F* ecalTP_hw = new TH1F("ecalTP_hw", ";TP E_{T}; # Entries", nTpBins, tpLo, tpHi);

  TH1F* nVtx_histo = new TH1F("nVtx", ";nVtx; # Events", 100, -0.5, 99.5);
//  TH1F* PUweights_histo = new TH1F("PUweights", ";PU weights; # Events", 500, 0, 99.5);

  TH1F* goodLumiEventCount_histo = new TH1F("goodLumiEventCount_histo", ";; # Entries", 1, 0.5, 1.5);
  TH1F* goodLumiEventCount_PUBin0_histo = new TH1F("goodLumiEventCount_PUBin0_histo", ";; # Entries", 1, 0.5, 1.5);
  TH1F* goodLumiEventCount_PUBin1_histo = new TH1F("goodLumiEventCount_PUBin1_histo", ";; # Entries", 1, 0.5, 1.5);
  TH1F* goodLumiEventCount_PUBin2_histo = new TH1F("goodLumiEventCount_PUBin2_histo", ";; # Entries", 1, 0.5, 1.5);

  /////////////////////////////////
  // loop through all the entries//
  /////////////////////////////////
  for (Long64_t jentry=0; jentry<nentries; jentry++){
    if((jentry%10000)==0) std::cout << "Done " << jentry  << " events of " << nentries << std::endl;

    //lumi break clause
    eventTree->GetEntry(jentry);
    //skip the corresponding event
    if (!isGoodLumiSection(event_->lumi)) continue;
    goodLumiEventCount++;
    goodLumiEventCount_histo->Fill(1);

//    if ((vtx_->nVtx < 1) | (vtx_->nVtx > 100)) continue;

    //do routine for L1 emulator quantites
    if (emuOn){

      treeL1hw->GetEntry(jentry);
      treeL1TPemu->GetEntry(jentry);
      double tpEt(0.);
      double tpiEta(0.);


      int nTPs = 0;
      for(int i=0; i < l1TPemu_->nHCALTP; i++){
	tpEt = l1TPemu_->hcalTPet[i];
	tpiEta = l1TPemu_->hcalTPieta[i];
	if (tpEt >= 1 && abs(tpiEta) < 29)nTPs+=1;
	hcalTP_emu->Fill(tpEt);
      }
      nTPs_emu->Fill(nTPs);
      for(int i=0; i < l1TPemu_->nECALTP; i++){
	tpEt = l1TPemu_->ecalTPet[i];
	ecalTP_emu->Fill(tpEt);
      }

      treeL1emu->GetEntry(jentry);
      // get jetEt*, egEt*, tauEt, htSum, mhtSum, etSum, metSum
      // ALL EMU OBJECTS HAVE BX=0...
      double jetEt_1 = 0;
      double jetEt_2 = 0;
      double jetEt_3 = 0;
      double jetEt_4 = 0;

      double jetEt_LLP_1 = 0;
      double jetEt_LLP_2 = 0;	    

      for (UInt_t i=0; i<l1emu_->nJets; i++){
        if(fabs(l1emu_->jetEta[i]) < 1.3){
          if(!includeHB) continue;
        }
        if((fabs(l1emu_->jetEta[i]) >= 1.3) && (fabs(l1emu_->jetEta[i]) < 2.5)){
          if(!includeHE1) continue;
        }
        if((fabs(l1emu_->jetEta[i]) >= 2.5) && (fabs(l1emu_->jetEta[i]) < 3.0)){
          if(!includeHE2) continue;
        }
        if(fabs(l1emu_->jetEta[i]) >= 3.0){
          if(!(includeHF)) continue;
        }


        if (l1emu_->jetEt[i] > jetEt_1){
          jetEt_4 = jetEt_3;
          jetEt_3 = jetEt_2;
          jetEt_2 = jetEt_1;
          jetEt_1 = l1emu_->jetEt[i];
        }
        else if (l1emu_->jetEt[i] <= jetEt_1 && l1emu_->jetEt[i] > jetEt_2){
          jetEt_4 = jetEt_3;
          jetEt_3 = jetEt_2;
          jetEt_2 = l1emu_->jetEt[i];
        }
        else if (l1emu_->jetEt[i] <= jetEt_2 && l1emu_->jetEt[i] > jetEt_3){
          jetEt_4 = jetEt_3;
          jetEt_3 = l1emu_->jetEt[i];
        }
        else if (l1emu_->jetEt[i] <= jetEt_3 && l1emu_->jetEt[i] > jetEt_4){
          jetEt_4 = l1emu_->jetEt[i];
        }

        if (jetEt_LLP_2 > 0) continue;
        if (l1emu_->jetHwQual[i] > 0 && jetEt_LLP_1 > 0) jetEt_LLP_2 = l1emu_->jetEt[i];
        if (l1emu_->jetHwQual[i] > 0 && jetEt_LLP_1 == 0) jetEt_LLP_1 = l1emu_->jetEt[i];	      
      }

      double egEt_1 = 0;
      double egEt_2 = 0;
      //EG pt's are not given in descending order...bx?
      for (UInt_t c=0; c<l1emu_->nEGs; c++){
        if(fabs(l1emu_->egEta[c]) < 1.3){
          if(!includeHB) continue;
        }
        if((fabs(l1emu_->egEta[c]) >= 1.3) && (fabs(l1emu_->egEta[c]) < 2.5)){
          if(!includeHE1) continue;
        }
        if((fabs(l1emu_->egEta[c]) >= 2.5) && (fabs(l1emu_->egEta[c]) < 3.0)){
          if(!includeHE2) continue;
        }
        if(fabs(l1emu_->egEta[c]) >= 3.0){
          if(!(includeHF)) continue;
        }

        if (l1emu_->egEt[c] > egEt_1){
          egEt_2 = egEt_1;
          egEt_1 = l1emu_->egEt[c];
        }
        else if (l1emu_->egEt[c] <= egEt_1 && l1emu_->egEt[c] > egEt_2){
          egEt_2 = l1emu_->egEt[c];
        }
      }

      double tauEt_1 = 0;
      double tauEt_2 = 0;
      //tau pt's are not given in descending order
      for (UInt_t c=0; c<l1emu_->nTaus; c++){
        if(fabs(l1emu_->tauEta[c]) < 1.3){
          if(!includeHB) continue;
        }
        if((fabs(l1emu_->tauEta[c]) >= 1.3) && (fabs(l1emu_->tauEta[c]) < 2.5)){
          if(!includeHE1) continue;
        }
        if((fabs(l1emu_->tauEta[c]) >= 2.5) && (fabs(l1emu_->tauEta[c]) < 3.0)){
          if(!includeHE2) continue;
        }
        if(fabs(l1emu_->tauEta[c]) >= 3.0){
          if(!(includeHF)) continue;
        }

        if (l1emu_->tauEt[c] > tauEt_1){
          tauEt_2 = tauEt_1;
          tauEt_1 = l1emu_->tauEt[c];
        }
        else if (l1emu_->tauEt[c] <= tauEt_1 && l1emu_->tauEt[c] > tauEt_2){
          tauEt_2 = l1emu_->tauEt[c];
        }
      }

      double egISOEt_1 = 0;
      double egISOEt_2 = 0;
      //EG pt's are not given in descending order...bx?
      for (UInt_t c=0; c<l1emu_->nEGs; c++){
        if(fabs(l1emu_->egEta[c]) < 1.3){
          if(!includeHB) continue;
        }
        if((fabs(l1emu_->egEta[c]) >= 1.3) && (fabs(l1emu_->egEta[c]) < 2.5)){
          if(!includeHE1) continue;
        }
        if((fabs(l1emu_->egEta[c]) >= 2.5) && (fabs(l1emu_->egEta[c]) < 3.0)){
          if(!includeHE2) continue;
        }
        if(fabs(l1emu_->egEta[c]) >= 3.0){
          if(!(includeHF)) continue;
        }

        if (l1emu_->egEt[c] > egISOEt_1 && l1emu_->egIso[c]>0){
          egISOEt_2 = egISOEt_1;
          egISOEt_1 = l1emu_->egEt[c];
        }
        else if (l1emu_->egEt[c] <= egISOEt_1 && l1emu_->egEt[c] > egISOEt_2 && l1emu_->egIso[c]>0){
          egISOEt_2 = l1emu_->egEt[c];
        }
      }

      double tauISOEt_1 = 0;
      double tauISOEt_2 = 0;
      //tau pt's are not given in descending order
      for (UInt_t c=0; c<l1emu_->nTaus; c++){
        if(fabs(l1emu_->tauEta[c]) < 1.3){
          if(!includeHB) continue;
        }
        if((fabs(l1emu_->tauEta[c]) >= 1.3) && (fabs(l1emu_->tauEta[c]) < 2.5)){
          if(!includeHE1) continue;
        }
        if((fabs(l1emu_->tauEta[c]) >= 2.5) && (fabs(l1emu_->tauEta[c]) < 3.0)){
          if(!includeHE2) continue;
        }
        if(fabs(l1emu_->tauEta[c]) >= 3.0){
          if(!(includeHF)) continue;
        }

        if (l1emu_->tauEt[c] > tauISOEt_1 && l1emu_->tauIso[c]>0){
          tauISOEt_2 = tauISOEt_1;
          tauISOEt_1 = l1emu_->tauEt[c];
        }
        else if (l1emu_->tauEt[c] <= tauISOEt_1 && l1emu_->tauEt[c] > tauISOEt_2 && l1emu_->tauIso[c]>0){
          tauISOEt_2 = l1emu_->tauEt[c];
        }
      }

      double htSum(0.0);
      double mhtSum(0.0);
      double etSum(0.0);
      double metSum(0.0);
      double metHFSum(0.0);
      for (unsigned int c=0; c<l1emu_->nSums; c++){
          if( l1emu_->sumBx[c] != 0 ) continue;
          if( l1emu_->sumType[c] == L1Analysis::kTotalEt ) etSum = l1emu_->sumEt[c];
          if( l1emu_->sumType[c] == L1Analysis::kTotalHt ) htSum = l1emu_->sumEt[c];
          if( l1emu_->sumType[c] == L1Analysis::kMissingEt ) metSum = l1emu_->sumEt[c];
	  if( l1emu_->sumType[c] == L1Analysis::kMissingEtHF ) metHFSum = l1emu_->sumEt[c];
          if( l1emu_->sumType[c] == L1Analysis::kMissingHt ) mhtSum = l1emu_->sumEt[c];
      }

      // for each bin fill according to whether our object has a larger corresponding energy
/*      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_1) >= jetLo + (bin*jetBinWidth) ) singleJetRates_emu->Fill(jetLo+(bin*jetBinWidth));  //GeV
      } 

      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_2) >= jetLo + (bin*jetBinWidth) ) doubleJetRates_emu->Fill(jetLo+(bin*jetBinWidth));  //GeV
      }  

      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_3) >= jetLo + (bin*jetBinWidth) ) tripleJetRates_emu->Fill(jetLo+(bin*jetBinWidth));  //GeV
      }  

      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_4) >= jetLo + (bin*jetBinWidth) ) quadJetRates_emu->Fill(jetLo+(bin*jetBinWidth));  //GeV
      } */


      for(int bin=0; bin<nJetBins1; bin++){
        if( (jetEt_1) >= edges1[bin] ) singleJetRates_emu->Fill(edges1[bin]);  //GeV
      } 

      for(int bin=0; bin<nJetBins1; bin++){
        if( (jetEt_2) >= edges1[bin] ) doubleJetRates_emu->Fill(edges1[bin]);  //GeV
      }  

      for(int bin=0; bin<nJetBins3; bin++){
        if( (jetEt_3) >= edges3[bin] ) tripleJetRates_emu->Fill(edges3[bin]);  //GeV
      }  

      for(int bin=0; bin<nJetBins4; bin++){
        if( (jetEt_4) >= edges4[bin] ) quadJetRates_emu->Fill(edges4[bin]);  //GeV
      } 

      for(int bin=0; bin<nJetBins1; bin++){
        if( (jetEt_LLP_1) >= edges1[bin] ) singleJetLLPRates_emu->Fill(edges1[bin]);  //GeV
      }
	    
      for(int bin=0; bin<nJetBins1; bin++){
        if( (jetEt_LLP_2) >= edges1[bin] ) doubleJetLLPRates_emu->Fill(edges1[bin]);  //GeV
      }

      for(int bin=0; bin<nJetBins1; bin++){
        if( htSum >= 120){
          if( (jetEt_LLP_1) >= edges1[bin] ) singleJetLLP_HTT120Rates_emu->Fill(edges1[1]);  //GeV
        }
	if( htSum >= 160){
          if( (jetEt_LLP_1) >= edges1[bin] ) singleJetLLP_HTT160Rates_emu->Fill(edges1[1]);  //GeV
        }
	if( htSum >= 200){
          if( (jetEt_LLP_1) >= edges1[bin] ) singleJetLLP_HTT200Rates_emu->Fill(edges1[1]);  //GeV
        }
	if( htSum >= 240){
          if( (jetEt_LLP_1) >= edges1[bin] ) singleJetLLP_HTT240Rates_emu->Fill(edges1[1]);  //GeV
        }	
      }
	    
      for(int bin=0; bin<nEgBins1; bin++){
        if( (egEt_1) >= EgEdges1[bin] ) singleEgRates_emu->Fill(EgEdges1[bin]);  //GeV
      } 

      for(int bin=0; bin<nEgBins2; bin++){
        if( (egEt_2) >= EgEdges2[bin] ) doubleEgRates_emu->Fill(EgEdges2[bin]);  //GeV
      }  

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauEt_1) >= tauLo + (bin*tauBinWidth) ) singleTauRates_emu->Fill(tauLo+(bin*tauBinWidth));  //GeV
      }

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauEt_2) >= tauLo + (bin*tauBinWidth) ) doubleTauRates_emu->Fill(tauLo+(bin*tauBinWidth));  //GeV
      } 

      for(int bin=0; bin<nEgBins1; bin++){
        if( (egISOEt_1) >= EgEdges1[bin] ) singleISOEgRates_emu->Fill(EgEdges1[bin]);  //GeV
      } 

      for(int bin=0; bin<nEgBins2; bin++){
        if( (egISOEt_2) >= EgEdges2[bin] ) doubleISOEgRates_emu->Fill(EgEdges2[bin]);  //GeV
      }  

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauISOEt_1) >= tauLo + (bin*tauBinWidth) ) singleISOTauRates_emu->Fill(tauLo+(bin*tauBinWidth));  //GeV
      }

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauISOEt_2) >= tauLo + (bin*tauBinWidth) ) doubleISOTauRates_emu->Fill(tauLo+(bin*tauBinWidth));  //GeV
      } 

      for(int bin=0; bin<nHtSumBins; bin++){
        if( (htSum) >= htSumLo+(bin*htSumBinWidth) ) htSumRates_emu->Fill(htSumLo+(bin*htSumBinWidth)); //GeV
      }

      htSum_emu->Fill(htSum);
      etSum_emu->Fill(etSum);

      for(int bin=0; bin<nMhtSumBins; bin++){
        if( (mhtSum) >= mhtSumLo+(bin*mhtSumBinWidth) ) mhtSumRates_emu->Fill(mhtSumLo+(bin*mhtSumBinWidth)); //GeV           
      }

      for(int bin=0; bin<nEtSumBins; bin++){
        if( (etSum) >= etSumLo+(bin*etSumBinWidth) ) etSumRates_emu->Fill(etSumLo+(bin*etSumBinWidth)); //GeV           
      }
/*
      for(int bin=0; bin<nMetSumBins; bin++){
        if( (metSum) >= metSumLo+(bin*metSumBinWidth) ) metSumRates_emu->Fill(metSumLo+(bin*metSumBinWidth)); //GeV           
      }
      for(int bin=0; bin<nMetHFSumBins; bin++){
        if( (metHFSum) >= metHFSumLo+(bin*metHFSumBinWidth) ) metHFSumRates_emu->Fill(metHFSumLo+(bin*metHFSumBinWidth)); //GeV           
      }
*/
      for(int bin=0; bin<nMetBins; bin++){
        if( (metSum) >= metEdges1[bin] ) metSumRates_emu->Fill(metEdges1[bin]); //GeV           
      }
      for(int bin=0; bin<nMetHFBins; bin++){
        if( (metHFSum) >= metEdges2[bin] ) metHFSumRates_emu->Fill(metEdges2[bin]); //GeV           
      }


    }// closes if 'emuOn' is true

  }// closes loop through events

  /////////////////////////////////
  // loop through all the entries//
  /////////////////////////////////
  for (Long64_t jentry=0; jentry<treeL1hw->GetEntries(); jentry++){

    //lumi break clause
    eventTree->GetEntry(jentry);
    //skip the corresponding event
    if (!isGoodLumiSection(event_->lumi)) continue;
    goodLumiEventCount2++;

//    if ((vtx_->nVtx < 25) | (vtx_->nVtx > 50)) continue;
    //do routine for L1 hardware quantities
    if (hwOn){

//      double PUweight = h_PUweight->GetBinContent(vtx_->nVtx - 1);
      double PUweight = 1.;
      treeL1TPhw->GetEntry(jentry);
      double tpEt(0.);
      double tpiEta(0.);
      
      int nTPs = 0;
      for(int i=0; i < l1TPhw_->nHCALTP; i++){
	tpEt = l1TPhw_->hcalTPet[i];
	tpiEta = l1TPhw_->hcalTPieta[i];
	if (tpEt >= 1 && abs(tpiEta) < 29)nTPs+=1;
	hcalTP_hw->Fill(tpEt);
      }
      nTPs_hw->Fill(nTPs);
      for(int i=0; i < l1TPhw_->nECALTP; i++){
	tpEt = l1TPhw_->ecalTPet[i];
	ecalTP_hw->Fill(tpEt);
      }

      treeL1hw->GetEntry(jentry);
      // get jetEt*, egEt*, tauEt, htSum, mhtSum, etSum, metSum
      // ALL EMU OBJECTS HAVE BX=0...
      double jetEt_1 = 0;
      double jetEt_2 = 0;
      double jetEt_3 = 0;
      double jetEt_4 = 0;

      double jetEt_LLP_1 = 0;
      double jetEt_LLP_2 = 0;	    

      for (UInt_t i=0; i<l1hw_->nJets; i++){
        if(fabs(l1hw_->jetEta[i]) < 1.3){
          if(!includeHB) continue;
        }
        if((fabs(l1hw_->jetEta[i]) >= 1.3) && (fabs(l1hw_->jetEta[i]) < 2.5)){
          if(!includeHE1) continue;
        }
        if((fabs(l1hw_->jetEta[i]) >= 2.5) && (fabs(l1hw_->jetEta[i]) < 3.0)){
          if(!includeHE2) continue;
        }
        if(fabs(l1hw_->jetEta[i]) >= 3.0){
          if(!(includeHF)) continue;
        }


        if (l1hw_->jetEt[i] > jetEt_1){
          jetEt_4 = jetEt_3;
          jetEt_3 = jetEt_2;
          jetEt_2 = jetEt_1;
          jetEt_1 = l1hw_->jetEt[i];
        }
        else if (l1hw_->jetEt[i] <= jetEt_1 && l1hw_->jetEt[i] > jetEt_2){
          jetEt_4 = jetEt_3;
          jetEt_3 = jetEt_2;
          jetEt_2 = l1hw_->jetEt[i];
        }
        else if (l1hw_->jetEt[i] <= jetEt_2 && l1hw_->jetEt[i] > jetEt_3){
          jetEt_4 = jetEt_3;
          jetEt_3 = l1hw_->jetEt[i];
        }
        else if (l1hw_->jetEt[i] <= jetEt_3 && l1hw_->jetEt[i] > jetEt_4){
          jetEt_4 = l1hw_->jetEt[i];
        }

        if (l1hw_->jetBx[i]==0 && l1hw_->jetHwQual[i] > 0 && l1hw_->jetEt[i] > jetEt_LLP_1){
          jetEt_LLP_2 = jetEt_LLP_1;
          jetEt_LLP_1 = l1hw_->jetEt[i];
        }
        else if (l1hw_->jetBx[i]==0 && l1hw_->jetHwQual[i] > 0 && l1hw_->jetEt[i] <= jetEt_LLP_1 && l1hw_->jetEt[i] > jetEt_LLP_2) jetEt_LLP_2 = l1hw_->jetEt[i];
	      
      }

      double egEt_1 = 0;
      double egEt_2 = 0;
      //EG pt's are not given in descending order...bx?
      for (UInt_t c=0; c<l1hw_->nEGs; c++){
        if(fabs(l1hw_->egEta[c]) < 1.3){
          if(!includeHB) continue;
        }
        if((fabs(l1hw_->egEta[c]) >= 1.3) && (fabs(l1hw_->egEta[c]) < 2.5)){
          if(!includeHE1) continue;
        }
        if((fabs(l1hw_->egEta[c]) >= 2.5) && (fabs(l1hw_->egEta[c]) < 3.0)){
          if(!includeHE2) continue;
        }
        if(fabs(l1hw_->egEta[c]) >= 3.0){
          if(!(includeHF)) continue;
        }

        if (l1hw_->egEt[c] > egEt_1){
          egEt_2 = egEt_1;
          egEt_1 = l1hw_->egEt[c];
        }
        else if (l1hw_->egEt[c] <= egEt_1 && l1hw_->egEt[c] > egEt_2){
          egEt_2 = l1hw_->egEt[c];
        }
      }

      double tauEt_1 = 0;
      double tauEt_2 = 0;
      //tau pt's are not given in descending order
      for (UInt_t c=0; c<l1hw_->nTaus; c++){
        if(fabs(l1hw_->tauEta[c]) < 1.3){
          if(!includeHB) continue;
        }
        if((fabs(l1hw_->tauEta[c]) >= 1.3) && (fabs(l1hw_->tauEta[c]) < 2.5)){
          if(!includeHE1) continue;
        }
        if((fabs(l1hw_->tauEta[c]) >= 2.5) && (fabs(l1hw_->tauEta[c]) < 3.0)){
          if(!includeHE2) continue;
        }
        if(fabs(l1hw_->tauEta[c]) >= 3.0){
          if(!(includeHF)) continue;
        }

        if (l1hw_->tauEt[c] > tauEt_1){
          tauEt_2 = tauEt_1;
          tauEt_1 = l1hw_->tauEt[c];
        }
        else if (l1hw_->tauEt[c] <= tauEt_1 && l1hw_->tauEt[c] > tauEt_2){
          tauEt_2 = l1hw_->tauEt[c];
        }
      }

      double egISOEt_1 = 0;
      double egISOEt_2 = 0;
      //EG pt's are not given in descending order...bx?
      for (UInt_t c=0; c<l1hw_->nEGs; c++){
        if(fabs(l1hw_->egEta[c]) < 1.3){
          if(!includeHB) continue;
        }
        if((fabs(l1hw_->egEta[c]) >= 1.3) && (fabs(l1hw_->egEta[c]) < 2.5)){
          if(!includeHE1) continue;
        }
        if((fabs(l1hw_->egEta[c]) >= 2.5) && (fabs(l1hw_->egEta[c]) < 3.0)){
          if(!includeHE2) continue;
        }
        if(fabs(l1hw_->egEta[c]) >= 3.0){
          if(!(includeHF)) continue;
        }

        if (l1hw_->egEt[c] > egISOEt_1 && l1hw_->egIso[c]>0){
          egISOEt_2 = egISOEt_1;
          egISOEt_1 = l1hw_->egEt[c];
        }
        else if (l1hw_->egEt[c] <= egISOEt_1 && l1hw_->egEt[c] > egISOEt_2 && l1hw_->egIso[c]>0){
          egISOEt_2 = l1hw_->egEt[c];
        }
      }

      double tauISOEt_1 = 0;
      double tauISOEt_2 = 0;
      //tau pt's are not given in descending order
      for (UInt_t c=0; c<l1hw_->nTaus; c++){
        if(fabs(l1hw_->tauEta[c]) < 1.3){
          if(!includeHB) continue;
        }
        if((fabs(l1hw_->tauEta[c]) >= 1.3) && (fabs(l1hw_->tauEta[c]) < 2.5)){
          if(!includeHE1) continue;
        }
        if((fabs(l1hw_->tauEta[c]) >= 2.5) && (fabs(l1hw_->tauEta[c]) < 3.0)){
          if(!includeHE2) continue;
        }
        if(fabs(l1hw_->tauEta[c]) >= 3.0){
          if(!(includeHF)) continue;
        }

        if (l1hw_->tauEt[c] > tauISOEt_1 && l1hw_->tauIso[c]>0){
          tauISOEt_2 = tauISOEt_1;
          tauISOEt_1 = l1hw_->tauEt[c];
        }
        else if (l1hw_->tauEt[c] <= tauISOEt_1 && l1hw_->tauEt[c] > tauISOEt_2 && l1hw_->tauIso[c]>0){
          tauISOEt_2 = l1hw_->tauEt[c];
        }
      }

      double htSum(0.0);
      double mhtSum(0.0);
      double etSum(0.0);
      double metSum(0.0);
      double metHFSum(0.0);
      for (unsigned int c=0; c<l1hw_->nSums; c++){
          if( l1hw_->sumBx[c] != 0 ) continue;
          if( l1hw_->sumType[c] == L1Analysis::kTotalEt ) etSum = l1hw_->sumEt[c];
          if( l1hw_->sumType[c] == L1Analysis::kTotalHt ) htSum = l1hw_->sumEt[c];
          if( l1hw_->sumType[c] == L1Analysis::kMissingEt ) metSum = l1hw_->sumEt[c];
	  if( l1hw_->sumType[c] == L1Analysis::kMissingEtHF ) metHFSum = l1hw_->sumEt[c];
          if( l1hw_->sumType[c] == L1Analysis::kMissingHt ) mhtSum = l1hw_->sumEt[c];
      }

      // for each bin fill according to whether our object has a larger corresponding energy

/*
      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_1) >= jetLo + (bin*jetBinWidth) ) singleJetRates_hw->Fill(jetLo+(bin*jetBinWidth));  //GeV
      } 

      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_2) >= jetLo + (bin*jetBinWidth) ) doubleJetRates_hw->Fill(jetLo+(bin*jetBinWidth));  //GeV
      }  

      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_3) >= jetLo + (bin*jetBinWidth) ) tripleJetRates_hw->Fill(jetLo+(bin*jetBinWidth));  //GeV
      }  

      for(int bin=0; bin<nJetBins; bin++){
        if( (jetEt_4) >= jetLo + (bin*jetBinWidth) ) quadJetRates_hw->Fill(jetLo+(bin*jetBinWidth));  //GeV
      }  
*/

      for(int bin=0; bin<nJetBins1; bin++){
        if( (jetEt_1) >= edges1[bin] ) singleJetRates_hw->Fill(edges1[bin]);  //GeV
      } 

      for(int bin=0; bin<nJetBins1; bin++){
        if( (jetEt_2) >= edges1[bin] ) doubleJetRates_hw->Fill(edges1[bin]);  //GeV
      }  

      for(int bin=0; bin<nJetBins3; bin++){
        if( (jetEt_3) >= edges3[bin] ) tripleJetRates_hw->Fill(edges3[bin]);  //GeV
      }  

      for(int bin=0; bin<nJetBins4; bin++){
        if( (jetEt_4) >= edges4[bin] ) quadJetRates_hw->Fill(edges4[bin]);  //GeV
      }  

      for(int bin=0; bin<nJetBins1; bin++){
        if( (jetEt_LLP_1) >= edges1[bin] ) singleJetLLPRates_hw->Fill(edges1[bin]);  //GeV
      }
	    
      for(int bin=0; bin<nJetBins1; bin++){
        if( (jetEt_LLP_2) >= edges1[bin] ) doubleJetLLPRates_hw->Fill(edges1[bin]);  //GeV
      }

      for(int bin=0; bin<nJetBins1; bin++){
        if( htSum >= 120){
          if( (jetEt_LLP_1) >= edges1[bin] ) singleJetLLP_HTT120Rates_hw->Fill(edges1[1]);  //GeV
        }
	if( htSum >= 160){
          if( (jetEt_LLP_1) >= edges1[bin] ) singleJetLLP_HTT160Rates_hw->Fill(edges1[1]);  //GeV
        }
	if( htSum >= 200){
          if( (jetEt_LLP_1) >= edges1[bin] ) singleJetLLP_HTT200Rates_hw->Fill(edges1[1]);  //GeV
        }
	if( htSum >= 240){
          if( (jetEt_LLP_1) >= edges1[bin] ) singleJetLLP_HTT240Rates_hw->Fill(edges1[1]);  //GeV
        }	
      }	    
            
      for(int bin=0; bin<nEgBins1; bin++){
        if( (egEt_1) >= EgEdges1[bin] ) singleEgRates_hw->Fill(EgEdges1[bin],PUweight);  //GeV
      } 

      for(int bin=0; bin<nEgBins2; bin++){
        if( (egEt_2) >= EgEdges2[bin] ) doubleEgRates_hw->Fill(EgEdges2[bin],PUweight);  //GeV
      }  

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauEt_1) >= tauLo + (bin*tauBinWidth) ) singleTauRates_hw->Fill(tauLo+(bin*tauBinWidth));  //GeV
      }

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauEt_2) >= tauLo + (bin*tauBinWidth) ) doubleTauRates_hw->Fill(tauLo+(bin*tauBinWidth));  //GeV
      } 

      for(int bin=0; bin<nEgBins1; bin++){
        if( (egISOEt_1) >= EgEdges1[bin] ) singleISOEgRates_hw->Fill(EgEdges1[bin]);  //GeV
      } 

      for(int bin=0; bin<nEgBins2; bin++){
        if( (egISOEt_2) >= EgEdges2[bin] ) doubleISOEgRates_hw->Fill(EgEdges2[bin]);  //GeV
      }  

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauISOEt_1) >= tauLo + (bin*tauBinWidth) ) singleISOTauRates_hw->Fill(tauLo+(bin*tauBinWidth));  //GeV
      }

      for(int bin=0; bin<nTauBins; bin++){
        if( (tauISOEt_2) >= tauLo + (bin*tauBinWidth) ) doubleISOTauRates_hw->Fill(tauLo+(bin*tauBinWidth));  //GeV
      } 

      for(int bin=0; bin<nHtSumBins; bin++){
        if( (htSum) >= htSumLo+(bin*htSumBinWidth) ) htSumRates_hw->Fill(htSumLo+(bin*htSumBinWidth)); //GeV
      }

      for(int bin=0; bin<nMhtSumBins; bin++){
        if( (mhtSum) >= mhtSumLo+(bin*mhtSumBinWidth) ) mhtSumRates_hw->Fill(mhtSumLo+(bin*mhtSumBinWidth)); //GeV           
      }

      for(int bin=0; bin<nEtSumBins; bin++){
        if( (etSum) >= etSumLo+(bin*etSumBinWidth) ) etSumRates_hw->Fill(etSumLo+(bin*etSumBinWidth)); //GeV           
      }
/*
      for(int bin=0; bin<nMetSumBins; bin++){
        if( (metSum) >= metSumLo+(bin*metSumBinWidth) ) metSumRates_hw->Fill(metSumLo+(bin*metSumBinWidth)); //GeV           
      }
      for(int bin=0; bin<nMetHFSumBins; bin++){
        if( (metHFSum) >= metHFSumLo+(bin*metHFSumBinWidth) ) metHFSumRates_hw->Fill(metHFSumLo+(bin*metHFSumBinWidth)); //GeV           
      }
*/
      for(int bin=0; bin<nMetBins; bin++){
        if( (metSum) >= metEdges1[bin] ) metSumRates_hw->Fill(metEdges1[bin]); //GeV           
      }
      for(int bin=0; bin<nMetHFBins; bin++){
        if( (metHFSum) >= metEdges2[bin] ) metHFSumRates_hw->Fill(metEdges2[bin]); //GeV           
      }


    }// closes if 'hwOn' is true

  }// closes loop through events


  //  TFile g( outputFilename.c_str() , "new");
  kk->cd();
  // normalisation factor for rate histograms (11kHz is the orbit frequency)
  double norm = 1;
/*  if (goodLumiEventCount > 0){
    norm = 11246*(numBunch/goodLumiEventCount); // no lumi rescale
  }*/
  //double norm = 11246*(numBunch/goodLumiEventCount2); // no lumi rescale

  //  double norm = 11246*(numBunch/goodLumiEventCount)*(expectedLum/runLum); //scale to nominal lumi

  std::cout << "Scaling Hists " << std::endl;
  if (emuOn){
    singleJetRates_emu->Scale(norm);
    doubleJetRates_emu->Scale(norm);
    tripleJetRates_emu->Scale(norm);
    quadJetRates_emu->Scale(norm);
    singleJetLLPRates_emu->Scale(norm);
    doubleJetLLPRates_emu->Scale(norm);
    singleJetLLP_HTT120Rates_emu->Scale(norm);
    singleJetLLP_HTT160Rates_emu->Scale(norm);
    singleJetLLP_HTT200Rates_emu->Scale(norm);
    singleJetLLP_HTT240Rates_emu->Scale(norm);	  
    singleEgRates_emu->Scale(norm);
    doubleEgRates_emu->Scale(norm);
    singleTauRates_emu->Scale(norm);
    doubleTauRates_emu->Scale(norm);
    singleISOEgRates_emu->Scale(norm);
    doubleISOEgRates_emu->Scale(norm);
    singleISOTauRates_emu->Scale(norm);
    doubleISOTauRates_emu->Scale(norm);
    htSumRates_emu->Scale(norm);
    mhtSumRates_emu->Scale(norm);
    etSumRates_emu->Scale(norm);
    metSumRates_emu->Scale(norm);
    metHFSumRates_emu->Scale(norm);

    //set the errors for the rates
    //want error -> error * sqrt(norm) ?

    nVtx_histo->Write();
    goodLumiEventCount_histo->Write();
    goodLumiEventCount_PUBin0_histo->Write();
    goodLumiEventCount_PUBin1_histo->Write();
    goodLumiEventCount_PUBin2_histo->Write();
    hcalTP_emu->Write();
    ecalTP_emu->Write();
    nTPs_emu->Write();
    singleJetRates_emu->Write();
    doubleJetRates_emu->Write();
    tripleJetRates_emu->Write();
    quadJetRates_emu->Write();
    singleJetLLPRates_emu->Write();
    doubleJetLLPRates_emu->Write();
    singleJetLLP_HTT120Rates_emu->Write();
    singleJetLLP_HTT160Rates_emu->Write();
    singleJetLLP_HTT200Rates_emu->Write();
    singleJetLLP_HTT240Rates_emu->Write();	  
    singleEgRates_emu->Write();
    doubleEgRates_emu->Write();
    singleTauRates_emu->Write();
    doubleTauRates_emu->Write();
    singleISOEgRates_emu->Write();
    doubleISOEgRates_emu->Write();
    singleISOTauRates_emu->Write();
    doubleISOTauRates_emu->Write();
    htSumRates_emu->Write();
    htSum_emu->Write();
    mhtSumRates_emu->Write();
    etSumRates_emu->Write();
    etSum_emu->Write();
    metSumRates_emu->Write();
    metHFSumRates_emu->Write();


  }

  if (hwOn){

    singleJetRates_hw->Scale(norm);
    doubleJetRates_hw->Scale(norm);
    tripleJetRates_hw->Scale(norm);
    quadJetRates_hw->Scale(norm);
    singleJetLLPRates_hw->Scale(norm);
    doubleJetLLPRates_hw->Scale(norm);
    singleJetLLP_HTT120Rates_hw->Scale(norm);
    singleJetLLP_HTT160Rates_hw->Scale(norm);
    singleJetLLP_HTT200Rates_hw->Scale(norm);
    singleJetLLP_HTT240Rates_hw->Scale(norm);	  
    singleEgRates_hw->Scale(norm);
    doubleEgRates_hw->Scale(norm);
    singleTauRates_hw->Scale(norm);
    doubleTauRates_hw->Scale(norm);
    singleISOEgRates_hw->Scale(norm);
    doubleISOEgRates_hw->Scale(norm);
    singleISOTauRates_hw->Scale(norm);
    doubleISOTauRates_hw->Scale(norm);
    htSumRates_hw->Scale(norm);
    mhtSumRates_hw->Scale(norm);
    etSumRates_hw->Scale(norm);
    metSumRates_hw->Scale(norm);
    metHFSumRates_hw->Scale(norm);

    nTPs_hw->Write();
    hcalTP_hw->Write();
    ecalTP_hw->Write();
    singleJetRates_hw->Write();
    doubleJetRates_hw->Write();
    tripleJetRates_hw->Write();
    quadJetRates_hw->Write();
    singleJetLLPRates_hw->Write();
    doubleJetLLPRates_hw->Write();
    singleJetLLP_HTT120Rates_hw->Write();
    singleJetLLP_HTT160Rates_hw->Write();
    singleJetLLP_HTT200Rates_hw->Write();
    singleJetLLP_HTT240Rates_hw->Write();	  
    singleEgRates_hw->Write();
    doubleEgRates_hw->Write();
    singleTauRates_hw->Write();
    doubleTauRates_hw->Write();
    singleISOEgRates_hw->Write();
    doubleISOEgRates_hw->Write();
    singleISOTauRates_hw->Write();
    doubleISOTauRates_hw->Write();
    htSumRates_hw->Write();
    mhtSumRates_hw->Write();
    etSumRates_hw->Write();
    metSumRates_hw->Write();
    metHFSumRates_hw->Write();
  }
  myfile << "using the following ntuple: " << inputFile << std::endl;
  myfile << "number of colliding bunches = " << numBunch << std::endl;
  myfile << "run luminosity = " << runLum << std::endl;
  myfile << "expected luminosity = " << expectedLum << std::endl;
  myfile << "norm factor used = " << norm << std::endl;
  myfile << "number of good events = " << goodLumiEventCount << std::endl;
  myfile.close(); 
 
  kk->Close();
}//closes the function 'rates'
