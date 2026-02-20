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

beta = 25 # [deg] angle between each light sensor and centreline

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
lux_sens01_angle = beta     # Lux sensor 1 placement angle in degrees
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
print(theta)

############### testing #################
theta_reals = [0,1,2.5,5,-5,10,-15,20,30,40,50,60,70,80,90]
thetas = []

for i in theta_reals:
    sensor_data = light_sensing_sim.compute_sensor_response(light_source_distance,
                                                            i, lux_sens01_angle,
                                                            lux_sens02_angle, rgb_sens_angle)
    Vs1 = sensor_data['lux_sens01_signal']
    Vs3 = sensor_data['lux_sens02_signal']
    Vs2 = sensor_data['rgb_sens_signal']
    thetas.append(180/np.pi * np.atan((Vs1 - Vs3) / ((A / B) * Vs2 * 2 * np.sin(beta * np.pi/180))))
    
    
print("Theta\t|\tTheta found\t\t\t|\tError")    
for i in range(len(thetas)):
    print(f"{theta_reals[i]}\t\t|\t{thetas[i]}\t|\t{abs(theta_reals[i] - thetas[i])}")
    
plt.figure(1)
plt.polar(0,0,"o",label="S/C")
# plt.polar(theta_real*np.pi/180, d0_real, "o", label="light source")
# plt.polar(theta*np.pi/180, d0_real, "x", label="predicted")

theta_reals = np.array(theta_reals)
thetas = np.array(thetas)

plt.polar(theta_reals * np.pi /180, [5+i for i in range(len(thetas))], "o", label="real")
plt.polar(thetas * np.pi /180, [5+i for i in range(len(thetas))], "x", label="predicted")

plt.legend()

plt.figure(2)

plt.plot(theta_reals, thetas, "o", label="Predicted")
plt.plot([-20,100], [-20, 100], "--")
error = abs((theta_reals - thetas)/(theta_reals+1e-10)) * 100
plt.plot(theta_reals, error, "ro", "Error")
plt.title("error at different angles")
plt.xlabel("theta")
plt.ylabel("predicted theta / error [%]")

plt.figure(3)
plt.plot(lux_angles, lux_sensitivity, "o", label="datasheet values")
ti = np.linspace(-91,91,500)
plt.plot(ti, f(ti,a,b,c), label="fitted cos")
plt.plot(ti, np.cos(ti*np.pi/180), label="cos")
plt.plot(ti[7:-7], abs(f(ti[7:-7], a, b, c) - np.cos(ti[7:-7]*np.pi/180)), label="error")
plt.title("Fitted trig curve vs cos")
plt.xlabel("theta")
plt.ylabel("relative sensitivity")
plt.legend()
# plt.savefig("cos_compared_with_error.png",dpi=300)