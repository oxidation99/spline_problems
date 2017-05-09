#include <spline_problems/adaptive_s_curve_profile.hpp>

AdaptiveSCurveProfile::AdaptiveSCurveProfile ( double s_init, double vi_init, double a_init, double s_final, double v_final, double a_final, double v_max, double a_max, double j_max ) {
  si_ = s_init;
  sf_ = s_final;
  vi_ = vi_init;
  vf_ = v_final;
  ai_ = a_init;
  af_ = a_final;
  v_max_ = v_max;
  a_max_ = a_max;
  j_max_ = j_max;
  period_ = 0.001;
}

AdaptiveSCurveProfile::AdaptiveSCurveProfile() {
  AdaptiveSCurveProfile(0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 1.0, 2.0, 5.0);
}

void AdaptiveSCurveProfile::set_period ( double period ) {
  period_ = period;
}

void AdaptiveSCurveProfile::compute_curves(){
  // Computation of times...
  // ...

  j_vect_.clear();
  j_vect_.push_back(0.0);
  a_vect_.clear();
  a_vect_.push_back(ai_);
  v_vect_.clear();
  v_vect_.push_back(vi_);
  s_vect_.clear();
  s_vect_.push_back(si_);
  t_vect_.clear();
  t_vect_.push_back(0.0);

  // If initial velocity is smaller than max vel
  if(vi_ <= v_max_){
    double t_ramp_rise = compute_ramp_rise_time(vi_, ai_, v_max_);
    double t_concave_rise = compute_concave_time(ai_,a_max_); 
    double t_convexe_rise = compute_convexe_time(a_max_, 0);
    double t_ramp_fall = compute_ramp_fall_time(v_max_, 0.0, vf_);
    double t_convexe_fall = compute_convexe_time(0, -a_max_);
    double t_concave_fall = compute_concave_time(-a_max_,0);
    
    
    double d_cruise = sf_-si_;
    if(t_ramp_rise > 0){
      d_cruise = d_cruise - compute_concave_distance(ai_, vi_, a_max_) - compute_ramp_rise_distance(vi_+ai_*t_concave_rise+j_max_/2*t_concave_rise*t_concave_rise,vi_, v_max_, ai_) - compute_convexe_distance(a_max_, vi_+ai_*t_concave_rise+j_max_/2*t_concave_rise*t_concave_rise+t_ramp_rise*a_max_, 0);
    }
    else{
      std::cerr << "ramp rise time is negative" << std::endl;
      d_cruise = d_cruise - compute_concave_distance(0,0,0) - compute_convexe_distance(0,0,0);
    }
    if(t_ramp_fall > 0){
     d_cruise = d_cruise - compute_convexe_distance(0, v_max_, -a_max_) - compute_ramp_fall_distance(v_max_-j_max_/2*t_convexe_fall*t_convexe_fall,v_max_,vf_,0.0) - compute_concave_distance(-a_max_,v_max_-j_max_/2*t_convexe_fall*t_convexe_fall-t_ramp_fall*a_max_, 0) ;
    }
    else{
      std::cerr << "ramp fall time is negative" << std::endl;
     d_cruise =  d_cruise - compute_convexe_distance(0,0,0) - compute_concave_distance(0,0,0);
    }
    double t_cruise = d_cruise/v_max_;
    
    if(t_cruise>0){
      // Concave rise
      compute_next_phase(t_concave_rise, j_max_);
      
      // Ramp rise
      if (t_ramp_rise >0.0)
        compute_next_phase(t_ramp_rise, 0.0);
      
      // Convexe rise
      compute_next_phase(t_convexe_rise, -j_max_);
        
      // Cruise
      compute_next_phase(t_cruise, 0.0);
      
      // Convexe fall
      compute_next_phase(t_convexe_fall,-j_max_);
      
      // Ramp fall
      if (t_ramp_fall >0.0){
        compute_next_phase(t_ramp_fall,0.0);
      }
      
      // Concave fall
      compute_next_phase(t_concave_fall,j_max_);
    }
    else{
      std::cerr << "cruise time is negative" << std::endl;
    }
    
    
    
    
    std::cout<<"Concave rise time : " << t_concave_rise<<std::endl;
    std::cout<<"Ramp rise time : " << t_ramp_rise<<std::endl;
    std::cout<<"Convexe rise time : " << t_convexe_rise<<std::endl;
    std::cout<<"Cruise time : " << t_cruise<<std::endl;
    std::cout<<"Convexe fall time : " << t_convexe_fall<<std::endl;
    std::cout<<"Ramp fall time : " << t_ramp_fall<<std::endl;
    std::cout<<"Concave fall time : " << t_concave_fall<<std::endl;
  }
  else{
    return;
  }
}

