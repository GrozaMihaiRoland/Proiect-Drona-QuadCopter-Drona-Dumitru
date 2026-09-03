#include "ElementStorage.h"
#include <BasicLinearAlgebra.h>
using namespace BLA;

//videoclip de la carbon aeronautics

/*
state variables:
x - state vector (2,1)
P - state covariance (2,2)

Matrix params:
F - system matrix (2,2)
B - input matrix (2,1)
Q - process noise covariance matrix (2,2)
H - ouptut matrix (1,2)
R - observation noise (1,1)
S - innovation covariance (2,2)
K - kalman gain 
*/


class Filter2D {
  private: 
    BLA::Matrix<2,2,float> P;
    BLA::Matrix<1,1,float> u;
    BLA::Matrix<2,1,float> x;
    BLA::Matrix<1,1,float> z;
    BLA::Matrix<1,1,float> y;

  public:
    Filter2D();

    void get_command(float acc);
    void get_measurement(float alt);

    void estimate(BLA::Matrix<2,2,float> F, BLA::Matrix<2,1,float> B, BLA::Matrix<2,2,float> Q);
    void update(BLA::Matrix<1,2,float> H, BLA::Matrix<1,1,float> R);

    BLA::Matrix<2,1,float> get_state();
};