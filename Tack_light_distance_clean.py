import numpy as np
import matplotlib.pyplot as plt
from light_sensing_simulation_tool import light_source_sensor_simulator


# Values of sensors when light directly ahead at distance 0.2m
B = 20812.0
A = 4882.0

# Constants for sensor value V = C * cos / d^2
C_lux = A / 250
C_RGB = 28.803808

##========== Testing Parameters
theta_reals = [0,1,2.5,5,10,15,20,30, 40] # [deg]
thetas = [] # calculated values for theta stored here
distances = [0.2, 0.25, 0.3, 0.4, 0.5, 0.6, 0.8, 1.0] # [m]
predicted_distance_lux = [] # distances calculated using lux sensor values
predicted_distance_RGB = [] # distances calculated using rgb sensor values
beta = 45 # lux sensor mounting angles # [deg]
lux_sens01_angle = beta
lux_sens02_angle = -beta
rgb_sens_angle = 0
light_sensing_sim = light_source_sensor_simulator()

def lux(angle, n, m):
    return np.exp(-(n * angle) ** 3) * np.cos(m * angle)

for theta_i in theta_reals:
    # Predicted value for this angle, at a range of distances
    predicted_distance_lux_i = []
    predicted_distance_RGB_i = []
    thetas_i = []
    for i in distances:
        # Grabbing simulation data 
        sensor_data = light_sensing_sim.compute_sensor_response(i,
                                                                theta_i, lux_sens01_angle,
                                                                lux_sens02_angle, rgb_sens_angle)
        Vs1 = sensor_data['lux_sens01_signal']
        Vs3 = sensor_data['lux_sens02_signal']
        Vs2 = sensor_data['rgb_sens_signal']
        
        # Formula derived by hand
        # Error negligable at small angles, increasing with angle due to cosine
        # approximation. cosine approximation much better for rgb sensor that
        # lux sensors.
        theta = np.atan((Vs1 - Vs3) / ((A / B) * Vs2 * 2 * np.sin(beta * np.pi/180)))
        thetas_i.append(180/np.pi * theta)
        
        # Normalised sensor values i.e. predicted sensor value if the realtive
        # angle between sensor normal and light source was zero.
        # Due to cosine behaviour at high angles, small errors make a big
        # difference.
        
        # Vs1_i = Vs1 / np.cos(theta + beta * np.pi/180)
        # Vs3_i = Vs3 / np.cos(theta - beta * np.pi/180)
        # values for lux sensors very poor at high angles.
        
        a = 0.004666810926917832
        b = 0.018159980049788253
        # lux approx V = e ^ (-(a * angle) ^ 3) * cos(b * angle)
        Vs1_i = Vs1 / lux(theta + beta, a, b)
        Vs3_i = Vs3 / lux(theta - beta, a, b)
        
        n = 1.088025599603545
        Vs2_i = Vs2 / np.cos(theta) ** n
        
        # RGB sensor response better approximated by cos^n
        # Resulting distance prediction from rgb sensor very accurate
        # Only using RGB -> suseptible to noise
        # Require better approximation for lux sensor response in order to use.
        
        # using V = C / d^2
        # Value for C_lux based on taking an average (root 0.5 absorbed)
        distance_lux = C_lux / np.sqrt((Vs1_i + Vs3_i))
        distance_rgb = C_RGB / np.sqrt(Vs2_i)
        
        predicted_distance_lux_i.append(distance_lux)
        predicted_distance_RGB_i.append(distance_rgb)
        
    predicted_distance_lux.append(predicted_distance_lux_i)
    predicted_distance_RGB.append(predicted_distance_RGB_i)
    thetas.append(thetas_i)

print("Values for distance fround with RGB sensor")
print("Distance [m]\t|\t\t\tAngles [deg]")
print("\t\t|\t" + "\t|\t".join([str(i) for i in theta_reals]))
for i in range(len(distances)):
    print(f"{distances[i]}\t\t|\t" + "\t|\t".join([str(round(j[i],4)) for j in predicted_distance_RGB]))

print("\nValues for distance found with lux sensors")
print("Distance [m]\t|\t\t\tAngles [deg]")
print("\t\t|\t" + "\t|\t".join([str(i) for i in theta_reals]))
for i in range(len(distances)):
    print(f"{distances[i]}\t\t|\t" + "\t|\t".join([str(round(j[i],4)) for j in predicted_distance_lux]))

print("\nValues for angle found with all sensors")
print("Distance [m]\t|\t\t\tAngles [deg]")
print("\t\t|\t" + "\t|\t".join([str(i) for i in theta_reals]))
for i in range(len(distances)):
    print(f"{distances[i]}\t\t|\t" + "\t|\t".join([str(round(j[i],4)) for j in thetas]))
