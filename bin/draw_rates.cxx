#include "PhysicsTools/Utilities/macros/setTDRStyle.C"

#include "TCanvas.h"
#include "TH1.h"
#include "TFile.h"
#include "TLegend.h"
#include "TROOT.h"
#include "THStack.h"

#include <map>
#include <string>
#include <vector>

int main()
{
  // include comparisons between HW and data TPs
  bool includeHW = false;
  int rebinFactor = 1;

  setTDRStyle();
  gROOT->ForceStyle();

  // default, then new conditions
  std::vector<std::string> filenames = {"rates_def.root", "rates_new_cond.root"};
  std::vector<std::string> rateTypes = {"singleJet", "doubleJet", "tripleJet", "quadJet",
					"singleEg", "singleISOEg", "doubleEg", "doubleISOEg",
					"singleTau", "singleISOTau", "doubleTau", "doubleISOTau",
					"htSum", "mhtSum", "etSum", "metSum", "metHFSum"};
  std::map<std::string, int> histColor;
  histColor["singleJet"] = histColor["singleEg"] = histColor["singleTau"] = histColor["etSum"] = histColor["metSum"] = kRed;
  histColor["doubleJet"] = histColor["singleISOEg"] = histColor["singleISOTau"] = histColor["htSum"] = histColor["mhtSum"] = kBlue;
  histColor["tripleJet"] = histColor["doubleEg"] = histColor["doubleTau"] = histColor["metHFSum"] = kGreen;
  histColor["quadJet"] = histColor["doubleISOEg"] = histColor["doubleISOTau"] = kBlack;

  std::map<std::string, TH1F*> rateHists_def;
  std::map<std::string, TH1F*> rateHists_new_cond;
  std::map<std::string, TH1F*> rateHists_hw;
  std::map<std::string, TH1F*> sumHists_def;
  std::map<std::string, TH1F*> sumHists_new_cond;
  std::map<std::string, TH1F*> sumHists_hw;
  std::map<std::string, TH1F*> rateHistsRatio;
  
  std::vector<TFile*> files;
  for(auto file : filenames) {
    files.push_back(TFile::Open(file.c_str()));
  }
  for(auto rateType : rateTypes) {
    std::string histName(rateType);
    std::string histNameHw(histName);
    histName += "Rates_emu";
    histNameHw += "Rates_hw";
    rateHists_def[rateType] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
    rateHists_hw[rateType] = dynamic_cast<TH1F*>(files.at(0)->Get(histNameHw.c_str()));
    rateHists_new_cond[rateType] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str())); 
    rateHists_def[rateType]->Rebin(rebinFactor);
    rateHists_hw[rateType]->Rebin(rebinFactor);
    rateHists_new_cond[rateType]->Rebin(rebinFactor);

    rateHists_def[rateType]->SetLineColor(histColor[rateType]);
    rateHists_hw[rateType]->SetLineColor(histColor[rateType]);
    rateHists_new_cond[rateType]->SetLineColor(histColor[rateType]);
    TString name(rateHists_new_cond[rateType]->GetName());
    name += "_ratio";
    if(includeHW) {
      rateHistsRatio[rateType] = dynamic_cast<TH1F*>(rateHists_def[rateType]->Clone(name));
      rateHistsRatio[rateType]->Divide(rateHists_hw[rateType]);
    }
    else {
      rateHistsRatio[rateType] = dynamic_cast<TH1F*>(rateHists_new_cond[rateType]->Clone(name));
      rateHistsRatio[rateType]->Divide(rateHists_def[rateType]);
    }
    rateHistsRatio[rateType]->SetMinimum(0.6);    
    rateHistsRatio[rateType]->SetMaximum(1.4);    
    rateHistsRatio[rateType]->SetLineWidth(2);    

    if (rateType.find("Sum") != std::string::npos) {
      std::string histName(rateType);
      std::string histNameHw(histName);
      histName += "_emu";
      histNameHw += "_hw";
      sumHists_def[rateType] = dynamic_cast<TH1F*>(files.at(0)->Get(histName.c_str()));
      sumHists_hw[rateType] = dynamic_cast<TH1F*>(files.at(0)->Get(histNameHw.c_str()));
      sumHists_new_cond[rateType] = dynamic_cast<TH1F*>(files.at(1)->Get(histName.c_str())); 
      sumHists_def[rateType]->Rebin(rebinFactor);
      sumHists_hw[rateType]->Rebin(rebinFactor);
      sumHists_new_cond[rateType]->Rebin(rebinFactor);

      sumHists_def[rateType]->SetLineColor(histColor[rateType]);
      sumHists_hw[rateType]->SetLineColor(histColor[rateType]);
      sumHists_new_cond[rateType]->SetLineColor(histColor[rateType]);
    }
  }
  for(auto pair : rateHists_new_cond) pair.second->SetLineWidth(2);
  for(auto pair : rateHists_hw) pair.second->SetLineStyle(kDashed);
  for(auto pair : rateHists_def) pair.second->SetLineStyle(kDotted);

  for(auto pair : sumHists_new_cond) pair.second->SetLineWidth(2);
  for(auto pair : sumHists_hw) pair.second->SetLineStyle(kDashed);
  for(auto pair : sumHists_def) pair.second->SetLineStyle(kDotted);

  std::vector<std::string> jetPlots = {"singleJet", "doubleJet", "tripleJet", "quadJet"};
  std::vector<std::string> egPlots = {"singleEg", "singleISOEg", "doubleEg", "doubleISOEg"};
  std::vector<std::string> tauPlots = {"singleTau", "singleISOTau", "doubleTau", "doubleISOTau"};
  std::vector<std::string> scalarSumPlots = {"etSum", "htSum"};
  std::vector<std::string> vectorSumPlots = {"metSum", "mhtSum", "metHFSum"};

  std::vector<TCanvas*> canvases;
  std::vector<TLegend*> legends;
  std::vector<THStack*> stacks;
  std::vector<TPad*> pad0;
  std::vector<TPad*> pad1;
  std::vector<TPad*> pad2;
  std::map<std::string, std::vector<std::string> > plots;
  plots["jet"] = jetPlots;
  plots["eg"] = egPlots;
  plots["tau"] = tauPlots;
  plots["scalarSum"] = scalarSumPlots;
  plots["vectorSum"] = vectorSumPlots;

  for(auto iplot : plots) {

    canvases.push_back(new TCanvas);
    canvases.back()->SetWindowSize(canvases.back()->GetWw(), 1.3*canvases.back()->GetWh());
    pad1.push_back(new TPad("pad1", "pad1", 0, 0.3, 1, 1));
    pad1.back()->SetLogy();
    pad1.back()->SetGrid();
    pad1.back()->Draw();
    pad2.push_back(new TPad("pad2", "pad2", 0, 0, 1, 0.3));
    pad2.back()->SetGrid();
    pad2.back()->Draw();

    pad1.back()->cd();
    
    rateHists_def[iplot.second.front()]->Draw("hist");
    legends.push_back(new TLegend(0.55, 0.9 - 0.1*iplot.second.size(), 0.95, 0.93));
    for(auto hist : iplot.second) {
      rateHists_def[hist]->Draw("hist same");
      if(includeHW) rateHists_hw[hist]->Draw("hist same");
      rateHists_new_cond[hist]->Draw("hist same");
      TString name(rateHists_def[hist]->GetName());
      TString nameHw(rateHists_hw[hist]->GetName());
      legends.back()->AddEntry(rateHists_def[hist], name + " (current)", "L");
      if(includeHW) legends.back()->AddEntry(rateHists_hw[hist], name + " (hw)", "L");
      legends.back()->AddEntry(rateHists_new_cond[hist], name + " (new)", "L"); 
    }
    legends.back()->SetBorderSize(0);
    legends.back()->Draw();
    
    pad2.back()->cd();
    rateHistsRatio[iplot.second.front()]->Draw("hist");
    if(includeHW) rateHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("Current/HW");
    else rateHistsRatio[iplot.second.front()]->GetYaxis()->SetTitle("New/Current");
    for(auto hist : iplot.second) {
      rateHistsRatio[hist]->Draw("hist same");
    }

    if(includeHW) canvases.back()->Print(Form("plots/%sRates_hw.pdf", iplot.first.c_str()));
    else canvases.back()->Print(Form("plots/%sRates_emu.pdf", iplot.first.c_str()));

    // For the case of "Sum" variables e.g. MET, HT, etc we make plots with out the ratios of the
    // distribution itself
    if (iplot.first.find("Sum") != std::string::npos) {
    
      canvases.push_back(new TCanvas);
      canvases.back()->SetWindowSize(canvases.back()->GetWw(), 1.3*canvases.back()->GetWh());

      pad0.push_back(new TPad("pad0", "pad0", 0, 0, 1, 1));
      pad0.back()->SetLogy();
      pad0.back()->SetGrid();
      pad0.back()->Draw();

      pad0.back()->cd();
    
      std::string xname = sumHists_def[iplot.second.front()]->GetXaxis()->GetTitle();
      std::string yname = sumHists_def[iplot.second.front()]->GetYaxis()->GetTitle();
      stacks.push_back(new THStack(Form("hs%s", iplot.first.c_str()), Form(";%s;%s", xname.c_str(),yname.c_str()))); stacks.back()->Draw();
      legends.push_back(new TLegend(0.45, 0.9 - 0.05*iplot.second.size(), 0.95, 0.95));
      legends.back()->SetNColumns(2);
      for(auto hist : iplot.second) {
        stacks.back()->Add(sumHists_def[hist]);
        if(includeHW) stacks.back()->Add(sumHists_hw[hist]);
        stacks.back()->Add(sumHists_new_cond[hist]);
        TString name(sumHists_def[hist]->GetName());
        TString nameHw(sumHists_hw[hist]->GetName());
        legends.back()->AddEntry(sumHists_def[hist], name + " (current)", "L");
        if(includeHW) legends.back()->AddEntry(sumHists_hw[hist], name + " (hw)", "L");
        legends.back()->AddEntry(sumHists_new_cond[hist], name + " (new)", "L"); 
      }
      legends.back()->SetBorderSize(0);
      stacks.back()->SetMaximum(4.0*stacks.back()->GetMaximum("nostack"));
      stacks.back()->Draw("nostack");
      legends.back()->Draw("same");

      if(includeHW) canvases.back()->Print(Form("plots/%s_hw.pdf", iplot.first.c_str()));
      else canvases.back()->Print(Form("plots/%s_emu.pdf", iplot.first.c_str()));

    }
  }

  return 0;
}
