/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MM_STRUCT_H
#define MM_STRUCT_H

#include <fstream>

#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonDigitContainer/MmDigit.h"
#include "MuonSimEvent/MMSimHitCollection.h"

#include "AthenaKernel/MsgStreamMember.h"


#include "TLorentzVector.h"
#include "TMath.h"

using namespace std;

//flags
const double crep_pt=200.;
/*
  Potential indexing problems that might slip through the cracks:
  --athena_event/event (hitData_entry)
  --VMM_chip (hitData_entry)
  --road (Finder in MMT_Finder)
*/

//constants and convsersions
const int m_yzdex=2,m_bkdex=13,m_zbardex=2;


template<unsigned char T> class float32fixed
{

private:
  float fixp_content;

public:
  float32fixed(){ fixp_content=0; }
  float32fixed(int value){  fixp_content=value; }
  float32fixed(float value){  fixp_content=value; }
  float32fixed(double value){  fixp_content=value; }
  ~float32fixed(){}




  float getFixed(int S=T) const{

    int nBits = S;
    nBits -= 1;
    //=== get scale
    int   scale  = 0;
    float absVal = std::fabs(fixp_content);
    if(absVal != 0.){
      scale = static_cast<int>( ::truncf(std::log((std::pow(2., static_cast<int>(nBits)) - 1.)/absVal)*(1./std::log(2.))) );
    }
    //=== return input value with fixed point precision
    return ::roundf(fixp_content * std::pow(2., scale)) / std::pow(2., scale);

  }



  // assignment
  float32fixed<T> operator=(float other){ this->fixp_content = other; return *this;  }
  template<unsigned char S>
  float32fixed<T> operator=(float32fixed<S> other){ this->fixp_content = other.getFixed(); return *this;  }

  // Float operators
  float32fixed<T> operator+(float other) const {return float32fixed<T>( this->fixp_content + float32fixed<T>(other).getFixed() );  }
  float32fixed<T> operator-(float other) const {return float32fixed<T>( this->fixp_content - float32fixed<T>(other).getFixed() );  }
  float32fixed<T> operator*(float other) const {return float32fixed<T>( this->fixp_content * float32fixed<T>(other).getFixed() );  }
  float32fixed<T> operator/(float other) const {return float32fixed<T>( this->fixp_content / float32fixed<T>(other).getFixed() );  }
  float32fixed<T> operator+=(float other) { *this = float32fixed<T>( (*this).fixp_content +  float32fixed<T>(other).getFixed() );   return *this;  }
  float32fixed<T> operator-=(float other) { *this = float32fixed<T>( (*this).fixp_content -  float32fixed<T>(other).getFixed() );   return *this;  }
  float32fixed<T> operator*=(float other) { *this = float32fixed<T>( (*this).fixp_content *  float32fixed<T>(other).getFixed() );   return *this;  }
  float32fixed<T> operator/=(float other) { *this = float32fixed<T>( (*this).fixp_content /  float32fixed<T>(other).getFixed() );   return *this;  }
  bool operator<(double other) const {return this->fixp_content<   float32fixed<T>(other).getFixed();}
  bool operator>(double other) const {return this->fixp_content>   float32fixed<T>(other).getFixed();}
  bool operator<=(double other) const {return this->fixp_content<= float32fixed<T>(other).getFixed();}
  bool operator>=(double other) const {return this->fixp_content>= float32fixed<T>(other).getFixed();}
  bool operator==(double other) const {return this->fixp_content== float32fixed<T>(other).getFixed();}


  // float32fixed operators
  float32fixed<T> operator+ (float32fixed<T> other) const {return float32fixed<T>( this->fixp_content + other.getFixed()  );  }
  float32fixed<T> operator- (float32fixed<T> other) const {return float32fixed<T>( this->fixp_content - other.getFixed()  );  }
  float32fixed<T> operator* (float32fixed<T> other) const {return float32fixed<T>( this->fixp_content * other.getFixed()  );  }
  float32fixed<T> operator/ (float32fixed<T> other) const {return float32fixed<T>( this->fixp_content / other.getFixed()  );  }
  float32fixed<T> operator+=(float32fixed<T> other) { *this = float32fixed<T>( (*this).fixp_content + other.getFixed()  );   return *this;  }
  float32fixed<T> operator-=(float32fixed<T> other) { *this = float32fixed<T>( (*this).fixp_content - other.getFixed()  );   return *this;  }
  float32fixed<T> operator*=(float32fixed<T> other) { *this = float32fixed<T>( (*this).fixp_content * other.getFixed()  );   return *this;  }
  float32fixed<T> operator/=(float32fixed<T> other) { *this = float32fixed<T>( (*this).fixp_content / other.getFixed()  );   return *this;  }
  bool operator< (float32fixed<T> other) const {return this->fixp_content<other.getFixed() ;}
  bool operator> (float32fixed<T> other) const {return this->fixp_content>other.getFixed() ;}
  bool operator<=(float32fixed<T> other) const {return this->fixp_content<=other.getFixed() ;}
  bool operator>=(float32fixed<T> other) const {return this->fixp_content>=other.getFixed() ;}
  bool operator==(float32fixed<T> other) const {return this->fixp_content==other.getFixed() ;}

  float32fixed<T> fabs() const{ return ((*this)>0.) ? *this : (*this)*-1.0 ; }

  explicit operator float () {return (double) this->fixp_content; }


};

