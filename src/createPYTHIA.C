//Created C. McGinn 2025.04.11; contact cffionn @ gmail
//Based on main05 from PYTHIA8.3.10
//creating initial inputs for LHC Jets for shape studies

//c and cpp
#include <iostream>
#include <string>
#include <vector>

//ROOT
#include "TEnv.h"
#include "TFile.h"
#include "TMath.h"
#include "TTree.h"

//PYTHIA
#include "Pythia8/Pythia.h"

//FASTJET
#include "fastjet/JetDefinition.hh"
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequence.hh"

//local
#include "include/globalDebugHandler.h"
#include "include/stringUtil.h"
#include "include/tenvUtil.h"

int createPYTHIA(const std::string inConfigName)
{
  globalDebugHandler gDebugger;
  const bool doGlobalDebug = gDebugger.GetDoGlobalDebug();

  if(doGlobalDebug) std::cout << "Initiating debug, File '" << __FILE__ << "', L" << __LINE__ << "..." << std::endl;

  //Define some default params + their input (separate for checking purposes)
  std::vector<std::string> expectedParams = {
    "OUTFILENAME",
    "PTHATMIN",
    "NEVENTSGEN",
    "JTPTMIN",
    "JTABSETAMAX",
    "JTRVALS"
  };

  const std::string defaultOutFileName = "NONAMEGIVEN_CreatePYTHIA.root";
  const Float_t defaultPtHatMin = 80.0;
  const Int_t defaultNEventsGen = 0;
  const Float_t defaultJtPtMin = 15.0;
  const Float_t defaultJtAbsEtaMax = 5.0;
  const std::string defaultJtRVals = "0.2,0.4";

  //Grab the input TEnv for configuring the job
  TEnv* inConfig_p = new TEnv(inConfigName.c_str());

  //Do some config checking
  checkTEnvParam("OUTFILENAME", defaultOutFileName.c_str(), inConfig_p);
  checkTEnvParam("PTHATMIN", defaultPtHatMin, inConfig_p);
  checkTEnvParam("NEVENTSGEN", defaultNEventsGen, inConfig_p);
  checkTEnvParam("JTPTMIN", defaultJtPtMin, inConfig_p);
  checkTEnvParam("JTABSETAMAX", defaultJtAbsEtaMax, inConfig_p);
  checkTEnvParam("JTRVALS", defaultJtRVals.c_str(), inConfig_p);

  //Now all should be defined; check anyways in case you added a param but forgot to the check line above - important for writing out correctly to file
  if(!checkAllTEnvParams(expectedParams, inConfig_p)) return 1;

  //Grab parameters
  const std::string outFileName = inConfig_p->GetValue("OUTFILENAME", defaultOutFileName.c_str());
  const Float_t ptHatMin = inConfig_p->GetValue("PTHATMIN", defaultPtHatMin);
  const ULong64_t nEventsGen = inConfig_p->GetValue("NEVENTSGEN", defaultNEventsGen);
  const Float_t jtPtMin = inConfig_p->GetValue("JTPTMIN", defaultJtPtMin);
  const Float_t jtAbsEtaMax = inConfig_p->GetValue("JTABSETAMAX", defaultJtAbsEtaMax);
  const std::string jtRValsStr = inConfig_p->GetValue("JTRVALS", defaultJtRVals.c_str());
  std::vector<float> jtRVals = commaSepStringToVectF(jtRValsStr);

  //Declare variables for evttree
  Float_t pthat;
  const Int_t nMaxPart = 10000;
  Int_t npart;
  Float_t pt[nMaxPart];
  Float_t phi[nMaxPart];
  Float_t eta[nMaxPart];
  Float_t m[nMaxPart];
  Int_t id[nMaxPart];

  //Declare variables for jttree
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

  //Initialize our TFile + TTree for the output
  TFile* outFile_p = new TFile(outFileName.c_str(), "RECREATE");
  //two ttrees, one for particle + global observables, one for jets
  TTree* evtTree_p = new TTree("evtTree", "");
  TTree* jetTree_p = new TTree("jetTree", "");

  //Declare evt tree branches
  evtTree_p->Branch("pthat", &pthat, "pthat/F");
  evtTree_p->Branch("npart", &npart, "npart/I");
  evtTree_p->Branch("pt", pt, "pt[npart]/F");
  evtTree_p->Branch("eta", eta, "eta[npart]/F");
  evtTree_p->Branch("phi", phi, "phi[npart]/F");
  evtTree_p->Branch("m", m, "m[npart]/F");
  evtTree_p->Branch("id", id, "id[npart]/I");

  //Declare jttree branches
  for(Int_t rI = 0; rI < nR; ++rI){
    std::string rStr = Form("R%.1f", rParams[rI]);
    rStr.replace(rStr.find("."), 1, "p");

    std::string nRStr = "njt" + rStr;
    jetTree_p->Branch(nRStr.c_str(), &(njt[rI]), (nRStr + "/I").c_str());
    jetTree_p->Branch(("jtpt" + rStr).c_str(), jtpt[rI], ("jtpt" + rStr + "[" + nRStr + "]/F").c_str());
    jetTree_p->Branch(("jteta" + rStr).c_str(), jteta[rI], ("jteta" + rStr + "[" + nRStr + "]/F").c_str());
    jetTree_p->Branch(("jtphi" + rStr).c_str(), jtphi[rI], ("jtphi" + rStr + "[" + nRStr + "]/F").c_str());
  }

  //following main05, initialize generator
  // Generator. LHC process and output selection. Initialization.
  Pythia8::Pythia pythia;
  pythia.readString("Beams:eCM = 5020.");//modded center of mass for HI
  pythia.readString("HardQCD:all = on");
  pythia.readString(Form("PhaseSpace:pTHatMin = %f", ptHatMin));//Lower pthat min
  pythia.readString("Next:numberShowInfo = 0");
  pythia.readString("Next:numberShowProcess = 0");
  pythia.readString("Next:numberShowEvent = 0");

  //Actual init; if failed, return with fail code
  if(!pythia.init()) return 1;

  ULong64_t totalEntries = 0;

  //use a while loop for rare pythia events that do not converge
  while(nEventsGen > totalEntries){
    //Generate event, continue on fail
    if(!pythia.next()) continue;

    pthat = pythia.info.pTHat();

    //Process the particle list to produce our jet collection
    npart = 0;
    std::vector <fastjet::PseudoJet> fjInputs;
    fjInputs.reserve(pythia.event.size());
    for(int i = 0; i < pythia.event.size(); ++i){
      //skip non-final particles
      if(!pythia.event[i].isFinal()) continue;

      //skip particles beyond ATLAS/CMS detector acceptance
      if(TMath::Abs(pythia.event[i].eta()) > 5.0) continue;

      //skip neutrinos
      if(pythia.event[i].id() == 12) continue;
      if(pythia.event[i].id() == 14) continue;
      if(pythia.event[i].id() == 16) continue;

      pt[npart] = pythia.event[i].pT();
      eta[npart] = pythia.event[i].eta();
      phi[npart] = pythia.event[i].phi();
      m[npart] = pythia.event[i].m();
      id[npart] = pythia.event[i].id();
      ++npart;

      //Append to vector a pseudojet
      fjInputs.push_back(fastjet::PseudoJet(pythia.event[i].px(), pythia.event[i].py(), pythia.event[i].pz(), pythia.event[i].e()));
    }

    //
    //Process all r for jets
    for(Int_t rI = 0; rI < nR; ++rI){
      //Jet def. is tied to rparam
      fastjet::JetDefinition jetDef(fastjet::antikt_algorithm, rParams[rI], fastjet::E_scheme, fastjet::Best);
      fastjet::ClusterSequence clustSeq(fjInputs, jetDef);
      std::vector<fastjet::PseudoJet> jets = fastjet::sorted_by_pt(clustSeq.inclusive_jets(jtPtMin));

      njt[rI] = 0;
      for(unsigned int jI = 0; jI < jets.size(); ++jI){
	if(TMath::Abs(jets[jI].eta()) > jtAbsEtaMax) continue;

	jtpt[rI][njt[rI]] = jets[jI].pt();
	jteta[rI][njt[rI]] = jets[jI].eta();
	jtphi[rI][njt[rI]] = jets[jI].phi();	
	++njt[rI];
      }
    }

    //fill the trees
    evtTree_p->Fill();
    jetTree_p->Fill();

    ++totalEntries;
  }

  //Write output
  outFile_p->cd();

  evtTree_p->Write("", TObject::kOverwrite);
  jetTree_p->Write("", TObject::kOverwrite);

  //Write the job config
  inConfig_p->SetValue("CONFIGNAME", inConfigName.c_str());
  inConfig_p->Write("createPYTHIAConfig", TObject::kOverwrite);

  //Cleanup
  delete evtTree_p;
  delete jetTree_p;

  outFile_p->Close();
  delete outFile_p;

  delete inConfig_p;

  return 0;
}

int main(const int argc, char* argv[])
{
  if(argc != 2){
    std::cout << "Usage: ./bin/createPYTHIA.exe <inConfigName>" << std::endl;
    std::cout << "TO DEBUG:" << std::endl;
    std::cout << " export DOGLOBALDEBUGROOT=1 #from command line" << std::endl;
    std::cout << "TO TURN OFF DEBUG:" << std::endl;
    std::cout << " export DOGLOBALDEBUGROOT=0 #from command line" << std::endl;
    std::cout << "return 1." << std::endl;
    return 1;
  }

  int retVal = 0;
  retVal += createPYTHIA(argv[1]);
  return retVal;
}
