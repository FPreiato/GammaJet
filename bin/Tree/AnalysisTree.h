//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Mar 28 15:59:52 2012 by ROOT version 5.32/00
// from TTree analysis/analysis tree
// found on file: output_mc.root
//////////////////////////////////////////////////////////

#pragma once

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class AnalysisTree {
  public :
    TTree          *fChain;   //!pointer to the analyzed TTree or TChain

    // Declaration of leaf types
    UInt_t                      run;
    UInt_t                      lumi_block;
    UInt_t                      event;
    //Double_t                      crossSection;
    Float_t                     ntrue_interactions;
    UInt_t                      nvertex;
    UInt_t                      nvertexGood;
    Int_t                       pu_nvertex;
    Float_t                     event_weight;
    Double_t                    generator_weight;
    Float_t                   evtWeightTot; //Federico
    std::vector<std::string>*   trigger_names;
    std::vector<bool>*          trigger_results;
    std::vector<double>*          trigger_prescale;

    // List of branches
    // TBranch        *b_crossSection;   //!
    TBranch        *b_ntrue_interactions;   //!
    TBranch        *b_nvertex;   //!
    TBranch        *b_nvertexGood;   //!
    TBranch        *b_pu_nvertex;   //!
    TBranch        *b_event_weight;   //!
    TBranch        *b_evtWeightTot;   //! Federico

    AnalysisTree();
    virtual ~AnalysisTree();
    virtual Int_t    GetEntry(Long64_t entry);

    virtual void     Init(TTree *tree);
};


AnalysisTree::AnalysisTree() : fChain(0), trigger_names(NULL), trigger_results(NULL), trigger_prescale(NULL)
{
}

AnalysisTree::~AnalysisTree()
{
  if (!fChain)
    return;

  delete fChain->GetCurrentFile();
}

Int_t AnalysisTree::GetEntry(Long64_t entry)
{
  // Read contents of entry.
  if (!fChain)
    return 0;

  return fChain->GetEntry(entry);
}


void AnalysisTree::Init(TTree *tree)
{
  // Set branch addresses and branch pointers
  if (!tree)
    return;

  fChain = tree;
  //fChain->SetMakeClass(1);

  fChain->SetBranchAddress("run", &run, NULL);
  fChain->SetBranchAddress("lumi_block", &lumi_block, NULL);
  fChain->SetBranchAddress("event", &event, NULL);
  //  fChain->SetBranchAddress("crossSection", &crossSection, &b_crossSection);
  fChain->SetBranchAddress("ntrue_interactions", &ntrue_interactions, &b_ntrue_interactions);
  fChain->SetBranchAddress("nvertex", &nvertex, &b_nvertex);
  fChain->SetBranchAddress("nvertexGood", &nvertexGood, &b_nvertexGood);
  fChain->SetBranchAddress("pu_nvertex", &pu_nvertex, &b_nvertex);
  fChain->SetBranchAddress("event_weight", &event_weight, &b_event_weight);
  fChain->SetBranchAddress("generator_weight", &generator_weight, NULL);
  fChain->SetBranchAddress("evtWeightTot", &evtWeightTot, &b_evtWeightTot); //Federico
  fChain->SetBranchAddress("trigger_names", &trigger_names, NULL);
  fChain->SetBranchAddress("trigger_results", &trigger_results, NULL);
  fChain->SetBranchAddress("trigger_prescale", &trigger_prescale, NULL); //Federico

  //fChain->SetCacheSize(-1);
  //fChain->AddBranchToCache("*");
}
