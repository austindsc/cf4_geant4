
import sys
import os
import numpy as np
import time

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
  def __init__(self, macName, skipRootHeights = [], verbose = 1):
    fullMac = macName+".mac"
    
    # expand the mac file
    self.mRunPars = RunParameters(macPath=fullMac)
    self.mDetPars = DetectorParameters(macPath=fullMac)
    self.mPhysPars = PhysicsParameters(macPath=fullMac)
    self.SKIPTHIS = False
    
    if self.mRunPars.gammaHeight is not None:
      if np.isclose(self.mRunPars.gammaHeight, np.array(skipRootHeights)).any():
        print("Skipping root file read for height:", self.mRunPars.gammaHeight) if verbose > 0 else None
        self.SKIPTHIS = True
        return
      
    file, ttree = FG.openMyROOT_2(sfile = macName, verbose=verbose) # get just the file name with no suffix
    self.file = file
    self.mTtree = ttree # using the uproot ttree object
    NPE = self.mTtree["fNPE"].array()
    self.mNPE = np.array(NPE[NPE > 0])
    self.mNPE = np.array(self.mNPE) # keep a copy of the original

    self.MtotalMeasured = len(self.mNPE)
    self.Nbeam = self.mRunPars.Nbeam
    self.eff = self.MtotalMeasured/self.Nbeam
    return
  
  ## 1- p / p = mean
  ## m p = 1-p
  ## (m + 1) p = 1
  ## p = 1/(m+1)
  def doCAcorrection_GeometricMC(self, nCA, rand = np.random.default_rng(seed = int(time.time()))):
    pGeo = 1./(1 + nCA) # this is the success probability for the geometric distribution, not the success of a CA happening.
    self.mCA = nCA
    self.mNPE_corrGeo = np.array([np.sum(rand.geometric(p = pGeo, size = pe)) for pe in self.mNPE])
    self.mNPE_corr = self.mNPE_corrGeo
    return self.mNPE_corrGeo

  def doCAcorrection_BinomialMC(self, nCA):
    self.mCA = nCA
    rng = np.random.default_rng(seed=69)
    self.mNPE_corrBinomial = self.mNPE + rng.binomial(self.mNPE, nCA)
    self.mNPE_corr = self.mNPE_corrBinomial
    return self.mNPE_corrBinomial
  
  # options are mNPE_corrBinomial, mNPE_corrGeo, mNPE
  # do a bunch of stuff to get some bins. moving to ROOT agnostic approach
  def setup_forComparison(self, Nbins = 100, lowRange=0.5, maxRange=100.5, variable = "mNPE"):
    npe = getattr(self, variable)
    
    self.mBin_Edges = np.linspace(lowRange, maxRange, Nbins+1)
    self.mRaw_Counts, _ = np.histogram(npe, bins = self.mBin_Edges, range=(self.mBin_Edges[0], self.mBin_Edges[-1]))
    self.mBinX = self.mBin_Edges[:-1] + np.diff(self.mBin_Edges)/2.
    
    ## normalize counts by Nbeam
    # self.mRaw_Counts = self.mRaw_Counts / self.Nbeam
    
    return True
  
  def getPEMean(self):
    return np.mean(self.mNPE), np.mean(self.mNPE_corrGeo)
  
  def mpl_errorbar(self, ax, scale=1, hshift=0, label=None, marker="o", linestyle="None", **kwargs):
    y = self.mRaw_Counts * scale
    x = self.mBinX - hshift
    yerr = np.sqrt(self.mRaw_Counts) * scale
    ax.errorbar(x, y, yerr=yerr, label=label, marker=marker, linestyle=linestyle, **kwargs)
    return ax
  
  def mpl_hist(self, ax, scale=1, hshift=0, label=None, **kwargs):
    y = self.mRaw_Counts * scale
    x = self.mBinX - hshift
    ax.hist(x, weights=y, bins=self.mBin_Edges, label=label, **kwargs)
    return ax

  ## for alpha analysis. dont use cause get empty bins. integer x float --> integer
  ## this will be a fairly expensive call
  # def RescaleLY(self, scale):
  #   # scaled = np.array(self.mNPE_original * scale, dtype=float)
  #   # ints = np.floor(scaled).astype(int)
  #   # frac = scaled - ints
  #   # self.mNPE = ints + (np.random.random(len(frac)) < frac)
  #   self.mNPE = np.array(self.mNPE_original * scale, dtype=float)
    
  #   self.MtotalMeasured = len(self.mNPE)
  #   if hasattr(self, "mNPE_corrGeo"):
  #     self.doCAcorrection_BinomialMC(self.mCA)
  #   if hasattr(self, "mNPE_corrBinomial"):
  #     self.doCAcorrection_GeometricMC(self.mCA)
    
  #   self.setup_forComparison(Nbins = len(self.mBin_Edges)-1, lowRange = self.mBin_Edges[0], maxRange = self.mBin_Edges[-1], variable = "mNPE")
  #   return
    

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
               gammaSrc = None, alphaHeight = None, gammaAngle = 2.5, Nbeam = 1e6, gammaCollimator = True, LPfilter = False, voltage=4) -> None:

    self.str_gammaHeight = "/setDetector/gammaHeight "
    self.str_gammaAngle = "/setDetector/GammaThetaMax "
    self.str_gammaSrc = "/setDetector/gammaSource "
    self.str_gammaColl = "/setDetector/useCollimator "
    self.str_alphaHeight = "/setDetector/alphaHeight "
    self.str_beamOn = "/run/beamOn "
    self.str_voltage = "/setDetector/setVoltage "

    self.str_lpFilter = "/setDetector/useLPFilter "
    self.str_fname="/analysis/setFileName "

    if macPath is None:
      self.gammaHeight = gammaHeight
      self.gammaSrc = gammaSrc
      self.gammaColl = gammaCollimator
      self.alphaHeight = alphaHeight
      self.Nbeam = Nbeam
      self.gammaAngle = gammaAngle
      self.fname = fname
      self.lpFilter = LPfilter
      self.voltage = voltage

    elif macPath is not None:
      gh = extract_words_after_string(macPath, self.str_gammaHeight)
      self.gammaHeight = float(gh) if gh is not None else None
      self.gammaSrc = extract_words_after_string(macPath, self.str_gammaSrc)
      self.gammaColl = extract_words_after_string(macPath, self.str_gammaColl)
      self.alphaHeight = extract_words_after_string(macPath, self.str_alphaHeight)
      self.Nbeam = extract_words_after_string(macPath, self.str_beamOn)
      self.gammaAngle = extract_words_after_string(macPath, self.str_gammaAngle)
      self.fname = macPath.rstrip(".mac")
      vh = extract_words_after_string(macPath, self.str_voltage)
      self.voltage = float(vh) if vh is not None else None

      # check if LP filter command exists in macro (it has no arguments)
      lpFilter_line = extract_words_after_string(macPath, self.str_lpFilter[:-2]) # remove the space and get the line
      self.lpFilter = True if lpFilter_line is not None else False
    return None
  
  def __str__(self):
    return f"RunParams print: {self.gammaSrc}, h:{self.gammaHeight}, filter:{self.lpFilter}, id: {id(self)}"
  
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

    if self.voltage is not None:
      outStr += self.str_voltage + str(self.voltage) + "\n"

    if self.lpFilter is not None and self.lpFilter == True:
      outStr += self.str_lpFilter.rstrip() + "\n"
    
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
  
  def __eq__(self, otherClass):
    if isinstance(otherClass, self.__class__):
      return all(self.__dict__.get(key) == otherClass.__dict__.get(key) for key in self.__dict__.keys())
    return False
  
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
    self.lyUnit = None
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
        self.ly, self.lyUnit = float(ly), "MeVLY"
      elif unit == "keVLY":
        self.ly, self.lyUnit = float(ly), "keVLY"
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
    stringOut += self.str_ly + str(self.ly) + str(" ") + str(self.lyUnit) + "\n"
    stringOut += self.str_vf + str(self.vf) + " \n"

    # birks, UV attenuation
    return stringOut
  
  def __str__(self):
    return "PhysicsParams print:\n" + self.to_Mac_string()
  
  def __eq__(self, otherClass):
    if isinstance(otherClass, self.__class__):
      return all(self.__dict__.get(key) == otherClass.__dict__.get(key) for key in self.__dict__.keys())
    return False
  
    

## building the .mac files from the three objects
def build_mac_from_items(physPar, detPar, runPar, fname = "testMacOutput", myVerbosity = 0, savePath="", g4Verbosity=0):

  verbosities = """
/vis/verbose {0}
/tracking/verbose {0}
/event/verbose {0}
/run/verbose {0}

/particle/verbose {0}
/tracking/verbose {0}
""".format(g4Verbosity)

  ## make filename in other code
  finalName = savePath+fname+".mac"
  with open(finalName, "w") as file:

    ## normal intro stuff
    file.write(verbosities + "\n\n")

    ## detector materials (phys Par)
    setMats = physPar.to_Mac_string()
    file.write(setMats + "\n\n")

    file.write("/setDetector/myVerbosity {0}".format(int(myVerbosity)) + "\n\n")

    ## Detector Properties
    setProperties = detPar.to_Mac_string()

    setDets = runPar.to_Mac_string()
    file.write(setDets + "\n\n")

    ## initialize
    file.write("/run/initialize" + "\n")

    ## runBeam
    file.write(runPar.get_str_nbeam())

  return finalName