struct std_align{
  //match alignments in the standard documentation;
  //note that we're working in a wedge with \phi_0 in the misalignment document of pi/2; hence,
  //the par_cor_mis (X,Y,Z)-->(T,S,Z)--(end cap side A, (Z>0))-->(s,z,t) rotations (gamma,beta,alpha)
  //so let's do show the correspondence between par_cor_mis and this new std_align, struct (we only do the back quadruplet)
  //(--,-dy,dz;dt,--,dp)=(s,z,t;gamma,beta,alpha)
  //our internal stuff will still match the (x,y,z) coordinate scheme used in the algorithm; we will just translate results into the standard coordinates for plots, etc.
  std_align(int qcm=0,const TVector3& trans=TVector3(),const TVector3& ang=TVector3());
  string par_title(int par_num,bool small_unit=false) const;
  string par_name(int par_num) const;
  string par_title_val(int par_num)const;
  string par_name_val(int par_num)const;
  string print() const;
  double par_val(int par_num) const;
  void set_val(int par_num,double par_val);
  int parmax() const { return 6; }
  string detail() const;
  bool is_nominal()const;

  //members
  int type;//corresponds to qcm; 0 (nominal case: no misal/correction), 1 (misalignment), 2 (correction), 3 (sim_correction; do corrections based on simulation results)
  TVector3 translate,rotate;

};

struct gcm_key{
  gcm_key(int _pt=0,int _ct=0,int _mis=0,int corr=0,int _eta=-1,int _qt=0,int _bg=0);
  bool operator==(const gcm_key& merp) const;
  bool operator!=(const gcm_key& merp) const;
  bool operator<(const gcm_key& merp) const;
  bool operator<=(const gcm_key& merp) const;
  bool operator>(const gcm_key& merp) const;
  bool operator>=(const gcm_key& merp) const;

  gcm_key ak()const {return gcm_key(pt,ct,mis,correct,-1,qt,bgcode);}//access key (ak for short; I'm a lazy typist); eta doesn't matter for access so set it to default so we access the map ok
  int varmax()const{return 7;}
  string print()const;
  void set_var(int var,int val);
  int get_var(int var)const;

  //these are the original variables....
  int pt,ct,mis;
  int correct;
  int eta;
  //these are the new ones--we will stack
  //qt is charge threshold 0, 1 (default), or 2
  //bgcode 0--3: std (y/n), wide (y/n)
  int qt,bgcode;
};

struct par_par{
  par_par(double _h,
          int xct=0,
          int uvct=0,
          double uver=0,
          const string& setup="",
          bool ql=true,
          bool q_dlm=false,
          bool qbg=0,
          double _qt=1.,
          std_align mis=std_align(0),
          std_align cor=std_align(0),
          bool fill_tab=true,
          int cs=0,
          const string&pd="",
          const string&tg="");

  string print_pars(const vector<int>&hide=vector<int>()) const;
  string detail()const;

  //members
  double h;
  int ctx,ctuv;
  double uverr;
  string setup;
  bool islarge,q_dlm,genbg;
  double qt;

  std_align misal,corr;
  vector<TString> misal_str;
  bool fill_val;
  int colskip;
  string pcrep_dir,tag;



};

class MMT_Parameters{
 public:
  MMT_Parameters(par_par inputParams,char wedgeSize, const MuonGM::MuonDetectorManager* m_detManager);

  std::vector<Amg::Vector3D> MM_firststrip_positions(const MuonGM::MuonDetectorManager* m_detManager, const std::string& wedge, int eta);
  int is_x(int plane);
  int is_u(int plane);
  int is_v(int plane);

  unsigned int m_n_multiplets = 2;
  unsigned int m_n_layers     = 4;

