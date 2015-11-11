//
// Package:    GammaJetFilter
// Class:      GammaJetFilter
// 
/**\class GammaJetFilter GammaJetFilter.cc JetMETCorrections/GammaJetFilter/src/GammaJetFilter.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/
//
// Original Author:  Sébastien Brochet
//         Created:  Thu Mar 15 11:27:48 CET 2012
// $Id$
//
//


// system include files
#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include "Math/GenVector/LorentzVector.h"

// Boost
#include "boost/shared_ptr.hpp"

// user include files
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CommonTools/Utils/interface/PtComparator.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Common/interface/PtrVector.h"
#include "DataFormats/Common/interface/RefProd.h"
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/Common/interface/RefVector.h"
#include "DataFormats/Common/interface/TriggerResults.h"

#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/Records/interface/CaloTopologyRecord.h"

#include "PhysicsTools/SelectorUtils/interface/JetIDSelectionFunctor.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"

#include "EgammaAnalysis/ElectronTools/interface/PFIsolationEstimator.h"

#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "JetMETCorrections/Objects/interface/JetCorrector.h"
#include "JetMETCorrections/GammaJetFilter/interface/json/json.h"

#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/FactorizedJetCorrector.h"

#include <TParameter.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TLorentzVector.h>

#include <boost/regex.hpp>

//
// class declaration
//

enum JetAlgorithm {
  AK4,
  AK8
};

struct JetInfos {
  JetAlgorithm algo;
  edm::InputTag inputTag;
};

#define FOREACH(x) for (std::vector<std::string>::const_iterator it = x.begin(); it != x.end(); ++it)

class GammaJetFilter : public edm::EDFilter {
public:
  explicit GammaJetFilter(const edm::ParameterSet&);
  ~GammaJetFilter();
  
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:
  virtual void beginJob();
  virtual bool filter(edm::Event&, const edm::EventSetup&);
  virtual void endJob();
  
  virtual bool beginRun(edm::Run&, edm::EventSetup const&);
  virtual bool endRun(edm::Run&, edm::EventSetup const&);
  virtual bool beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);
  virtual bool endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);
  //giulia--- correct photon not necessary until we have data @13 TeV 
  void correctPhoton(pat::Photon& photon, edm::Event& iEvent, int isData, int nPV);
  //federico
  void photonStudy(pat::Photon* photon, edm::Event& iEvent);

  void correctJets(pat::JetCollection& jets, edm::Event& iEvent, const edm::EventSetup& iSetup);
  void extractRawJets(pat::JetCollection& jets);
  //giulia --- comment qg tagging stuff
  void processJets(pat::Photon* photon, pat::JetCollection& jets, const JetAlgorithm algo, /* edm::Handle<edm::ValueMap<float>>& qgTagMLP, edm::Handle<edm::ValueMap<float>>& qgTagLikelihood,*/ const edm::Handle<pat::JetCollection>& handleForRef, std::vector<TTree*>& trees);
  
  //federico -- redefine of rawMET on-the-fly --- not constant
  void correctMETWithTypeI(pat::MET& rawMet, pat::MET& met, const pat::JetCollection& jets, edm::Event& event);
  // not used - regression already implemented in the pat photon
  void correctMETWithRegressionAndTypeI(const pat::MET& rawMet, pat::MET& met, const pat::JetCollection& jets,  edm::Event& event, pat::Photon& photon, const pat::PhotonRef& photonRef);
  //federico -- re-implementation of FPR
  void correctMETWithFootprintAndTypeI(pat::MET& rawMet, pat::MET& met, const pat::JetCollection& jets,  edm::Event& event, pat::Photon& photon);
  
  bool isValidPhotonEB_SPRING15(const pat::PhotonRef& photonRef, edm::Event& event, double generatorWeight);
  bool isValidJet(const pat::Jet& jet);
  
  void readJSONFile();
  void readCSVFile();
  void updateLuminosity(const edm::LuminosityBlock& lumiBlock);
  
  // ----------member data ---------------------------
  bool mIsMC;
  bool mFilterData;
  std::string mJSONFile;
  std::string mCSVFile;
  boost::shared_ptr<Json::Value> mValidRuns;
  boost::shared_ptr<Json::Value> mCurrentRunValidLumis;
  std::map<std::pair<unsigned int, unsigned int>, double> mLumiByLS;
  std::map<std::pair<unsigned int, unsigned int>, double> mTruePUByLS;
  bool mIsValidLumiBlock;
  double mCurrentTruePU;
  
  // Photon ID
  PFIsolationEstimator mPFIsolator;
  bool mCorrPhotonWRegression; // regression in implemented from 73, not done (false)
  
  bool mRedoTypeI; 
  bool mDoFootprint;
  bool mDoJEC;  
  bool mJECFromRaw;
  std::string mCorrectorLabel;
  GreaterByPt<pat::Jet> mSorter;
  
  bool mFirstJetPtCut;
  double mFirstJetThreshold;

  std::vector<std::string> mJetCollections;
  std::map<std::string, JetInfos> mJetCollectionsData;

  // Input Tags
  edm::InputTag mPhotonsIT;
  edm::InputTag mJetsAK4PFlowIT;
  edm::InputTag mJetsAK8PFlowIT;
  edm::InputTag mJetsAK4CaloIT;
  edm::InputTag mJetsAK8CaloIT;
  
  // federico -- Photon variables computed upstream in a special producer
  //    edm::EDGetTokenT<edm::ValueMap<float> > full5x5SigmaIEtaIEtaMapToken_; // from rel73 ok in photon class
  edm::EDGetTokenT<edm::ValueMap<float> > phoChargedIsolationToken_; 
  edm::EDGetTokenT<edm::ValueMap<float> > phoNeutralHadronIsolationToken_; 
  edm::EDGetTokenT<edm::ValueMap<float> > phoPhotonIsolationToken_;
  
  edm::EDGetTokenT<pat::PackedTriggerPrescales> triggerPrescales_;
  edm::EDGetTokenT<pat::PackedCandidateCollection> pfToken_;
  

  // Events Counter
  int Event_Initial =0 ;
  int Event_VtxCut =0 ;
  int Event_MCGen =0 ;
  int Event_NPhotons=0 ;
  int Event_AfterJets =0;
  int Event_Final =0;
  
  int N_Photons_Total = 0;
  int N_Photons_Eta13 = 0;
  int N_Photons_PtRange = 0;
  
  int NoGoodPhotons = 0;
  int OnlyOneGoodPhotons = 0;
  int MoreGoodPhotons =0;
  // Counter inside the photon method
  int N_pho_Initial =0 ;
  int N_pho_genPhoton =0 ;
  int N_pho_HE =0 ;
  int N_pho_R9 =0 ;
  int N_pho_Sigma =0 ;
  int N_pho_ElecVeto =0 ;
  int N_pho_ChargedIsolation =0 ;
  int N_pho_NeutralIsolation =0 ;
  int N_pho_PhotonIsolation =0 ;
  int N_pho_Isolation =0 ;
  
  boost::shared_ptr<JetIDSelectionFunctor> mCaloJetID;
  pat::strbitset mCaloJetIDRet;
  
  double mPtHatMin;
  double mPtHatMax;

  std::vector<double>  NEvent_vec;
  std::vector<double>  R_min_vec;
  std::vector<double>  R_max_vec;
  std::vector<double>  SumE_vec;
  std::vector<double>  SumPt_vec;
  std::vector<double>  KL1FastJet_vec;
  std::vector<double>  KL1RC_vec;
  int NEvent_array[20]={0};
  double SumE_array[20]={0};
  double R_min_array[20]={0};
  double R_max_array[20]={0};
  double SumPt_array[20]={0};
  double KL1FastJet_array[20]={0};
  double KL1RC_array[20]={0};

  // Trees
  void createTrees(const std::string& rootName, TFileService& fs);
  TTree* mGenParticlesTree;
  TTree* mPhotonTree;
  TTree* mPhotonGenTree;
  TTree* mAnalysisTree;
  TTree* mElectronsTree;
  TTree* mMuonsTree;
  TTree* mPhotonStudy;
  TParameter<double>*    mTotalLuminosity;
  float                  mEventsWeight;
  double               crossSection;
  TParameter<long long>* mProcessedEvents;
  TParameter<long long>* mSelectedEvents;
  
  std::map<std::string, std::vector<TTree*> > mJetTrees;
  std::map<std::string, std::vector<TTree*> > mMETTrees;
  std::map<std::string, TTree*>               mMiscTrees;
  std::map<std::string, TTree*> mMETNFTrees;
  
  // TParameters for storing current config (JEC, correctorLabel, Treshold, etc...
  TParameter<bool>*             mJECRedone;
  TParameter<bool>*             mJECFromRawParameter;
  TNamed*                               mJECCorrectorLabel;
  TParameter<bool>*             mFirstJetPtCutParameter;
  TParameter<double>*          mFirstJetThresholdParameter;
  
  // to store sum of event weights
  TH1F* h_sumW;
  
  // DEBUG
  TH1F* EventCounter;
  TH1F* EventCounter_Raw;
  TH1F* EventCounterPhoton;
  TH1F* EventCounterPhoton_Raw;
  
  TH1F* h_N_photons;
  TH1F* h_N_photons_Raw;
  
  TH1F* mFirstJetPhotonDeltaPhi;
  TH1F* mFirstJetPhotonDeltaR;
  TH1F* mFirstJetPhotonDeltaPt;
  TH2F* mFirstJetPhotonDeltaPhiDeltaR;
  
  TH1F* mSelectedFirstJetIndex;
  TH1F* mSelectedSecondJetIndex;
  
  TH1F* mSecondJetPhotonDeltaPhi;
  TH1F* mSecondJetPhotonDeltaR;
  TH1F* mSecondJetPhotonDeltaPt;
  
  TH1F* mSelectedFirstJetPhotonDeltaPhi;
  TH1F* mSelectedFirstJetPhotonDeltaR;
  
  TH1F* mSelectedSecondJetPhotonDeltaPhi;
  TH1F* mSelectedSecondJetPhotonDeltaR;
  
  TH1F* PhotonIsolation;
  
  TH1F* PtPhotons;

  // For B / C jets neutrinos
  TClonesArray* mNeutrinos;
  TClonesArray* mNeutrinosPDG;
  
  // Cache for MC particles
  bool mDumpAllMCParticles;
  std::unordered_map<const reco::Candidate*, int> mParticlesIndexes;
  
  void particleToTree(const reco::Candidate* particle, TTree* t, std::vector<boost::shared_ptr<void> >& addresses);
  
  void updateBranch(TTree* tree, void* address, const std::string& name, const std::string& type = "F");
  template<typename U>
  void updateBranch(TTree* tree, std::vector<U>*& address, const std::string& name);
  
  void updateBranchArray(TTree* tree, void* address, const std::string& name, const std::string& size, const std::string& type = "F");
  
  void photonToTree(const pat::PhotonRef& photonRef, pat::Photon& photon, const edm::Event& event);
  void metsToTree(const pat::MET& met, const pat::MET& rawMet, const std::vector<TTree*>& trees);
  void metToTree(const pat::MET* met, TTree* tree, TTree* genTree);
  void jetsToTree(const pat::Jet* firstJet, const pat::Jet* secondJet, const std::vector<TTree*>& trees);
  void jetToTree(const pat::Jet* jet, bool findNeutrinos, TTree* tree, TTree* genTree);
  void electronsToTree(const edm::Handle<pat::ElectronCollection>& electrons, const reco::Vertex& pv);
  void muonsToTree(const edm::Handle<pat::MuonCollection>& muons, const reco::Vertex& pv);
  
  int getMotherIndex(const edm::Handle<reco::GenParticleCollection>& genParticles, const reco::Candidate* mother);
  void genParticlesToTree(const edm::Handle<reco::GenParticleCollection>& genParticles);
  //FactorizedJetCorrector
  FactorizedJetCorrector *jetCorrector;
  FactorizedJetCorrector *jetCorrectorForTypeI;
  FactorizedJetCorrector *jetCorrectorForTypeIL1;
  std::vector<JetCorrectorParameters> vPar;
  std::vector<JetCorrectorParameters> vParTypeI;
  std::vector<JetCorrectorParameters> vParTypeIL1;

  //photon study
  FactorizedJetCorrector *jetCorrectorForL1FastJet;
  FactorizedJetCorrector *jetCorrectorForL1RC;
  std::vector<JetCorrectorParameters> vParL1FastJet;
  std::vector<JetCorrectorParameters> vParL1RC;

  //giulia ---- regression no more necessary in 73X
  //define (once for all) corrector for regression
  //EnergyScaleCorrection_class *RegressionCorrector;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
