%% inertia in realtion to center of gravity

%% Still missing the ship hull

mall = 13; % kg
m1 = 0.500;
m2 = 0.750;
m3 = 1.060;
m4 = 0.650;
m5 = 0.800;
mbatt = 0.360;
mrest = mall-m1-m2-m3-m4-m5-6*mbatt;
x1 = 0.490;
y1 = 0;
z1 = -1*-0.05;
x2 = 0.290;
y2 = 0;
z2 = z1;
x3 = 0.100;
y3 = 0;
z3 = z1;
x4 = 0.095;
y4 = 0;
z4 = z1;
x5 = 0.300;
y5 = 0;
z5 = z1;

% Battery positions
xb1 = 0.3;
xb2 = 0.1;
xb3 = -1*-0.3;
xb4 = -1*-0.3;
xb5 = 0.1;
xb6 = 0.3;
yb = 0.025;
zb = 0.020;

% Battery dimensions
bx = 0.135;
by = 0.025;
bz = 0.045;

% Dimensions for the weights
wx = 0.10;
wy = 0.10;
wz = 0.01;

% Dimensions for hull approximation and the rest
hx = 1.05;
hy = 0.25;
hz = 0.15;

% Inertia Calculations
Ix1 = 1/12 * m1 * (wz^2 + wy^2);
Iy1 = 1/12 * m1 * (wz^2 + wx^2);
Iz1 = 1/12 * m1 * (wy^2 + wx^2);

Ix2 = 1/12 * m2 * (wz^2 + wy^2);
Iy2 = 1/12 * m2 * (wz^2 + wx^2);
Iz2 = 1/12 * m2 * (wy^2 + wx^2);

Ix3 = 1/12 * m3 * (wz^2 + wy^2);
Iy3 = 1/12 * m3 * (wz^2 + wx^2);
Iz3 = 1/12 * m3 * (wy^2 + wx^2);

Ix4 = 1/12 * m4 * (wz^2 + wy^2);
Iy4 = 1/12 * m4 * (wz^2 + wx^2);
Iz4 = 1/12 * m4 * (wy^2 + wx^2);

Ix5 = 1/12 * m5 * (wz^2 + wy^2);
Iy5 = 1/12 * m5 * (wz^2 + wx^2);
Iz5 = 1/12 * m5 * (wy^2 + wx^2);

Ixbatt = 1/12 * mbatt * (bz^2 + by^2);
Iybatt = 1/12 * mbatt * (bz^2 + bx^2);
Izbatt = 1/12 * mbatt * (by^2 + bx^2);

Ixrest = 1/12 * mrest * (hz^2 + hy^2);
Iyrest = 1/12 * mrest * (hz^2 + hx^2);
Izrest = 1/12 * mrest * (hy^2 + hx^2);

Ixx = Ix1 + m1*(y1^2+z1^2) ...
    + Ix2 + m2*(y2^2+z2^2) ...
    + Ix3 + m3*(y3^2+z3^2) ...
    + Ix4 + m4*(y4^2+z4^2) ...
    + Ix5 + m5*(y5^2+z5^2) ...
    + Ixbatt + mbatt*(yb^2+zb^2) ...
    + Ixbatt + mbatt*(yb^2+zb^2) ...
    + Ixbatt + mbatt*(yb^2+zb^2) ...
    + Ixbatt + mbatt*(yb^2+zb^2) ...
    + Ixbatt + mbatt*(yb^2+zb^2) ...
    + Ixbatt + mbatt*(yb^2+zb^2) ...
    + Ixrest;

Iyy = Iy1 + m1*(x1^2+z1^2) ...
    + Iy2 + m2*(x2^2+z2^2) ...
    + Iy3 + m3*(x3^2+z3^2) ...
    + Iy4 + m4*(x4^2+z4^2) ...
    + Iy5 + m5*(x5^2+z5^2) ...
    + Iybatt + mbatt*(xb1^2+zb^2) ...
    + Iybatt + mbatt*(xb2^2+zb^2) ...
    + Iybatt + mbatt*(xb3^2+zb^2) ...
    + Iybatt + mbatt*(xb4^2+zb^2) ...
    + Iybatt + mbatt*(xb5^2+zb^2) ...
    + Iybatt + mbatt*(xb6^2+zb^2) ...
    + Iyrest;

Izz = Iz1 + m1*(x1^2+y1^2) ...
    + Iz2 + m2*(x2^2+y2^2) ...
    + Iz3 + m3*(x3^2+y3^2) ...
    + Iz4 + m4*(x4^2+y4^2) ...
    + Iz5 + m5*(x5^2+y5^2) ...
    + Izbatt + mbatt*(xb1^2+yb^2) ...
    + Izbatt + mbatt*(xb2^2+yb^2) ...
    + Izbatt + mbatt*(xb3^2+yb^2) ...
    + Izbatt + mbatt*(xb4^2+yb^2) ...
    + Izbatt + mbatt*(xb5^2+yb^2) ...
    + Izbatt + mbatt*(xb6^2+yb^2) ...
    + Izrest;

Ixy = 0 + m1*x1*y1 ...
    + 0 + m2*x2*y2 ...
    + 0 + m3*x3*y3 ...
    + 0 + m4*x4*y4 ...
    + 0 + m5*x5*y5 ...
    + 0 + mbatt*xb1*yb ...
    + 0 + mbatt*xb2*yb ...
    + 0 + mbatt*xb3*yb ...
    + 0 + mbatt*xb4*yb ...
    + 0 + mbatt*xb5*yb ...
    + 0 + mbatt*xb6*yb;

Ixz = 0 + m1*x1*z1 ...
    + 0 + m2*x2*z2 ...
    + 0 + m3*x3*z3 ...
    + 0 + m4*x4*z4 ...
    + 0 + m5*x5*z5 ...
    + 0 + mbatt*xb1*zb ...
    + 0 + mbatt*xb2*zb ...
    + 0 + mbatt*xb3*zb ...
    + 0 + mbatt*xb4*zb ...
    + 0 + mbatt*xb5*zb ...
    + 0 + mbatt*xb6*zb;
    
Iyz = 0 + m1*y1*z1 ...
    + 0 + m2*y2*z2 ...
    + 0 + m3*y3*z3 ...
    + 0 + m4*y4*z4 ...
    + 0 + m5*y5*z5 ...
    + 0 + mbatt*yb*zb ...
    + 0 + mbatt*yb*zb ...
    + 0 + mbatt*yb*zb ...
    + 0 + mbatt*yb*zb ...
    + 0 + mbatt*yb*zb ...
    + 0 + mbatt*yb*zb;
    

I = [Ixx -Ixy -Ixz;
     -Ixy Iyy -Iyz;
     -Ixz -Iyz Izz]
 
save('inertia.mat','I')
