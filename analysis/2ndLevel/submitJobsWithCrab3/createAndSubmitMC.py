#! /usr/bin/env python

import os
import sys
import argparse
import datetime 
import re

usage = "Usage: python createAndSubmitMC.py -d Output_Run2 -v qq_RSG_gg_pu20bx25_TagXXX -i Inputs/InputList_XXX.txt -t Inputs/crab3_template_mc.py -c ../runFilter_MC.py --submit"
print usage

parser = argparse.ArgumentParser(description='Process options.')

parser.add_argument("-i", "--inputList", type=str, dest="inputList", default="",
        help="input datasets with global tag",
	    )
parser.add_argument("-d", "--storageDir", type=str, dest="storagedir", default="",
        help="working directory",
	    )
parser.add_argument("-t", type=str, dest="template_crab", default="",
        help="template of crab configuration file",
	    )
parser.add_argument("-c",  type=str, dest="template_cmssw", default="",
        help="template of cmssw file to execute",
	    )
parser.add_argument("-p",  type=str, dest="publish_name", default="",
        help="name for publication",
	    )
parser.add_argument("-v",  type=str, dest="tagname", default="",
        help="template of cmssw file to execute",
	    )
parser.add_argument("-n",  type=str, dest="username", default="",
        help="username",
	    )
parser.add_argument("--submit",  action="store_true", dest="submit", default=False,
        help="submit jobs with CRAB",
	    )


args = parser.parse_args()
print args 

inputList = args.inputList
storagedir = args.storagedir
template_crab = args.template_crab
template_cmssw = args.template_cmssw
publish_name = args.publish_name
tagname = args.tagname
username = args.username
submit = args.submit
#######################

#read input file
ins = open(args.inputList,"r")
#datasets = []
#list_processedevents = []
#list_filesperjob = []
#globaltags = []

current_time = datetime.datetime.now()
namedir = tagname+"_%04d%02d%02d_%02d%02d" % (current_time.year,current_time.month,current_time.day,current_time.hour,current_time.minute)  
os.system("mkdir "+storagedir+"/"+namedir)
os.system("mkdir "+storagedir+"/"+namedir+"/cfg")
os.system("mkdir "+storagedir+"/"+namedir+"/workdir")


for line in ins:
  dataset = line.split()[0]
  processedevents = line.split()[1]
  xsection = line.split()[2]
  ptHatMin = line.split()[3]
  ptHatMax = line.split()[4]
  filesperjob = line.split()[5]
  globaltag = line.split()[6]
  globaltag = globaltag.strip()

  #datasets.append(dataset)  
  #list_processedevents.append(processedevents)
  #list_filesperjob.append(filesperjob)
  #globaltags.append(globaltag)

  if (line.startswith("#")):
   continue

  print "line : "+line
  print "dataset : "+dataset
  print "processedevents : "+processedevents +  "\n ******* \nNOTE that for flat samples the correct thing to put in processedEvents is 1!!\n*************"
  print "cross section : "+xsection
  print "ptHat Min : "+ptHatMin
  print "ptHat Max : "+ptHatMax
  print "filesperjob : "+filesperjob
  print "globaltag : "+globaltag

  sample = dataset.split("/")[1]
  print sample


  dict = {
      "THISROOTFILE":"\""+sample+"__"+dataset.split("/")[2]+"__"+dataset.split("/")[3]+".root"+"\"",
      "THISGLOBALTAG":"\""+globaltag+"\"",
      "WORKINGAREA":storagedir+"/"+namedir+"/workdir", 
#      "WORKINGDIR":sample+"__"+dataset.split("/")[2]+"__"+dataset.split("/")[3], 
      "WORKINGDIR":sample, 
      "CMSSWCFG":storagedir+"/"+namedir+"/cfg/"+sample+"_cmssw.py",
      "OUTFILENAME":sample+"__"+dataset.split("/")[2]+"__"+dataset.split("/")[3]+".root",
      "INPUTDATASET":dataset,
      "FILESPERJOB":filesperjob,
      "procEvents":processedevents,
      "xsec":xsection,
      "ptMin":ptHatMin,
      "ptMax":ptHatMax
      }
##create cmssw configuration file
  cmssw_cfgfile = storagedir+"/"+namedir+"/cfg/"+sample+"_cmssw.py"
  with open(cmssw_cfgfile, "wt") as fout:
    with open(template_cmssw, "rt") as fin:
      for line_ in fin:
	#fout.write(line_)
	line_=line_.strip()
	for k,v in dict.items():
	  line_ = re.sub(k,v,line_)
	fout.write(line_+"\n")


##create crab 3 configuration file
  crab_cfgfile = storagedir+"/"+namedir+"/cfg/"+sample+"_crab.py"
  with open(crab_cfgfile, "wt") as fout:
    with open(template_crab, "rt") as fin:
      for line_ in fin:
	line_=line_.strip()
        for k,v in dict.items():
	  line_ = re.sub(k,v,line_)
        fout.write(line_+"\n")

  if submit:
    os.system("crab submit -c "+crab_cfgfile)


