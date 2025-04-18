//Created C. McGinn 2025.04.11; contact cffionn @ gmail
//creating LHC Jet spectra and shape for studies
//Take as input the output of createPYTHIA.exe

//c and cpp
#include <iostream>
#include <string>
#include <vector>

//ROOT
#include "TEnv.h"
#include "TFile.h"
#include "TH1F.h"
#include "TMath.h"
#include "TTree.h"

//local
#include "include/globalDebugHandler.h"
#include "include/getLinBins.h"
#include "include/getLogBins.h"
#include "include/stringUtil.h"
#include "include/tenvUtil.h"

int createJetSpectraAndShapes(const std::string inConfigName)
{
  globalDebugHandler gDebugger;
  const bool doGlobalDebug = gDebugger.GetDoGlobalDebug();

  if(doGlobalDebug) std::cout << "Initiating debug, File '" << __FILE__ << "', L" << __LINE__ << "..." << std::endl;

  //Define some default params + their input (separate for checking purposes)
  //This is for the input config - we will do something similar for the input file
  std::vector<std::string> expectedParams = {
    "INFILENAME",
    "OUTFILENAME",
    "JTABSETAMAX",
    "NJTPTBINS",
    "JTPTMIN",
    "JTPTMAX",
    "DOJTPTLOGBINS"
 };

  //Default params of input config
  const std::string defaultInFileName = "NONAMEGIVEN_InFile.root";
  const std::string defaultOutFileName = "NONAMEGIVEN_CreateJetSpectraAndShapes.root";
  const Float_t defaultJtAbsEtaMax = 5.0;
  const Int_t defaultNJtPtBins = 20;
  const Float_t defaultJtPtMin = 15.0;
  const Float_t defaultJtPtMax = 200.0;
  const Bool_t defaultDoJtPtLogBins = false;

  //Grab the input TEnv for configuring the job
  TEnv* inConfig_p = new TEnv(inConfigName.c_str());

  //Do some config checking
  checkTEnvParam("INFILENAME", defaultInFileName.c_str(), inConfig_p);
  checkTEnvParam("OUTFILENAME", defaultOutFileName.c_str(), inConfig_p);
  checkTEnvParam("JTABSETAMAX", defaultJtAbsEtaMax, inConfig_p);
  checkTEnvParam("NJTPTBINS", defaultNJtPtBins, inConfig_p);
  checkTEnvParam("JTPTMIN", defaultJtPtMin, inConfig_p);
  checkTEnvParam("JTPTMAX", defaultJtPtMax, inConfig_p);
  checkTEnvParam("DOJTPTLOGBINS", defaultDoJtPtLogBins, inConfig_p);

  //Now all should be defined; check anyways in case you added a param but forgot to the check line above - important for writing out correctly to file
  if(!checkAllTEnvParams(expectedParams, inConfig_p)) return 1;

  //Grab parameters
  const std::string inFileName = inConfig_p->GetValue("INFILENAME", defaultInFileName.c_str());
  const std::string outFileName = inConfig_p->GetValue("OUTFILENAME", defaultOutFileName.c_str());
  const Float_t jtAbsEtaMax = inConfig_p->GetValue("JTABSETAMAX", defaultJtAbsEtaMax);
  const Float_t nJtPtBins = inConfig_p->GetValue("NJTPTBINS", defaultNJtPtBins);
  const Float_t jtPtMin = inConfig_p->GetValue("JTPTMIN", defaultJtPtMin);
  const Float_t jtPtMax = inConfig_p->GetValue("JTPTMAX", defaultJtPtMax);
  const Float_t doJtPtLogBins = inConfig_p->GetValue("DOJTPTLOGBINS", defaultDoJtPtLogBins);

  //Construct our jtptBins array
  const Int_t nMaxBins = 200;
  if(nJtPtBins > nMaxBins){
    std::cout << __PRETTY_FUNCTION__ << ": given NJTPTBINS '" << nJtPtBins << "' exceeds max '" << nMaxBins << "'. fix, return 1" << std::endl;
    return 1;
  }
  Double_t jtPtBins[nMaxBins];
  if(doJtPtLogBins) getLogBins(jtPtMin, jtPtMax, nJtPtBins, jtPtBins);
  else getLinBins(jtPtMin, jtPtMax, nJtPtBins, jtPtBins);

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
  /*
  //Declare variables for evttree
  Float_t pthat;
  const Int_t nMaxPart = 10000;
  Int_t npart;
  Float_t pt[nMaxPart];
  Float_t phi[nMaxPart];
  Float_t eta[nMaxPart];
  Float_t m[nMaxPart];
  Int_t id[nMaxPart];
  */
  //Declare variables for jettree
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

  const Int_t nMaxJt = 500;
  Int_t njt[nRMax];
  Float_t jtpt[nRMax][nMaxJt];
  Float_t jteta[nRMax][nMaxJt];
  Float_t jtphi[nRMax][nMaxJt];

  //All variables declared, grab ttrees
  TTree* jetTree_p = (TTree*)inFile_p->Get("jetTree");
  jetTree_p->SetBranchStatus("*", 0);

  //Assign jet ttree branch status + addresses
  for(Int_t rI = 0; rI < nR; ++rI){
    std::string rStr = Form("R%.1f", rParams[rI]);
    rStr.replace(rStr.find("."), 1, "p");

    std::string nRStr = "njt" + rStr;
    jetTree_p->SetBranchStatus(nRStr.c_str(), 1);
    jetTree_p->SetBranchStatus(("jtpt" + rStr).c_str(), 1);
    jetTree_p->SetBranchStatus(("jteta" + rStr).c_str(), 1);
    jetTree_p->SetBranchStatus(("jtphi" + rStr).c_str(), 1);

    jetTree_p->SetBranchAddress(nRStr.c_str(), &(njt[rI]));
    jetTree_p->SetBranchAddress(("jtpt" + rStr).c_str(), jtpt[rI]);
    jetTree_p->SetBranchAddress(("jteta" + rStr).c_str(), jteta[rI]);
    jetTree_p->SetBranchAddress(("jtphi" + rStr).c_str(), jtphi[rI]);
  }


  //Initialize our output TFile + Histograms
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  TH1F* jtSpectra_p[nRMax];
  for(Int_t rI = 0; rI < nR; ++rI){
    std::string rStr = Form("R%.1f", rParams[rI]);
    rStr.replace(rStr.find("."), 1, "p");

    std::string name = "jtSpectra_" + rStr + "_h";
    std::string title = ";Jet p_{T} (GeV);Counts";
    jtSpectra_p[rI] = new TH1F(name.c_str(), title.c_str(), nJtPtBins, jtPtBins);
  }

  const ULong64_t nEntries = (ULong64_t)jetTree_p->GetEntries();
  for(ULong64_t entry = 0; entry < nEntries; ++entry){
    jetTree_p->GetEntry(entry);

    //iterate over jet R
    for(Int_t rI = 0; rI < nR; ++rI){
      for(Int_t jI = 0; jI < njt[rI]; ++jI){
	if(TMath::Abs(jteta[rI][jI]) > jtAbsEtaMax) continue;
	if(jtpt[rI][jI] < jtPtMin) continue;
	if(jtpt[rI][jI] >= jtPtMax) continue;

	jtSpectra_p[rI]->Fill(jtpt[rI][jI]);
      }
    }
  }

  //Write output
  outFile_p->cd();

  for(Int_t rI = 0; rI < nR; ++rI){
    jtSpectra_p[rI]->Write("", TObject::kOverwrite);
  }

  //Write the preceeding config
  inFileConfig_p->Write(inFileConfigName.c_str(), TObject::kOverwrite);

  //Write the job config
  inConfig_p->SetValue("CONFIGNAME", inConfigName.c_str());
  inConfig_p->Write("createJetSpectraAndShapesConfig", TObject::kOverwrite);

  //Cleanup
  inFile_p->Close();
  delete inFile_p;

  for(Int_t rI = 0; rI < nR; ++rI){
    delete jtSpectra_p[rI];
  }

  outFile_p->Close();
  delete outFile_p;

  delete inConfig_p;

  return 0;
}

int main(const int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/createJetSpectraAndShapes.exe <inConfigName>" << std::endl;
    std::cout << "TO DEBUG:" << std::endl;
    std::cout << " export DOGLOBALDEBUGROOT=1 #from command line" << std::endl;
    std::cout << "TO TURN OFF DEBUG:" << std::endl;
    std::cout << " export DOGLOBALDEBUGROOT=0 #from command line" << std::endl;
    std::cout << "return 1." << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += createJetSpectraAndShapes(argv[1]);
  return retVal;
}