GammaJetFilter::GammaJetFilter(const edm::ParameterSet& iConfig):
  mIsMC(false), mIsValidLumiBlock(false),
  // Isolations
  phoChargedIsolationToken_(consumes <edm::ValueMap<float> >
			    (iConfig.getParameter<edm::InputTag>("phoChargedIsolation"))),
  phoNeutralHadronIsolationToken_(consumes <edm::ValueMap<float> >
				  (iConfig.getParameter<edm::InputTag>("phoNeutralHadronIsolation"))),
  phoPhotonIsolationToken_(consumes <edm::ValueMap<float> >
			   (iConfig.getParameter<edm::InputTag>("phoPhotonIsolation"))),
  triggerPrescales_(consumes<pat::PackedTriggerPrescales>(iConfig.getParameter<edm::InputTag>("prescales"))),
  pfToken_(consumes<pat::PackedCandidateCollection>(iConfig.getParameter<edm::InputTag>("pfCands")))
{
  
  mIsMC = iConfig.getUntrackedParameter<bool>("isMC", "false");
  
  //giulia --- no more necessary, regression integrated in release 73X
  //Photon energy regression corrector (need to define it once for data once for mc)
  //if (mIsMC) {
  //  RegressionCorrector = new EnergyScaleCorrection_class("",edm::FileInPath("JetMETCorrections/GammaJetFilter/data/step8-stochasticSmearing-invMass_SC_regrCorrSemiParV5_pho-loose-Et_20-trigger-noPF-HggRunEtaR9Et.dat").fullPath());
  //  } else {
  //   RegressionCorrector = new EnergyScaleCorrection_class(edm::FileInPath("JetMETCorrections/GammaJetFilter/data/22Jan2012-runDepMCAll_v3-noR9shift-step8-invMass_SC_regrCorrSemiParV5_pho-loose-Et_20-trigger-noPF-HggRunEtaR9Et.dat").fullPath(),"");
  //  }
  //
  // Load the corrections files
  if (! mIsMC) { // DATA
    mJSONFile  = iConfig.getParameter<std::string>("json");
    mCSVFile    = iConfig.getParameter<std::string>("csv");
    mFilterData = iConfig.getUntrackedParameter<bool>("filterData", true);
    // Create the JetCorrectorParameter objects, the order does not matter.
    
    JetCorrectorParameters *L3JetPar = new JetCorrectorParameters(edm::FileInPath("JetMETCorrections/GammaJetFilter/data/Summer15_25nsV5/Summer15_25nsV5_DATA_L3Absolute_AK4PFchs.txt").fullPath());    
    JetCorrectorParameters *L2JetPar = new JetCorrectorParameters(edm::FileInPath("JetMETCorrections/GammaJetFilter/data/Summer15_25nsV5/Summer15_25nsV5_DATA_L2Relative_AK4PFchs.txt").fullPath());
    JetCorrectorParameters *L1JetPar = new JetCorrectorParameters(edm::FileInPath("JetMETCorrections/GammaJetFilter/data/Summer15_25nsV5/Summer15_25nsV5ch1pv_DATA_L1FastJet_AK4PFchs.txt").fullPath());
    // for Type-I MET --- To use RC instead FastJet
    JetCorrectorParameters *L1JetParForTypeI = new JetCorrectorParameters(edm::FileInPath("JetMETCorrections/GammaJetFilter/data/Summer15_25nsV5/Summer15_25nsV5ch1pv_DATA_L1RC_AK4PFchs.txt").fullPath());

    // Residual corrections for the closure test --- only for data   ---- Last version: L2L3Res_V3M3 == L2L3Res_V5
    //    JetCorrectorParameters *ResJetPar = new JetCorrectorParameters(edm::FileInPath("JetMETCorrections/GammaJetFilter/data/Summer15_25nsV5/Summer15_25nsV3M3_DATA_L2L3Residual_AK4PFchs.txt").fullPath());
    
    // Load the JetCorrectorParameter objects into a vector, IMPORTANT: THE ORDER MATTERS HERE !!!!
    
    vPar.push_back(*L1JetPar);
    vPar.push_back(*L2JetPar);
    vPar.push_back(*L3JetPar);
    //vPar.push_back(*ResJetPar); //comment if you dont want residuals
    jetCorrector = new FactorizedJetCorrector(vPar);
    //FAKE vPar for typeI fix
    vParTypeI.push_back(*L1JetParForTypeI);
    vParTypeI.push_back(*L2JetPar);
    vParTypeI.push_back(*L3JetPar);
    //vParTypeI.push_back(*ResJetPar); //comment if you dont want residuals
    jetCorrectorForTypeI = new FactorizedJetCorrector(vParTypeI);
    //FAKE vPar for typeI fix only L1
    vParTypeIL1.push_back(*L1JetParForTypeI);
    jetCorrectorForTypeIL1 = new FactorizedJetCorrector(vParTypeIL1);

    //photon study
    vParL1FastJet.push_back(*L1JetPar);
    jetCorrectorForL1FastJet = new FactorizedJetCorrector(vParL1FastJet);
    vParL1RC.push_back(*L1JetParForTypeI);
    jetCorrectorForL1RC = new FactorizedJetCorrector(vParL1RC);


    
    delete L3JetPar;
    delete L2JetPar;
    delete L1JetPar;
    //delete ResJetPar;
    delete L1JetParForTypeI;
  } else {  // MC
    // Create the JetCorrectorParameter objects, the order does not matter.

    JetCorrectorParameters *L3JetPar = new JetCorrectorParameters(edm::FileInPath("JetMETCorrections/GammaJetFilter/data/Summer15_25nsV5/Summer15_25nsV5_MC_L3Absolute_AK4PFchs.txt").fullPath());    
    JetCorrectorParameters *L2JetPar = new JetCorrectorParameters(edm::FileInPath("JetMETCorrections/GammaJetFilter/data/Summer15_25nsV5/Summer15_25nsV5_MC_L2Relative_AK4PFchs.txt").fullPath());
    JetCorrectorParameters *L1JetPar = new JetCorrectorParameters(edm::FileInPath("JetMETCorrections/GammaJetFilter/data/Summer15_25nsV5/Summer15_25nsV5_MC_L1FastJet_AK4PFchs.txt").fullPath());
    // For Type-I --- To use RC instead FastJet
    JetCorrectorParameters *L1JetParForTypeI = new JetCorrectorParameters(edm::FileInPath("JetMETCorrections/GammaJetFilter/data/Summer15_25nsV5/Summer15_25nsV5ch1pv_MC_L1RC_AK4PFchs.txt").fullPath());

    // Load the JetCorrectorParameter objects into a vector, IMPORTANT: THE ORDER MATTERS HERE !!!!
    vPar.push_back(*L1JetPar);
    vPar.push_back(*L2JetPar);
    vPar.push_back(*L3JetPar);
    jetCorrector = new FactorizedJetCorrector(vPar);
    //FAKE vPar for typeI fix
    vParTypeI.push_back(*L1JetParForTypeI);   
    vParTypeI.push_back(*L2JetPar);
    vParTypeI.push_back(*L3JetPar);
    jetCorrectorForTypeI = new FactorizedJetCorrector(vParTypeI);
    //FAKE vPar for typeI fix only L1
    vParTypeIL1.push_back(*L1JetParForTypeI);
    jetCorrectorForTypeIL1 = new FactorizedJetCorrector(vParTypeIL1);

    //photon study
    vParL1FastJet.push_back(*L1JetPar);
    jetCorrectorForL1FastJet = new FactorizedJetCorrector(vParL1FastJet);
    vParL1RC.push_back(*L1JetParForTypeI);
    jetCorrectorForL1RC = new FactorizedJetCorrector(vParL1RC);

    //
    delete L3JetPar;
    delete L2JetPar;
    delete L1JetPar;
    delete L1JetParForTypeI;
  }
  
  mPhotonsIT = iConfig.getUntrackedParameter<edm::InputTag>("photons", edm::InputTag("slimmedPhotons"));
  mCorrPhotonWRegression = iConfig.getUntrackedParameter<bool>("doPhotonRegression", false);
  mJetsAK4PFlowIT = iConfig.getUntrackedParameter<edm::InputTag>("jetsAK4PFlow", edm::InputTag("slimmedJets"));
  mJetsAK8PFlowIT = iConfig.getUntrackedParameter<edm::InputTag>("jetsAK8PFlow", edm::InputTag("slimmedJetsAK8"));
  //mJetsAK4CaloIT = iConfig.getUntrackedParameter<edm::InputTag>("jetsAK4Calo", edm::InputTag("selectedPatJets"));
  //mJetsAK8CaloIT = iConfig.getUntrackedParameter<edm::InputTag>("jetsAK8Calo", edm::InputTag("selectedPatJetsCaloAK8"));
  mDoJEC         = iConfig.getUntrackedParameter<bool>("doJetCorrection", false);
  mRedoTypeI     = iConfig.getUntrackedParameter<bool>("redoTypeIMETCorrection", false);
  mDoFootprint     = iConfig.getUntrackedParameter<bool>("doFootprintMETCorrection", false);

  if (mDoJEC){ 
    mJECFromRaw = iConfig.getUntrackedParameter<bool>("correctJecFromRaw", false);
    //    mCorrectorLabel = iConfig.getUntrackedParameter<std::string>("correctorLabel", "ak4PFResidual");
  }
  
  mFirstJetPtCut = iConfig.getUntrackedParameter<bool>("firstJetPtCut", true);
  mFirstJetThreshold = iConfig.getUntrackedParameter<double>("firstJetThreshold", 0.3);
  
  bool runOnCHS    = iConfig.getUntrackedParameter<bool>("runOnCHS", true);
  bool runOnNonCHS = iConfig.getUntrackedParameter<bool>("runOnNonCHS", false);
  
  bool runOnPFAK4    = iConfig.getUntrackedParameter<bool>("runOnPFAK4", true);
  bool runOnPFAK8    = iConfig.getUntrackedParameter<bool>("runOnPFAK8", false);
  bool runOnCaloAK4  = iConfig.getUntrackedParameter<bool>("runOnCaloAK4", false);
  bool runOnCaloAK8  = iConfig.getUntrackedParameter<bool>("runOnCaloAK8", false);
  //giulia --- default is chs, so the inputtag is the same
  //run module to do chs ab initio!
  edm::InputTag jetsAK4PFlowITchs = iConfig.getUntrackedParameter<edm::InputTag>("jetsAK4PFlowchs", edm::InputTag("slimmedJets"));
  edm::InputTag jetsAK8PFlowITchs = iConfig.getUntrackedParameter<edm::InputTag>("jetsAK8PFlowchs", edm::InputTag("slimmedJetsAK8"));
  
  if (! mIsMC && mFilterData) {
    // Load JSON file of good runs
    readJSONFile();
    readCSVFile();
  }
  
  edm::Service<TFileService> fs;
  mPhotonTree = fs->make<TTree>("photon", "photon tree");
  
  if (mIsMC){
    mPhotonGenTree = fs->make<TTree>("photon_gen", "photon gen tree");
  }  else{
    mPhotonGenTree = nullptr;
  }
  mAnalysisTree = fs->make<TTree>("analysis", "analysis tree");
  mMuonsTree = fs->make<TTree>("muons", "muons tree");
  mElectronsTree = fs->make<TTree>("electrons", "electrons tree");
  //federico
  mPhotonStudy = fs->make<TTree>("photonStudy", "photon Study tree");

  mTotalLuminosity = fs->make<TParameter<double> >("total_luminosity", 0.);
  
  crossSection = 1.;
  mEventsWeight = 1.;
  mPtHatMin     = -1.;
  mPtHatMax     = -1.;
  
  if (mIsMC) {
    // Read cross section and number of generated events
    crossSection = iConfig.getParameter<double>("crossSection");
    unsigned long long generatedEvents = iConfig.getParameter<unsigned long long>("generatedEvents");
    mEventsWeight = crossSection / (float) generatedEvents;
    
    mPtHatMin = iConfig.getUntrackedParameter<double>("ptHatMin", -1.);
    mPtHatMax = iConfig.getUntrackedParameter<double>("ptHatMax", -1.);
  }
  
  if (runOnNonCHS) {
    if (runOnPFAK4) {
      mJetCollections.push_back("PFlowAK4");
      mJetCollectionsData["PFlowAK4"] = {AK4, mJetsAK4PFlowIT};
    }
    if (runOnPFAK8) {
      mJetCollections.push_back("PFlowAK8");
      mJetCollectionsData["PFlowAK8"] = {AK8, mJetsAK8PFlowIT};
    }
  }
  
  if (runOnCHS) {
    if (runOnPFAK4) {
      mJetCollections.push_back("PFlowAK4chs");
      mJetCollectionsData["PFlowAK4chs"] = {AK4, jetsAK4PFlowITchs};
    }
    if (runOnPFAK8) {
      mJetCollections.push_back("PFlowAK8chs");
      mJetCollectionsData["PFlowAK8chs"] = {AK8, jetsAK8PFlowITchs};
    }
  }
  
  if (runOnCaloAK4) {
    mJetCollections.push_back("CaloAK4");
    mJetCollectionsData["CaloAK4"]  = {AK4, mJetsAK4CaloIT};
  }
  
  if (runOnCaloAK8) {
    mJetCollections.push_back("CaloAK8");
    mJetCollectionsData["CaloAK8"]  = {AK8, mJetsAK8CaloIT};
  }
  
  FOREACH(mJetCollections) {
    createTrees(*it, *fs);
  }
  
  mProcessedEvents = fs->make<TParameter<long long> >("total_events", 0);
  mSelectedEvents = fs->make<TParameter<long long> >("passed_events", 0);
  
  mJECRedone = fs->make<TParameter<bool> >("jec_redone", mDoJEC, '*');
  mFirstJetPtCutParameter = fs->make<TParameter<bool> >("cut_on_first_jet_pt", mFirstJetPtCut, '*');
  if (mDoJEC) {
    mJECFromRawParameter = fs->make<TParameter<bool> >("jec_from_raw_jet", mJECFromRaw, '*');
    //    mJECCorrectorLabel = fs->make<TNamed>("jec_corrector_label", mCorrectorLabel);
  }
  
  if (mFirstJetPtCut) {
    mFirstJetThresholdParameter = fs->make<TParameter<double> >("cut_on_first_jet_treshold", mFirstJetThreshold);
  }
  

  if (mIsMC){
    // to store the sum of weights
    h_sumW = fs->make<TH1F>("h_sumW", "h_sumW", 1, -0.5, 5.5);
    h_sumW->Sumw2();
  }  else{
    h_sumW = nullptr;
  }
  
  EventCounter = fs->make<TH1F>("EventCounter", "EventCounter", 7, -0.5, 6.5);
  EventCounter_Raw = fs->make<TH1F>("EventCounter_Raw", "EventCounter_Raw", 7, -0.5, 6.5);
  EventCounterPhoton = fs->make<TH1F>("EventCounterPhoton", "EventCounterPhoton", 11, -0.5, 10.5);
  EventCounterPhoton_Raw = fs->make<TH1F>("EventCounterPhoton_Raw", "EventCounterPhoton_Raw", 11, -0.5, 10.5);

  PtPhotons = fs->make<TH1F>("PtPhotons", "PtPhotons", 100, 0, 500); //all photons ->no selection
  
  mFirstJetPhotonDeltaPhi = fs->make<TH1F>("firstJetPhotonDeltaPhi", "firstJetPhotonDeltaPhi", 50, 0., M_PI);
  mFirstJetPhotonDeltaR = fs->make<TH1F>("firstJetPhotonDeltaR", "firstJetPhotonDeltaR", 80, 0, 10);
  mFirstJetPhotonDeltaPt = fs->make<TH1F>("firstJetPhotonDeltaPt", "firstJetPhotonDeltaPt", 100, 0, 50);
  mFirstJetPhotonDeltaPhiDeltaR = fs->make<TH2F>("firstJetPhotonDeltaPhiDeltaR", "firstJetPhotonDeltaPhiDeltaR", 50, 0, M_PI, 80, 0, 10);

  mSelectedFirstJetIndex = fs->make<TH1F>("selectedFirstJetIndex", "selectedFirstJetIndex", 20, 0, 20);
  mSelectedSecondJetIndex = fs->make<TH1F>("selectedSecondJetIndex", "selectedSecondJetIndex", 20, 0, 20);

  mSecondJetPhotonDeltaPhi = fs->make<TH1F>("secondJetPhotonDeltaPhi", "secondJetPhotonDeltaPhi", 50, 0., M_PI);
  mSecondJetPhotonDeltaR = fs->make<TH1F>("secondJetPhotonDeltaR", "secondJetPhotonDeltaR", 80, 0, 10);
  mSecondJetPhotonDeltaPt = fs->make<TH1F>("secondJetPhotonDeltaPt", "secondJetPhotonDeltaPt", 100, 0, 50);
  
  mSelectedFirstJetPhotonDeltaPhi = fs->make<TH1F>("selectedFirstJetPhotonDeltaPhi", "selectedFirstJetPhotonDeltaPhi", 50, 0., M_PI);
  mSelectedFirstJetPhotonDeltaR = fs->make<TH1F>("selectedFirstJetPhotonDeltaR", "selectedFirstJetPhotonDeltaR", 80, 0, 10);
  
  mSelectedSecondJetPhotonDeltaPhi = fs->make<TH1F>("selectedSecondJetPhotonDeltaPhi", "selectedSecondJetPhotonDeltaPhi", 50, 0., M_PI);
  mSelectedSecondJetPhotonDeltaR = fs->make<TH1F>("selectedSecondJetPhotonDeltaR", "selectedSecondJetPhotonDeltaR", 80, 0, 10);
  
  mPFIsolator.initializePhotonIsolation(true);
  mPFIsolator.setConeSize(0.3);
  
  mNeutrinos = NULL;
  mNeutrinosPDG = NULL;
  if (mIsMC) {
    mNeutrinos = new TClonesArray("TLorentzVector", 3);
    mNeutrinosPDG = new TClonesArray("TParameter<int>", 3);
  }
}


