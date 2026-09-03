#include "ElementStorage.h"
#include "KalmanFilter.h"

Filter2D::Filter2D() {
  P.Fill(0); 
  x.Fill(0); 
  z.Fill(0);
  y.Fill(0);
}

void Filter2D::get_command(float acc){
  u = {acc};
}

void Filter2D::get_measurement(float alt) {
  z = {alt};
}

void Filter2D::estimate(BLA::Matrix<2,2,float> F, BLA::Matrix<2,1,float> B, BLA::Matrix<2,2,float> Q){
  x = F * x + B * u;
  P = F * P * (~F) + Q;
} 

void Filter2D::update(BLA::Matrix<1,2,float> H, BLA::Matrix<1,1,float> R){
  y = z - H * x;
  BLA::Matrix<1,1,float> S = H * P * (~H) + R;

  //float det = S(0,0)*S(1,1) - S(0,1)*S(1,0);
  //if(abs(det) < 1e-6f) return;
  
  BLA::Matrix<2,1,float> K = P * (~H) * Inverse(S);

  x = x + K * y;
  BLA::Matrix<2,2,float> I = {1, 0, 0, 1};
  P = (I - K * H) * P;
}

BLA::Matrix<2,1,float> Filter2D::get_state(){
  return x;
}