import numpy as np
import matplotlib.pyplot as plt
from light_sensing_simulation_tool import light_source_sensor_simulator
from scipy.optimize import curve_fit

# --- Sensor angular sensitivity model (generic interpolation) ---
lux_angles = np.array([-90.29032258, -85.06451613, -80.70967742, -76.06451613,
                       -72.29032258, -68.51612903, -64.74193548, -60.09677419,
                       -55.74193548, -51.38709677, -47.03225806, -42.67741935,
                       -36.87096774, -29.90322581, -23.22580645, -14.51612903,
                        -5.51612903,  -0.29032258,   9.00000000,  16.54838710,
                        22.35483871,  29.32258065,  35.70967742,  40.64516129,
                        45.58064516,  51.38709677,  55.16129032,  59.22580645,
                        64.16129032,  68.51612903,  72.87096774,  76.35483871,
                        80.12903226,  83.90322581,  90.58064516])

lux_sensitivity = np.array([0.003236246, 0.072815534, 0.126213592, 0.199029126, 0.281553398,
                   0.343042071, 0.402912621, 0.470873786, 0.546925566, 0.600323625,
                   0.665048544, 0.729773463, 0.802588997, 0.860841424, 0.914239482,
                   0.962783172, 0.988673139, 0.995145631, 0.977346278, 0.949838188,
                   0.922330097, 0.868932039, 0.802588997, 0.744336570, 0.687702265,
                   0.603559871, 0.540453074, 0.472491909, 0.401294498, 0.317152104,
                   0.254045307, 0.199029126, 0.137540453, 0.085760518, 0.0])

def f(theta, a, b, c):
    #formula to replace cos using datasheet
    return a * np.cos(np.pi/180 * theta * b) + c

a,b,c = curve_fit(f, lux_angles, lux_sensitivity)[0]

beta = 45 # [deg] angle between each light sensor and centreline

# values compared to what is found
theta_real = -10 # [deg] angle between centreline and light source
d0_real = 0.2 # [m]

# a = 254.919935461

# Vs1 = 4762.0
# Vs2 = 1

### Using simulation to get sensor values
light_sensing_sim = light_source_sensor_simulator()

light_source_distance = d0_real # Distance to light source in meters
light_source_angle = theta_real    # Angle with light source in degrees
lux_sens01_angle = 60     # Lux sensor 1 placement angle in degrees
lux_sens02_angle = -beta     # Lux sensor 2 placement angle in degrees
rgb_sens_angle = 0.0        # RGB sensor placement angle in degrees

sensor_data = light_sensing_sim.compute_sensor_response(light_source_distance,
                                                        light_source_angle, lux_sens01_angle,
                                                        lux_sens02_angle, rgb_sens_angle)

Vs1 = sensor_data['lux_sens01_signal']
Vs3 = sensor_data['lux_sens02_signal']
Vs2 = sensor_data['rgb_sens_signal']

# Values of sensors when light directly ahead at distance 0.2m
B = 20812.0
A = 4882.0
t = (Vs1 - Vs3) / ((A / B) * Vs2 * 2 * np.sin(beta * np.pi/180))
theta = 180/np.pi * np.atan(t)
# print(theta)

############### testing #################
# theta_reals = [0,1,2.5,5,-5,10,-15,20,30,40,50,60,70,80,90]
theta_reals = [0,1,2.5,5,10,15,20,30,40,45,50]
thetas = []
arr1 = []
arr2 = []
arr3 = []
C_lux = A / 250
C_RGB = B * 0.00026613247118877 * 5
arrV1 = []
arrV2 = []
arrV3 = []

