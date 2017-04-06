#include "ros/ros.h"
#include "aauship_control/Attitude.h"
#include "aauship_control/ADIS16405.h"
#include "aauship_control/LLIinput.h"

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>

//#include <aauship_control/ekf.h>
//#include <aauship_control/ukf.h>
#include <sys/time.h>
#include <std_msgs/Float32.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

//Constants
#define KF_ATTITUDE_RATE 20.0
#define N_STATES 9
#define N_INPUTS 2
#define N_MEAS 6
#define TS 0.05
//System constants
#define DROLL 0.1094 
#define DPITCH 7.0203
#define DYAW 0.26285
#define IX 0.06541
#define IY 1.08921
#define IZ 1.10675 
#define L1 0.05
#define L2 0.05
//State variances
#define SIGMA2_ROLL 1
#define SIGMA2_PITCH 1
#define SIGMA2_YAW 1
#define SIGMA2_ROLLD 1
#define SIGMA2_PITCHD 1
#define SIGMA2_YAWD 1
#define SIGMA2_ROLLDD 1
#define SIGMA2_PITCHDD 1
#define SIGMA2_YAWDD 1
//Sensor variances
#define SIGMA2_ACCROLL 1
#define SIGMA2_ACCPITCH 1
#define SIGMA2_MAGYAW 1
#define SIGMA2_GYROX 1
#define SIGMA2_GYROY 1
#define SIGMA2_GYROZ 1


//Global variables
float meas[N_MEAS] = {0,0,0,0,0,0};
float states[N_STATES] = {0,0,0,0,0,0,0,0,0};
float inputs[N_INPUTS] = {0,0};


//Callback functions
void imu_callback(const aauship_control::ADIS16405::ConstPtr& sensor)
{
	float acc[3] = {sensor->xaccl,0,0};
	float gyro[3] = {0,0,0};
	float mag[3] = {0,0,0};
	float magxh = 0;
	float magyh = 0;

	//Calculate roll and pitch from the accelerometer measurements
	//Roll = atan(yacc / sqrt(xacc^2 + zacc^2)) and pitch = atan(xacc / sqrt(yacc^2 + zacc^2))
	meas[0] = atan((sensor->yaccl)/sqrt((sensor->xaccl)*(sensor->xaccl)+(sensor->zaccl)*(sensor->zaccl)));
	meas[1] = atan((sensor->xaccl)/sqrt((sensor->yaccl)*(sensor->yaccl)+(sensor->zaccl)*(sensor->zaccl)));

	//Calculate yaw using the meagnetometer data
	magxh = (sensor->xmagn) * cos(states[1]) + (sensor->ymagn) * sin(states[0]) * sin(states[1]) + (sensor->zmagn) * cos(states[0]) * sin(states[1]);
	magyh = (sensor->ymagn) * cos(states[0]) + (sensor->zmagn) * sin(states[0]);
	meas[2] = atan(magyh/magxh);

	//Store the gyro measurements
	meas[3] = sensor->xgyro;
	meas[4] = sensor->ygyro;
	meas[5] = sensor->zgyro;
}

void lli_callback(const aauship_control::LLIinput::ConstPtr& commands)
{

}


//Matrix multiplication function
void matrix_multiplication(gsl_matrix * a,gsl_matrix * b,gsl_matrix * result)
{
	int n = a->size1;	//Rows of a
	int m = a->size2;	//Columns of a a and rows of b
	int p = b->size2;	//Rows of b
	float mult = 0;

	for (int i = 0; i < n; i++)
	{
		for(int j = 0; j < p; j++)
		{
			mult = 0;
			for(int k = 0; k < m; k++)
				mult = mult + gsl_matrix_get(a,i,k) * gsl_matrix_get(b,j,k);
			gsl_matrix_set(result,i,j,mult);
		}

	}
}

//Matrix vector multiplication function
void matrix_vector_multiplication(gsl_matrix * a, float * b, float * result)
{
	int n = a->size1;	//Rows of a
	int m = a->size2;	//Columns of a
	float mult = 0;

	for (int i = 0; i < n; i++)
	{
		mult = 0;
		for(int j = 0; j < m; j++)
			mult = mult + gsl_matrix_get(a,i,j) * b[j];
		result[i] = mult;
	}
}

