#include <stdlib.h>
#include "drawExtrap.h"
#include "fitTools.h"
#include <TParameter.h>
#include <TColor.h>

#include "tclap/CmdLine.h"

bool useMCassoc_ = false;
bool NOQ = false;
bool FIXM = false;
bool EXCLUDE_FIRST_POINT = false;
bool OUTPUT_GRAPHS = true;

int main(int argc, char* argv[]) {

  try {
    TCLAP::CmdLine cmd("Perform Gamma+Jet extrapolation", ' ', "0.1");

    std::vector<std::string> jetTypes;
    jetTypes.push_back("pf");
    jetTypes.push_back("calo");
    TCLAP::ValuesConstraint<std::string> allowedJetTypes(jetTypes);

    TCLAP::ValueArg<std::string> typeArg("", "type", "jet type", false, "pf", &allowedJetTypes, cmd);

    std::vector<std::string> algoTypes;
    algoTypes.push_back("ak4");
    algoTypes.push_back("ak8");
    TCLAP::ValuesConstraint<std::string> allowedAlgoTypes(algoTypes);

    TCLAP::ValueArg<std::string> algoArg("", "algo", "jet algo", false, "ak4", &allowedAlgoTypes, cmd);

    std::vector<std::string> resoTypes;
    resoTypes.push_back("FIT");
    resoTypes.push_back("RMS");
    resoTypes.push_back("RMS70");
    resoTypes.push_back("RMS95");
    resoTypes.push_back("RMS99");
    TCLAP::ValuesConstraint<std::string> allowedResoTypes(resoTypes);

    TCLAP::ValueArg<std::string> resoArg("", "reso-algo", "algo for resolution calculation", false, "RMS99", &allowedResoTypes, cmd);

    TCLAP::UnlabeledValueArg<std::string> dataArg("data_dataset", "data dataset name", true, "Photon_Run2011", "string", cmd);
    TCLAP::UnlabeledValueArg<std::string> mc1Arg("mc1_dataset", "first MC dataset name", true, "G", "string", cmd);
    TCLAP::UnlabeledValueArg<std::string> mc2Arg("mc2_dataset", "second MC dataset name", true, "QCD", "string", cmd);

    cmd.parse(argc, argv);

    std::string data_dataset = dataArg.getValue();
    std::string mc_dataset = mc1Arg.getValue();
    std::string mc2_dataset = mc2Arg.getValue();
    std::string FIT_RMS = resoArg.getValue();
    std::string flags = "";

    /*std::string flags = "";
      if (argc == 8) {
      std::string flags_str(argv[7]);
      flags = flags_str;
      std::cout << "flags set." << std::endl;
      }*/

    /*bool useGenJets = false;
      if (argc == 9) {
      std::string flags_str(argv[8]);
      if (flags_str == "GENJETS") useGenJets = true;
      }*/


    std::string jetAlgo = (algoArg.getValue() == "ak4") ? "AK4" : "AK8";
    std::string recoType = (typeArg.getValue() == "pf") ? "PFlow" : "Calo";

    std::string postFix = recoType + jetAlgo;
    postFix += "chs";

    std::string algoType;
    if (recoType == "Calo")
      algoType = jetAlgo;
    else
      algoType = recoType + jetAlgo;

    if (recoType == "JPT" && jetAlgo == "AK4")
      algoType = "jptak4";

    //std::string mcFlags = (useGenJets) ? "GENJETS_" + flags : flags;

    //debugging
    //    if(typeArg.getValue() == "pf") std::cout<< "typeARG GIUSTO" << std::endl; 
    //    if(recoType == "PFlow") std::cout<< "recoType GIUSTO" << std::endl; 

    drawExtrap* db = new drawExtrap("PhotonJet", recoType, jetAlgo, true);
    db->set_pdf_aussi(false);
    db->set_isCMSArticle(false);

    db->set_FITRMS(resoArg.getValue());

    std::string NOQtext = (NOQ) ? "_NOQ" : "";
    std::string FIXMtext = (FIXM) ? "_FIXM" : "";
    std::string NOFIRSTPtext = (EXCLUDE_FIRST_POINT) ? "_NOFIRSTP" : "";

    char outputdir_char[200];
    /*if (mcFlags != "") {
      sprintf(outputdir_char, "PhotonJetExtrapPlots_%s_vs_%s_%s_%s_%s%s%s%s", data_dataset.c_str(), mc_dataset.c_str(), algoType.c_str(), mcFlags.c_str(), FIT_RMS.c_str(), FIXMtext.c_str(), NOFIRSTPtext.c_str(), NOQtext.c_str());
      } else*/
    {
      sprintf(outputdir_char, "PhotonJetExtrapPlots_%s_vs_%s_%s_%s%s%s%s", data_dataset.c_str(), mc_dataset.c_str(), algoType.c_str(), FIT_RMS.c_str(), FIXMtext.c_str(), NOFIRSTPtext.c_str(), NOQtext.c_str());
    }
    std::string outputdir_str(outputdir_char);

    //std::vector< float > ptPhot_binning = fitTools::getPtPhot_binning();

    db->set_outputdir(outputdir_str);

    TString dataFileName;
    if (flags.length() > 0) {
      dataFileName = TString::Format("PhotonJet_%s_%s_%s.root", data_dataset.c_str(), postFix.c_str(), flags.c_str());
    } else {
      dataFileName = TString::Format("PhotonJet_%s_%s.root", data_dataset.c_str(), postFix.c_str());
    }

    TFile* dataFile = TFile::Open(dataFileName);
    std::cout << "Opened data file '" << dataFileName << "'." << std::endl;

    db->add_dataFile(dataFile, data_dataset);

    TString mc1FileName;
    if (flags.length() > 0) {
      mc1FileName = TString::Format("PhotonJet_%s_%s_%s.root", mc_dataset.c_str(), postFix.c_str(), flags.c_str());
    } else {
      mc1FileName = TString::Format("PhotonJet_%s_%s.root", mc_dataset.c_str(), postFix.c_str());
    }
    TFile* mcPhotonJetFile = TFile::Open(mc1FileName);
    std::cout << "Opened mc file '" << mc1FileName << "'." << std::endl;

    db->add_mcFile(mcPhotonJetFile, mc_dataset, "#gamma + jets MC", TColor::GetColor(217, 91, 67));

    if (mc2_dataset != " ") {
      TString mc2FileName;
      if (flags.length() > 0) {
        mc2FileName = TString::Format("PhotonJet_%s_%s_%s.root", mc2_dataset.c_str(), postFix.c_str(), flags.c_str());
      } else {
        mc2FileName = TString::Format("PhotonJet_%s_%s.root", mc2_dataset.c_str(), postFix.c_str());
      }
      TFile* mcQCDFile = TFile::Open(mc2FileName);
      std::cout << "Opened mc file '" << mc2FileName << "'." << std::endl;

      if (mc_dataset != mc2_dataset) {
        db->add_mcFile(mcQCDFile, mc2_dataset, "QCD MC", TColor::GetColor(192, 41, 66));
      }
    }

    //Federico --> restore lumi
    double dLumi = 1e6;
    //Read luminosity
    if(dataFile) {
    TParameter<double>* lumi = static_cast<TParameter<double>*>(dataFile->Get("analysis/luminosity"));
    dLumi = lumi -> GetVal();
    }

    std::cout<< "Lumi  "<< dLumi << std::endl;

    //    db->set_lumiNormalization( dLumi * 1e-6);
    db->set_lumiNormalization( dLumi);

    db->set_NOQ(NOQ);
    db->set_FIXM(FIXM);
    db->set_EXCLUDEFIRSTPOINT(EXCLUDE_FIRST_POINT);

    //federico: correct pT Mean --- everything work fine
    TH1D *ptPhot = (TH1D*)dataFile->Get("analysis/ptPhoton_passedID");
    PtBinning ptBinning;
    size_t ptBinningSize = ptBinning.size();
    std::vector<std::pair<float, float> > ptBins = ptBinning.getBinning();
    // federico
    std::vector<float> ptMean;
    for( size_t i = 0 ; i< ptBinningSize ; i++){ 
      std::pair<float, float> currentBin = ptBinning.getBinValue(i);
      //    std::cout<< "Bin " << currentBin.first<< "-"<<currentBin.second <<std::endl; 
      ptPhot ->GetXaxis()->SetRangeUser(currentBin.first, currentBin.second);
      double Mean = ptPhot->GetMean();
      //federico --> debugging
      std::cout<< "Bin " << currentBin.first<< "-"<<currentBin.second<<" -> Mean  "<< Mean << std::endl; 
      ptMean.push_back(Mean);
    }
    //federico debug
    //  for(int i = 0 ; i<10 ; i++){
    //   std::cout<<"Vector check: Mean # " <<i << " : "<<ptMean.at(i)<<std::endl;
    // }
    


    EtaBinning etaBinning;
    size_t etaBinningSize = etaBinning.size();

    for (size_t i = 0; i < etaBinningSize; i++) {
      db->set_legendTitle(etaBinning.getBinTitle(i)); 

      db->drawResponseExtrap(ptMean, etaBinning.getBinName(i), etaBinning.getBinTitle(i), false);
      // db->drawResponseExtrap(ptMean, etaBinning.getBinName(i), etaBinning.getBinTitle(i), true);
    }

    //special case
    db->set_legendTitle("|#eta| < 1.3");
    db->drawResponseExtrap(ptMean, "eta0013", "|#eta| < 1.3", false);
    // db->drawResponseExtrap(ptMean, "eta0013", "|#eta| < 1.3", true);

    delete db;
  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }


  return 0;
}


