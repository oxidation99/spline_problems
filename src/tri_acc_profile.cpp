#include <ros/ros.h>
#include <std_msgs/Float32.h>
#include <eigen_conversions/eigen_kdl.h>
#include <spline_problems/matplotlibcpp.h>

double j_max = 5;
double v_max = 1;
double period = 0.001;
double qi = 0.0, qdi = .4, qddi = 0.0;  
double qf = 2.0;

namespace plt = matplotlibcpp;

void curves_plot(std::vector<double>& t_vect, std::vector<double>& q_vect, std::vector<double>& v_vect, std::vector<double>& a_vect, std::vector<double>& j_vect, double T);

int main(int argc, char** argv){
  ros::init(argc, argv, "tri_acc_profile");
  ros::NodeHandle nh;
  

  
  double T = 2*std::sqrt((v_max  -qdi)/j_max);
  
  std::vector<double> t_vect, q_vect, v_vect, a_vect, j_vect;
  std::vector<double> t_ligne, value_ligne;
  
  // RISE CONCAVE
  for(double i = 0; i<=T/2.0; i +=period)
    t_vect.push_back(i);
  
  int nb_rise_concave = t_vect.size();
  for(int i=0; i<nb_rise_concave;i++){
    j_vect.push_back(j_max);
    a_vect.push_back(j_max*t_vect[i]);
    v_vect.push_back(qdi+j_max / 2.0 * t_vect[i]* t_vect[i]);
    q_vect.push_back(qi+qdi*t_vect[i]+j_max/6.0*t_vect[i]*t_vect[i]*t_vect[i]);
  }
  
  // RISE CONVEXE
  double th = t_vect[t_vect.size()-1];
  double am = a_vect[t_vect.size()-1];
  double vh = v_vect[t_vect.size()-1];
  double sh = q_vect[t_vect.size()-1];
  for(double i = T/2.0+period; i<=T; i +=period)
    t_vect.push_back(i);
  
  int nb_rise_convexe = t_vect.size();
  
  double current_t;
  for(int i=nb_rise_concave; i<nb_rise_convexe;i++){
    current_t = t_vect[i] - th;
    j_vect.push_back(-j_max);
    a_vect.push_back(am - j_max*current_t);
    v_vect.push_back(vh+am*current_t - j_max*current_t*current_t/2.0);
    q_vect.push_back(sh+vh*current_t+am*current_t*current_t/2.0-j_max/6.0*current_t*current_t*current_t);
  }
  
  // CRUISE
  double ts = t_vect[t_vect.size()-1];
  double ss = q_vect[t_vect.size()-1];
  double time_left = (qf - ss)/v_max;
  for(double i = T+period; i<=T+time_left; i +=period)
    t_vect.push_back(i);
  
  int nb_cruise = t_vect.size();
  
  for(int i=nb_rise_convexe; i<nb_cruise;i++){
    current_t = t_vect[i] - ts;
    j_vect.push_back(0.0);
    a_vect.push_back(0);
    v_vect.push_back(v_max);
    q_vect.push_back(ss+v_max*current_t);
  }
  
  // Plot
  curves_plot(t_vect, q_vect, v_vect, a_vect, j_vect, T);

  ros::shutdown();
  return 1;
}

void curves_plot(std::vector<double>& t_vect, std::vector<double>& q_vect, std::vector<double>& v_vect, std::vector<double>& a_vect, std::vector<double>& j_vect, double T){
  
  std::vector<double> line_t, line_val;
  
  // Plot position
  plt::subplot(2,2,1);
  plt::plot(t_vect, q_vect);
  plt::ylabel("position");
  line_t.clear();
  line_t.push_back(T/2.0);
  line_t.push_back(T/2.0);
  line_val.clear();
  line_val.push_back(0);
  line_val.push_back(qf);
  plt::plot(line_t, line_val,"r--");
  line_t.clear();
  line_t.push_back(T);
  line_t.push_back(T);
  line_val.clear();
  line_val.push_back(0);
  line_val.push_back(qf);
  plt::plot(line_t, line_val,"r--");
  
  // Plot velocity
  plt::subplot(2,2,2);
  plt::plot(t_vect, v_vect);
  plt::ylabel("velocity");
  line_t.clear();
  line_t.push_back(T/2.0);
  line_t.push_back(T/2.0);
  line_val.clear();
  line_val.push_back(0);
  line_val.push_back(v_max);
  plt::plot(line_t, line_val,"r--");
  line_t.clear();
  line_t.push_back(T);
  line_t.push_back(T);
  line_val.clear();
  line_val.push_back(0);
  line_val.push_back(v_max);
  plt::plot(line_t, line_val,"r--");
  
  // Plot acceleration
  plt::subplot(2,2,3);
  plt::plot(t_vect, a_vect);
  plt::ylabel("acceleration");
  line_t.clear();
  line_t.push_back(T/2.0);
  line_t.push_back(T/2.0);
  line_val.clear();
  line_val.push_back(0);
  line_val.push_back(j_max*T/2.0);
  plt::plot(line_t, line_val,"r--");
  line_t.clear();
  line_t.push_back(T);
  line_t.push_back(T);
  line_val.clear();
  line_val.push_back(0);
  line_val.push_back(j_max*T/2.0);
  plt::plot(line_t, line_val,"r--");
  
  // Plot jerk
  plt::subplot(2,2,4);
  plt::plot(t_vect, j_vect);
  plt::ylabel("jerk");
  line_t.clear();
  line_t.push_back(T/2.0);
  line_t.push_back(T/2.0);
  line_val.clear();
  line_val.push_back(-j_max);
  line_val.push_back(j_max);
  plt::plot(line_t, line_val,"r--");
  line_t.clear();
  line_t.push_back(T);
  line_t.push_back(T);
  line_val.clear();
  line_val.push_back(-j_max);
  line_val.push_back(j_max);
  plt::plot(line_t, line_val,"r--");
  
  plt::show();
  
}