GammaJetFilter::~GammaJetFilter()
{
  
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  
  delete mNeutrinos;
  delete mNeutrinosPDG;
}

void GammaJetFilter::createTrees(const std::string& rootName, TFileService& fs) {
  
  TFileDirectory dir = fs.mkdir(rootName);
  std::vector<TTree*>& trees = mJetTrees[rootName];
  
  trees.push_back(dir.make<TTree>("first_jet", "first jet tree"));
  trees.push_back(dir.make<TTree>("second_jet", "second jet tree"));
  
  trees.push_back(dir.make<TTree>("first_jet_raw", "first raw jet tree"));
  trees.push_back(dir.make<TTree>("second_jet_raw", "second raw jet tree"));
  
  if (mIsMC) {
    trees.push_back(dir.make<TTree>("first_jet_gen", "first gen jet tree"));
    trees.push_back(dir.make<TTree>("second_jet_gen", "second gen jet tree"));
  } else {
    trees.push_back(nullptr);
    trees.push_back(nullptr);
  }
  
  // MET
  std::vector<TTree*>& met = mMETTrees[rootName];
  met.push_back(dir.make<TTree>("met", "met tree"));
  met.push_back(dir.make<TTree>("met_raw", "met raw tree"));
  
  if (mIsMC)
    met.push_back(dir.make<TTree>("met_gen", "met gen tree"));
  else
    met.push_back(nullptr);
  
  // Misc
  mMiscTrees[rootName] = dir.make<TTree>("misc", "misc tree");
}

void GammaJetFilter::updateBranch(TTree* tree, void* address, const std::string& name, const std::string& type/* = "F"*/) {
  TBranch* branch = tree->GetBranch(name.c_str());
  if (branch == NULL) {
    branch = tree->Branch(name.c_str(), address, std::string(name + "/" + type).c_str()); 
  } else {
    branch->SetAddress(address);
  }
}

template<typename U> void GammaJetFilter::updateBranch(TTree* tree, std::vector<U>*& address, const std::string& name) {
  TBranch* branch = tree->GetBranch(name.c_str());
  if (branch == NULL) {
    branch = tree->Branch(name.c_str(), &address); 
  } else {
    branch->SetAddress(&address);
  }
}

void GammaJetFilter::updateBranchArray(TTree* tree, void* address, const std::string& name, const std::string& size, const std::string& type/* = "F"*/) {
  TBranch* branch = tree->GetBranch(name.c_str());
  if (branch == NULL) {
    branch = tree->Branch(name.c_str(), address, std::string(name + "[" + size + "]/" +type).c_str()); 
  } else {
    branch->SetAddress(address);
  }
}

//
// member functions
//


// ------------ method called on each new Event  ------------
bool GammaJetFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  
  mProcessedEvents->SetVal(mProcessedEvents->GetVal() + 1);
  
  if (! mIsMC && mFilterData && ! mIsValidLumiBlock) {
    return false;
  }
  
  double generatorWeight = 1.;
  
  if (mIsMC) {
    edm::Handle<GenEventInfoProduct> eventInfos;
    iEvent.getByLabel("generator", eventInfos);
    if (eventInfos.isValid() && eventInfos->hasBinningValues()) {
      double genPt = eventInfos->binningValues()[0];
      
      if (mPtHatMin >= 0. && genPt < mPtHatMin)
	return false;
      if (mPtHatMax >= 0. && genPt > mPtHatMax)
	return false;
    }
    generatorWeight = eventInfos->weight();
    h_sumW->Fill(0.,generatorWeight);
    if (generatorWeight == 0.) {
      generatorWeight = 1.;
    }
  }// end IsMC
  
  Event_Initial++;
  EventCounter -> AddBinContent(1, generatorWeight );
  
  Event_MCGen++;
  EventCounter -> AddBinContent(2, generatorWeight );
  
  // Vertex
  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByLabel("offlineSlimmedPrimaryVertices", vertices);
  
  // Keep events with at least one vertex
  if (!vertices.isValid() || vertices->size() == 0 || vertices->front().isFake())
    return false;
  
  const reco::Vertex& primaryVertex = vertices->at(0);
  
  EventCounter -> AddBinContent(3, generatorWeight );
  Event_VtxCut++;
  
  int nPVGood =0;  
  reco::VertexCollection::const_iterator i_pv, endpv = vertices->end();
  for (i_pv = vertices->begin();  i_pv != endpv;  ++i_pv) {
    if ( !i_pv->isFake() && i_pv->ndof() > 4){
      nPVGood++;
    }
  }
  
  
  edm::Handle<double> pFlowRho;
  iEvent.getByLabel(edm::InputTag("offlineSlimmedPrimaryVertices"), pFlowRho); // For photon ID
  
  // Necessery collection for calculate sigmaIPhiIPhi
  // 2011 Photon ID
  edm::ESHandle<CaloTopology> topology;
  iSetup.get<CaloTopologyRecord>().get(topology);
  
  edm::Handle<pat::PhotonCollection> photons; //handle for pat::photonref
  iEvent.getByLabel(mPhotonsIT, photons);
  pat::PhotonCollection photons_nonconst = *photons; //handle for pat::photon
  pat::PhotonCollection::iterator it = photons_nonconst.begin();
  std::vector<pat::Photon> photonsVec;
  pat::Photon pho_tmp;
  
  uint32_t index = 0;
  uint32_t goodPhoIndex = -1;
  for (; it != photons_nonconst.end(); ++it, index++) { 
    pho_tmp=*it;
    
    N_Photons_Total++ ;
    EventCounterPhoton -> AddBinContent(1, generatorWeight );
    
    if (fabs(it->eta()) <= 1.3) {       
      N_Photons_Eta13++;          
      EventCounterPhoton -> AddBinContent(2, generatorWeight );
      // pT distrubution for all photon      
      PtPhotons -> Fill(it->pt() );
  
      N_Photons_PtRange++;      
      EventCounterPhoton -> AddBinContent(3, generatorWeight );
      
      pat::PhotonRef PhotonReftmp(photons, index);
      
      if (isValidPhotonEB_SPRING15(PhotonReftmp, iEvent, generatorWeight)) {
	photonsVec.push_back(*it);
	goodPhoIndex=index;
      }
    }
  }
  
  //  std::cout <<"Number of good photons per event = " << photonsVec.size() << std::endl;
  
  if(photonsVec.size() == 0)    NoGoodPhotons++;
  if(photonsVec.size() == 1)    OnlyOneGoodPhotons++;
  if(photonsVec.size()    > 1)    MoreGoodPhotons++;
  
  // Only one good photon per event
  if (photonsVec.size() != 1)    return false;
  
  Event_NPhotons++;   
  EventCounterPhoton -> AddBinContent(10, generatorWeight );
  EventCounter -> AddBinContent(4, generatorWeight );
  
  pat::Photon photon = photonsVec[0];
  pat::PhotonRef GoodphotonRef(photons, goodPhoIndex);
  
  //for technical reasons i need a photonref and a photon. 
  //Since there is only one photon in these events, we are sure that the 
  //goodPhoIndex is referring to the same photon as photonsVec[0];
  //
  //float regressionCorr=1.;

  //giulia --- regression integrated in 73X , putting FALSE by hand!
  mCorrPhotonWRegression = false;
  
  if (mCorrPhotonWRegression) { // do nothing    
    //calculate the regression energy using photonRef and getting the reco object
    edm::Handle<edm::ValueMap<float>> regressionEnergyHandle;
    iEvent.getByLabel(edm::InputTag("eleNewEnergiesProducer", "energySCEleJoshPhoSemiParamV5ecorr", "PAT"),regressionEnergyHandle);
    edm::Ptr<reco::Candidate> GoodrecoObject = GoodphotonRef->originalObjectRef();
    //float GoodregressionEnergy = (*regressionEnergyHandle)[GoodrecoObject] ;
    //correct this january 15
    //  regressionCorr = GoodregressionEnergy/(GoodphotonRef->energy());
    //rescale the photon to the regression energy (rescale the whole p4 by the ratio of regression energy over uncorrected energy)
    //correct this january 15
    //  photon.setP4(photon.p4()*regressionCorr);
    //now apply the additional correction (data) or smearing (mc)
    //
    int processingdata=1;
    if(mIsMC) processingdata=0;
    correctPhoton(photon,iEvent, processingdata, int(vertices->size()));
  } // end regression
  
  // Process jets
  edm::Handle<pat::JetCollection> jetsHandle;
  
  FOREACH(mJetCollections) {
    
    JetInfos infos = mJetCollectionsData[*it];
    
    iEvent.getByLabel(infos.inputTag, jetsHandle);
    pat::JetCollection jets = *jetsHandle;
    if (mDoJEC) {
      correctJets(jets, iEvent, iSetup);
    } else {
      extractRawJets(jets);
    }
    
    //giulia --- comment QG tagging stuff
    //edm::Handle<edm::ValueMap<float>>  qgTagHandleMLP;
    //edm::Handle<edm::ValueMap<float>>  qgTagHandleLikelihood;
    //iEvent.getByLabel("QGTagger" + *it,"qgMLP", qgTagHandleMLP);
    //iEvent.getByLabel("QGTagger" + *it,"qgLikelihood", qgTagHandleLikelihood);
    
    processJets(&photon, jets, infos.algo, /*qgTagHandleMLP, qgTagHandleLikelihood, */jetsHandle, mJetTrees[*it]);
    
    Event_AfterJets++;     
    EventCounter -> AddBinContent(5, generatorWeight );
    
    // MET
    edm::Handle<pat::METCollection> metsHandle;
    iEvent.getByLabel(edm::InputTag("slimmedMETs"), metsHandle);
    
    edm::Handle<pat::METCollection> rawMetsHandle;
    iEvent.getByLabel(edm::InputTag("slimmedMETs"),  rawMetsHandle);
    
    pat::METCollection mets = *metsHandle;
    pat::MET& met = mets[0];
    
    pat::METCollection rawMets = *rawMetsHandle;
    pat::MET& rawMet = rawMets[0];
     
    if (mDoJEC || mRedoTypeI) { // authomatic done if mDoJEC is done
      if (mDoFootprint) {
	correctMETWithFootprintAndTypeI(rawMet, met, jets, iEvent, photon);
      } else {
	if (mCorrPhotonWRegression) { // regression done nothing
	  correctMETWithRegressionAndTypeI(rawMet, met, jets, iEvent, photon, GoodphotonRef);
	} else {
	  correctMETWithTypeI(rawMet, met, jets, iEvent);
	}
      }
    }

    if (rawMetsHandle.isValid())
      metsToTree(met, rawMet, mMETTrees[*it]);
    else {
      pat::MET emptyRawMet = pat::MET();
      metsToTree(met, emptyRawMet, mMETTrees[*it]);
    }
       
    // Rho
    edm::Handle<double> rhos;
    if (it->find("Calo") != std::string::npos)
      iEvent.getByLabel(edm::InputTag("fixedGridRhoFastjetAllCalo"), rhos);
    else
      iEvent.getByLabel(edm::InputTag("fixedGridRhoFastjetAll"), rhos);
    
    double rho = *rhos;
    updateBranch(mMiscTrees[*it], &rho, "rho", "D");
    
    mMiscTrees[*it]->Fill();
  }
  
  // Number of vertices for pu reweighting
  edm::Handle<std::vector<PileupSummaryInfo> > puInfos;
  iEvent.getByLabel(edm::InputTag("slimmedAddPileupInfo"), puInfos);
  
  float nTrueInteractions = -1;
  int nPUVertex = -1;
  unsigned int nVertex = vertices->size();
  
  edm::EventID eventId = iEvent.id();
  EventNumber_t event = eventId.event();
  RunNumber_t run = eventId.run();
  LuminosityBlockNumber_t lumiBlock = eventId.luminosityBlock();
  
  
  if (mIsMC) {
    /* // old stuff -- do the same thing
       for (std::vector<PileupSummaryInfo>::const_iterator it = puInfos->begin(); it != puInfos->end();
       ++it) {
       
       int BX = it->getBunchCrossing();
       if (BX == 0) {
       nPUVertex = it->getPU_NumInteractions();
       nTrueInteractions = it->getTrueNumInteractions();
       break;
       }
     }     
     if (nPUVertex < 0) {
       throw cms::Exception("PUReweighting") << "No in-time beam crossing found!" << std::endl;
       }*/ 
    nTrueInteractions = puInfos->at(1).getTrueNumInteractions();
  } else {
    // updated and saved in the 2nd step (from file)
    nTrueInteractions = -1;
  }
  
  updateBranch(mAnalysisTree, &run, "run", "i");
  updateBranch(mAnalysisTree, &lumiBlock, "lumi_block", "i");
  updateBranch(mAnalysisTree, &event, "event", "i");
  //   updateBranch(mAnalysisTree, &crossSection, "crossSection", "D");
  updateBranch(mAnalysisTree, &nVertex, "nvertex", "i");
  updateBranch(mAnalysisTree, &nPVGood, "nvertexGood", "i");
  updateBranch(mAnalysisTree, &nTrueInteractions, "ntrue_interactions");
  updateBranch(mAnalysisTree, &nPUVertex, "pu_nvertex", "I");
  updateBranch(mAnalysisTree, &mEventsWeight, "event_weight"); // Only valid for binned samples
  updateBranch(mAnalysisTree, &generatorWeight, "generator_weight", "D"); // Only valid for flat samples
  
  // Triggers
  edm::Handle<edm::TriggerResults> triggerResults;
  iEvent.getByLabel(edm::InputTag("TriggerResults", "", "HLT"), triggerResults);
  
  edm::Handle<pat::PackedTriggerPrescales> triggerPrescales;
  iEvent.getByToken(triggerPrescales_, triggerPrescales);
  
  std::vector<std::string>* trigNames = new std::vector<std::string>();
  std::vector<bool>* trigResults = new std::vector<bool>();
  std::vector<double>* trigPrescale = new std::vector<double>();
  
  if (triggerResults.isValid()) {
    static std::vector<boost::regex> validTriggers = { boost::regex("HLT_Photon.*_v.*", boost::regex_constants::icase) };
    const edm::TriggerNames& triggerNames = iEvent.triggerNames(*triggerResults);
    
    size_t size = triggerResults->size();
    
    for (size_t i = 0; i < size; i++) {
      std::string triggerName = triggerNames.triggerName(i);
      bool isValid = false;
      
      for (boost::regex& validTrigger: validTriggers) {
	if (boost::regex_match(triggerName, validTrigger)) {
	  isValid = true;
	  break;
	}
      }
      
      if (!isValid)
	continue;
      
      unsigned int index = triggerNames.triggerIndex(triggerName);
      bool passed = triggerResults->accept(index);
      double prescale = triggerPrescales->getPrescaleForIndex(index);
      
      trigPrescale -> push_back(prescale);	
      trigResults->push_back(passed);
      trigNames->push_back(triggerName);
    }
  }
  
  // Create branches, even if they're empty
  updateBranch(mAnalysisTree, trigNames, "trigger_names");
  updateBranch(mAnalysisTree, trigResults, "trigger_results");
  updateBranch(mAnalysisTree, trigPrescale, "trigger_prescale");
  
  mAnalysisTree->Fill();
  
  delete trigNames;
  delete trigResults;
  delete trigPrescale;
  
  photonToTree(GoodphotonRef, photon, iEvent);
  
  photonStudy(&photon, iEvent);


  // Electrons
  edm::Handle<pat::ElectronCollection> electrons;
  iEvent.getByLabel("slimmedElectrons", electrons);
  electronsToTree(electrons, primaryVertex);
  
  // Muons
  edm::Handle<pat::MuonCollection> muons;
  iEvent.getByLabel("slimmedMuons", muons);
  muonsToTree(muons, primaryVertex);
  
  mSelectedEvents->SetVal(mSelectedEvents->GetVal() + 1);
  
  EventCounter -> AddBinContent(6, generatorWeight );
  Event_Final ++ ;
  
  return true;
}