  // start counting at 1 to be consistent with mmIdHelper.
  std::vector<unsigned int> m_planes_x = {1, 2, 7, 8};
  std::vector<unsigned int> m_planes_u = {3, 5};
  std::vector<unsigned int> m_planes_v = {4, 6};

  vector<int> q_planes(const string& type) const;
  par_par param_par() const;
  double y_from_eta_wedge(double eta,int plane)const;
  double eta_wedge_from_y(double y,int plane)const;
  int ybin(float32fixed<18> y,int plane=0)const;
  int ybin(float32fixed<m_yzdex> y,int plane=0)const;
  int ybin(double y,int plane=0)const;

  //fill the tables
  void Local_Slope_A_B();
  void Slope_Components_ROI();
  void Delta_theta_optimization_LG();
  void fill_full_Ak_Bk();
  void fill_slims();
  void fill_crep_table(const string&dir,const string&tag);
  void fill_yzmod();
  void index_key_test();

  //eta-phi stuff
  int eta_bin(double theta) const;
  string eta_str(int eta) const;
  string phi_str(int eta) const;
  int phi_bin(double theta) const;

  //table calculations
  pair<double,double> ak_bk_hit_bins(const vector<int>& hits)const;

  vector<bool> lcl_int_to_xhits(int lcl_int)const;
  int xhits_to_lcl_int(const vector<bool>& xhits) const;
  string lcl_int_to_xhit_str(int lcl_int)const;
  bool toggle_key(vector<int>& key,int lo_value,int hi_value)const;
  void key_to_indices(const vector<int>& key,int& xdex,int& ybin,int& which)const;
  vector<int> indices_to_key(int xdex,int ybin,int which)const;

  //sim corr indexing stuff
  int nsimmax_1d()const;//number of sim cases
  int bool_to_index(const vector<bool>&track)const;
  vector<bool> index_to_bool(int index)const;
  string index_to_hit_str(int index)const;
  string bool_to_hit_str(const vector<bool>&track)const;

  //table
  map<vector<int>,pair<float32fixed<2>,float32fixed<2> > > m_AB_k_local;
  vector<vector<vector<float32fixed<m_zbardex> > > > m_Ak_local_slim;//[x_hit combo][ybin][case #]
  vector<vector<vector<float32fixed<m_bkdex> > > > m_Bk_local_slim;//[x_hit combo][ybin][case #]
  vector<vector<vector<float32fixed<4> > > > m_Slope_to_ROI;
  vector<vector<float32fixed<2> > > m_DT_Factors;
  //theta, phi, hit code, theta/phi/dtheta
  //hit code: binary stuff....
  //old hit code...%mis X, %mis UV: 2-4 X, 1-4 UV possible fractions: 0,1/2,1/3,2/3,1/4,(3/4,not possible with only one misaligned multiplet), 1: 0,3,4,6,8,12
  vector<vector<vector<vector<float> > > >m_crep_table;
  vector<vector<vector<float32fixed<m_yzdex> > > >m_ymod,m_zmod;

  //a toggle
  bool m_fill0;

  //how many etabins (of equal size) for algorithm results
  int m_n_etabins,m_n_phibins;
  vector<double>m_etabins,m_phibins;
  //currently configurable parameters
  bool m_diag,m_dlm_new;
  float32fixed<2> m_h;
  int m_CT_x,m_CT_uv;
  float32fixed<2> m_uv_error;
  double m_dtheta_cut;
  string m_setup;
  bool m_islarge,m_genbg;
  double m_chargeThreshold;
  //new, standardized, misalignment and correction information
  std_align m_misal,m_correct;

  //misalignment parameters
  int m_ybins,m_n_stations_eta;
  bool m_misalign,m_val_tbl;

  //dimensions
  float32fixed<18> m_w1, m_w2, m_w3, m_h1, m_H, m_Hnom, m_wedge_opening_angle;
  float32fixed<4> m_strip_width;
  float32fixed<4> m_stereo_degree;

  vector<float32fixed<18> > m_z_nominal;
  vector<vector<float32fixed<18> > > m_z_large;//[y bin][plane]
  vector<vector<float32fixed<18> > > m_ybases;//by stationEta--saved from file, hardcoded, alternative is equally spaced, in MMT_Loader::Get_Strip_Id
  float32fixed<2> m_x_min,m_x_max,m_y_min,m_y_max,m_h_mx, m_h_my;
  int m_n_x,m_n_y;

  float32fixed<3> m_slope_min, m_slope_max;
  float32fixed<2> m_x_error;
  int m_CT, m_CT_u, m_CT_v;

