import numpy as np

def fresnel_equations(n1, n2, theta_i):
  """
  Calculate the reflection and transmission coefficients for S and P polarized light using the Fresnel equations.

  Parameters:
  - n1: refractive index of the incident medium
  - n2: refractive index of the transmitted medium
  - theta_i: angle of incidence (in radians)

  Returns:
  - rs: reflection coefficient for S polarized light
  - rp: reflection coefficient for P polarized light
  - ts: transmission coefficient for S polarized light
  - tp: transmission coefficient for P polarized light
  """

  # Calculate the angle of transmission
  theta_t = np.arcsin((n1 / n2) * np.sin(theta_i))

  # Calculate the reflection coefficients
  rs = (n1 * np.cos(theta_i) - n2 * np.cos(theta_t)) / (n1 * np.cos(theta_i) + n2 * np.cos(theta_t))
  rp = (n2 * np.cos(theta_i) - n1 * np.cos(theta_t)) / (n2 * np.cos(theta_i) + n1 * np.cos(theta_t))

  # Calculate the transmission coefficients
  ts = 2 * n1 * np.cos(theta_i) / (n1 * np.cos(theta_i) + n2 * np.cos(theta_t))
  tp = 2 * n1 * np.cos(theta_i) / (n2 * np.cos(theta_i) + n1 * np.cos(theta_t))

  return rs, rp, ts, tp


def fresnel_Rboth(n1, n2, theta):
  
  rs, rp, ts, tp = fresnel_equations(n1, n2, theta)
  
  Rs = np.abs(rs)**2
  Rp = np.abs(rp)**2
  
  return Rp, Rs

def fresnel_Ref(n1, n2, theta):
  
  rs, rp, ts, tp = fresnel_equations(n1, n2, theta)
  
  Rs = np.abs(rs)**2
  Rp = np.abs(rp)**2
  
  return (Rs + Rp)/2.

print("done loading in fresnel equations")