// giulia -- correct energy of the photon already implemented in 73X
// smearing - scale factors not necessary until we have new data @13 TeV
//DO NOTHING!
void GammaJetFilter::correctPhoton(pat::Photon& photon, edm::Event& iEvent, int isData, int nPV) {
  //  edm::EventID eventId = iEvent.id();
  
  //  if(isData==1) {
  //    
  //    float scalecorr = RegressionCorrector->ScaleCorrection(eventId.run(),true,photon.r9(),photon.eta(),photon.pt(),nPV,15.); //nPVmean is not actually used in the function, so it is set to a dummy value
  //    
  //    
  //    //ScaleCorrection(int runNumber, bool isEBEle, double R9Ele, double etaSCEle, double EtEle, int nPV, float nPVmean=0);
  //    photon.setP4(photon.p4()*scalecorr);
  //  } else {
  //    reco::SuperClusterRef superCluster = photon.superCluster();
  //    float smearcorr = RegressionCorrector->getSmearing(eventId.run(),photon.energy(),true,photon.r9(),superCluster->eta());
  //    //getSmearing(int runNumber, float energy, bool isEBEle, float R9Ele, float etaSCEle);
  //    photon.setP4(photon.p4()*smearcorr);
  //  }
}
//////////////////////////
//federico -- 
void GammaJetFilter::photonStudy(pat::Photon* photon, edm::Event& iEvent) {

  bool verbose = true;

  edm::Handle<pat::PackedCandidateCollection> pfs;
  iEvent.getByToken(pfToken_, pfs);

  std::vector<reco::CandidatePtr> footprint;
  for (unsigned int i = 0, n = photon->numberOfSourceCandidatePtrs(); i < n; ++i) {
    // pfCandidate associated with the photon
    footprint.push_back(photon->sourceCandidatePtr(i) );
  }

  for(int ii=0;  ii<11;  ii++){  //loop on annulus

    double R_min = ii / 10.; //radius min of annulus
    double R_max = R_min + 0.1; // radius max 
    float SumE_PFCandidate = 0;
    float SumPt_PFCandidate = 0;

    if(verbose) std::cout<< "pT Photon  "<< photon->pt() << std::endl;    
    if(verbose) std::cout<<"Range radius: "<< R_min <<" e "<<R_max<<std::endl; 

    // first method -- Energy density by hand: sum of pf candidate energy in an annulus of R_min and R_max       
    for (unsigned int i = 0, n = pfs->size(); i < n; ++i) { //loop on of candidate
      const pat::PackedCandidate &pf = (*pfs)[i];

      double deltaR = reco::deltaR(pf, *photon);

      if ( deltaR > R_min && deltaR<=R_max ) {
	if(verbose) std::cout<< "DeltaR " << deltaR <<std::endl;
	// don't use the photon pfCandidate  (important only in the first bin)
	if (std::find(footprint.begin(), footprint.end(), reco::CandidatePtr(pfs,i)) != footprint.end()){
	  if(verbose) std::cout<<"Skip photon  pfCandidate"<< std::endl;
	  if(verbose) std::cout<<"Energy NOT in the sum: "<<pf.energy()  <<std::endl;
	  continue;
	}      
	if(verbose) std::cout<<"Energy to sum:  "<< pf.energy()<< std::endl;
	SumE_PFCandidate += pf.energy();
	SumPt_PFCandidate += pf.pt();
      }// range deltaR      
    }// loop over pfCand
     
    if(verbose) std::cout<<"Final Energy:  "<< SumE_PFCandidate<< std::endl;
    if(verbose) std::cout<<"Final Pt:  "<< SumPt_PFCandidate<< std::endl;
    
    // other methods used the area of annulus -- calculated:
    double Area = M_PI * ( R_max*R_max - R_min*R_min );
    if(verbose) std::cout<< "Area  "<< Area << std::endl;    

    edm::Handle<double> rho_;
    iEvent.getByLabel(edm::InputTag("fixedGridRhoFastjetAll"), rho_);
    
    // second method -- corrector factor using L1FastJet
    double correctionsL1FastJet =1.;
    jetCorrectorForL1FastJet->setJetEta(photon->eta());
    jetCorrectorForL1FastJet->setJetPt(photon->pt());
    jetCorrectorForL1FastJet->setJetA(Area);
    jetCorrectorForL1FastJet->setRho(*rho_);
    correctionsL1FastJet = jetCorrectorForL1FastJet->getCorrection();        
    if(verbose) std::cout<< "Correction L1FastJet  "<< correctionsL1FastJet << std::endl;
    // K = energy density * Area (?) = pt(phot) - Corr * pt(phot)
    double K_L1FastJet = photon->pt() -  (correctionsL1FastJet * photon->pt() ) ;
    if(verbose) std::cout<< "K L1FastJet  "<< K_L1FastJet << std::endl;

    // third method -- corrector factor using L1RC
    double correctionsL1RC =1.;
    jetCorrectorForL1RC->setJetEta(photon->eta());
    jetCorrectorForL1RC->setJetPt(photon->pt());
    jetCorrectorForL1RC->setJetA(Area);
    jetCorrectorForL1RC->setRho(*rho_);
    correctionsL1RC = jetCorrectorForL1RC->getCorrection();         
    if(verbose) std::cout<< "CorrectionL1RC  "<< correctionsL1RC << std::endl;   
    double K_L1RC =photon->pt() - (correctionsL1RC * photon->pt()) ;
    if(verbose) std::cout<< "K L1RC  "<< K_L1RC << std::endl;

    //////////// sum over all events -- Mean will be done in a second step

    if(verbose) std::cout << "ii "<< ii << std::endl;
    R_min_array[ii] = R_min;
    R_max_array[ii] = R_max;
    NEvent_array[ii]++;
    SumE_array[ii] +=SumE_PFCandidate;
    SumPt_array[ii] +=SumPt_PFCandidate;
    KL1FastJet_array[ii] +=K_L1FastJet;
    KL1RC_array[ii] +=K_L1RC;

   }// for on radius
 

}// end method "photonStudy"
/////////////////////////////
void GammaJetFilter::correctJets(pat::JetCollection& jets, edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Get Jet corrector
  //  const JetCorrector* corrector = JetCorrector::getJetCorrector(mCorrectorLabel, iSetup);
  
  // Correct jets
  for (pat::JetCollection::iterator it = jets.begin(); it != jets.end(); ++it)  {
    pat::Jet& jet = *it;
    
    // Store raw jet, it's not possible to get it after corrections
    pat::Jet rawJet = jet.correctedJet("Uncorrected");
    jet.addUserData("rawJet", rawJet, true); // Store raw jet inside our jet. This allow us to correctly sort the resulting collection
    pat::Jet L1Jet  = jet.correctedJet("L1FastJet");
    jet.addUserData("L1Jet", L1Jet, true); // Embed L1 corrected jet for TypeI correction
    
    if (mJECFromRaw) {
      double toRaw = jet.jecFactor("Uncorrected");
      jet.setP4(jet.p4() * toRaw); // It's now a raw jet
    }
    
    //    std::cout<<"Raw Jet   "<< jet.pt() <<std::endl;
    
    double corrections =1.;
    /*   
	 if(mIsMC){
	 //MC: before corrections from GlobalTag maybe
	 corrections = corrector->correction(jet, iEvent, iSetup);
	 std::cout<<"corrections   "<< corrections <<std::endl;
	 } else {
	 // Data: correction from txt file
	 edm::Handle<double> rho_;
	 iEvent.getByLabel(edm::InputTag("fixedGridRhoFastjetAll"), rho_);
	 jetCorrector->setJetEta(jet.eta());
	 jetCorrector->setJetPt(jet.pt());
	 jetCorrector->setJetA(jet.jetArea());
	 jetCorrector->setRho(*rho_);
	 corrections = jetCorrector->getCorrection();
	 std::cout<<"corrections   "<< corrections <<std::endl;
	 }
    */ 
    // NOW: corrections from txt file for both DATA and MC
    edm::Handle<double> rho_;
    iEvent.getByLabel(edm::InputTag("fixedGridRhoFastjetAll"), rho_);
    jetCorrector->setJetEta(jet.eta());
    jetCorrector->setJetPt(jet.pt());
    jetCorrector->setJetA(jet.jetArea());
    jetCorrector->setRho(*rho_);
    corrections = jetCorrector->getCorrection();     
    
    jet.scaleEnergy(corrections);
    
    //    std::cout<<"Corrected Jet   "<< jet.pt() <<std::endl;
  }
  
  // Sort collection by pt
  std::sort(jets.begin(), jets.end(), mSorter);
}


void GammaJetFilter::correctMETWithTypeI(pat::MET& rawMet, pat::MET& met, const pat::JetCollection& jets, edm::Event& event) {
  
  //  std::cout<< " Correct MET only Type I "<< std::endl; 

  //  rawMet.setP4(reco::Candidate::PolarLorentzVector(met.uncorrectedPt(), met.eta(), met.uncorrectedPhi(), met.uncorrectedSumEt() ) );
  rawMet.setP4(reco::Candidate::PolarLorentzVector(met.uncorPt(), met.eta(), met.uncorPhi(), met.uncorSumEt() ) ); //new release

  //  std::cout<< "componenti della met:  "<< met.uncorPt() << "  " << met.eta() << " " << met.uncorPhi() << " " << met.uncorSumEt() << std::endl;

  double deltaPx = 0., deltaPy = 0.;
  // See https://indico.cern.ch/getFile.py/access?contribId=1&resId=0&materialId=slides&confId=174324 slide 4
  
  for (pat::JetCollection::const_iterator it = jets.begin(); it != jets.end(); ++it) {
    const pat::Jet& jet = *it;
    
    if (jet.pt() > 10) {
      
      const pat::Jet* rawJet = jet.userData<pat::Jet>("rawJet");
      /*//without typei fix
	const pat::Jet* L1Jet  = jet.userData<pat::Jet>("L1Jet");
	reco::Candidate::LorentzVector L1JetP4  = L1Jet->p4();
      */
      
      //with typeI fix
      double corrsForTypeI =1.;
      double corrsForTypeIL1=1.;
      edm::Handle<double> rho_;
      event.getByLabel(edm::InputTag("fixedGridRhoFastjetAll"), rho_);
      
      jetCorrectorForTypeIL1->setJetEta(rawJet->eta());
      jetCorrectorForTypeIL1->setJetPt(rawJet->pt());
      jetCorrectorForTypeIL1->setJetA(rawJet->jetArea());
      jetCorrectorForTypeIL1->setRho(*rho_);
      corrsForTypeIL1 = jetCorrectorForTypeIL1->getCorrection();
      
      pat::Jet jetL1 = *rawJet;
      jetL1.scaleEnergy(corrsForTypeIL1);
      
      jetCorrectorForTypeI->setJetEta(rawJet->eta());
      jetCorrectorForTypeI->setJetPt(rawJet->pt());
      jetCorrectorForTypeI->setJetA(rawJet->jetArea());
      jetCorrectorForTypeI->setRho(*rho_);
      corrsForTypeI = jetCorrectorForTypeI->getCorrection();
      
      pat::Jet jet = *rawJet;
      jet.scaleEnergy(corrsForTypeI);
      
      //typeI fix
      reco::Candidate::LorentzVector L1JetP4  = jetL1.p4();
      
      double emEnergyFraction = rawJet->chargedEmEnergyFraction() + rawJet->neutralEmEnergyFraction();
      if (emEnergyFraction > 0.90)
        continue;
      
      ////without typei fix
      //     //reco::Candidate::LorentzVector rawJetP4 = rawJet->p4();
      //      reco::Candidate::LorentzVector L1JetP4  = L1Jet->p4();
      
      deltaPx += (jet.px() - L1JetP4.px());
      deltaPy += (jet.py() - L1JetP4.py());
    } // jet.pt() > 10
  }//loop over jets
  
  double correctedMetPx = rawMet.px() - deltaPx;
  double correctedMetPy = rawMet.py() - deltaPy;
  double correctedMetPt = sqrt(correctedMetPx * correctedMetPx + correctedMetPy * correctedMetPy);
  
  met.setP4(reco::Candidate::LorentzVector(correctedMetPx, correctedMetPy, 0., correctedMetPt));
}

