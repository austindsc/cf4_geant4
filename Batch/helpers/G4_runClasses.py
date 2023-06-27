
import sys
import os
import numpy as np
import numpy.random as npRand

current_directory = os.path.dirname(os.path.abspath(__file__))
sys.path.append(current_directory)

from G4_macHelpers import *
import FileGetters as FG
import uproot
import os

# the space is required on the "str_*" members,
# otherwise lines of text like mediumLY and mediumLY_visFraction will get mixed together
# when looking for "mediumLY"

## Handle all this using Uproot
class FullMCRun(object):

  # pass the full path to the mac and root files
  def __init__(self, macName):
    fullMac = macName+".mac"
    
    # expand the mac file
    self.mRunPars = RunParameters(macPath=fullMac)
    self.mDetPars = DetectorParameters(macPath=fullMac)
    self.mPhysPars = PhysicsParameters(macPath=fullMac)

    file, ttree = FG.openMyROOT_2(sfile = macName) # get just the file name with no suffix
    self.file = file
    self.mTtree = ttree # using the uproot ttree object
    NPE = self.mTtree["fNPE"].array()
    self.mNPE = NPE[NPE > 0]
    return
  
  # def doCAcorrection(self, nCA):
  
  def setup_forComparison(self, Nbins, lowRange=0, maxRange=100):
    # do a bunch of stuff to get some bins. moving to ROOT agnostic approach
    Raw_counts, self.mBin_Edges = np.histogram(self.mNPE, bins = int(Nbins), range=(lowRange, maxRange))
    self.mRaw_Counts = np.array(Raw_counts)
    self.mBinX = self.mBin_Edges[:-1]
    return True

  # def scale(self, scaleFactor):
  #   self.mScaled_counts = self.mRaw_Counts * scaleFactor
  
  # def normAndscale(self, desiredArea, binStart=0, binEnd=100):
  #   area = np.sum(self.mRaw_counts[binStart:binEnd])
  #   self.mNorm_Counts = np.array(self.mRaw_counts)/area
    
  #   self.mNorm_Counts *= desiredArea
  #   return area * desiredArea

# this tracks things like radiation source location,
# how many events are simulated, what medium is inside
class RunParameters(object):

  def __init__(self, gammaHeight:'cm above SiPM' = None, fname = None, macPath = None,
               gammaSrc = None, alphaHeight = None, gammaAngle = 2.5, Nbeam = 1e6, gammaCollimator = True) -> None:

    self.str_gammaHeight = "/setDetector/gammaHeight "
    self.str_gammaAngle = "/setDetector/GammaThetaMax "
    self.str_gammaSrc = "/setDetector/gammaSource "
    self.str_gammaColl = "/setDetector/useCollimator "
    self.str_alphaHeight = "/setDetector/alphaHeight "
    self.str_beamOn = "/run/beamOn "

    self.str_fname="/analysis/setFileName "

    if macPath is None:
      self.gammaHeight = gammaHeight
      self.gammaSrc = gammaSrc
      self.gammaColl = gammaCollimator
      self.alphaHeight = alphaHeight
      self.Nbeam = Nbeam
      self.gammaAngle = gammaAngle
      self.fname = fname

    elif macPath is not None:
      self.gammaHeight = extract_words_after_string(macPath, self.str_gammaHeight)
      self.gammaSrc = extract_words_after_string(macPath, self.str_gammaSrc)
      self.gammaColl = extract_words_after_string(macPath, self.str_gammaColl)
      self.alphaHeight = extract_words_after_string(macPath, self.str_alphaHeight)
      self.Nbeam = extract_words_after_string(macPath, self.str_beamOn)
      self.gammaAngle = extract_words_after_string(macPath, self.str_gammaAngle)
      self.fname = macPath
    return None
  
  # this returns a string with everything we need to build the mac file
  def to_Mac_string(self):
    outStr = ""

    if self.gammaColl is not None:
      outStr += self.str_gammaColl + str(str(self.gammaColl)) + "\n"
    if self.gammaAngle is not None:
      outStr += self.str_gammaAngle + str(self.gammaAngle) + "\n"
    if self.gammaHeight is not None:
      outStr  += self.str_gammaHeight + str(self.gammaHeight) + "\n"
    if self.gammaSrc is not None:
      outStr += self.str_gammaSrc + str(self.gammaSrc) + "\n"
    
    if self.alphaHeight is not None:
      outStr += self.str_alphaHeight + str(self.alphaHeight) + "\n"
    
    if self.fname is not None:
      outStr += self.str_fname + self.fname + "\n"
  
    return outStr

  def get_str_nbeam(self):
    return self.str_beamOn + str(int(self.Nbeam)) + "\n"



