from WMCore.Configuration import Configuration
config = Configuration()
config.section_('General')
config.General.transferOutputs = True
config.General.transferLogs = True
config.General.workArea = 'WORKINGAREA'
config.General.requestName = 'WORKINGDIR'
config.section_('JobType')
config.JobType.psetName = 'CMSSWCFG'
config.JobType.pluginName = 'Analysis'
config.JobType.outputFiles = ['OUTFILENAME']
config.section_('Data')
config.Data.inputDataset = 'INPUTDATASET'
config.Data.inputDBS = 'global'
config.Data.splitting = 'LumiBased'
config.Data.unitsPerJob = 50
#config.Data.lumiMask = '/cmshome/gdimperi/GammaJet/JetCorrections/CMSSW_7_4_3/src/JetMETCorrections/GammaJetFilter/analysis/2ndLevel/jsonFile_245155.json'
#config.Data.lumiMask = '/cmshome/gdimperi/GammaJet/JetCorrections/CMSSW_7_4_3/src/JetMETCorrections/GammaJetFilter/json/jsonFile_246908.json'
#config.Data.lumiMask = 'https://cms-service-dqm.web.cern.ch/cms-service-dqm/CAF/certification/Collisions12/8TeV/Prompt/Cert_190456-208686_8TeV_PromptReco_Collisions12_JSON.txt'
config.Data.lumiMask = '/cmshome/fpreiato/GammaJet/CMSSW_7_6_3_patch2/src/JetMETCorrections/GammaJetFilter/json/Cert_13TeV_16Dec2015ReReco_Collisions15_25ns_JSON.txt'
#config.Data.runRange = '251244-251252' # '193093-194075'
config.Data.ignoreLocality = True
config.Data.publication = False
config.Data.outLFNDirBase = '/store/user/fpreiato/JetMET2015/data/'
config.section_('User')
config.section_('Site')
config.Site.storageSite = 'T2_IT_Rome'
