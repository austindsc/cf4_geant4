import matplotlib.colors as mplColors
import matplotlib.pyplot as plt

class StyleThing:
  def __init__(self, color, linestyle):
    self.col = color
    self.ls = linestyle
    return
    
  def printMembers(self):
    style_thing_members = [attr for attr in dir(self) if not attr.startswith('__')]
    print("style object members:\n \t", style_thing_members)
    return
    
class MyStyleSet:
  def __init__(self, option = "nobles"):
    
    # medium types. these are what populate the other arrays 
    self.air = StyleThing('goldenrod', '-')
    
    self.lxe = StyleThing('darkviolet', '-')
    self.lxe2 = StyleThing('fuchsia', '-')

    self.cf4 = StyleThing('deepskyblue', '-')
    # self.cf4_2 = StyleThing('deepskyblue', '-')
    
    self.green = StyleThing('springgreen', '-')
    
    self.red = StyleThing('red', '-')
    
    self.test = StyleThing('#d94ff5', '-')
    
    # line styles
    self.line1 = "-"
    self.line2 = "--"
    self.line3 = "-."
    
    # all colors
    self.names = [x for x in self.__dict__.keys() if isinstance(self.__dict__[x], StyleThing)]
    self.allColsStr = [x.col for x in self.__dict__.values() if isinstance(x, StyleThing)]
    self.allColsObj = mplColors.to_rgba_array(self.allColsStr).reshape(1, -1, 4)
    
  def show(self):
    fig, ax = plt.subplots(figsize = (len(self.names)*1.5, 2))
    ax.set_xticks([])
    ax.set_yticks([])
    ax.imshow(self.allColsObj, aspect=0.5)
    for i, (color, name) in enumerate(zip(self.allColsStr, self.names)):
      txt = f"{name} \n {color}"
      ax.text(i, 0, txt, ha='center', va='center')
    plt.show()
    return
  
  
  def SetMPLcolors(self, toggle = "on"):
    if toggle == "off" or toggle == 0:
      # plt.rcParams['axes.prop_cycle'] = plt.cycler(color=plt.rcParamsDefault['axes.prop_cycle'])
      plt.rcParams.update(plt.rcParamsDefault)
      return
    cycleCols = [getattr(self, name).col for name in self.names]
    plt.rcParams['axes.prop_cycle'] = plt.cycler(color=cycleCols)
    return

def ShowMyStyles():
  ms = MyStyleSet()
  print(ms.names)
  ms.air.printMembers()
  ms.show()
  return

## how to use
# %run MyStyle1.py
# or
# import MyStyle1 as MS

# MS.ShowMyStyles() # for colors
# ms = MyStyleSet() # object to use like
# ms.air.col # to get the color of 'air'

## toggle the matplotlib colors to these
# ms.SetMPLcolors() 
# ms.SetMPLcolors("off")