  float32fixed<4> m_minimum_large_theta, m_maximum_large_theta;
  float32fixed<4> m_minimum_large_phi, m_maximum_large_phi;

  int m_n_theta_rois, m_n_phi_rois, m_BC_window;

  float32fixed<18> m_mid_plane_large_X, m_mid_plane_large, m_mid_plane_large_UV;
  float32fixed<4> m_vertical_strip_width_UV;

  /// Log a message using the Athena controlled logging system
  MsgStream& msg( MSG::Level lvl ) const { return m_msg << lvl; }
  /// Check whether the logging system is active at the provided verbosity level
  bool msgLvl( MSG::Level lvl ) const { return m_msg.get().level() <= lvl; }

  /// Private message stream member
  mutable Athena::MsgStreamMember m_msg;


};

struct mm_digit_entry{
  int multiplet,gas_gap;
  double global_time,time;
  TVector2 local_pos, stripl_pos;
  TVector3 stripg_pos;
  double charge;
};

struct evInf_entry{
  evInf_entry(int event=0,int pdg=0,double e=0,double p=0,double ieta=0,double peta=0,double eeta=0,double iphi=0,double pphi=0,double ephi=0,
	      double ithe=0,double pthe=0,double ethe=0,double dth=0,int trn=0,int mun=0,const TVector3& tex=TVector3(),
	      int troi=0,int antev=0,int postv=0,int nxh=0,int nuvh=0,int nxbg=0,int nuvbg=0,double adt=0,int difx=0,
	      int difuv=0,bool cut=false,bool bad=false);
  void print() const;

  int athena_event,pdg_id;
  double E,pt,eta_ip,eta_pos,eta_ent,phi_ip,phi_pos,phi_ent,theta_ip,theta_pos,theta_ent,dtheta;
  int truth_n,mu_n;
  TVector3 vertex;
  int truth_roi;
  int N_hits_preVMM,N_hits_postVMM/*<--true*/;
  int N_X_hits,N_UV_hits;//signal pre vmm; the X and UV hits that count in efficiency denominator
  int NX_bg_preVMM,NUV_bg_preVMM;
  double avg_drift_time;
  int max_less_min_hitsteps_X,max_less_min_hitsteps_UV;
  bool pass_cut,bad_wedge;


};

struct hitData_key{
  hitData_key(int bct=0, double t=0, double gt=0, int vmm=-1,int ev=-1);
  hitData_key(const hitData_key& key){BC_time=key.BC_time;time=key.time;gtime=key.gtime;VMM_chip=key.VMM_chip;event=key.event;}
  //for these operators, "less" means what you think it means
  //with precedence of quantities the same as order in the constructor
  //in the context of "earlier," it might make sense to say that a hit is earlier if,
  //when all times tied, the larger plane is earlier, but this assumes a everything is signal like
  bool operator==(const hitData_key& merp) const;
  bool operator!=(const hitData_key& merp) const;
  bool operator<(const hitData_key& merp) const;
  bool operator>(const hitData_key& merp) const;
  bool operator<=(const hitData_key& merp) const;
  bool operator>=(const hitData_key& merp) const;
  string hdr()const;
  string str()const;
  void print() const;

  int BC_time;
  double time,gtime;
  int VMM_chip,event;


};

struct evFit_entry{
  evFit_entry(int event=0,float32fixed<4> fthe=0.,float32fixed<4> fphi=0.,
	      float32fixed<2> fdth=0.,int roi=-1,int xhit=false,int uvhit=false,
	      int bgx=false,int bguv=false,float32fixed<2> dth_nd=0.,int hc=0,int tph=0,
	      int bgph=0);

  void print() const;

  int athena_event;
  float32fixed<4> fit_theta,fit_phi;
  float32fixed<2> fit_dtheta;
  int fit_roi,X_hits_in_fit,UV_hits_in_fit,bg_X_fit,bg_UV_fit;
  float32fixed<2> dtheta_nodiv;
  int hcode,truth_planes_hit,bg_planes_hit;
  vector<hitData_key> fit_hit_keys;


};

struct evAna_entry{
  int athena_event;
  double theta_err,phi_err,dtheta_err;
  bool qualified_event, fit, good_fit;
  int X_hits_in_fit,UV_hits_in_fit;
  int bg_X_fit,bg_UV_fit;
};

