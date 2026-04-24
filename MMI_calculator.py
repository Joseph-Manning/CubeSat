# -*- coding: utf-8 -*-
"""
Created on Mon Apr 20 11:20:51 2026

@author: wildb
"""
#IMPORTS===================================================================
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import math
#==========================================================================
#GLOBAL VARIABLES==========================================================
filepath = r"C:\Users\wildb\Main_OFFLINE\UNI_OF_SOTON\Year_2\SESA2030\misc\hairybollocks.csv"
DATA = {} #dictionary might be excessive but is good practice
torque = 42 #Nm

#==========================================================================
#FUNCTIONS=================================================================
def CSV(filepath):
    df = pd.read_csv(filepath,sep=',') #glob.glob(f"{filepath}/*.csv")
    
    position_values = df['p'].values
    time_values = df['t'].values
    DATA["data"] = {"position": position_values,"time": time_values}
    return print("data seperated m'lord")

def plotter():#made it a function so it only has to be a single line of code
    data = DATA["data"]
    calc = DATA["calc"]
    list_acc = []
    list_v = []
    for i in range(len(data["time"])-2):
        list_acc.append(data["time"][i])
    for i in range(len(data["time"])-1):
        list_v.append(data["time"][i])
    #print(len(list_v))
    #print(len(list_acc))
    fig1 =  plt.figure()
    plt.plot(data["time"],data["position"])
    plt.xlabel("time / ms")
    plt.ylabel("position /rad")
    fig2 =  plt.figure()
    plt.plot(list_v,calc["rad/s"])
    plt.xlabel("time / ms")
    plt.ylabel("angular velocity /rads^-1")
    fig2 =  plt.figure()
    plt.plot(list_acc,calc["alpha"])
    plt.xlabel("time / ms")
    plt.ylabel("angular /rads^-2")
    plt.show()
    return print("plottin' done m'lord")

def calc():
    data = DATA["data"]
    temp = []
    for i in range(1,len(DATA["data"]["time"])):
        dt = data["time"][i] - data["time"][i-1]
        temp.append(dt)
        
    av_dt = sum(temp) / len(temp)
    n = 2
    p = data["position"]
    a = np.diff(p,n) / av_dt
    v = np.diff(p,(n-1)) / av_dt
    DATA["calc"] = {"alpha": a,"rad/s": v,"dt": av_dt}
    return print("acceleration found m'lord")
    
def MMI(torque):
    alpha = DATA["calc"]["alpha"]
    av_alpha = sum(alpha) / len(alpha)
    I = torque / av_alpha
    print("mass moment of inertia (kgm^2):", I)
    return I
    
def done(filepath,torque):
    CSV(filepath)
    CSV(filepath) 
    calc()
    plotter()
    MMI(torque)
    return print("all done m'lord")
#==========================================================================
#RUNNING===================================================================
done(filepath,torque)
#==========================================================================