void AdaptiveSCurveProfile::compute_next_phase(double time, double j){
  
  double last_a = a_vect_[a_vect_.size()-1];
  double last_v = v_vect_[v_vect_.size()-1];
  double last_s = s_vect_[s_vect_.size()-1];
  double last_t = t_vect_[s_vect_.size()-1];
  
  for(double t=period_; t<=time; t+=period_){
    j_vect_.push_back(j);
    a_vect_.push_back(last_a + j*t);
    v_vect_.push_back(last_v + last_a*t + j/2.0 * t*t);
    s_vect_.push_back(last_s + last_v*t + last_a/2.0*t*t + j/6.0*t*t*t);
    t_vect_.push_back(last_t+t);
  }
}

double AdaptiveSCurveProfile::compute_phase_distance(double time_in_phase, double j_phase, double phase_acc_start, double phase_vel_start){
  return phase_vel_start*time_in_phase+phase_acc_start/2.0*time_in_phase*time_in_phase+j_phase/6.0*time_in_phase*time_in_phase*time_in_phase;
}

double AdaptiveSCurveProfile::compute_concave_distance(double phase_acc_start, double phase_vel_start, double phase_acc_final){
  return compute_phase_distance(compute_concave_time(phase_acc_start, phase_acc_final), j_max_, phase_acc_start, phase_vel_start);
}

double AdaptiveSCurveProfile::compute_concave_time(double phase_acc_start, double phase_acc_final){
  return (phase_acc_final-phase_acc_start)/j_max_;
}

double AdaptiveSCurveProfile::compute_convexe_distance(double phase_acc_start, double phase_vel_start, double phase_acc_final){
  return compute_phase_distance(compute_convexe_time(phase_acc_start, phase_acc_final), -j_max_, phase_acc_start, phase_vel_start);
}

double AdaptiveSCurveProfile::compute_convexe_time(double phase_acc_start, double phase_acc_final){
  return (phase_acc_start-phase_acc_final)/j_max_;
}

double AdaptiveSCurveProfile::compute_ramp_rise_distance(double phase_vel_start, double rise_vel_start, double rise_vel_final, double rise_acc_start){
  return compute_phase_distance(compute_ramp_rise_time(rise_vel_start, rise_acc_start, rise_vel_final), 0 , a_max_, phase_vel_start);
}

double AdaptiveSCurveProfile::compute_ramp_rise_time(double rise_vel_start, double rise_acc_start, double rise_vel_final){
  return ((rise_vel_final-rise_vel_start)-(a_max_*a_max_-rise_acc_start*rise_acc_start/2)/j_max_)/a_max_;
}

double AdaptiveSCurveProfile::compute_ramp_fall_distance(double phase_vel_start, double rise_vel_start, double rise_vel_final, double rise_acc_final){
  return compute_phase_distance(compute_ramp_fall_time(rise_vel_start, rise_acc_final, rise_vel_final), 0 , -a_max_, phase_vel_start);
}

double AdaptiveSCurveProfile::compute_ramp_fall_time(double rise_vel_start, double rise_acc_final, double rise_vel_final){
  return ((rise_vel_start-rise_vel_final)-(a_max_*a_max_-rise_acc_final*rise_acc_final/2)/j_max_)/a_max_;
}

double AdaptiveSCurveProfile::compute_cruise_distance(double cruise_vel, double phase_pos_start, double phase_pos_final){
  return compute_phase_distance(compute_cruise_time(cruise_vel, phase_pos_start, phase_pos_final), 0, 0, cruise_vel);
}

double AdaptiveSCurveProfile::compute_cruise_time(double cruise_vel, double phase_pos_start, double phase_pos_final){
  return (phase_pos_final-phase_pos_start)/cruise_vel;
}




void AdaptiveSCurveProfile::plot_curves() {

  // Plot position
  matplotlibcpp::subplot(2,2,1);
  matplotlibcpp::plot(t_vect_, s_vect_);
  matplotlibcpp::ylabel("position");
  
  // Plot velocity
  matplotlibcpp::subplot(2,2,2);
  matplotlibcpp::plot(t_vect_, v_vect_);
  matplotlibcpp::ylabel("velocity");
  
  // Plot acceleration
  matplotlibcpp::subplot(2,2,3);
  matplotlibcpp::plot(t_vect_, a_vect_);
  matplotlibcpp::ylabel("acceleration");
  
  // Plot jerk
  matplotlibcpp::subplot(2,2,4);
  matplotlibcpp::plot(t_vect_, j_vect_);
  matplotlibcpp::ylabel("jerk");
  
  matplotlibcpp::show();
}

int main(int argc, char** argv){
  ros::init(argc, argv, "adaptive_s_curve_profile");
  ros::NodeHandle nh;
  
  AdaptiveSCurveProfile s_curve(0, -0.5, -2, 1, 0, 0, 1, 3, 50);
  s_curve.set_period(0.00001);
  s_curve.compute_curves();
  s_curve.plot_curves();
  
  ros::shutdown();
  return 1;
}
