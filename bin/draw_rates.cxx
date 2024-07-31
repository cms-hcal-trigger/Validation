//#include "PhysicsTools/Utilities/macros/setTDRStyle.C"
//#include "Alignment/OfflineValidation/macros/CMS_lumi.h"
//#include "HcalTrigger/Validation/macros/CMS_lumi.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TFile.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TGraph.h"
#include "TLine.h"
#include "TGraphAsymmErrors.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <boost/range/adaptor/reversed.hpp>

int main()
{
  // include comparisons between HW and data TPs
  bool includeHW = false;
  //int rebinFactor = 1;

//  setTDRStyle();
  gROOT->ForceStyle();

  //gStyle->SetErrorX(0.5);

  // default, then new conditions
  std::vector<std::string> filenames = {"rates_def.root", "rates_new_cond.root"};
  std::vector<std::string> rateTypes = {"singleJet", "doubleJet", "tripleJet", "quadJet",
					"singleJetLLP",
					"singleEg", "singleISOEg", "doubleEg", "doubleISOEg",
					"singleTau", "singleISOTau", "doubleTau", "doubleISOTau",
					"htSum", "etSum", "metSum", "metHFSum",
  					"nTPs"};
  std::map<std::string, int> histColor;
  histColor["singleJet"] = histColor["singleJetLLP"] = histColor["singleEg"] = histColor["singleTau"] = histColor["etSum"] = histColor["metSum"] = histColor["nTPs"] = kRed;
  histColor["doubleJet"] = histColor["singleISOEg"] = histColor["singleISOTau"] = histColor["htSum"] = histColor["metHFSum"] = kBlue;
  histColor["tripleJet"] = histColor["doubleEg"] = histColor["doubleTau"] = kGreen;
  histColor["quadJet"] = histColor["doubleISOEg"] = histColor["doubleISOTau"] = kBlack;

  std::map<std::string, TH1F*> rateHists_def;
  std::map<std::string, TH1F*> rateHists_new_cond;
  std::map<std::string, TH1F*> rateHists_hw;
  std::map<std::string, TH1F*> rateHistsRatio;

  std::vector<TFile*> files;
  for(auto file : filenames) {
    files.push_back(TFile::Open(file.c_str()));
  }

  TH1F* h_goodLumiEventCount_def = dynamic_cast<TH1F*>(files.at(0)->Get("goodLumiEventCount_histo"));
  TH1F* h_goodLumiEventCount_new_cond = dynamic_cast<TH1F*>(files.at(1)->Get("goodLumiEventCount_histo"));
  double numBunch = 2352; //the number of bunches colliding for the run of interest
  double norm_def = 11246*(numBunch/h_goodLumiEventCount_def->GetBinContent(1));
  double norm_new_cond = 11246*(numBunch/h_goodLumiEventCount_new_cond->GetBinContent(1));
  for(auto rateType : rateTypes) {
    std::string histName(rateType);
    std::string histNameHw(histName);
    histName += "Rates_emu";
    histNameHw += "Rates_hw";
    rateHists_def[rateType] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
    rateHists_hw[rateType] = dynamic_cast<TH1F*>(files.at(0)->Get(histNameHw.c_str()));
    rateHists_new_cond[rateType] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str())); 

    if (rateType == "nTPs"){
      rateHists_def[rateType]->Scale(1.);
      rateHists_new_cond[rateType]->Scale(1.);
    }else{
      rateHists_def[rateType]->Scale(norm_def);
      rateHists_new_cond[rateType]->Scale(norm_new_cond);
    }