void GammaJetFilter::correctMETWithFootprintAndTypeI(pat::MET& rawMet, pat::MET& met, const pat::JetCollection& jets, edm::Event& event, pat::Photon& photon) {
  
  //  std::cout<< " Correct MET With FootPrint "<< std::endl; 
  
  edm::Handle<pat::PackedCandidateCollection> pfs;
  event.getByToken(pfToken_, pfs);
  
  float FootprintMEx = 0;
  float FootprintMEy = 0;
  // std::cout<< " Inizializzazione "<< std::endl; 
  // std::cout<< " FootprintMEx "<< FootprintMEx << std::endl; 
  // std::cout<< " FootprintMEy "<< FootprintMEy << std::endl; 
  // std::cout<< " NPF Candidate To Remove "<< photon.numberOfSourceCandidatePtrs() << std::endl; 
  
  std::vector<reco::CandidatePtr> footprint;
  for (unsigned int i = 0, n = photon.numberOfSourceCandidatePtrs(); i < n; ++i) {
    footprint.push_back(photon.sourceCandidatePtr(i) );
  }
  // now loop on pf candidates
  for (unsigned int i = 0, n = pfs->size(); i < n; ++i) {
    const pat::PackedCandidate &pf = (*pfs)[i];
    // pfcandidate-based footprint removal
    if (std::find(footprint.begin(), footprint.end(), reco::CandidatePtr(pfs,i)) != footprint.end()) {
      //   std::cout<< "pfCandidate ESCLUSE # "<< i << std::endl; 
      //   std::cout<< " pf.px "<< pf.px() << std::endl; 
      //   std::cout<< " pf.py "<< pf.py() << std::endl; 
      //   std::cout<< " pf.pt "<< pf.pt() << std::endl; 
      continue;
    }
    FootprintMEx += -1.* pf.px();
    FootprintMEy += -1.* pf.py();    
    //std::cout<< "pfCandidate # "<< i << std::endl; 
    // std::cout<< " pf.px "<< pf.px() << std::endl; 
    // std::cout<< " pf.py "<< pf.py() << std::endl; 
  }// loop over pfCand
  
  
  // Re-adding  photon but reco 
  FootprintMEx += -1.* photon.px();
  FootprintMEy += -1.* photon.py();

  double FootprintMEPt = sqrt(FootprintMEx * FootprintMEx + FootprintMEy * FootprintMEy);   
  
  //  std::cout<< "MEx MEy Finale corretta con Footprint "<< std::endl; 
  //  std::cout<< " FootprintMEx "<< FootprintMEx << std::endl; 
  //  std::cout<< " FootprintMEy "<< FootprintMEy << std::endl; 
  //  std::cout<< " FootprintMEPt "<< FootprintMEPt << std::endl;   
  
  rawMet.setP4(reco::Candidate::LorentzVector(FootprintMEx, FootprintMEy, 0., FootprintMEPt));
  
  //  std::cout<< " rawMet.et "<< rawMet.et() << std::endl;   
  
  /////////////// Propagate the JEC to MET 
  // See https://indico.cern.ch/getFile.py/access?contribId=1&resId=0&materialId=slides&confId=174324 slide 4
  //TypeI fix : use different L1corrections - only for typeI calculation! - 
  double deltaPx = 0., deltaPy = 0.;
  
  for (pat::JetCollection::const_iterator it = jets.begin(); it != jets.end(); ++it) { 
    //embed raw and l1 jet
    const pat::Jet* rawJet = it->userData<pat::Jet>("rawJet");
    //apply the ad hoc corrections
    //calculate the corrections
    double corrsForTypeI =1.;
    double corrsForTypeIL1=1.;
    edm::Handle<double> rho_;
    event.getByLabel(edm::InputTag("fixedGridRhoFastjetAll"), rho_);
    
    jetCorrectorForTypeIL1->setJetEta(rawJet->eta());
    jetCorrectorForTypeIL1->setJetPt(rawJet->pt());
    jetCorrectorForTypeIL1->setJetA(rawJet->jetArea());
    jetCorrectorForTypeIL1->setRho(*rho_);
    corrsForTypeIL1 = jetCorrectorForTypeIL1->getCorrection();
    
    pat::Jet jetL1 = *rawJet;
    jetL1.scaleEnergy(corrsForTypeIL1);
    
    jetCorrectorForTypeI->setJetEta(rawJet->eta()); 
    jetCorrectorForTypeI->setJetPt(rawJet->pt());
    jetCorrectorForTypeI->setJetA(rawJet->jetArea());
    jetCorrectorForTypeI->setRho(*rho_);
    corrsForTypeI = jetCorrectorForTypeI->getCorrection();
    
    pat::Jet jet = *rawJet;
    jet.scaleEnergy(corrsForTypeI);
    //go ahead with typeI
    if (jet.pt() > 10) {
      
       double emEnergyFraction = rawJet->chargedEmEnergyFraction() + rawJet->neutralEmEnergyFraction();
       if (emEnergyFraction > 0.90)
	 continue;
       
       reco::Candidate::LorentzVector L1JetP4  = jetL1.p4();
       
       deltaPx += (jet.px() - L1JetP4.px());
       deltaPy += (jet.py() - L1JetP4.py());
    }// pt >10
  } // end loop on jet
  
  
  // define MET with JEC
  double correctedMetPx = FootprintMEx  - deltaPx;
  double correctedMetPy = FootprintMEy  - deltaPy;
  double correctedMetPt = sqrt(correctedMetPx * correctedMetPx + correctedMetPy * correctedMetPy);
  
  met.setP4(reco::Candidate::LorentzVector(correctedMetPx, correctedMetPy, 0., correctedMetPt));
  
  //  std::cout<< " Met.pt "<< met.pt() << std::endl;   
  //  std::cout<< " Met.et "<< met.et() << std::endl;   
} 


// not used -- regression is already implemented in 73X 
void GammaJetFilter::correctMETWithRegressionAndTypeI(const pat::MET& rawMet, pat::MET& met, const pat::JetCollection& jets,  edm::Event& event, pat::Photon& photon, const pat::PhotonRef& photonRef) {
  //photonRef is the one before regression
  //photon is the one after
  
  //  std::cout<< " Correct MET With Regression "<< std::endl; 
  
  
  double deltaPx = 0., deltaPy = 0.;
  for (pat::JetCollection::const_iterator it = jets.begin(); it != jets.end(); ++it) {
    const pat::Jet& jet = *it;
    
    if (jet.pt() > 10) {
      
      const pat::Jet* rawJet = jet.userData<pat::Jet>("rawJet");
      double corrsForTypeI =1.;
      double corrsForTypeIL1=1.;
      edm::Handle<double> rho_;
      event.getByLabel(edm::InputTag("fixedGridRhoFastjetAllCalo"), rho_);
      
      jetCorrectorForTypeIL1->setJetEta(rawJet->eta());
      jetCorrectorForTypeIL1->setJetPt(rawJet->pt());
      jetCorrectorForTypeIL1->setJetA(rawJet->jetArea());
      jetCorrectorForTypeIL1->setRho(*rho_);
      corrsForTypeIL1 = jetCorrectorForTypeIL1->getCorrection();
      
      pat::Jet jetL1 = *rawJet;
      jetL1.scaleEnergy(corrsForTypeIL1);
      
      jetCorrectorForTypeI->setJetEta(rawJet->eta());
      jetCorrectorForTypeI->setJetPt(rawJet->pt());
      jetCorrectorForTypeI->setJetA(rawJet->jetArea());
      jetCorrectorForTypeI->setRho(*rho_);
      corrsForTypeI = jetCorrectorForTypeI->getCorrection();
      
      pat::Jet jet = *rawJet;
      jet.scaleEnergy(corrsForTypeI);
      
      reco::Candidate::LorentzVector L1JetP4  = jetL1.p4();
      
      double emEnergyFraction = rawJet->chargedEmEnergyFraction() + rawJet->neutralEmEnergyFraction();
      if (emEnergyFraction > 0.90) continue;
      
      deltaPx += (jet.px() - L1JetP4.px());
      deltaPy += (jet.py() - L1JetP4.py());
    }
  }
  
  double correctedMetPx = rawMet.px() + photonRef->px() - photon.px() - deltaPx;
  double correctedMetPy = rawMet.py() + photonRef->py() - photon.py() - deltaPy;
  double correctedMetPt = sqrt(correctedMetPx * correctedMetPx + correctedMetPy * correctedMetPy);
  
  met.setP4(reco::Candidate::LorentzVector(correctedMetPx, correctedMetPy, 0., correctedMetPt));  
}


void GammaJetFilter::extractRawJets(pat::JetCollection& jets) {
  
  for (pat::JetCollection::iterator it = jets.begin(); it != jets.end(); ++it) {
    pat::Jet& jet = *it;
    
    const pat::Jet rawJet = jet.correctedJet("Uncorrected");
    jet.addUserData("rawJet", rawJet, true);
    const pat::Jet L1Jet  = jet.correctedJet("L1FastJet");
    jet.addUserData("L1Jet", L1Jet, true); // Embed L1 corrected jet for TypeI correction
  }
  
}

void GammaJetFilter::processJets(pat::Photon* photon, pat::JetCollection& jets, const JetAlgorithm algo,/* edm::Handle<edm::ValueMap<float>>& qgTagMLP, edm::Handle<edm::ValueMap<float>>& qgTagLikelihood,*/ const edm::Handle<pat::JetCollection>& handleForRef, std::vector<TTree*>& trees) {
  
  pat::JetCollection selectedJets;
  
  pat::JetCollection::iterator it = jets.begin();
  
  uint32_t index = 0;
  uint32_t goodJetIndex = -1;
  for (; it != jets.end(); ++it, index++) {
    
    if (! isValidJet(*it)) 
      continue;
    
    goodJetIndex++;
    
    if (goodJetIndex == 0) {
      mFirstJetPhotonDeltaPhi->Fill(fabs(reco::deltaPhi(*photon, *it)));
      mFirstJetPhotonDeltaR->Fill(reco::deltaR(*photon, *it));
      mFirstJetPhotonDeltaPt->Fill(fabs(photon->pt() - it->pt()));
      
      mFirstJetPhotonDeltaPhiDeltaR->Fill(fabs(reco::deltaPhi(*photon, *it)), reco::deltaR(*photon, *it));
    } else if (goodJetIndex == 1) {
      mSecondJetPhotonDeltaPhi->Fill(fabs(reco::deltaPhi(*photon, *it)));
      mSecondJetPhotonDeltaR->Fill(reco::deltaR(*photon, *it));
      mSecondJetPhotonDeltaPt->Fill(fabs(photon->pt() - it->pt()));
    }
    
    //giulia --- comment QG tagging stuff
    // Extract Quark Gluon tagger value
    pat::JetRef jetRef(handleForRef, index);
    //it->addUserFloat("qgTagMLP", (*qgTagMLP)[jetRef]);
    //it->addUserFloat("qgTagLikelihood", (*qgTagLikelihood)[jetRef]);
    
    const double deltaR_threshold = (algo == AK4) ? 0.4 : 0.8;
    
    if (selectedJets.size() == 0) {
      // First jet selection
      
      if (index > 1) {
      	// It's the third jet of the event. We only want to consider the first two jets for our leading jet,
	// so, throw this event
	break;
      }
      
      const double deltaPhi = reco::deltaPhi(*photon, *it);
      if (fabs(deltaPhi) < M_PI / 2.)
	continue; // Only back 2 back event are interesting
      
      const double deltaR = reco::deltaR(*photon, *it);
      if (deltaR < deltaR_threshold) // This jet is inside the photon. This is probably the photon mis-reconstructed as a jet
      	continue;
      
      // Jet are ordered by pt value.
      // Events are supposed to be balanced between Jet and Gamma
      // If the leading jet has less than 30% of the Photon pt,
      // dump the event as it's not interesting
      if (mFirstJetPtCut && (it->pt() < photon->pt() * mFirstJetThreshold))
      	break;
      
      mSelectedFirstJetIndex->Fill(goodJetIndex);
      selectedJets.push_back(*it);
      
    } else {
      // Second jet selection
      
      const double deltaR = reco::deltaR(*photon, *it);
      
      if (deltaR > deltaR_threshold) {
	mSelectedSecondJetIndex->Fill(goodJetIndex);
	selectedJets.push_back(*it);
      } else {
      	continue;
      }     
      break;
    }
    
  }// end loop over jets
  
  const pat::Jet* firstJet = NULL;
  const pat::Jet* secondJet = NULL;
  
  if (selectedJets.size() > 0) {
    
    firstJet = &selectedJets[0];
    mSelectedFirstJetPhotonDeltaPhi->Fill(fabs(reco::deltaPhi(*photon, *firstJet)));
    mSelectedFirstJetPhotonDeltaR->Fill(reco::deltaR(*photon, *firstJet));
    
    if (selectedJets.size() > 1) {
      secondJet = &selectedJets[1];
      mSelectedSecondJetPhotonDeltaPhi->Fill(fabs(reco::deltaPhi(*photon, *secondJet)));
      mSelectedSecondJetPhotonDeltaR->Fill(reco::deltaR(*photon, *secondJet));
    }
  }
  
  jetsToTree(firstJet, secondJet, trees);
  
  return;
}