class DetectorParameters(object):

  def __init__(self, PDEscaling = 1.0, noise = 0, macPath = None):
    self.str_pde = "setDetector/pdeScaling"
    # self.noiseFrac = "setDetector/noise"
    # this is not a g4 parameter

    if macPath is None:
      self.pdeScale = PDEscaling
      self.noiseFrac = noise
    elif macPath is not None:
      # self.pdeScale = extract_words_after_string(macPath, "/setMaterial/mediumLY_VisFraction")
      # self.noiseFrac = extract_words_after_string(macPath, "/setMaterial/mediumLY_VisFraction")
      pass
    return None
  
  def to_Mac_string(self):

    outStr = ""

    #same as above
    return outStr


## physics of interest, and active medium parameters. But MEDIUM is in run parameters
class PhysicsParameters(object):
    
  def __init__(self, LY:"LY per MeV" = None, vf:"visible fraction" = None, birks:"mm/MeV" = None,
               medium = "lCF4", uvAttenuation = None, visAttenuation = None, temp:"in kelvin" = 128., pressure:"in bar" = 10., macPath = None) -> None:

    self.str_medium = "/setMaterial/detectionMedium "
    self.str_ly = "/setMaterial/mediumLY "
    self.str_vf = "/setMaterial/mediumLY_VisFraction "
    # self.str_birks = "/setMaterial/birks "
    # self.str_uvAttenuation = "/setMaterial/uvAttenuation "
    # self.str_visAttenuation = "/setMaterial/visAttenuation "

    self.str_temp = "/setMaterial/mediumTemp "
    self.str_pressure = "/setMaterial/mediumPressure "

    ## building from parameters
    if macPath is None:
      self.ly = LY
      self.vf = vf
      self.medium = medium
      self.temp = temp
      self.pressure = pressure
      # self.birks = birks
      # self.uvAttenuation = uvAttenuation
      # self.visAttenuation = visAttenuation
    ## building object from .mac file
    elif macPath is not None: 
      self.medium = extract_words_after_string(macPath, self.str_medium) 
      # self.birks = mediumLY_VisFraction
      # self.uvAttenuation = 
      # self.visAttenuation = mediumLY_VisFraction

      ly, unit = extract_words_after_string(macPath, self.str_ly)
      if unit == "MeVLY":
        self.ly = ly
      else:
        print("error in unit on LY from .mac, in func", __name__)
        exit(0)
      
      self.vf = extract_words_after_string(macPath, self.str_vf)

      temp, unit = extract_words_after_string(macPath, self.str_temp)
      if unit == "kelvin":
        self.temp = temp
      else:
        print("error in unit on temp from .mac, in func", __name__)
        exit(0)
      
      pressure, unit = extract_words_after_string(macPath, self.str_pressure)
      if unit == "bar":
        self.pressure = pressure
      else:
        print("error in unit on temp from .mac, in func", __name__)
        exit(0)
    return None
  
  def to_Mac_string(self):
    stringOut = self.str_medium + str(self.medium) + " \n"
    stringOut += self.str_temp + str(self.temp) + " kelvin \n"
    stringOut += self.str_pressure + str(self.pressure) + " bar \n"
    stringOut += self.str_ly + str(self.ly) + " MeVLY \n"
    stringOut += self.str_vf + str(self.vf) + " \n"

    # birks, UV attenuation
    return stringOut
    

## building the .mac files from the three objects
def build_mac_from_items(physPar, detPar, runPar, fname = "testMacOutput", myVerbosity = 0, savePath=".", g4Verbosity=0):

  verbosities = """
/vis/verbose {0}
/tracking/verbose {0}
/event/verbose {0}
/run/verbose {0}

/process/em/verbose {0}
/process/had/deex/verbose {0}
/particle/verbose {0}
/tracking/verbose {0}
""".format(g4Verbosity)

  ## make filename in other code
  
  with open(savePath+"/"+fname+".mac", "w") as file:

    ## normal intro stuff
    file.write(verbosities + "\n\n")

    ## detector materials (phys Par)
    setMats = physPar.to_Mac_string()
    file.write(setMats + "\n\n")

    file.write("/setDetector/myVerbosity {0}".format(myVerbosity) + "\n\n")

    ## Detector Properties
    setProperties = detPar.to_Mac_string()

    setDets = runPar.to_Mac_string()
    file.write(setDets + "\n\n")

    ## initialize
    file.write("/run/initialize" + "\n")

    ## runBeam
    file.write(runPar.get_str_nbeam())

  return