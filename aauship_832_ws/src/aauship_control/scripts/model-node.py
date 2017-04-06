#!/usr/bin/env python2
# Simulation model

import roslib; roslib.load_manifest('aauship_control')

import rospy
from std_msgs.msg import String
from aauship_control.msg import *
from std_msgs.msg import Float64MultiArray, Header
import tf
# import scipy.io as sio
# import kalmanfilterfoo as kfoo # temporary implementation for the simulation
# import gpsfunctions as geo
import numpy as np
# from math import pi, sqrt, atan2, acos, sin, fmod, cos
# import scipy.linalg as linalg
import time
import os

class modelSim(object):
    """docstring for modelSim"""
    def __init__(self):
        self.n = 3; # number of states
        self.m = 2; # number of inputs, also outputs
        # init ss matrices
        self.A = np.matrix([[ 1, 0.0497, 0],
            [ 0, 0.9882, 0],
            [ 0, 0, 0.9891]])
        self.B = np.matrix([[0.0001, -0.0001],
            [0.0022, -0.0022],
            [0.0038, 0.0038]])
        # init states
        self.x = np.zeros([self.n])
        self.x_old = self.x
        self.sub = rospy.Subscriber('lli_input', LLIinput, self.llicb,queue_size=1000)
        self.pub = rospy.Publisher('kf_statesnew', KFStates, queue_size=1000)
        self.pubmsg = KFStates()

        # variables to convert PWM2force
        self.m = 0.26565
        self.c = 24.835


        # Variables for the thrusters
        self.leftthruster = 0.0
        self.rightthruster = 0.0

        rospy.init_node('kalmanfilter_node')
        rate = rospy.Rate(20)
        before = 0;
        while not rospy.is_shutdown():
            # print after - before
            # Calculates new states
            #print self.x
            for i in range(0,self.n):
                # Check this implementation in LQR-node
                self.x_old = self.x
                self.x[i] = (self.A[i,0]*self.x_old[0] + self.A[i,1]*self.x_old[1] + self.A[i,2]*self.x_old[2]) \
                    + (self.B[i,0]*self.leftthruster + self.B[i,1]*self.rightthruster)
            # self.pubmsg.r = rospy.get_time()
            self.pub.publish(self.pubmsg)
            print self.x[2]
            rate.sleep()
            # before = after

    def llicb(self, data):
        # Get inputs from controller
        if data.MsgID == 3:
            if self.rightthruster<100:
                self.rightthruster = 0
            self.rightthruster = (float(data.Data)*self.m)-self.c


        if data.MsgID == 5:
            if self.leftthruster<100:
                self.leftthruster = 0
            self.leftthruster = (float(data.Data)*self.m)-self.c
        #print "Data: ", float(data.Data)
        #print "Left Thruster: ", self.leftthruster, " Right Thruster: ", self.rightthruster
        #print self.leftthruster
        #print self.rightthruster

        # Limits for motors
        # if self.rightthruster > 40:
        #     self.rightthruster = 40
        # elif self.rightthruster < -25:
        #     self.rightthruster = -25

        # if self.leftthruster > 40:
        #     self.leftthruster = 40
        # elif self.leftthruster < -25:
        #     self.leftthruster = -25

        self.pubmsg.psi = self.x[0]
        self.pubmsg.p = self.x[1]
        self.pubmsg.u = self.x[2]
        #print "Yaw: ", self.x[0]," YawDot: ", self.x[1], " Xdot: ", self.x[2]
        #print self.x[2]
        #self.pub.publish(self.pubmsg)

    def run(self):
        #cself.sub.subscribe(self.sub)
        rospy.spin()

        print("Exiting model node")
        exit()



if __name__ == '__main__':
    w = modelSim()
    w.run()