struct hitData_info{
  hitData_info(int plane,int station_eta,int strip,MMT_Parameters *m_par,const TVector3& tru,double tpos,double ppos);
  hitData_info(int pl=0,double _y=0,double _z=-999);
  hitData_info(const hitData_info& info){plane=info.plane;y=info.y;z=info.z;slope=info.slope;}
  double mis_dy(int pl,MMT_Parameters *m_par,double tpos,double ppos)const;
  string hdr()const;
  string str()const;
  void print()const;
  bool operator==(const hitData_info& rhs) const;

  //members
  int plane;
  //char addc,vmm,strip;//strip is in a vmm (0-63); char for storage as a byte
  float32fixed<m_yzdex> y,z;//actual values divided by store_const() to make fixed point calculations doable--all this stuff is dimensionless in the end, so it's okay.
  float32fixed<2> slope;

};

struct Hit{
  //make a well-behaved constructor
  Hit(const hitData_key&k=hitData_key(),const hitData_info&i=hitData_info());
  bool operator==(const Hit& rhs) const;
  void print_track(const vector<Hit>& track) const;
  void print() const;

  //the members:
  hitData_key key;
  hitData_info info;
};

struct hitData_entry{
  hitData_entry(int ev=0, double gt=0, double q=0, int vmm=0, int pl=0, int st=0, int est=0, double tr_the=0, double tru_phi=0,
	     bool q_tbg=0, int bct=0, double time=0,const TVector3& tru=TVector3(), const TVector3& rec=TVector3(),
	     double fit_the=0, double fit_phi=0, double fit_dth=0, double tru_dth=0,// double tru_thl=0, double tru_thg=0,
	     double mxg=0, double mug=0, double mvg=0, double mxl=0, double _mx=0, double _my=0, int _roi=0);

  Hit entry_hit(MMT_Parameters *m_par)const;
  hitData_key entry_key() const;
  hitData_info entry_info(MMT_Parameters *m_par)const;
  void fit_fill(float32fixed<4> fthe,float32fixed<4> fphi, float32fixed<2> fdth, float32fixed<2> mxg=0., float32fixed<2> mug=0., float32fixed<2> mvg=0., float32fixed<2> mxl=0., float32fixed<2> m_x=0., float32fixed<2> m_y=0., int king=0);
  void print() const;

  int event;
  double gtime,charge;
  int VMM_chip,plane,strip,station_eta;
  double tru_theta_ip,tru_phi_ip;
  bool truth_nbg;//truth (i.e. not bg) if true,
  int BC_time;/*fit theta, phi, dtheta originally here*/
  double time;
  TVector3 truth,recon;
  float32fixed<4> fit_theta,fit_phi;
  float32fixed<2> fit_dtheta;
  double tru_dtheta;//,tru_theta_local,tru_theta_global;
  float32fixed<2> M_x_global,M_u_global,M_v_global,M_x_local,mx,my;
  int roi;


};

struct finder_entry{
  //make a well-behaved constructor
  finder_entry(bool _is_hit=0, int _clock=-1,const Hit& k=Hit());
  bool operator==(const finder_entry& merp) const;
  bool operator!=(const finder_entry& merp) const;

  //Finder stuff:
  bool is_hit,is_ct;
  int clock;
  //Finder_Detail stuff:
  Hit hit;
};

struct ROI{
  //make a well-behaved constructor
  ROI(double _theta, double _phi, double _m_x, double _m_y, int _roi);

  //the members:
  float32fixed<4> theta,phi;
  float32fixed<2> m_x,m_y;
  int roi;
};

struct athena_header{
  //make a well-behaved constructor
  athena_header(const TLorentzVector& par=TLorentzVector(), int tpn=0, double etp=0, double ete=0, double php=0, double phe=0, int mun=0, const TVector3& ver=TVector3());

  //the members:
  TLorentzVector the_part;
  int trupart_n;
  double etapos,etaent,phipos,phient;
  int muent_n;
  TVector3 vertex;
};

struct digitWrapper{
  digitWrapper(const MmDigit* digit=0,
               double tmpGTime=0,
               const TVector3& truthLPos=TVector3(),
               const TVector3& stripLPos=TVector3(),
               const TVector3& stripGPos=TVector3()
               );


  const MmDigit* digit;
  double gTime;

  TVector3 truth_lpos;//4,5
  TVector3 strip_lpos;
  TVector3 strip_gpos;//6-11

  inline Identifier id(){ return digit->identify(); };

};

struct track_address{//cartesian_hit_rot entry
  track_address(int bct=0,bool big=false,int wed=0,int pl=-1,int sh=0,const TVector3& chr=TVector3());

  int BC;
  bool islarge;
  int wedge,plane,strip_hit;
  TVector3 cart_hit;
};

#endif