for i in theta_reals:
    sensor_data = light_sensing_sim.compute_sensor_response(light_source_distance,
                                                            i, lux_sens01_angle,
                                                            lux_sens02_angle, rgb_sens_angle)
    Vs1 = sensor_data['lux_sens01_signal']
    Vs3 = sensor_data['lux_sens02_signal']
    Vs2 = sensor_data['rgb_sens_signal']
    theta = np.atan((Vs1 - Vs3) / ((A / B) * Vs2 * 2 * np.sin(beta * np.pi/180)))
    thetas.append(180/np.pi * theta)
    n=1 # 1.5 -> rgb totally flat
    Vs1_i = Vs1 / np.cos((i + beta) * np.pi/180) ** n
    Vs3_i = Vs3 / np.cos((i - beta) * np.pi/180) ** n
    Vs2_i = Vs2 / np.cos(i * np.pi/180) ** n
    # distance_lux = np.sqrt(C_lux / (Vs1_i + Vs3_i))
    # distance_rgb = np.sqrt(C_RGB / Vs2_i)
    # arr1.append(Vs1_i)
    # arr3.append(Vs3_i)
    # arr2.append(Vs2_i)
    # arrV1.append(Vs1 / A)
    # arrV2.append(Vs2 / B)
    # arrV3.append(Vs3 / A)
    # print(i)
    # print(Vs1_i)
    # print(Vs3_i)
    # print(Vs2_i)

# plt.figure(1)
# plt.plot(theta_reals[:-4], arr1[:-4], label="Vs1")
# plt.plot(theta_reals[:-4], arr3[:-4], label="Vs3")
# plt.plot(theta_reals[:-4], (np.array(arr1)[:-4] + np.array(arr3)[:-4]) / 2, label="avg lux")
# plt.plot(theta_reals[:-4], arr2[:-4], label="RGB Vs2")
# plt.legend()

# plt.figure(2)
# plt.plot(theta_reals, arrV1, label="V1")
# plt.plot(theta_reals, arrV2, label="V2")
# plt.plot(theta_reals, arrV3, label="V3")
# theta_reals = np.array(theta_reals)
# plt.plot(theta_reals, np.cos(theta_reals*np.pi/180)**1.5)
# plt.plot(theta_reals, np.cos((theta_reals+beta)*np.pi/180)**1.3)
# plt.plot(theta_reals, np.cos((theta_reals-beta)*np.pi/180)**1.5)
# plt.legend()
# theta_reals = [0,1,2.5,5,-5,10,-15,20,30,40,50,60,70,80,90]
theta_reals = [0,1,2.5,5,10,15,20,30]
thetas = []
distances = [0.2, 0.25, 0.3, 0.4, 0.5, 0.6, 0.8, 1.0] # [m]
predicted_distance_lux = []
predicted_distance_RGB = []
beta = 45
lux_sens01_angle = beta
lux_sens02_angle = -beta
rgb_sens_angle = 0

for theta_i in theta_reals:
    predicted_distance_lux_i = []
    predicted_distance_RGB_i = []
    thetas_i = []
    for i in distances:
        sensor_data = light_sensing_sim.compute_sensor_response(i,
                                                                theta_i, lux_sens01_angle,
                                                                lux_sens02_angle, rgb_sens_angle)
        Vs1 = sensor_data['lux_sens01_signal']
        Vs3 = sensor_data['lux_sens02_signal']
        Vs2 = sensor_data['rgb_sens_signal']
        
        theta = np.atan((Vs1 - Vs3) / ((A / B) * Vs2 * 2 * np.sin(beta * np.pi/180)))
        thetas_i.append(180/np.pi * theta)
        Vs1_i = Vs1 / np.cos(theta + beta * np.pi/180)
        Vs3_i = Vs3 / np.cos(theta - beta * np.pi/180)
        n = 1.088025599603545
        Vs2_i = Vs2 / np.cos(theta) ** n
        
        if Vs1_i < 0 or Vs2_i < 0 or Vs3_i < 0:
            distance_lux = 0
            distance_rgb = 0
        else:
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
for i in range(len(thetas)):
    print(f"{distances[i]}\t\t|\t" + "\t|\t".join([str(round(j[i],4)) for j in thetas]))