//Main function
int main(int argc, char **argv)
{
	ros::init(argc,argv,"KF_attitude_node");
	ros::NodeHandle n;
	ros::Subscriber imu_update = n.subscribe("/imu",1000,imu_callback);
	ros::Subscriber lli_update = n.subscribe("/lli_input",1000,lli_callback);
	ros::Publisher att_pub = n.advertise<aauship_control::Attitude>("/kf_attitude", 1);
	ros::Rate KF_attitude_rate(KF_ATTITUDE_RATE);
	std::cout<<std::endl<<"######ATTITUDE KF RUNNING######"<<std::endl;

	//Temporary matrices
	gsl_matrix * TEMP_9x9 = gsl_matrix_alloc(N_STATES,N_STATES);
	gsl_matrix * TEMP_6x9 = gsl_matrix_alloc(N_MEAS,N_STATES);
	gsl_matrix * TEMP_9x6 = gsl_matrix_alloc(N_STATES,N_MEAS);
	gsl_matrix * TEMP_6x6 = gsl_matrix_alloc(N_MEAS,N_MEAS);
	//Identity matrix
	gsl_matrix * I_9x9 = gsl_matrix_alloc(N_STATES,N_STATES);
	gsl_matrix_set_identity(I_9x9);  

	//////System matrices//////
	float Ainit[N_STATES][N_STATES] = {
		{1,0,0,TS,0,0,0,0,0},
		{0,1,0,0,TS,0,0,0,0},
		{0,0,1,0,0,TS,0,0,0},
		{0,0,0,1,0,0,TS,0,0},
		{0,0,0,0,1,0,0,TS,0},
		{0,0,0,0,0,1,0,0,TS},
		{0,0,0,-DROLL/IX,0,0,-TS*DROLL/IX,0,0},
		{0,0,0,0,-DPITCH/IY,0,0,-TS*DPITCH/IY,0},
		{0,0,0,0,0,-DYAW/IZ,0,0,-TS*DYAW/IZ}
	};
	float Binit[N_STATES][N_INPUTS] = {
		{0,0},
		{0,0},
		{0,0},
		{0,0},
		{0,0},
		{0,0},
		{0,0},
		{0,0},
		{L1/IZ,-L2/IZ},
	};
	float Cinit[N_MEAS][N_STATES] = {
		{1,0,0,0,0,0,0,0,0},
		{0,1,0,0,0,0,0,0,0},
		{0,0,1,0,0,0,0,0,0},
		{0,0,0,1,0,0,0,0,0},
		{0,0,0,0,1,0,0,0,0},
		{0,0,0,0,0,1,0,0,0}
	};
	//Store the matrices in gsl form
	gsl_matrix * A = gsl_matrix_alloc(N_STATES,N_STATES);
	gsl_matrix * Atrans = gsl_matrix_alloc(N_STATES,N_STATES);
	gsl_matrix * B = gsl_matrix_alloc(N_STATES,N_INPUTS);
	gsl_matrix * C = gsl_matrix_alloc(N_MEAS,N_STATES);
	gsl_matrix * Ctrans = gsl_matrix_alloc(N_STATES,N_MEAS);
	for (int i = 0; i < N_STATES; i++)    //Use the same loop for the three matrices
	{
		for (int j = 0; j < N_STATES; j++)
			gsl_matrix_set(A,i,j,Ainit[i][j]);
		for (int k = 0; k < N_INPUTS; k++)
			gsl_matrix_set(B,i,k,Binit[i][k]);
		for (int l = 0; l < N_MEAS; l++)
			gsl_matrix_set(C,l,i,Cinit[l][i]);
	}
	gsl_matrix_transpose_memcpy(Atrans,A);
	gsl_matrix_transpose_memcpy(Ctrans,C);

	//////Kalman filter matrices initialization//////
	gsl_matrix * P = gsl_matrix_alloc(N_STATES,N_STATES);  //Initialize as identity
	gsl_matrix_set_identity(P);  
	gsl_matrix * K = gsl_matrix_calloc(N_STATES,N_MEAS);    //Initialize as zeros

	//////Weight matrices//////
	float Qinit[N_STATES] = {SIGMA2_ROLL,SIGMA2_PITCH,SIGMA2_YAW,SIGMA2_ROLLD,SIGMA2_PITCHD,SIGMA2_YAWD,SIGMA2_ROLLDD,SIGMA2_PITCHDD,SIGMA2_YAWDD};
	float Rinit[N_MEAS] = {SIGMA2_ACCROLL,SIGMA2_ACCPITCH,SIGMA2_MAGYAW,SIGMA2_GYROX,SIGMA2_GYROY,SIGMA2_GYROZ};
	//Store the matrices in gsl form
	gsl_matrix * Q = gsl_matrix_calloc(N_STATES,N_STATES);
	gsl_matrix * R = gsl_matrix_calloc(N_MEAS,N_MEAS);
	for (int i = 0; i < N_STATES; i++)   
		gsl_matrix_set(Q,i,i,Qinit[i]);
	for (int i = 0; i < N_MEAS; i++)
		gsl_matrix_set(R,i,i,Rinit[i]);

	//////First prediction//////
	//Predict states  as states = A * states + B * inputs
	float temp_states[N_STATES];
	float temp_states2[N_STATES];
	float temp_meas[N_MEAS];
	matrix_vector_multiplication(A, states, temp_states);
	matrix_vector_multiplication(B, inputs, temp_states2);
	for(int i = 0 ; i < N_STATES ; i++)
		states[i] = temp_states[i] + temp_states2[i];
	//Calculate new P as A*P*A'+Q
	matrix_multiplication(A,P,TEMP_9x9);
	matrix_multiplication(TEMP_9x9,Atrans,P);
	gsl_matrix_add (P,Q);

	//Debug
	//gsl_linalg_cholesky_decomp(Ptemp);
	//gsl_linalg_cholesky_invert (Ptemp);
	//std::cout<<gsl_matrix_get(Ptemp,0,0)<<" "<<gsl_matrix_get(Ptemp,0,1)<<" "<<std::endl;
	//std::cout<<gsl_matrix_get(Ptemp,1,0)<<" "<<gsl_matrix_get(Ptemp,1,1)<<" "<<std::endl;
	// std::cout<<" "<<states[0]<<" "<<std::endl;
	// std::cout<<" "<<states[1]<<" "<<std::endl;

	while(ros::ok)
	{	
		ros::spinOnce();

		//////Update step//////
		//Calculate K as P*C'/(C*P*C'+R)
		matrix_multiplication(C,P,TEMP_6x9);
		matrix_multiplication(TEMP_6x9,Ctrans,TEMP_6x6);
		gsl_matrix_add (TEMP_6x6,R);
		gsl_linalg_cholesky_decomp(TEMP_6x6);
		gsl_linalg_cholesky_invert (TEMP_6x6);	
		matrix_multiplication(Ctrans,TEMP_6x6,TEMP_9x6);
		matrix_multiplication(P,TEMP_9x6,K);
		//Correct the states as states = states + K * (meas - C * states)
		matrix_vector_multiplication(C, states, temp_meas);
		for(int i = 0 ; i < N_STATES ; i++)
			temp_meas[i] = meas[i] - temp_meas[i];
		matrix_vector_multiplication(K,temp_meas,temp_states);
		for(int i = 0 ; i < N_STATES ; i++)
			states[i] = states[i] + temp_states[i];
		//Update P as (I - K * C) * P
		matrix_multiplication(K,C,TEMP_9x9);
		gsl_matrix_scale (TEMP_9x9, -1.0);
		gsl_matrix_add(TEMP_9x9,I_9x9);
		matrix_multiplication(TEMP_9x9,P,P);

		//////Prediction step//////
		//Predict states as states = A * states + B * inputs
		matrix_vector_multiplication(A, states, temp_states);
		matrix_vector_multiplication(B, inputs, temp_states2);
		for(int i = 0 ; i < N_STATES ; i++)
			states[i] = temp_states[i] + temp_states2[i];
		//Calculate new P as A*P*A'+Q
		matrix_multiplication(A,P,TEMP_9x9);
		matrix_multiplication(TEMP_9x9,Atrans,P);
		gsl_matrix_add (P,Q); 	

		KF_attitude_rate.sleep();
	}

	return 0;
}