// ------------ method called once each job just before starting event loop  ------------
void GammaJetFilter::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void GammaJetFilter::endJob() {
  
  EventCounter -> GetXaxis()->SetBinLabel(1,"All events");
  EventCounter -> GetXaxis()->SetBinLabel(2,"Range PtHat Cut");
  EventCounter -> GetXaxis()->SetBinLabel(3,"Vertex Cut");
  EventCounter -> GetXaxis()->SetBinLabel(4,"N photons Cut");
  EventCounter -> GetXaxis()->SetBinLabel(5,"After process Jets");
  EventCounter -> GetXaxis()->SetBinLabel(6,"Final events");
  
  EventCounter_Raw -> GetXaxis()->SetBinLabel(1,"All events");
  EventCounter_Raw -> GetXaxis()->SetBinLabel(2,"Range PtHat Cut");
  EventCounter_Raw -> GetXaxis()->SetBinLabel(3,"Vertex Cut");
  EventCounter_Raw -> GetXaxis()->SetBinLabel(4,"N photons Cut");
  EventCounter_Raw -> GetXaxis()->SetBinLabel(5,"After process Jets");
  EventCounter_Raw -> GetXaxis()->SetBinLabel(6,"Final events");
  
  EventCounter_Raw -> SetBinContent(1,Event_Initial);
  EventCounter_Raw -> SetBinContent(2,Event_MCGen);
  EventCounter_Raw -> SetBinContent(3,Event_VtxCut);
  EventCounter_Raw -> SetBinContent(4,Event_NPhotons);
  EventCounter_Raw -> SetBinContent(5,Event_AfterJets);
  EventCounter_Raw -> SetBinContent(6,Event_Final);
  
  EventCounterPhoton -> GetXaxis()->SetBinLabel(1,"All photons");
  EventCounterPhoton -> GetXaxis()->SetBinLabel(2,"Photons Eta Cut");
  EventCounterPhoton -> GetXaxis()->SetBinLabel(3,"Photons Pt Range");
  EventCounterPhoton -> GetXaxis()->SetBinLabel(4,"! Photon Gen");
  EventCounterPhoton -> GetXaxis()->SetBinLabel(5,"HE  cut");
  EventCounterPhoton -> GetXaxis()->SetBinLabel(6,"R9  cut");
  EventCounterPhoton -> GetXaxis()->SetBinLabel(7,"Sigma cut");
  EventCounterPhoton -> GetXaxis()->SetBinLabel(8,"Electron Veto");
  EventCounterPhoton -> GetXaxis()->SetBinLabel(9,"Isolation Cuts");
  EventCounterPhoton -> GetXaxis()->SetBinLabel(10,"Only One Good Photon");
  
  EventCounterPhoton_Raw -> GetXaxis()->SetBinLabel(1,"All photons");
  EventCounterPhoton_Raw -> GetXaxis()->SetBinLabel(2,"Photons Eta Cut");
  EventCounterPhoton_Raw -> GetXaxis()->SetBinLabel(3,"Photons Pt Range");
  EventCounterPhoton_Raw -> GetXaxis()->SetBinLabel(4,"! Photon Gen");
  EventCounterPhoton_Raw -> GetXaxis()->SetBinLabel(5,"HE  cut");
  EventCounterPhoton_Raw -> GetXaxis()->SetBinLabel(6,"R9  cut");
  EventCounterPhoton_Raw -> GetXaxis()->SetBinLabel(7,"Sigma cut");
  EventCounterPhoton_Raw -> GetXaxis()->SetBinLabel(8,"Electron Veto");
  EventCounterPhoton_Raw -> GetXaxis()->SetBinLabel(9,"Isolation Cuts");
  EventCounterPhoton_Raw -> GetXaxis()->SetBinLabel(10,"Only One Good Photon");
  
  EventCounterPhoton_Raw -> SetBinContent(1,N_Photons_Total);
  EventCounterPhoton_Raw -> SetBinContent(2,N_Photons_Eta13);
  EventCounterPhoton_Raw -> SetBinContent(3,N_Photons_PtRange);
  EventCounterPhoton_Raw -> SetBinContent(4,N_pho_genPhoton);
  EventCounterPhoton_Raw -> SetBinContent(5,N_pho_HE);
  EventCounterPhoton_Raw -> SetBinContent(6,N_pho_R9);
  EventCounterPhoton_Raw -> SetBinContent(7,N_pho_Sigma);
  EventCounterPhoton_Raw -> SetBinContent(8,N_pho_ElecVeto);
  EventCounterPhoton_Raw -> SetBinContent(9,N_pho_Isolation);
  EventCounterPhoton_Raw -> SetBinContent(10,OnlyOneGoodPhotons);
    
  std::cout << " " << std::endl;
  std::cout<< "Event_Initial =  " << Event_Initial << std::endl ;
  std::cout<< "Event_MCGen = " << Event_MCGen << std::endl ;
  std::cout<< "Event_VtxCut = " <<  Event_VtxCut << std::endl ;
  std::cout<< "Event_NPhotons = " << Event_NPhotons << std::endl ;
  std::cout<< "Event_AfterJets = " << Event_AfterJets << std::endl ;
  std::cout<< "Event_Final = " << Event_Final << std::endl ;
  
  std::cout<< "N_Photons_Total = " << N_Photons_Total << std::endl ;
  std::cout<< "N_Photons_Eta13 = " << N_Photons_Eta13 << std::endl ;
  std::cout<< "N_Photons_PtRange = " << N_Photons_PtRange << std::endl ;
  std::cout<< "N_pho_genPhoton =  " << N_pho_genPhoton << std::endl ;
  std::cout<< "N_pho_HE =  " << N_pho_HE << std::endl ;
  std::cout<< "N_pho_R9 =  " << N_pho_R9 << std::endl ;
  std::cout<< "N_pho_Sigma =  " << N_pho_Sigma << std::endl ;
  std::cout<< "N_pho_ElecVeto =  " << N_pho_ElecVeto << std::endl ;
  std::cout<< "N_pho_Isolation =  " << N_pho_Isolation << std::endl ;  
  
  // std::cout<< "NoGoodPhotons = " << NoGoodPhotons << std::endl ;
  std::cout<< "OnlyOneGoodPhotons = " << OnlyOneGoodPhotons << std::endl ;
  // std::cout<< "MoreGoodPhotons = " << MoreGoodPhotons << std::endl ;

  for(int ii =0; ii<11; ii++){
    NEvent_vec.push_back(NEvent_array[ii]);
    R_min_vec.push_back(R_min_array[ii]);
    R_max_vec.push_back(R_max_array[ii]);
    SumE_vec.push_back(SumE_array[ii]);
    SumPt_vec.push_back(SumPt_array[ii]);
    KL1FastJet_vec.push_back(KL1FastJet_array[ii]);
    KL1RC_vec.push_back(KL1RC_array[ii]);
    std::cout<<"NEvent_array["<<ii<<"] = "<< NEvent_array[ii]<<std::endl;
    std::cout<<"R_min_array["<<ii<<"] = "<< R_min_array[ii]<<std::endl;
    std::cout<<"R_max_array["<<ii<<"] = "<< R_max_array[ii]<<std::endl;
    std::cout<<"SumE_array["<<ii<<"] = "<< SumE_array[ii]<<std::endl;
    std::cout<<"SumPt_array["<<ii<<"] = "<< SumPt_array[ii]<<std::endl;
    std::cout<<"KL1FastJet_array["<<ii<<"] = "<< KL1FastJet_array[ii]<<std::endl;
    std::cout<<"KL1RC_array["<<ii<<"] = "<< KL1RC_array[ii]<<std::endl;
  }

  mPhotonStudy -> Branch("NEvents", "vector<double>", &NEvent_vec);             
  mPhotonStudy -> Branch("R_min", "vector<double>", &R_min_vec);                                                                                                                    
  mPhotonStudy -> Branch("R_max", "vector<double>", &R_max_vec);                                                                                                                    
  mPhotonStudy -> Branch("SumE_pfCandidate", "vector<double>", &SumE_vec);                                                                                                                    
  mPhotonStudy -> Branch("SumPt_pfCandidate", "vector<double>", &SumPt_vec);
  mPhotonStudy -> Branch("K_L1FastJet", "vector<double>", &KL1FastJet_vec);          
  mPhotonStudy -> Branch("K_L1RC", "vector<double>", &KL1RC_vec);          

   mPhotonStudy->Fill();



}

// ------------ method called when starting to processes a run  ------------
bool GammaJetFilter::beginRun(edm::Run& run, edm::EventSetup const&)
{
  if (! mIsMC && mFilterData) {
    // Check if this run is valid
    std::stringstream stream;
    stream << run.run();
    
    if (! mValidRuns->isMember(stream.str()))
      return false; // Drop run
    
    mCurrentRunValidLumis.reset(new Json::Value((*mValidRuns)[stream.str()]));
  }
  
  return true;
}

// ------------ method called when ending the processing of a run  ------------
bool GammaJetFilter::endRun(edm::Run& run, edm::EventSetup const&)
{
  
  return true;
}

// ------------ method called when starting to processes a luminosity block  ------------
bool GammaJetFilter::beginLuminosityBlock(edm::LuminosityBlock& lumiBlock, edm::EventSetup const&)
{
  if (! mIsMC && mFilterData) {
    
    mIsValidLumiBlock = false;

    if (! mCurrentRunValidLumis.get())
      return false;
    
    // Check if this lumi block is valid
    assert(mCurrentRunValidLumis->isArray());
    for (Json::ArrayIndex i = 0; i < mCurrentRunValidLumis->size(); i++) {
      Json::Value lumiRange = (*mCurrentRunValidLumis)[i];
      
      assert(lumiRange.size() == 2);
      edm::LuminosityBlockNumber_t lumi = lumiBlock.luminosityBlock();
      if (lumi >= lumiRange[0].asUInt64() && lumi <= lumiRange[1].asUInt64()) {
      	mIsValidLumiBlock = true;
	
	mCurrentTruePU = mTruePUByLS[std::make_pair(lumiBlock.id().run(), lumiBlock.id().luminosityBlock())];
	return true;
      }
    }
    
    return false;
  }
  
  return true;
}

