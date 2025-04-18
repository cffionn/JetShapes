//Created C. McGinn 2025.04.18; contact cffionn @ gmail
//plotting LHC Jet spectra and shape for studies
//Take as input the output of createJetSpectraAndShapes.exe

//c and cpp
#include <iostream>
#include <string>
#include <vector>

//ROOT
#include "TCanvas.h"
#include "TEnv.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TStyle.h"

//local
#include "include/globalDebugHandler.h"
#include "include/kirchnerPalette.h"
#include "include/stringUtil.h"
#include "include/tenvUtil.h"

Double_t getMinGTZero(TH1* inHist_p)
{
  Double_t min = inHist_p->GetMaximum();
  for(Int_t bIX = 0; bIX < inHist_p->GetNbinsX(); ++bIX){
    if(inHist_p->GetBinContent(bIX+1) <= 0) continue;
    if(inHist_p->GetBinContent(bIX+1) < min) min = inHist_p->GetBinContent(bIX+1);
  }
  return min;
}

int plotJetSpectraAndShapes(const std::string inConfigName)
{
  //Define and check the debugger
  globalDebugHandler gDebugger;
  const bool doGlobalDebug = gDebugger.GetDoGlobalDebug();

  if(doGlobalDebug) std::cout << "Initiating debug, File '" << __FILE__ << "', L" << __LINE__ << "..." << std::endl;

  //Define the kirchnerPalette and some marker styles, text size
  kirchnerPalette kPal;
  std::vector<Int_t> styles = {24, 25, 27, 28, 46, 42};
  const int titleFont = 42;
  const Double_t titleSize = 0.04;  
  const Double_t labelSize = titleSize*0.8;  
  
  //Define some default params + their input (separate for checking purposes)
  //This is for the input config - we will do something similar for the input file
  std::vector<std::string> expectedParams = {
    "INFILENAME",
    "DOLOGX",
    "DOLOGY",
    "LEGX",
    "LEGY",
    "LABELX",
    "LABELY",
    "LABELALIGNRIGHT",
    "NLABELS"
 };

  //Default params of input config
  const std::string defaultInFileName = "NONAMEGIVEN_InFile.root";
  const Bool_t defaultDoLogX = false;
  const Bool_t defaultDoLogY = false;
  const Double_t defaultLegX = 0.7;
  const Double_t defaultLegY = 0.7;
  const Double_t defaultLabelX = 0.2;
  const Double_t defaultLabelY = 0.7;
  const Bool_t defaultLabelAlignRight = false;
  const Int_t defaultNLabels = 0;
  
  //Grab the input TEnv for configuring the job
  TEnv* inConfig_p = new TEnv(inConfigName.c_str());

  //Do some config checking
  checkTEnvParam("INFILENAME", defaultInFileName.c_str(), inConfig_p);
  checkTEnvParam("DOLOGX", defaultDoLogX, inConfig_p);
  checkTEnvParam("DOLOGY", defaultDoLogY, inConfig_p);
  checkTEnvParam("LEGX", defaultLegX, inConfig_p);
  checkTEnvParam("LEGY", defaultLegY, inConfig_p);
  checkTEnvParam("LABELX", defaultLabelX, inConfig_p);
  checkTEnvParam("LABELY", defaultLabelY, inConfig_p);
  checkTEnvParam("LABELALIGNRIGHT", defaultLabelAlignRight, inConfig_p);
  checkTEnvParam("NLABELS", defaultNLabels, inConfig_p);

  inConfig_p->Print("ALL");
  
  //Now all should be defined; check anyways in case you added a param but forgot to the check line above - important for writing out correctly to file
  if(!checkAllTEnvParams(expectedParams, inConfig_p, {"LABEL."})) return 1;

  std::cout << "WHERE" << std::endl;
  
  //Grab parameters
  const std::string inFileName = inConfig_p->GetValue("INFILENAME", defaultInFileName.c_str());
  const Bool_t doLogX = inConfig_p->GetValue("DOLOGX", defaultDoLogX);
  const Bool_t doLogY = inConfig_p->GetValue("DOLOGY", defaultDoLogY);
  const Double_t legX = inConfig_p->GetValue("LEGX", defaultLegX);
  const Double_t legY = inConfig_p->GetValue("LEGY", defaultLegY);
  const Double_t labelX = inConfig_p->GetValue("LABELX", defaultLabelX);
  const Double_t labelY = inConfig_p->GetValue("LABELY", defaultLabelY);
  const Bool_t labelAlignRight = inConfig_p->GetValue("LABELALIGNRIGHT", defaultLabelAlignRight);
  const Int_t nLabels = inConfig_p->GetValue("NLABELS", defaultNLabels);

  //Need to handle individual labels separately
  bool allLabelsFound = true;
  std::vector<std::string> labels;
  for(Int_t lI = 0; lI < nLabels; ++lI){
    std::string labelHandle = "LABEL." + std::to_string(lI);
    std::string label = inConfig_p->GetValue(labelHandle.c_str(), "");    

    //Label check
    if(label.size() == 0){
      std::cout << __PRETTY_FUNCTION__ << ": NLABELS " << nLabels << " specified, but " << labelHandle << " is not found. check config '" << inConfigName << "'. return 1" << std::endl;
      allLabelsFound = false;
    }
    else labels.push_back(label);
  }
  if(!allLabelsFound) return 1;
  
  //Grab input file for additional config params
  TFile* inFile_p = new TFile(inFileName.c_str(), "READ");
  //grab the config from the createpythia step
  const std::string inFileConfigName = "createPYTHIAConfig";
  TEnv* inFileConfig_p = (TEnv*)inFile_p->Get(inFileConfigName.c_str());
  //Grab the necessary rvals param - if this doesn't exist, bail function
  const std::string jtRValsStr = inFileConfig_p->GetValue("JTRVALS", "");
  std::vector<float> jtRVals = commaSepStringToVectF(jtRValsStr);
  if(jtRValsStr.size() == 0 || jtRVals.size() == 0){
    std::cout << __PRETTY_FUNCTION__ << ": JTRVALS '" << jtRValsStr << "' from createPYTHIAConfig in file '" << inFileName << "' is not valid. check, return 1" << std::endl;
    return 1;
  }

  //Grab the radius parameter set
  const Int_t nRMax = 10;
  Float_t rParams[nRMax];
  const Int_t nR = (Int_t)jtRVals.size();
  if(nR > nRMax){
    std::cout << __PRETTY_FUNCTION__ << " given jtRVals '" << jtRValsStr << "' has size " << nR << " exceeds max " << nRMax << ". fix. return 1" << std::endl;
    return 1;
  }
  for(Int_t rI = 0; rI < nR; ++rI){
    rParams[rI] = jtRVals[rI];
  }

  //Grab our histograms
  TH1F* jtSpectra_p[nRMax];
  for(Int_t rI = 0; rI < nR; ++rI){
    std::string rStr = Form("R%.1f", rParams[rI]);
    rStr.replace(rStr.find("."), 1, "p");

    std::string name = "jtSpectra_" + rStr + "_h";
    jtSpectra_p[rI] = (TH1F*)inFile_p->Get(name.c_str());
  }

  //Create the TCanvas for spectra overlay
  TCanvas* canv_p = new TCanvas("canv", "", 900, 900);
  canv_p->SetTopMargin(0.03);
  canv_p->SetRightMargin(0.03);
  canv_p->SetLeftMargin(0.12);
  canv_p->SetBottomMargin(0.12);
  
  canv_p->cd();

  //Define a max and min
  Double_t spectraMax = -1.0;
  Double_t spectraMin = TMath::Max(1000000.0, jtSpectra_p[0]->GetMaximum());  
  for(Int_t rI = 0; rI < nR; ++rI){
    Double_t localMax = jtSpectra_p[rI]->GetMaximum();
    Double_t localMin = getMinGTZero(jtSpectra_p[rI]);

    if(localMax > spectraMax) spectraMax = localMax;
    if(localMin < spectraMin) spectraMin = localMin;
  }

  //Tweak the min/max basesd on log or lin y scale
  if(doLogY){
    spectraMax *= 2.0;
    spectraMin /= 2.0;
  }
  else{
    spectraMax *= 1.2;
    spectraMin *= 0.8;
  }

  //Create TLatex
  TLatex* label_p = new TLatex();
  label_p->SetTextFont(titleFont);
  label_p->SetTextSize(titleSize);
  label_p->SetNDC();
  if(labelAlignRight) label_p->SetTextAlign(31);
  
  //Create TLegend
  TLegend* leg_p = new TLegend(legX, legY, legX+0.2, legY+0.04*nR);
  leg_p->SetBorderSize(0);
  leg_p->SetFillStyle(0);
  leg_p->SetTextSize(titleSize);
  
  //Now plot
  for(Int_t rI = 0; rI < nR; ++rI){    
    //set max/min
    jtSpectra_p[rI]->SetMaximum(spectraMax); 
    jtSpectra_p[rI]->SetMinimum(spectraMin);

    //marker color line color size etc.
    jtSpectra_p[rI]->SetMarkerColor(kPal.getColor(rI));
    jtSpectra_p[rI]->SetLineColor(kPal.getColor(rI));
    jtSpectra_p[rI]->SetMarkerSize(1.5);
    jtSpectra_p[rI]->SetMarkerStyle(styles[rI%styles.size()]);

    //Set text sizes
    jtSpectra_p[rI]->GetXaxis()->SetTitleSize(titleSize);
    jtSpectra_p[rI]->GetYaxis()->SetTitleSize(titleSize);
    jtSpectra_p[rI]->GetXaxis()->SetLabelSize(labelSize);
    jtSpectra_p[rI]->GetYaxis()->SetLabelSize(labelSize);
    
    if(rI == 0) jtSpectra_p[rI]->DrawCopy("HIST E1 P");
    else jtSpectra_p[rI]->DrawCopy("HIST E1 P SAME");

    leg_p->AddEntry(jtSpectra_p[rI], Form("R=%.1f", jtRVals[rI]), "P L");
  }

  //Draw your legend
  leg_p->Draw("SAME");

  //draw your labels
  const Double_t labelDelY = 0.05;
  for(unsigned int lI = 0; lI < labels.size(); ++lI){
    Double_t yPos = labelY - ((Double_t)lI)*labelDelY;
    label_p->DrawLatex(labelX, yPos, labels[lI].c_str());
  }
  
  //Canvas style tweaks
  if(doLogX) canv_p->SetLogx();
  if(doLogY) canv_p->SetLogy();
  gStyle->SetOptStat(0);
  gPad->SetTicks();
  
  canv_p->SaveAs("jetSpectraOverlay.png");
  delete canv_p;

  delete leg_p;
  delete label_p;
  
  //Cleanup
  inFile_p->Close();
  delete inFile_p;

  delete inConfig_p;

  return 0;
}

int main(const int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/plotJetSpectraAndShapes.exe <inConfigName>" << std::endl;
    std::cout << "TO DEBUG:" << std::endl;
    std::cout << " export DOGLOBALDEBUGROOT=1 #from command line" << std::endl;
    std::cout << "TO TURN OFF DEBUG:" << std::endl;
    std::cout << " export DOGLOBALDEBUGROOT=0 #from command line" << std::endl;
    std::cout << "return 1." << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += plotJetSpectraAndShapes(argv[1]);
  return retVal;
}
