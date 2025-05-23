#ifndef getLogBins_h
#define getLogBins_h

#include "TMath.h"

inline void getLogBins(const Float_t lower, const Float_t higher, const Int_t nBins, Double_t bins[])
{
  const Int_t nMaxBins = 1000;
  if(nBins > nMaxBins){
    std::cout << __PRETTY_FUNCTION__ << ": nBins '" << nBins << "' exceeds max '" << nMaxBins << "'. return" << std::endl;
    return;
  }

  Float_t logBins[nMaxBins+1];
  bins[0] = lower;
  bins[nBins] = higher;

  logBins[0] = TMath::Log10(lower);
  logBins[nBins] = TMath::Log10(higher);

  Float_t interval = (logBins[nBins] - logBins[0])/nBins;

  for(Int_t iter = 1; iter < nBins; iter++){
    logBins[iter] = logBins[0] + iter*interval;
    bins[iter] = TMath::Power(10, logBins[iter]);
  }

  return;
}

#endif