// ------------ method called when ending the processing of a luminosity block  ------------
bool GammaJetFilter::endLuminosityBlock(edm::LuminosityBlock& lumiBlock, edm::EventSetup const&)
{
  if (! mIsMC && mFilterData && mIsValidLumiBlock) {
    updateLuminosity(lumiBlock);
  }
  return true;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void GammaJetFilter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

bool GammaJetFilter::isValidJet(const pat::Jet& jet) {
  // First, check if this pat::Jet has a gen jet
  if (mIsMC && !jet.genJet()) {
    return false;
  }
  
  if (jet.isPFJet()) { // we use only this now
    
    // Jet ID
    // From https://twiki.cern.ch/twiki/bin/view/CMS/JetID
    
    // Jet ID works on uncorrected jets. *EnergyFraction take that into account when calculating the fraction,
    // so there's *NO* need to use an uncorrected jet
    bool isValid = jet.neutralHadronEnergyFraction() < 0.99;
    isValid &= jet.neutralEmEnergyFraction() < 0.99;
    //isValid &= jet.getPFConstituents().size() > 1;
    if (fabs(jet.eta()) < 2.4) {
      isValid &= jet.chargedHadronEnergyFraction() > 0.;
      isValid &= jet.chargedMultiplicity() > 0;
      isValid &= jet.chargedEmEnergyFraction() < 0.99;
    }
    
    return isValid;
    
  } else if (jet.isCaloJet() || jet.isJPTJet()) { // these jets not used
    
    if (! mCaloJetID.get()) {
      mCaloJetID.reset(new JetIDSelectionFunctor(JetIDSelectionFunctor::PURE09, JetIDSelectionFunctor::LOOSE));
      mCaloJetIDRet = mCaloJetID->getBitTemplate();
    }
    
    mCaloJetIDRet.set(false);
    return (*mCaloJetID)(jet, mCaloJetIDRet);
    
  } else {
    throw cms::Exception("UnsupportedJetType")
      << "Only PF and Calo jets are supported at this time" << std::endl;
  }
  
  return false;
}

enum class IsolationType {
  CHARGED_HADRONS,
    NEUTRAL_HADRONS,
    PHOTONS
    };


//updated effective areas for SPRING15 50ns
float getEffectiveArea(float eta, IsolationType type) {
  eta = fabs(eta);
  switch (type) {
  case IsolationType::CHARGED_HADRONS:
    if (eta < 1.0)
      return 0.0158;
    else if (eta < 1.479)
      return 0.0143;
    else if (eta < 2.0)
      return 0.0115;
    else if (eta < 2.2)
      return 0.0094;
    else if (eta < 2.3)
      return 0.0095;
    else if (eta < 2.4)
      return 0.0068;
    else
      return 0.0053;
    break;
    
  case IsolationType::NEUTRAL_HADRONS:
    if (eta < 1.0)
      return 0.0143;
    else if (eta < 1.479)
      return 0.0210;
    else if (eta < 2.0)
      return 0.0147;
    else if (eta < 2.2)
      return 0.0082;
    else if (eta < 2.3)
      return 0.0124;
    else if (eta < 2.4)
      return 0.0186;
    else
      return 0.0320;
    break;
    
    
  case IsolationType::PHOTONS:
    if (eta < 1.0)
      return 0.0725;
    else if (eta < 1.479)
      return 0.0604;
    else if (eta < 2.0)
      return 0.0320;
    else if (eta < 2.2)
      return 0.0512;
    else if (eta < 2.3)
      return 0.0766;
    else if (eta < 2.4)
      return 0.0949;
    else
      return 0.1160;
    break;
  }
  
  return -1;
}

double getCorrectedPFIsolation(double isolation, double rho, float eta, IsolationType type) {
  float effectiveArea = getEffectiveArea(eta, type);
  
  return std::max(isolation - rho * effectiveArea, 0.);
}

//-----------------
// See https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedPhotonIdentificationRun2 -- tight WP 77% 
bool GammaJetFilter::isValidPhotonEB_SPRING15(const pat::PhotonRef& photonRef, edm::Event& event, double generatorWeight) {
  
  //  real photon matching a gen level photon 
  if (mIsMC && !photonRef->genPhoton())   
    return false;
  
  EventCounterPhoton -> AddBinContent(4, generatorWeight );
  N_pho_genPhoton++;
  
  bool isValid = true;
  
  // Photon variables computed upstream in a special producer
  //  edm::EDGetTokenT<edm::ValueMap<float> > full5x5SigmaIEtaIEtaMapToken_; 
  //   Note: starting from CMSSW 7.2.1 or so one can get any full5x5 quantity
  //   directly from the photon object, there is no need for value maps anymore.
  //   However 7.2.0 and prior (this includes PHYS14 MC samples) requires ValueMaps.
  
  isValid &= photonRef->hadTowOverEm() < 0.05;
  
  if (! isValid)
    return false;
  
  EventCounterPhoton -> AddBinContent(5, generatorWeight );
  N_pho_HE++;
  
  // added to emule trigger
  isValid &= photonRef->r9() >0.9;
  
  if (! isValid)
    return false;
  
  EventCounterPhoton -> AddBinContent(6, generatorWeight );
  N_pho_R9++;
  
  isValid &= photonRef->sigmaIetaIeta() < 0.0100;
  
  if (! isValid)
    return false;
  
  EventCounterPhoton -> AddBinContent(7, generatorWeight );
  N_pho_Sigma++;
  
  //giulia --  use the method " passElectronVeto" for the PAT
  // Isolations are produced at PAT level by the PḧotonPFIsolation producer
  //edm::Handle<edm::ValueMap<bool>> hasMatchedPromptElectronHandle;
  //event.getByLabel(edm::InputTag("photonPFIsolation", "hasMatchedPromptElectron", "PAT"), hasMatchedPromptElectronHandle);
  
  //isValid &= ! (*hasMatchedPromptElectronHandle)[photonRef];
  
  isValid &= photonRef->passElectronVeto();
  
  if (! isValid)
    return false;
  
  EventCounterPhoton -> AddBinContent(8, generatorWeight );
  N_pho_ElecVeto++;
  
  edm::Handle<double> rhos;
  event.getByLabel(edm::InputTag("fixedGridRhoFastjetAll"), rhos);
  double rho = *rhos;
  
  // Get the isolation maps
  edm::Handle<edm::ValueMap<float> > phoChargedIsolationMap;
  event.getByToken(phoChargedIsolationToken_, phoChargedIsolationMap);
  edm::Handle<edm::ValueMap<float> > phoNeutralHadronIsolationMap;
  event.getByToken(phoNeutralHadronIsolationToken_, phoNeutralHadronIsolationMap);
  edm::Handle<edm::ValueMap<float> > phoPhotonIsolationMap;
  event.getByToken(phoPhotonIsolationToken_, phoPhotonIsolationMap);
  
  isValid &= getCorrectedPFIsolation((*phoChargedIsolationMap)[photonRef], rho, photonRef->eta(), IsolationType::CHARGED_HADRONS) < 0.91;
  isValid &= getCorrectedPFIsolation((*phoNeutralHadronIsolationMap)[photonRef], rho, photonRef->eta(), IsolationType::NEUTRAL_HADRONS) < (0.33 + exp(0.0044*photonRef->pt()+0.5809) );
  isValid &= getCorrectedPFIsolation((*phoPhotonIsolationMap)[photonRef], rho, photonRef->eta(), IsolationType::PHOTONS) < (0.61 + 0.0043 * photonRef->pt());
  
  if (! isValid)
    return false;
  
  EventCounterPhoton -> AddBinContent(9, generatorWeight );
  N_pho_Isolation++;
  
  return isValid;
  
}
//---------------------

void GammaJetFilter::readJSONFile() {
  Json::Value root;
  Json::Reader reader;
  std::ifstream file(mJSONFile.c_str());
  if (! reader.parse(file, root)) {
    throw cms::Exception("ReadError")
      << "Failed to parse luminosity JSON file '" << mJSONFile << "'" << std::endl;
  }
  
  mValidRuns.reset(new Json::Value(root));
}

void GammaJetFilter::readCSVFile() {
  FILE* iff = fopen(mCSVFile.c_str(), "r");
  
  if(iff == 0) {
    throw cms::Exception("ReadError")
      << "Failed to parse luminosity CSV file '" << mCSVFile << "'" << std::endl;
  }
  
  int run = 0, fill = 0;
  int lumiSection_left = 0, lumiSection_right = 0;
  double lumiRecorded = 0.;
  double true_pu = 0;
  
  
  /* lumiCalc2 format :
   * Run:Fill,LS,UTCTime,Beam Status,E(GeV),Delivered(/ub),Recorded(/ub),avgPU
   * use 'lumiCalc2.py -i lumiSummary.json -o output.csv -b stable lumibyls' to generate file
   */
  
  // Skip header line
  char buffer[1024];
  fgets(buffer, 1024, iff);
  
  while (fscanf(iff, "%d:%d,%d:%d,%*[^,],%*[^,],%*f,%*f,%lf,%lf", &run, &fill, &lumiSection_left, &lumiSection_right, &lumiRecorded, &true_pu) > 0 ) {
    
    if (lumiSection_right == 0)
      continue;
    
    mLumiByLS[std::pair<unsigned int, unsigned int>(run, lumiSection_right)] = lumiRecorded; //in mb^(-1)
    mTruePUByLS[std::pair<unsigned int, unsigned int>(run, lumiSection_right)] = true_pu; //in mb^(-1)
  }
  
  fclose(iff);
  
  assert(mLumiByLS.size() > 0);
}

void GammaJetFilter::updateLuminosity(const edm::LuminosityBlock& lumiBlock) {
  double eventLumi = mLumiByLS[std::pair<unsigned int, unsigned int>(lumiBlock.id().run(), lumiBlock.id().luminosityBlock())];
  double newLumi = mTotalLuminosity->GetVal() + eventLumi;
  mTotalLuminosity->SetVal(newLumi);
}

void GammaJetFilter::particleToTree(const reco::Candidate* particle, TTree* t, std::vector<boost::shared_ptr<void> >& addresses) {
  addresses.clear();
  
  addresses.push_back(boost::shared_ptr<void>(new int((particle) ? 1 : 0)));
  addresses.push_back(boost::shared_ptr<void>(new float((particle) ? particle->et() : 0)));
  addresses.push_back(boost::shared_ptr<void>(new float((particle) ? particle->pt() : 0)));
  addresses.push_back(boost::shared_ptr<void>(new float((particle) ? particle->eta() : 0)));
  addresses.push_back(boost::shared_ptr<void>(new float((particle) ? particle->phi() : 0)));
  addresses.push_back(boost::shared_ptr<void>(new float((particle) ? particle->px() : 0)));
  addresses.push_back(boost::shared_ptr<void>(new float((particle) ? particle->py() : 0)));
  addresses.push_back(boost::shared_ptr<void>(new float((particle) ? particle->pz() : 0)));
  addresses.push_back(boost::shared_ptr<void>(new float((particle) ? particle->energy() : 0)));

  updateBranch(t, addresses[0].get(), "is_present", "I");
  updateBranch(t, addresses[1].get(), "et");
  updateBranch(t, addresses[2].get(), "pt");
  updateBranch(t, addresses[3].get(), "eta");
  updateBranch(t, addresses[4].get(), "phi");
  updateBranch(t, addresses[5].get(), "px");
  updateBranch(t, addresses[6].get(), "py");
  updateBranch(t, addresses[7].get(), "pz");
  updateBranch(t, addresses[8].get(), "e");
}


void GammaJetFilter::photonToTree(const pat::PhotonRef& photonRef, pat::Photon& photon, const edm::Event& event) {
  std::vector<boost::shared_ptr<void> > addresses;
  //redefine the common variables instead of using particleTotree, because the photon has been 
  //corrected for regression etc.
  int pho_is_present =1;
  float pho_et = photon.et();
  float pho_pt = photon.pt();
  float pho_eta = photon.eta();
  float pho_phi = photon.phi();
  float pho_px = photon.px();
  float pho_py = photon.py();
  float pho_pz = photon.pz();
  float pho_e = photon.energy();
  updateBranch(mPhotonTree,&pho_is_present,"is_present","I");
  updateBranch(mPhotonTree,&pho_et,"et");
  updateBranch(mPhotonTree,&pho_pt,"pt");
  updateBranch(mPhotonTree,&pho_eta,"eta");
  updateBranch(mPhotonTree,&pho_phi,"phi"); 
  updateBranch(mPhotonTree,&pho_px,"px");
  updateBranch(mPhotonTree,&pho_py,"py");
  updateBranch(mPhotonTree,&pho_pz,"pz");
  updateBranch(mPhotonTree,&pho_e,"e");
  
  bool hasPixelSeed = photonRef->hasPixelSeed();
  updateBranch(mPhotonTree, &hasPixelSeed, "has_pixel_seed", "O");
  
  // Photon ID related
  float hadTowOverEm = photonRef->hadTowOverEm();
  updateBranch(mPhotonTree, &hadTowOverEm, "hadTowOverEm");
  
  float sigmaIetaIeta = photonRef->sigmaIetaIeta();
  updateBranch(mPhotonTree, &sigmaIetaIeta, "sigmaIetaIeta");

  edm::Handle<double> rhos;
  event.getByLabel(edm::InputTag("fixedGridRhoFastjetAll"), rhos);
  float rho = *rhos;
  updateBranch(mPhotonTree, &rho, "rho");
  
  //giulia -- taking iso from pat:Photon
  // Isolations are produced at PAT level by the PḧotonPFIsolation producer
  //edm::Handle<edm::ValueMap<bool>> hasMatchedPromptElectronHandle;
  //event.getByLabel(edm::InputTag("photonPFIsolation", "hasMatchedPromptElectron", "PAT"), hasMatchedPromptElectronHandle);
  
  //bool hasMatchedPromptElectron = (*hasMatchedPromptElectronHandle)[photonRef];
  
  bool hasMatchedPromptElectron ; 
  if (photonRef->passElectronVeto())
    hasMatchedPromptElectron = false;
  else 
    hasMatchedPromptElectron = true;
  
  updateBranch(mPhotonTree, &hasMatchedPromptElectron, "hasMatchedPromptElectron", "O");
  
   //giulia ---- regression integrated in 73X  
  ////regression energy
  //  edm::Handle<edm::ValueMap<float>> regressionEnergyHandle;
  //  event.getByLabel(edm::InputTag("eleNewEnergiesProducer", "energySCEleJoshPhoSemiParamV5ecorr", "PAT"),regressionEnergyHandle);
  //  edm::Ptr<reco::Candidate> recoObject = photonRef->originalObjectRef();
  // float regressionEnergy = (*regressionEnergyHandle)[recoObject] ;
  // updateBranch(mPhotonTree, &regressionEnergy, "regressionEnergy");
  // float originalEnergy = photonRef->energy();
  // updateBranch(mPhotonTree, &originalEnergy, "originalEnergy");
  
  //giulia --- comment footprint stuff
  
  ////retrieve px and py of pfcandidates to exclude from met calculation
  //edm::Handle<edm::ValueMap<double>> footprintMExCorrHandle;
  //event.getByLabel(edm::InputTag("photonPFIsolation", "footprintMExCorr", "PAT"), footprintMExCorrHandle);
  //edm::Handle<edm::ValueMap<double>> footprintMEyCorrHandle;
  //event.getByLabel(edm::InputTag("photonPFIsolation", "footprintMEyCorr", "PAT"), footprintMEyCorrHandle);
  ////MEx/yCorr are the quantities to compare to rawMex/y
  //float footprintMExCorr = (*footprintMExCorrHandle)[photonRef]- photonRef->px();
  //float footprintMEyCorr = (*footprintMEyCorrHandle)[photonRef]- photonRef->py();
  //
  //  updateBranch(mPhotonTree, &footprintMExCorr, "footprintMExCorr");
  //  updateBranch(mPhotonTree, &footprintMEyCorr, "footprintMEyCorr");

  edm::Handle<edm::ValueMap<float> > phoChargedIsolationMap;
  event.getByToken(phoChargedIsolationToken_, phoChargedIsolationMap);
  edm::Handle<edm::ValueMap<float> > phoNeutralHadronIsolationMap;
  event.getByToken(phoNeutralHadronIsolationToken_, phoNeutralHadronIsolationMap);
  edm::Handle<edm::ValueMap<float> > phoPhotonIsolationMap;
  event.getByToken(phoPhotonIsolationToken_, phoPhotonIsolationMap);
  
  float chargedHadronsIsolation = getCorrectedPFIsolation((*phoChargedIsolationMap)[photonRef], rho, photonRef->eta(), IsolationType::CHARGED_HADRONS);
  float neutralHadronsIsolation  = getCorrectedPFIsolation((*phoNeutralHadronIsolationMap)[photonRef], rho, photonRef->eta(), IsolationType::NEUTRAL_HADRONS);
  float photonIsolation               = getCorrectedPFIsolation((*phoPhotonIsolationMap)[photonRef], rho, photonRef->eta(), IsolationType::PHOTONS);     
  
  updateBranch(mPhotonTree, &chargedHadronsIsolation, "chargedHadronsIsolation");
  updateBranch(mPhotonTree, &neutralHadronsIsolation, "neutralHadronsIsolation");
  updateBranch(mPhotonTree, &photonIsolation, "photonIsolation");
  
  mPhotonTree->Fill();
  
  if (mIsMC) {
    particleToTree(photonRef->genPhoton(), mPhotonGenTree, addresses);
    mPhotonGenTree->Fill();
  }
}

void GammaJetFilter::jetsToTree(const pat::Jet* firstJet, const pat::Jet* secondJet, const std::vector<TTree*>& trees) {
  jetToTree(firstJet, mIsMC, trees[0], trees[4]);
  jetToTree(secondJet, false, trees[1], trees[5]);
  
  // Raw jets
  const pat::Jet* rawJet = (firstJet) ? firstJet->userData<pat::Jet>("rawJet") : NULL;
  jetToTree(rawJet, false, trees[2], NULL);
  
  rawJet = (secondJet) ? secondJet->userData<pat::Jet>("rawJet") : NULL;
  jetToTree(rawJet, false, trees[3], NULL);
}

void findNeutrinos(const reco::Candidate* parent, std::vector<const reco::Candidate*>& neutrinos) {
  
  int pdg_id = abs(parent->pdgId());
  if (pdg_id == 12 || pdg_id == 14 || pdg_id == 16) {
    if (std::find_if(neutrinos.begin(), neutrinos.end(), [parent] (const reco::Candidate* candidate) -> bool {
          static double EPSILON = 0.0001;
          bool same = (candidate->pdgId() == parent->pdgId());
          same &= (fabs(candidate->px() - candidate->px()) < EPSILON);
          same &= (fabs(candidate->py() - candidate->py()) < EPSILON);
          same &= (fabs(candidate->pz() - candidate->pz()) < EPSILON);
	  
	  return same;
	  
	}) == neutrinos.end()) {
      neutrinos.push_back(parent);
    }
    return;
  }
  
  for (unsigned int i = 0; i < parent->numberOfDaughters(); i++) {
    const reco::Candidate* d = parent->daughter(i);
    findNeutrinos(d, neutrinos);
  }
}

void GammaJetFilter::jetToTree(const pat::Jet* jet, bool _findNeutrinos, TTree* tree, TTree* genTree) {
  std::vector<boost::shared_ptr<void> > addresses;
  particleToTree(jet, tree, addresses);
  
  if (mIsMC) {
    mNeutrinos->Clear("C");
    mNeutrinosPDG->Clear("C");
  }
  
  if (jet) {
    float area = jet->jetArea();
    updateBranch(tree, &area, "jet_area");
    
    //giulia --- comment b-tagging  and qg tagging stuff
    // // B-Tagging
    // float tcHighEfficiency = jet->bDiscriminator("trackCountingHighEffBJetTags");
    // float tcHighPurity = jet->bDiscriminator("trackCountingHighPurBJetTags");
    
    // float ssvHighEfficiency = jet->bDiscriminator("simpleSecondaryVertexHighEffBJetTags");
    // float ssvHighPurity = jet->bDiscriminator("simpleSecondaryVertexHighPurBJetTags");

    // float jetProbability = jet->bDiscriminator("jetProbabilityBJetTags");
    // float jetBProbability = jet->bDiscriminator("jetBProbabilityBJetTags");
    
    // // New 2012
    // float csv = jet->bDiscriminator("combinedSecondaryVertexBJetTags");
    
    // // Quark Gluon tagging
    // float qgTagMLP = jet->userFloat("qgTagMLP");
    // float qgTagLikelihood = jet->userFloat("qgTagLikelihood");
    
    // updateBranch(tree, &tcHighEfficiency, "btag_tc_high_eff");
    // updateBranch(tree, &tcHighPurity, "btag_tc_high_pur");
    // updateBranch(tree, &ssvHighEfficiency, "btag_ssv_high_eff");
    // updateBranch(tree, &ssvHighPurity, "btag_ssv_high_pur");
    // updateBranch(tree, &jetProbability, "btag_jet_probability");
    // updateBranch(tree, &jetBProbability, "btag_jet_b_probability");
    // updateBranch(tree, &csv, "btag_csv");
    // updateBranch(tree, &qgTagMLP, "qg_tag_mlp");
    // updateBranch(tree, &qgTagLikelihood, "qg_tag_likelihood");
    
    //jet energy composition
    float jetCHEn = jet->chargedHadronEnergy();
    float jetNHEn = jet->neutralHadronEnergy();
    float jetCEEn = jet->chargedEmEnergy();
    float jetNEEn = jet->neutralEmEnergy();
    float jetPhEn = jet->photonEnergy();
    float jetElEn = jet->electronEnergy();
    float jetMuEn = jet->chargedMuEnergy();
    //jet constituents multiplicities
    int jetPhMult = jet->photonMultiplicity();
    int jetNHMult = jet->neutralHadronMultiplicity();
    int jetElMult = jet->electronMultiplicity();
    int jetCHMult = jet->chargedHadronMultiplicity();
    
    updateBranch(tree, &jetCHEn, "jet_CHEn");
    updateBranch(tree, &jetNHEn, "jet_NHEn");
    updateBranch(tree, &jetPhEn, "jet_PhEn");
    updateBranch(tree, &jetElEn, "jet_ElEn");
    updateBranch(tree, &jetMuEn, "jet_MuEn");
    updateBranch(tree, &jetCEEn, "jet_CEEn");
    updateBranch(tree, &jetNEEn, "jet_NEEn");
    updateBranch(tree, &jetPhMult, "jet_PhMult", "I");
    updateBranch(tree, &jetNHMult, "jet_NHMult", "I");
    updateBranch(tree, &jetElMult, "jet_ElMult", "I");
    updateBranch(tree, &jetCHMult, "jet_CHMult", "I");
    
    tree->Fill(); // This Fill() must be called inside the {} block, otherwise it'll crash. Don't move it!
  } else {
    tree->Fill();
  }
  
  if (genTree) {
    particleToTree((jet) ? jet->genJet() : NULL, genTree, addresses);
    
    if (_findNeutrinos) {
      static bool init = false;
      if (! init) {
        genTree->Branch("neutrinos", &mNeutrinos, 32000, 0);
        genTree->Branch("neutrinos_pdg_id", &mNeutrinosPDG, 32000, 0);
        init = true;
      }
    }
    
    if (jet && _findNeutrinos) {
      const reco::Candidate* parton = (jet) ? jet->genParton() : NULL;
      
      if (parton) {
        if (abs(parton->pdgId()) == 5 || abs(parton->pdgId()) == 4) {
	  
          std::vector<const reco::Candidate*> neutrinos;
          findNeutrinos(parton, neutrinos);
	  
          if (neutrinos.size() > 0) {
            // Build TCloneArray of TLorentzVector
            unsigned int index = 0;
            for (const reco::Candidate* neutrino: neutrinos) {
              TLorentzVector* p4 = (TLorentzVector*) mNeutrinos->ConstructedAt(index);
              p4->SetPxPyPzE(neutrino->px(), neutrino->py(), neutrino->pz(), neutrino->energy());
	      
              TParameter<int>* pdg_id = (TParameter<int>*) mNeutrinosPDG->ConstructedAt(index++);
              pdg_id->SetVal(neutrino->pdgId());
            }
          }
        }
      }
    }
    
    // Add parton id and pt
    const reco::Candidate* parton = (jet) ? jet->genParton() : NULL;
    int pdgId = (parton) ? parton->pdgId() : 0;
    updateBranch(genTree, &pdgId, "parton_pdg_id", "I");
    
    TLorentzVector parton_p4;
    if (parton) {
      parton_p4.SetPxPyPzE(parton->px(), parton->py(), parton->pz(), parton->energy());
    }
    TLorentzVector* p_parton_p4 = &parton_p4;
    
    TBranch* branch = genTree->GetBranch("parton_p4");
    if (branch == NULL) {
      branch = genTree->Branch("parton_p4", &p_parton_p4);
    } else {
      branch->SetAddress(&p_parton_p4);
    }
    
    int flavour = (jet) ? jet->partonFlavour() : 0;
    updateBranch(genTree, &flavour, "parton_flavour", "I");
    
    genTree->Fill();
  }
}

void GammaJetFilter::metsToTree(const pat::MET& met, const pat::MET& rawMet, const std::vector<TTree*>& trees) {
  metToTree(&met, trees[0], trees[2]);
  metToTree(&rawMet, trees[1], NULL);
}

void GammaJetFilter::metToTree(const pat::MET* met, TTree* tree, TTree* genTree) {
  std::vector<boost::shared_ptr<void> > addresses;
  particleToTree(met, tree, addresses);
  
  tree->Fill();
  
  if (genTree) {
    particleToTree((met) ? met->genMET() : NULL, genTree, addresses);
    genTree->Fill();
  }
}

void GammaJetFilter::electronsToTree(const edm::Handle<pat::ElectronCollection>& electrons, const reco::Vertex& pv) {
  
  int n = electrons->size();
  static int   id[30];
  static float isolation[30];
  static float pt[30];
  static float px[30];
  static float py[30];
  static float pz[30];
  static float eta[30];
  static float phi[30];
  static int   charge[30];
  
  int i = 0;
  for (pat::ElectronCollection::const_iterator it = electrons->begin(); it != electrons->end(); ++it, i++) {
    const pat::Electron& electron = *it;
    
    if (i >= 30)
      break;
    
    // See https://twiki.cern.ch/twiki/bin/view/CMS/TopLeptonPlusJetsRefSel_el
    bool elecID = fabs(pv.z() - it->vertex().z()) < 1.;
    elecID     &= it->et() > 30.;
    elecID     &= fabs(it->eta()) < 2.5 && (it->superCluster()->eta() > 1.4442 && it->superCluster()->eta() < 1.5660);
    elecID     &= it->dB() < 0.02;
    elecID     &= ((int) it->electronID("eidLoose") & 0x1);
    
    float iso     = (it->dr03TkSumPt() + it->dr03EcalRecHitSumEt() + it->dr03HcalTowerSumEt()) / it->et();
    
    id[i]         = elecID;
    isolation[i]  = iso;
    pt[i]         = electron.pt();
    px[i]         = electron.px();
    py[i]         = electron.py();
    pz[i]         = electron.pz();
    eta[i]        = electron.eta();
    phi[i]        = electron.phi();
    charge[i]     = electron.charge();
  }

  updateBranch(mElectronsTree, &n, "n", "I");
  updateBranchArray(mElectronsTree, id, "id", "n", "I");
  updateBranchArray(mElectronsTree, isolation, "isolation", "n");
  updateBranchArray(mElectronsTree, pt, "pt", "n");
  updateBranchArray(mElectronsTree, px, "px", "n");
  updateBranchArray(mElectronsTree, py, "py", "n");
  updateBranchArray(mElectronsTree, pz, "pz", "n");
  updateBranchArray(mElectronsTree, eta, "eta", "n");
  updateBranchArray(mElectronsTree, phi, "phi", "n");
  updateBranchArray(mElectronsTree, charge, "charge", "n", "I");
  
  mElectronsTree->Fill();
}

void GammaJetFilter::muonsToTree(const edm::Handle<pat::MuonCollection>& muons, const reco::Vertex& pv) {
  
  int n = muons->size();
  static int   id[30];
  static float isolation[30];
  static float delta_beta_isolation[30];
  static int isLooseMuon[30];
  int nLooseMuon = 0;
  static float pt[30];
  static float px[30];
  static float py[30];
  static float pz[30];
  static float eta[30];
  static float phi[30];
  static int   charge[30];
  
  int i = 0;
  for (pat::MuonCollection::const_iterator it = muons->begin(); it != muons->end(); ++it, i++) {
    const pat::Muon& muon = *it;
    
    if (i >= 30)
      break;
    
    // See https://twiki.cern.ch/twiki/bin/view/CMS/TopLeptonPlusJetsRefSel_mu
    bool muonID = it->isGlobalMuon();
    //FIXME: reco::Tracks need to be keept in PF2PAT.
    //It's not the case right now, so muon ID will be incorrect
    if (it->globalTrack().isNull() || it->innerTrack().isNull() || it->muonBestTrack().isNull() || it->track().isNull()) {
      muonID = false;
    } else {
      muonID     &= it->globalTrack()->normalizedChi2() < 10.;
      muonID     &= it->globalTrack()->hitPattern().numberOfValidMuonHits() > 0;
      muonID     &= it->numberOfMatchedStations() > 1;
      muonID     &= it->dB() < 0.2;
      muonID     &= fabs(it->muonBestTrack()->dz(pv.position())) < 0.5;
      muonID     &= it->innerTrack()->hitPattern().numberOfValidPixelHits() > 0;
      muonID     &= it->track()->hitPattern().trackerLayersWithMeasurement() > 5;
    }
    
    float relIso = (it->chargedHadronIso() + it->neutralHadronIso() + it->photonIso()) / it->pt();
    float deltaBetaRelIso = (it->chargedHadronIso() + std::max((it->neutralHadronIso() + it->photonIso()) - 0.5 * it->puChargedHadronIso(), 0.0)) / it->pt();
    
    id[i]          = muonID;
    isolation[i]   = relIso;
    delta_beta_isolation[i] = deltaBetaRelIso;
    isLooseMuon[i]          = muon.isLooseMuon();
    pt[i]          = muon.pt();
    px[i]          = muon.px();
    py[i]          = muon.py();
    pz[i]          = muon.pz();
    eta[i]         = muon.eta();
    phi[i]         = muon.phi();
    charge[i]      = muon.charge();
    
    if(pt[i] > 5 && isLooseMuon[i]) nLooseMuon++ ;
  }
  
  updateBranch(mMuonsTree, &n, "n", "I");
  updateBranchArray(mMuonsTree, id, "id", "n", "I");
  updateBranchArray(mMuonsTree, isolation, "relative_isolation", "n");
  updateBranchArray(mMuonsTree, delta_beta_isolation, "delta_beta_relative_isolation", "n");
  updateBranchArray(mMuonsTree, isLooseMuon, "isLooseMuon", "n", "I");
  updateBranch(mMuonsTree, &nLooseMuon, "nLooseMuon", "I");
  updateBranchArray(mMuonsTree, pt, "pt", "n");
  updateBranchArray(mMuonsTree, px, "px", "n");
  updateBranchArray(mMuonsTree, py, "py", "n");
  updateBranchArray(mMuonsTree, pz, "pz", "n");
  updateBranchArray(mMuonsTree, eta, "eta", "n");
  updateBranchArray(mMuonsTree, phi, "phi", "n");
  updateBranchArray(mMuonsTree, charge, "charge", "n", "I");
  
  mMuonsTree->Fill();
}

//define this as a plug-in
DEFINE_FWK_MODULE(GammaJetFilter);
