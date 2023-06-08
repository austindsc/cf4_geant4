import os
import numpy as np
import uproot

from ROOT import TFile

def get_all_root_files(directory: "path to directory"):
  
  # Get all files in the directory
  files = os.listdir(directory)
  
  # Filter for .root files
  root_files = [f for f in files if f.endswith('.root')]
  
  return root_files

def openMyROOT_2(sfile: str, treeName = "simOutput"):
    if os.path.exists(sfile+".root") and os.path.exists(sfile+".mac"):
        f = uproot.open(sfile+".root")
        print("loaded in:", sfile+".root")
        return f, f[treeName]
    elif not os.path.exists(sfile+".root"):
        print("does not exist:", sfile+".root")
        exit(1)
    elif not os.path.exists(sfile+".mac"):
        print("does not exist:", sfile+".mac")
        exit(1)
    return None, None


# returns file f and ttree
def openMyROOT(sfile: str, treename = "simOutput"):
    if os.path.exists(sfile+".root") and os.path.exists(sfile+".mac"):
        f = TFile(sfile+".root", "READ")
        print("loaded in:", sfile+".root")
        return f, f.Get(treename)
    elif not os.path.exists(sfile+".root"):
        print("does not exist:", sfile+".root")
        return None, None
    elif not os.path.exists(sfile+".mac"):
        print("does not exist:", sfile+".mac")
        return None, None
    return None, None