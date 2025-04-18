//Created Chris McGinn 2025.04.13; contact cffionn @ gmail
//Utilities for TEnv

//c+cpp
#include <iostream>
#include <string>
#include <vector>

//ROOT
#include "TEnv.h"
#include "THashList.h"

template <typename T>
bool checkTEnvParam(std::string paramName, T paramDefault, TEnv* inConfig_p)
{ 
  bool paramFound = true;  
  if(!inConfig_p->Defined(paramName.c_str())){
    std::cout << __PRETTY_FUNCTION__ << ": Job inConfig_p missing param '" << paramName << "', setting default '" << paramDefault << "'." << std::endl;
    inConfig_p->SetValue(paramName.c_str(), paramDefault);
  }
  return paramFound;
}

bool checkAllTEnvParams(std::vector<std::string> paramList, TEnv* inConfig_p, std::vector<std::string> skipStrings = {})
{
  bool allParamsFound = true;
  for(unsigned int pI = 0; pI < paramList.size(); ++pI){
    if(!inConfig_p->Defined(paramList[pI].c_str())){
      std::cout << __PRETTY_FUNCTION__ << ": Parameter '" << paramList[pI] << "'. returning false" << std::endl;
      allParamsFound = false;
    }
  }

  THashList* inConfigList_p = inConfig_p->GetTable();
  for(Int_t entry = 0; entry < inConfigList_p->GetEntries(); ++entry){
    std::string entryName = inConfigList_p->At(entry)->GetName();

    bool currParamFound = false;
    for(unsigned int pI = 0; pI < paramList.size(); ++pI){
      if(entryName == paramList[pI]){
	currParamFound = true;
	break;
      }
    } 

    if(!currParamFound){
      std::cout << __PRETTY_FUNCTION__ << ": '" << entryName << "' not accounted for in default values." << std::endl;
      //Check if listed in skip strings
      bool isSkipString = false;
      std::string skipString = "";
      for(unsigned int sI = 0; sI < skipStrings.size(); ++sI){
	if(entryName.find(skipStrings[sI]) != std::string::npos){
	  isSkipString = true;
	  skipString = skipStrings[sI];
	  break;
	}
      }
      
      if(isSkipString){
	std::cout << " Found as skipString '" << skipString << "'. returning true" << std::endl;
      }
      else{
	allParamsFound = false;
	std::cout << " Also not found in skip strings, return false" << std::endl;
      }
    }
  }
  
  return allParamsFound;
}