/*    if (rateType == "tripleJet"){
      int nJetBins1 = 24;
      Double_t edges1[nJetBins1+1] = {1.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0, 114., 131., 150., 200.};
      rateHists_def[rateType] = dynamic_cast<TH1F*>(rateHists_def[rateType]->Rebin(nJetBins1,rateHists_def[rateType]->GetName(),edges1));
      rateHists_hw[rateType] = dynamic_cast<TH1F*>(rateHists_hw[rateType]->Rebin(nJetBins1,rateHists_hw[rateType]->GetName(),edges1));
      rateHists_new_cond[rateType] = dynamic_cast<TH1F*>(rateHists_new_cond[rateType]->Rebin(nJetBins1,rateHists_new_cond[rateType]->GetName(),edges1));
    }

    if (rateType == "quadJet"){
      int nJetBins1 = 19;
      Double_t edges1[nJetBins1+1] = {1.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 50.0, 60.0, 70.0, 90., 130.};
      rateHists_def[rateType] = dynamic_cast<TH1F*>(rateHists_def[rateType]->Rebin(nJetBins1,rateHists_def[rateType]->GetName(),edges1));
      rateHists_hw[rateType] = dynamic_cast<TH1F*>(rateHists_hw[rateType]->Rebin(nJetBins1,rateHists_hw[rateType]->GetName(),edges1));
      rateHists_new_cond[rateType] = dynamic_cast<TH1F*>(rateHists_new_cond[rateType]->Rebin(nJetBins1,rateHists_new_cond[rateType]->GetName(),edges1));
    }*/

    rateHists_def[rateType]->SetLineColor(histColor[rateType]);
    rateHists_hw[rateType]->SetLineColor(histColor[rateType]);
    rateHists_new_cond[rateType]->SetLineColor(histColor[rateType]);

    rateHists_def[rateType]->SetMarkerColor(histColor[rateType]);
    rateHists_hw[rateType]->SetMarkerColor(histColor[rateType]);
    rateHists_new_cond[rateType]->SetMarkerColor(histColor[rateType]);

    TString name(rateHists_new_cond[rateType]->GetName());
    name += "_ratio";
    if(includeHW) {
      rateHistsRatio[rateType] = dynamic_cast<TH1F*>(rateHists_def[rateType]->Clone(name));
      rateHistsRatio[rateType]->Divide(rateHists_hw[rateType]);
    }
    else {
//      rateHistsRatio[rateType] = dynamic_cast<TH1F*>(rateHists_hw[rateType]->Clone(name));
      rateHistsRatio[rateType] = dynamic_cast<TH1F*>(rateHists_new_cond[rateType]->Clone(name));
      rateHistsRatio[rateType]->Divide(rateHists_def[rateType]);
    }
    rateHistsRatio[rateType]->SetMinimum(0.8);    
    rateHistsRatio[rateType]->SetMaximum(1.2);    
    rateHistsRatio[rateType]->SetLineWidth(2);    


  }

  for(auto pair : rateHists_new_cond) pair.second->SetLineWidth(2);
  for(auto pair : rateHists_hw) pair.second->SetLineStyle(kDashed);
  for(auto pair : rateHists_def) pair.second->SetLineWidth(2);
  for(auto pair : rateHists_def) pair.second->SetLineStyle(kDashed);

  std::vector<std::string> jetPlots = {"singleJet", "doubleJet", "tripleJet", "quadJet"};
  std::vector<std::string> jetllpPlots = {"singleJetLLP"};	
  std::vector<std::string> egPlots = {"singleEg", "singleISOEg", "doubleEg", "doubleISOEg"};
  std::vector<std::string> tauPlots = {"singleTau", "singleISOTau", "doubleTau", "doubleISOTau"};
  std::vector<std::string> scalarSumPlots = {"etSum", "htSum"};
  std::vector<std::string> vectorSumPlots = {"metSum", "metHFSum"};
  std::vector<std::string> nTPsPlots = {"nTPs"};

  std::vector<TCanvas*> canvases;
  std::vector<TPad*> pad1;
  std::vector<TPad*> pad2;
  std::map<std::string, std::vector<std::string> > plots;
  plots["jet"] = jetPlots;
  plots["jetllp"] = jetllpPlots;
  plots["eg"] = egPlots;
  plots["tau"] = tauPlots;
  plots["scalarSum"] = scalarSumPlots;
  plots["vectorSum"] = vectorSumPlots;
  plots["nTPs"] = nTPsPlots;

  for(auto iplot : plots) {

    canvases.push_back(new TCanvas);
//    canvases.back()->SetWindowSize(canvases.back()->GetWw(), 1.3*canvases.back()->GetWh());
    canvases.back()->SetWindowSize(1000, 600);

    pad1.push_back(new TPad("pad1", "pad1", 0, 0.3, 1, 1));
    pad1.back()->SetLogy();
    pad1.back()->SetTopMargin(0.07);
    pad1.back()->SetBottomMargin(0);
//    pad1.back()->SetGrid();
    pad1.back()->Draw();
    pad2.push_back(new TPad("pad2", "pad2", 0, 0, 1, 0.3));
    pad2.back()->SetTopMargin(0.);
    pad2.back()->SetBottomMargin(0.27);
//    pad2.back()->SetGrid();
    pad2.back()->Draw();
    
    pad1.back()->cd();
    
    rateHists_def[iplot.second.front()]->Draw("hist");
    rateHists_def[iplot.second.front()]->GetYaxis()->SetTitle("Rate (Hz)");
    //rateHists_def[iplot.second.front()]->GetXaxis()->SetTickSize(0);
    if (strcmp(rateHists_def[iplot.second.front()]->GetName() ,"nTPsRates_emu")==0){
      pad1.back()->SetLogy(false);
      rateHists_def[iplot.second.front()]->GetYaxis()->SetTitle("Events");
      //rateHists_def[iplot.second.front()]->SetMaximum(2e+4);
//      rateHists_def[iplot.second.front()]->SetMaximum(rateHists_def[iplot.second.front()]->GetMaximum()*2.);
      rateHists_def[iplot.second.front()]->SetMinimum(0);
    }else{
      rateHists_def[iplot.second.front()]->SetMaximum(2e+8);
      rateHists_def[iplot.second.front()]->SetMinimum(4e+1);
    }

    TLegend *leg = new TLegend(0.56, 0.35, 0.93, 0.9);
    for(auto hist : iplot.second) {
      rateHists_def[hist]->Draw("hist same");
      if(includeHW) rateHists_hw[hist]->Draw("hist same");
//      rateHists_hw[hist]->Draw("hist same");
      rateHists_new_cond[hist]->Draw("hist same");
      TString name;
      if (strcmp(rateHists_def[hist]->GetName() ,"singleJetRates_emu")==0){
	name = "Single Jet";
      }else if (strcmp(rateHists_def[hist]->GetName() , "doubleJetRates_emu")==0){
	name = "Double Jet";
      }else if (strcmp(rateHists_def[hist]->GetName() , "tripleJetRates_emu")==0){
	name = "Triple Jet";
      }else if (strcmp(rateHists_def[hist]->GetName() , "quadJetRates_emu")==0){
	name = "Quad Jet";
      }else{
	name = rateHists_def[hist]->GetName();
      }
//      TString name(rateHists_def[hist]->GetName());
      TString nameHw(rateHists_hw[hist]->GetName());
      leg->AddEntry(rateHists_def[hist], name + " (old cond)", "L");
      if(includeHW) leg->AddEntry(rateHists_hw[hist], name + " (hw)", "L");
//      leg->AddEntry(rateHists_hw[hist], name + " (PFA1')", "L"); 
      leg->AddEntry(rateHists_new_cond[hist], name + " (new cond)", "L"); 
    }
    leg->SetBorderSize(0);
    leg->SetTextSize(0.035);
    //leg->SetNColumns(2);
    leg->Draw();
    
    pad2.back()->cd();
/*    TGraphAsymmErrors *rateRatios1 = new TGraphAsymmErrors(rateHistsRatio[iplot.second.front()]);

    rateRatios1->Draw("AP");
    rateRatios1->SetMarkerStyle(2);
//    rateRatios1->SetMarkerSize(3);

    if(includeHW) rateRatios1->GetYaxis()->SetTitle("Current/HW");
    else rateRatios1->GetYaxis()->SetTitle("New/Current");
    rateRatios1->GetYaxis()->SetTitleSize(0.09);
    rateRatios1->GetYaxis()->SetTitleOffset(0.0);
*/
    rateHistsRatio[iplot.second.front()]->Draw("PE");
//    rateHistsRatio[iplot.second.front()]->Draw("E HIST");
    rateHistsRatio[iplot.second.front()]->SetMarkerStyle(2);
//    rateHistsRatio[iplot.second.front()]->SetMarkerSize(3);

    if(includeHW) rateHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("Current/HW");
    else rateHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("New/Old");
    rateHistsRatio[iplot.second.front()]->GetYaxis()->SetTitleSize(0.13);
    rateHistsRatio[iplot.second.front()]->GetYaxis()->SetTitleOffset(0.5);
    rateHistsRatio[iplot.second.front()]->GetYaxis()->SetLabelSize(0.11);
    rateHistsRatio[iplot.second.front()]->GetYaxis()->SetNdivisions(505);

    rateHistsRatio[iplot.second.front()]->GetXaxis()->SetTitleSize(0.13);
//    rateHistsRatio[iplot.second.front()]->GetXaxis()->SetTitleOffset(0.65);
    rateHistsRatio[iplot.second.front()]->GetXaxis()->SetLabelSize(0.11);

    TLine *l1 = new TLine(0, 1, 200, 1);
    l1->Draw("SAME");
    l1->SetLineStyle(2);
    l1->SetLineWidth(2);
    l1->SetLineColor(kBlack);


    for(auto hist : boost::adaptors::reverse(iplot.second)) {

//      TGraphAsymmErrors *rateRatios2 = new TGraphAsymmErrors(rateHistsRatio[hist]);
//      rateRatios2->Draw("P same");
//      rateRatios2->SetMarkerStyle(2);

      rateHistsRatio[hist]->Draw("PE same");
      //rateHistsRatio[hist]->Draw("E HIST ][ same");
      rateHistsRatio[hist]->SetMarkerStyle(2);
//      rateHistsRatio[hist]->SetMarkerSize(0.1);

      std::cout<<hist<<" "<<rateHistsRatio[hist]->GetBinCenter(90)<<" "<<rateHistsRatio[hist]->GetBinContent(90)<<std::endl;
      std::cout<<hist<<" "<<rateHistsRatio[hist]->GetBinCenter(160)<<" "<<rateHistsRatio[hist]->GetBinContent(160)<<std::endl;
    }

    //int iPeriod = 6;
    //int iPos = 10;
    //CMS_lumi(canvases.back(), iPeriod, iPos);

    if(includeHW) canvases.back()->Print(Form("plots/%sRates_hw.png", iplot.first.c_str()));
    else canvases.back()->Print(Form("plots/%sRates_emu.pdf", iplot.first.c_str()));

  }

  return 0;
}
