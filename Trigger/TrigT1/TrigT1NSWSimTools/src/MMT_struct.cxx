/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/MMT_struct.h"
#include "MuonAGDDDescription/MMDetectorDescription.h"
#include "MuonAGDDDescription/MMDetectorHelper.h"
#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"


float MMTStructConst = 8192.;

std_align::std_align(int qcm,const TVector3& trans,const TVector3& ang):type(qcm),translate(trans),rotate(ang){
  if(type==0){
    translate=TVector3();rotate=TVector3();
  }
}

string std_align::par_title(int par_num,bool small_unit)const{
  string par_title("invalid");
  if(par_num==0) par_title="#Deltas [mm]";
  else if(par_num==1) par_title="#Deltaz [mm]";
  else if(par_num==2) par_title="#Deltat [mm]";
  else if(par_num==3) par_title="#gamma_{s}";
  else if(par_num==4) par_title="#beta_{z}";
  else if(par_num==5) par_title="#alpha_{t}";
  else{
    exit(3);
  }
  if(par_num>2){
    if(small_unit)par_title+=" [mrad]";
    else par_title+=" [rad]";
  }
  return par_title;
}

string std_align::par_name(int par_num)const{
  string par_name("invalid");
  if(par_num==0) par_name="dts";
  else if(par_num==1) par_name="dtz";
  else if(par_num==2) par_name="dtt";
  else if(par_num==3) par_name="drs";
  else if(par_num==4) par_name="drz";
  else if(par_num==5) par_name="drt";
  else{
    exit(3);
  }
  return par_name;
}

string std_align::par_title_val(int par_num)const{
  assert(par_num>=0&&par_num<parmax());
  ostringstream par_title;bool do_it=false;
  for(int i=0;i<6;i++){
    if(par_val(i)!=0)do_it=true;
  }
  if(!do_it)return "nominal";
  if(par_num==0) par_title<<"#Deltas="<<par_val(0)<<" mm";
  else if(par_num==1) par_title<<"#Deltaz="<<par_val(1)<<" mm";
  else if(par_num==2) par_title<<"#Deltat="<<par_val(2)<<" mm";
  else if(par_num==3) par_title<<"#gamma_{s}="<<par_val(3)<<" rad";
  else if(par_num==4) par_title<<"#beta_{z}="<<par_val(4)<<" rad";
  else if(par_num==5) par_title<<"#alpha_{t}="<<par_val(5)<<" rad";
  return par_title.str();
}

string std_align::par_name_val(int par_num)const{
  assert(par_num>=0&&par_num<parmax());
  ostringstream par_name;
  if(par_num==0) par_name<<"dts";
  else if(par_num==1) par_name<<"dtz";
  else if(par_num==2) par_name<<"dtt";
  else if(par_num==3) par_name<<"drs";
  else if(par_num==4) par_name<<"drz";
  else if(par_num==5) par_name<<"drt";
  par_name<<par_val(par_num);
  return par_name.str();
}

string std_align::print()const{
  if(type==0) return string("_NOM");//nominal
  string gordon;int maxpar=0;
  if(type==1){
    gordon+="_MIS";maxpar=parmax();
  }
  else if(type==2){
    gordon+="_COR";maxpar=parmax();
  }
  else if(type==3){
    gordon+="_3CR";maxpar=parmax();
  }
  for(int imal=0; imal<maxpar;imal++){
    ostringstream addme;
    if(par_val(imal)!=0){
      addme<<"_"<<par_name(imal)<<par_val(imal);
    }
    gordon+=addme.str();
  }
  return gordon;
}

double std_align::par_val(int par_num) const{
  assert(par_num>=0&&par_num<parmax());
  double par_val=-999.;
  if(par_num==0) par_val=translate.X();
  else if(par_num==1) par_val=translate.Y();
  else if(par_num==2) par_val=translate.Z();
  else if(par_num==3) par_val=rotate.X();
  else if(par_num==4) par_val=rotate.Y();
  else if(par_num==5) par_val=rotate.Z();
  return par_val;
}

void std_align::set_val(int par_num,double par_val){
  assert(par_num>=0&&par_num<parmax());
  if(log10(abs(par_val))<-7)par_val=0;
  if(par_num==0) translate.SetX(par_val);
  else if(par_num==1) translate.SetY(par_val);
  else if(par_num==2) translate.SetZ(par_val);
  else if(par_num==3) rotate.SetX(par_val);
  else if(par_num==4) rotate.SetY(par_val);
  else if(par_num==5) rotate.SetZ(par_val);
}

string std_align::detail()const{
  string logan;
  if(type==0)return string("nominal");
  else if(type==1) logan+="MIS";
  else if(type==2) logan+="COR";
  for(int imal=0; imal<parmax();imal++){
    ostringstream addme;
    if(par_val(imal)!=0)addme<<", "<<par_title(imal)<<"="<<par_val(imal);
    logan+=addme.str();
  }
  if(logan.compare("")==0)logan+=" nominal";
  return logan;
}

bool std_align::is_nominal()const{
  for(int i=0;i<parmax();i++){
    if(par_val(i)!=0) return false;//sometimes doubles as zero get set to really small but not zero values...
  }
  return true;
}

gcm_key::gcm_key(int _pt,int _ct,int _mis,int corr,int _eta,int _qt,int _bg):pt(_pt),ct(_ct),mis(_mis),correct(corr),eta(_eta),qt(_qt),bgcode(_bg){}

bool gcm_key::operator==(const gcm_key& rhs) const{
  if(this->pt==rhs.pt&&this->ct==rhs.ct&&this->mis==rhs.mis&&this->correct==rhs.correct&&this->eta==rhs.eta&&this->qt==rhs.qt&&this->bgcode==rhs.bgcode) return true;
  return false;
}

bool gcm_key::operator!=(const gcm_key& rhs) const{
  return !(*this==rhs);
}
bool gcm_key::operator<(const gcm_key& rhs) const{
  if(this->pt<rhs.pt) return true;
  else if(this->pt==rhs.pt&&this->ct<rhs.ct) return true;
  else if(this->pt==rhs.pt&&this->ct==rhs.ct&&this->mis<rhs.mis) return true;
  else if(this->pt==rhs.pt&&this->ct==rhs.ct&&this->mis==rhs.mis&&this->correct<rhs.correct) return true;
  else if(this->pt==rhs.pt&&this->ct==rhs.ct&&this->mis==rhs.mis&&this->correct==rhs.correct&&this->eta<rhs.eta) return true;
  else if(this->pt==rhs.pt&&this->ct==rhs.ct&&this->mis==rhs.mis&&this->correct==rhs.correct&&this->eta==rhs.eta&&this->qt<rhs.qt) return true;
  else if(this->pt==rhs.pt&&this->ct==rhs.ct&&this->mis==rhs.mis&&this->correct==rhs.correct&&this->eta==rhs.eta&&this->qt==rhs.qt&&this->bgcode<rhs.bgcode) return true;
  return false;
}
bool gcm_key::operator<=(const gcm_key& rhs) const{
  return(*this<rhs || *this==rhs);
}
bool gcm_key::operator>(const gcm_key& rhs) const{
  return !(*this>=rhs);
}
bool gcm_key::operator>=(const gcm_key& rhs) const{
  return(*this>rhs || *this==rhs);
}

string gcm_key::print()const{
  ostringstream ricola;ricola<<"(p"<<pt<<",C"<<ct<<",m"<<mis<<",c"<<correct<<",e"<<eta<<",q"<<qt<<",b"<<bgcode<<")";
  return ricola.str();
}

void gcm_key::set_var(int var,int val){
  assert(var>=0&&var<varmax());
  if(var==0)pt=val;
  else if(var==1)ct=val;
  else if(var==2)mis=val;
  else if(var==3)correct=val;
  else if(var==4)eta=val;
  else if(var==5)qt=val;
  else if(var==6)bgcode=val;
}

int gcm_key::get_var(int var)const{
  assert(var>=0&&var<varmax());
  if(var==0)return pt;
  else if(var==1)return ct;
  else if(var==2)return mis;
  else if(var==3)return correct;
  else if(var==4)return eta;
  else if(var==5)return qt;
  else if(var==6)return bgcode;
  return -999;
}

par_par::par_par(double _h,int xct,int uvct,double uver,const string& set,bool ql,bool dlm,bool qbg,double _qt,std_align mis,std_align cor,bool fill_tab,int cs,const string&pd,const string&tg):
  h(_h),ctx(xct),ctuv(uvct),uverr(uver),setup(set),islarge(ql),q_dlm(dlm),genbg(qbg),qt(_qt),misal(mis),corr(cor),fill_val(fill_tab),colskip(cs),pcrep_dir(pd),tag(tg) {}

string par_par::print_pars(const vector<int>&hide) const{
  vector<bool>sho(gcm_key().varmax(),true);
  for(int i=0;i<(int)hide.size();i++){
    if(hide[i]<0||hide[i]>=gcm_key().varmax())continue;
    sho[hide[i]]=false;sho[2]=false;
  }
  ostringstream mars;
  if(sho[6])mars<<"_h"<<h;
  if(sho[1])mars<<"_ctx"<<ctx<<"_ctuv"<<ctuv;
  if(sho[6])mars<<"_uverr"<<uverr;
  mars<<"_set"<<setup<<"_ql"<<islarge<<"_qdlm"<<q_dlm;
  if(sho[6])mars<<"_qbg"<<genbg;
  if(sho[5])mars<<"_qt"<<qt;
  if(sho[2])mars<<misal.print()<<corr.print();
  return mars.str();
}

string par_par::detail()const{
  return misal.detail()+"; "+corr.detail();
}

MMT_Parameters::MMT_Parameters(par_par inputParams, char wedgeSize, const MuonGM::MuonDetectorManager* m_detManager){
  if(inputParams.misal.is_nominal())inputParams.misal.type=0;
  //can still do sim_corrections for nominal
  if(inputParams.corr.is_nominal()&&inputParams.corr.type==2)inputParams.corr.type=0;
  m_n_etabins=10;
  m_n_phibins=10;
  m_dtheta_cut=0.016;//16 mrad dtheta cut (VMM limitation...may revise)
  m_diag=true;
  m_fill0=false;
  //stuff pulled from the inputParams
  m_h=float32fixed<2>(inputParams.h);
  m_CT_x=inputParams.ctx;
  m_CT_uv=inputParams.ctuv;
  m_uv_error=float32fixed<2>(inputParams.uverr);
  m_setup=(inputParams.setup);
  m_islarge=inputParams.islarge;
  m_dlm_new=inputParams.q_dlm;
  m_chargeThreshold=inputParams.qt;
  m_genbg=inputParams.genbg;
  m_misal=inputParams.misal;
  m_misalign=(m_misal.type==1);
  m_correct=inputParams.corr;
  m_ybins=8; m_n_stations_eta=(m_dlm_new?2:4);
  m_val_tbl=inputParams.fill_val;

  // Get the modules for each multiplet, the sector and side arguement (third and fifth argument, respectively) shouldn't matter
  // since the algorithm computes locally
  char side   = 'A';
  char sector = wedgeSize;
  MMDetectorHelper aHelper;
  MMDetectorDescription* mm_top_mult1    = aHelper.Get_MMDetector(sector, 2, 5, 1, side);
  MMDetectorDescription* mm_top_mult2    = aHelper.Get_MMDetector(sector, 2, 5, 2, side);
  MMDetectorDescription* mm_bottom_mult1 = aHelper.Get_MMDetector(sector, 1, 5, 1, side);
  MMDetectorDescription* mm_bottom_mult2 = aHelper.Get_MMDetector(sector, 1, 5, 2, side);

  MMReadoutParameters roParam_top_mult1    = mm_top_mult1->GetReadoutParameters();
  MMReadoutParameters roParam_top_mult2    = mm_top_mult2->GetReadoutParameters();
  MMReadoutParameters roParam_bottom_mult1 = mm_bottom_mult1->GetReadoutParameters();
  MMReadoutParameters roParam_bottom_mult2 = mm_bottom_mult2->GetReadoutParameters();

  //Needed to get the max channels for the deadZone in the x planes... a little annoying...
  //Only changes for small or large and per module. Phi, layer, etc don't matter (just set to 1)
//const MuonGM::MuonDetectorManager* m_MuonGeoMgr;
  std::string wedgeString = (wedgeSize=='L' ? "MML" : "MMS");

  if(!m_islarge){
    ATH_MSG_WARNING("We haven't configured the small wedge parameters yet!  Go bother the developer...kindly...this will likely crash now!\n");
    return;
  }
  //y = vertical distance from beamline
  //z = into wedge along beamline
  //x = horizontal distance from beam looking down
  ////////////  Define the large wedge /////////////////
  m_w1=float32fixed<18>(mm_top_mult1->lWidth());   //top
  m_w2=float32fixed<18>(mm_top_mult1->lWidth()*1./(cos(roParam_top_mult1.stereoAngle.at(3))));  //determined by 33deg angle   //shelves part
  m_w3=float32fixed<18>(mm_bottom_mult1->sWidth());//582.3);  //bottom
  m_h1=float32fixed<18>(roParam_top_mult1.roLength+roParam_bottom_mult1.roLength+5.0); //how tall wedge is at w1, ie total height

  m_wedge_opening_angle = float32fixed<18>(33.);  //degree

  m_z_large=vector<vector<float32fixed<18> > >(m_ybins,vector<float32fixed<18> >(m_setup.length(),float32fixed<18>(0.)));

  // retrieve the z-position of the planes
  m_z_nominal.clear();
  int eta = 1;
  for (auto pos: MM_firststrip_positions(m_detManager, wedgeString, eta)){
    m_z_nominal.push_back(float32fixed<18>(pos.z()));
  }

  if(m_z_nominal.size() != m_setup.size()){
    ATH_MSG_WARNING( "Number of planes in setup is "<< m_setup.size()
              << ", but we have a nominal "     << m_z_nominal.size() << " planes.");
    exit(9);
  }

  m_mid_plane_large=float32fixed<18>(0.);
  for(unsigned int iz=0; iz<m_z_nominal.size(); iz++) m_mid_plane_large+=m_z_nominal[iz].getFixed();
  m_mid_plane_large/=m_z_nominal.size();

  vector<int> xp=q_planes("x"),up=q_planes("u"),vp=q_planes("v");
  m_mid_plane_large_X=float32fixed<18>(0.);
  for(unsigned int ix=0;ix<xp.size();ix++) m_mid_plane_large_X+=m_z_nominal[xp[ix]].getFixed();
  m_mid_plane_large_X /= 1.*xp.size();

  m_mid_plane_large_UV=float32fixed<18>(0.);
  for(unsigned int iu=0;iu<up.size();iu++) m_mid_plane_large_UV+=m_z_nominal[up[iu]].getFixed();
  for(unsigned int iv=0;iv<vp.size();iv++) m_mid_plane_large_UV+=m_z_nominal[vp[iv]].getFixed();
  m_mid_plane_large_UV /= 1.*(up.size()+vp.size());

  m_H=float32fixed<18>(roParam_bottom_mult1.distanceFromZAxis);//982.); //bottom of wedge to the beamline
  m_Hnom=float32fixed<18>(roParam_bottom_mult1.distanceFromZAxis);//982.); //bottom of wedge to the beamline

  m_strip_width = float32fixed<4>(roParam_top_mult1.stripPitch);  // 0.5;
  m_stereo_degree = float32fixed<4>(TMath::RadToDeg()*roParam_top_mult1.stereoAngle.at(2)); //0.75 //3 in degrees!
  float32fixed<2> degree=roParam_top_mult1.stereoAngle.at(2);
  m_vertical_strip_width_UV = m_strip_width.getFixed()/cos(degree.getFixed());
  m_ybases=vector<vector<float32fixed<18> > >(m_setup.size(),vector<float32fixed<18> >(m_n_stations_eta,float32fixed<18>(0.)));

  //gposx for X1; gpos-lpos X2,UV1,UV2
  vector<float32fixed<18> > xeta,ueta,veta;

  // MM_firststrip_positions returns the position for phi sector 1.
  // => for the small sectors, rotate this by -1/16 of a rotation to make our lives easier.
  // in this coordinate basis, x is up/down the wedge (radial), and y is left/right (phi).
  float cos_rotation = cos(-2*TMath::Pi() / 16.0);
  float sin_rotation = sin(-2*TMath::Pi() / 16.0);
  float x_rotated = 0.0;
  float y_rotated = 0.0;

  float st_angle = 0.0;

  float radial_pos      = 0;
  float radial_pos_xx_1 = 0, radial_pos_xx_2 = 0;
  float radial_pos_uv_1 = 0, radial_pos_uv_2 = 0;

  for (unsigned int eta = 1; eta <= 2; eta++){
    unsigned int layer = 1;
    for (auto pos: MM_firststrip_positions(m_detManager, wedgeString, eta)){

      if (wedgeString=="MMS"){
        x_rotated = pos.x()*cos_rotation - pos.y()*sin_rotation;
        y_rotated = pos.x()*sin_rotation + pos.y()*cos_rotation;
      }
      else{
        x_rotated = pos.x();
        y_rotated = pos.y();
      }

      if      (is_u(layer)) st_angle = -1*abs(m_stereo_degree.getFixed());
      else if (is_v(layer)) st_angle =    abs(m_stereo_degree.getFixed());
      else                  st_angle = 0;

      // walk from the center of the strip to the position of the strip at the center of the wedge.
      // NB: for X-planes, this is simply the center of the strip: tan(0) = 0.
      radial_pos = abs(x_rotated - y_rotated*tan(st_angle * TMath::Pi()/180.0));

      if (is_x(layer) && eta==1) radial_pos_xx_1 = radial_pos;
      if (is_x(layer) && eta==2) radial_pos_xx_2 = radial_pos;
      if (is_u(layer) && eta==1) radial_pos_uv_1 = radial_pos;
      if (is_u(layer) && eta==2) radial_pos_uv_2 = radial_pos;

      layer++;
    }
  }

  // store radial positions as fixed point
  std::vector< float32fixed<18> > radial_pos_xx = {float32fixed<18>(radial_pos_xx_1), float32fixed<18>(radial_pos_xx_2)};
  std::vector< float32fixed<18> > radial_pos_uv = {float32fixed<18>(radial_pos_uv_1), float32fixed<18>(radial_pos_uv_2)};
  m_ybases[0] = radial_pos_xx;
  m_ybases[1] = radial_pos_xx;
  m_ybases[2] = radial_pos_uv;
  m_ybases[3] = radial_pos_uv;
  m_ybases[4] = radial_pos_uv;
  m_ybases[5] = radial_pos_uv;
  m_ybases[6] = radial_pos_xx;
  m_ybases[7] = radial_pos_xx;


  //now put in the positions at `ly spaced points for a y dependent z
  bool hack=false;
  m_z_large=vector<vector<float32fixed<18> > >(m_ybins,m_z_nominal);
  double pitch_f=1.*sin(m_correct.rotate.X())*m_h1.getFixed()/m_ybins,pitch_b=0,bumper_up=0.0;
  if(hack){double factor=-1;pitch_f*=factor;pitch_b*=factor;}
  ATH_MSG_DEBUG("Specs: correct.rotate.X()="<<m_correct.rotate.X()<<",correct.translate.Z()="<<m_correct.translate.Z()<<",pitch_f="<<pitch_f);
  for(int iy=0;iy<m_ybins;iy++){
    //z axis in misalignment line points in the opposite direction....TOO LATE! (right handed coordinate system the way we expect for x and y makes this happen)
    double over_f=pitch_f*(iy+bumper_up)-m_correct.translate.Z(),over_b=0;//pitch_b*(iy+bumper_up)+correct.dzb;
    ATH_MSG_DEBUG("iy="<<iy<<"over_f="<<over_f<<",over_b="<<over_b);
    for(int jp=0;jp<8;jp++){
      ATH_MSG_DEBUG("m_z_large["<<iy<<"]["<<jp<<"]"<<m_z_large[iy][jp].getFixed()<<"--->");
      if(jp<4)m_z_large[iy][jp]+=over_f;
      else m_z_large[iy][jp]+=over_b;
      ATH_MSG_DEBUG(m_z_large[iy][jp].getFixed());
    }
  }

  //////// TABLE GENERATORS ///////////////  //size of cartesian steps
  m_h_mx = float32fixed<2>(0.0001); // 0.005;  //0.001;
  m_h_my = float32fixed<2>(0.0001); //0.005;  //0.001;
  m_y_max = ( (m_Hnom+m_h1)/m_z_nominal.front() );
  m_y_min = ( m_H/m_z_nominal.back() );
  m_x_max = (m_w2/2.)/m_z_nominal.front();
  m_x_min = (m_w2/-2.)/m_z_nominal.back(); //-2;

  m_n_x = ceil((m_x_max - m_x_min).getFixed()/m_h_mx.getFixed());
  m_n_y = ceil((m_y_max - m_y_min).getFixed()/m_h_my.getFixed());
  /////////////////////////////////////////

  //////////  for cut applications  /////////////////
  double tol = 0;  //0.02;

  //BLC had some interesting bounds...let's do the ones that make sense to me
  m_minimum_large_theta = float32fixed<4>(atan(m_H.getFixed()/m_z_nominal.back().getFixed())+tol);
  m_maximum_large_theta = float32fixed<4>(atan(sqrt(pow( (m_Hnom+m_h1).getFixed(),2)+0.25*pow(m_w1.getFixed(),2))/m_z_nominal.back().getFixed())-tol);
  m_minimum_large_phi = float32fixed<4>(-TMath::DegToRad()*0.5*m_wedge_opening_angle.getFixed())+tol;  m_maximum_large_phi = float32fixed<4>(TMath::DegToRad()*(0.5*m_wedge_opening_angle.getFixed())-tol);
  ///////////////////////////////////////////////////

  double phiseg= ((m_maximum_large_phi-m_minimum_large_phi)/m_n_phibins).getFixed();
  m_phibins.clear();
  for(int i=0;i<=m_n_phibins;i++)  m_phibins.push_back(  (m_minimum_large_phi+phiseg*i).getFixed()   );
  double etalo=-log(tan(0.5*m_maximum_large_theta.getFixed())),etahi=-log(tan(0.5*m_minimum_large_theta.getFixed()));bool custom=false;
  if(custom){
    //these custom eta bins have the top station as one bin (since that has normal shape for the phi resolutions)
    //and separates the old part from gap to 2.5 (where the simulation used to be limited) and then one bin from 2.5 to max (new stuff)
    m_etabins.clear();
    m_etabins.push_back(etalo);
    double etamid=asinh(  (m_z_nominal.front()/m_ybases.front()[1] ).getFixed()  );
    double intermid=(2.5-etamid)/8.;
    for(int i=0;i<8;i++)m_etabins.push_back(etamid+intermid*i);
    m_etabins.push_back(2.5);
    m_etabins.push_back(etahi);
    m_n_etabins=m_etabins.size()-1;
  }
  else if(m_n_etabins==(int)xeta.size()){
    m_etabins.push_back(etalo);
    for(int i=m_n_etabins-1;i>=0;i--){
      m_etabins.push_back(asinh( (m_z_nominal.front()/xeta[i]).getFixed() ));
    }
  }
  else{
    double increment=(etahi-etalo)/m_n_etabins*1.;
    for(int i=0;i<=m_n_etabins;i++)m_etabins.push_back(etalo+increment*i);
  }

  ///////   Rough ROI  ////////////
  m_n_theta_rois = 32;
  m_n_phi_rois = 16;
  //////////////////////////////

  /////// Front Filter /////////////
  double theta_max=m_maximum_large_theta.getFixed();
  double theta_min=m_minimum_large_theta.getFixed();

  m_slope_max = float32fixed<3>(tan(theta_max));
  m_slope_min = float32fixed<3>(tan(theta_min));

  m_CT_u = 0;
  m_CT_v = 0;
  m_CT = m_CT_x + m_CT_uv;
  m_x_error = .0035;//h*0.5;
  m_BC_window = 2;
  //////////////////////////////////
  //tables--with parameters set, we can now generate them
  if(m_val_tbl){
    //Reference for local slope least squares fit
    Local_Slope_A_B();

    //cartesian slopes to phi and theta reference table
    Slope_Components_ROI();

    //Delta theta reference table to avoid division operation
    Delta_theta_optimization_LG();

    //simulation-based correction fill...if available
    if(m_correct.type==3)fill_crep_table(inputParams.pcrep_dir,inputParams.tag);
    fill_yzmod();
  }
}

std::vector<Amg::Vector3D> MMT_Parameters::MM_firststrip_positions(const MuonGM::MuonDetectorManager* m_detManager, const std::string& wedge, int eta){

  std::vector<Amg::Vector3D> positions;
  positions.clear();

  Identifier strip_id;
  int        phi = 1, strip = 1;

  for (unsigned int mult = 1; mult <= m_n_multiplets; mult++) {
    for (unsigned int layer = 1; layer <= m_n_layers; layer++) {

      Amg::Vector3D pos(0.0, 0.0, 0.0);
      strip_id = m_detManager->mmIdHelper()->channelID(wedge, eta, phi, mult, layer, strip);
      const MuonGM::MMReadoutElement* readout = m_detManager->getMMReadoutElement(strip_id);
      readout->stripGlobalPosition(strip_id, pos);
      positions.push_back(pos);

      ATH_MSG_DEBUG( "global z-pos. (using MMReadoutElement) for"
                           << " wedge size "       << wedge
                           << " multiplet "        << mult
                           << " layer "            << layer
                           << " | z = "            << pos.z() );
    }
  }

   return positions;
}

int MMT_Parameters::is_x(int plane){ return (std::find(m_planes_x.begin(), m_planes_x.end(), plane) != m_planes_x.end()) ? 1 : 0; }
int MMT_Parameters::is_u(int plane){ return (std::find(m_planes_u.begin(), m_planes_u.end(), plane) != m_planes_u.end()) ? 1 : 0; }
int MMT_Parameters::is_v(int plane){ return (std::find(m_planes_v.begin(), m_planes_v.end(), plane) != m_planes_v.end()) ? 1 : 0; }

vector<int> MMT_Parameters::q_planes(const string& type) const{
  if(type.length()!=1) exit(0);
  if(type.compare("x")!=0&&type.compare("u")!=0&&type.compare("v")!=0){
    ATH_MSG_WARNING("Unsupported plane type " << type << " in q_planes...aborting....\n");
    exit(0);
  }
  vector<int> q_planes;
  for(unsigned int ip=0;ip<m_setup.size();ip++){
    if(m_setup.compare(ip,1,type)==0){ q_planes.push_back(ip);
      }
  }
  return q_planes;
}

par_par MMT_Parameters::param_par() const{
  return par_par(m_h.getFixed(),m_CT_x,m_CT_uv,m_uv_error.getFixed(),m_setup,m_islarge,m_dlm_new,m_genbg,m_chargeThreshold,m_misal,m_correct,m_val_tbl);
}

double MMT_Parameters::y_from_eta_wedge(double eta,int plane)const{
  //assumes wedge geometry--average x^2, is 1/3 y^2*tan^2(stereo_degree), for eta/y correspondence
  double z=m_z_nominal[plane].getFixed(),zeta=TMath::DegToRad()*(0.5*m_stereo_degree.getFixed());
  return z*tan(2*atan(exp(-1.*eta)))/sqrt(1+tan(zeta)*tan(zeta)/3.);
}

double MMT_Parameters::eta_wedge_from_y(double y,int plane)const{
  double z=m_z_nominal[plane].getFixed(),zeta=TMath::DegToRad()*(0.5*m_stereo_degree.getFixed());
  return -1.*log(tan(0.5*atan(y/z*sqrt(1+tan(zeta)*tan(zeta)/3.))));
}

int MMT_Parameters::ybin(double y,int plane)const{
  double base=m_ybases[plane].front().getFixed(),seg_len=m_h1.getFixed()/m_ybins;
  //if it's over, just keep it...for now--if it's below keep it, too: perhaps necessary for larger slope road sizes
  //update: DON'T keep the below ones....terrible things for drs
  int the_bin=m_ybases.front().size()-1;
  for(int i=0;i<=m_ybins;i++){
    if(y<base+seg_len*i)return i-1;
  }
  return the_bin;

  double eta_min_y=eta_wedge_from_y(base+m_h1.getFixed(),plane),eta_max_y=eta_wedge_from_y(base,plane),eta_y=eta_wedge_from_y(y,plane);
  double segment_length=(eta_max_y-eta_min_y)/m_ybins;
  for(int i=m_ybins;i>=0;i--){
    if(eta_y>(eta_max_y-segment_length*i)){
      return i-1;
    }
  }
}

int MMT_Parameters::ybin(float32fixed<18> y,int plane)const{
  return ybin(y.getFixed()*MMTStructConst,plane);
}
int MMT_Parameters::ybin(float32fixed<m_yzdex> y,int plane)const{
  return ybin(y.getFixed()*MMTStructConst,plane);
}

//make the local slope ab for all ybins
//track percent in number of same bin
//maybe do a th2d scatter in bins for all/for a given ybin in plane 0

void MMT_Parameters::fill_full_Ak_Bk(){
  m_AB_k_local.clear();
  vector<int> x_planes=q_planes("x"); int nx=x_planes.size();
  vector<int> variable_key(nx,-1);//a grid with all possible
  do{
    m_AB_k_local[variable_key]=ak_bk_hit_bins(variable_key);
  }while(!toggle_key(variable_key,-1,m_ybins-1));
}

//The problem is this: we don't a priori know how many X planes there will be (configured at run time)
//so, we don't know how many Ak, Bk constants we'll need (it's n_Xplanes * ybins)
//The idea, then is to run through the possible values like a key with n_Xplanes digits, each of which can take values in [-1,ybins-1]. (-1 is the no hit here value)
//We need to toggle through all possible values; the idea is to start with the lowest entry (-1,-1,...,-1) and increment right to left
//We start at the rightmost entry and check values until we get a value != highest possible value and then
// increment this up, resetting all values to the right of it to lo_value.
bool MMT_Parameters::toggle_key(vector<int>& key,int lo_value,int hi_value)const{
  for(int idex=(int)(key.size()-1);idex>=0;idex--){
    if(key[idex]==hi_value) continue;
    else{
      key[idex]++;
      for(unsigned int reset_dex=idex+1;reset_dex<key.size();reset_dex++) key[reset_dex]=lo_value;
      return false;
    }
  }
  return true;
}

int MMT_Parameters::eta_bin(double theta) const{
  int ebin=-999; double eta=-log(tan(0.5*theta));
  if(theta==-999||theta==-16)return -1;
  for(int i=0;i<=m_n_etabins;i++){
    if(eta<m_etabins[i]){
      ebin=i-1;
      break;
    }
  }
  //we want the histograms binned on truth eta
  //event selection eta cuts are done on theta_ip, not theta_pos
  if(ebin==-1) return 0;
  else if(ebin==-999) return m_n_etabins-1;
  return ebin;
}

string MMT_Parameters::eta_str(int eta) const{
  ostringstream what; what<<"_eta";
  if(eta>=0&&eta<m_n_etabins) what<<m_etabins[eta]<<"_"<<m_etabins[eta+1];
  else what << "all";
  return what.str();
}

int MMT_Parameters::phi_bin(double phi) const{
  int pbin=-999;
  if(phi==-999||phi==-16)return m_n_phibins*0.5;
  for(int i=0;i<=m_n_phibins;i++){
    if(phi<m_phibins[i]){
      pbin=i-1;
      break;
    }
  }
  //we want the histograms binned on truth phi
  //event selection eta cuts are done on phi_ip, not phi_pos
  if(pbin==-1) return 0;
  else if(pbin==-999) return m_n_phibins-1;
  return pbin;
}

string MMT_Parameters::phi_str(int phi) const{
  ostringstream what; what<<"_phi";
  if(phi>=0&&phi<m_n_phibins) what<<m_phibins[phi]<<"_"<<m_phibins[phi+1];
  else what << "all";
  return what.str();
}

//calculate A_k and B_k
//these are NOT the constants defined in the note--firstly, they use an altered length scale for fixed point
//B_k is otherwise the same
//A_k is the average z
pair<double,double> MMT_Parameters::ak_bk_hit_bins(const vector<int>& hits)const{
  vector<int> x_planes=q_planes("x");
  assert(hits.size()==x_planes.size());
  int nhits=0; double sum_x=0,sum_xx=0;
  for(int ih=0;ih<(int)(hits.size());ih++){
    if(hits[ih]<0||hits[ih]>=m_ybins)continue;
    double addme=m_z_large[hits[ih]][x_planes[ih]].getFixed()/MMTStructConst;
    sum_x  += addme;
    sum_xx += addme*addme;
    nhits++;
  }
  double diff = nhits*sum_xx-sum_x*sum_x;
  return pair<double,double>(1.*nhits/sum_x, 1.*sum_x/diff);
}

void MMT_Parameters::fill_slims(){
  m_Ak_local_slim=vector<vector<vector<float32fixed<m_zbardex> > > >(11,vector<vector<float32fixed<m_zbardex> > >(m_ybins,vector<float32fixed<m_zbardex> >()));
  m_Bk_local_slim=vector<vector<vector<float32fixed<m_bkdex> > > >(11,vector<vector<float32fixed<m_bkdex> > >(m_ybins,vector<float32fixed<m_bkdex> >()));
  for(int xdex=0;xdex<11;xdex++){
    vector<bool>xhits(lcl_int_to_xhits(xdex));
    int ntru=0;
    for(int k=0;k<(int)xhits.size();k++)ntru+=xhits[k];
    ATH_MSG_DEBUG("xdex="<<xdex<<" ("<<ntru<<" hits)");
    for(int ybin=0;ybin<m_ybins;ybin++){
      ATH_MSG_DEBUG("\tybin="<<ybin );
      if(ybin==m_ybins-1)ntru=1;
      for(int which=0;which<ntru;which++){
 	ATH_MSG_DEBUG("\t\twhich="<<which);
	vector<int>indices(indices_to_key(xdex,ybin,which));
  for(int i=0;i<(int)indices.size();i++)  ATH_MSG_DEBUG(indices[i]);
	pair<double,double>howdy(ak_bk_hit_bins(indices));
 	ATH_MSG_DEBUG(setprecision(8)<<"---akbk.first="<<howdy.first<<", second="<<howdy.second);
	m_Ak_local_slim[xdex][ybin].push_back(howdy.first);
	m_Bk_local_slim[xdex][ybin].push_back(howdy.second);
      }
    }
  }
}

void MMT_Parameters::fill_crep_table(const string&dir,const string&tag){
  par_par bill=param_par();
  bill.ctx=2;bill.ctuv=1;
  bill.corr.type=0;
  double fudge_factor=(m_correct.translate.Z()!=0||m_correct.rotate.X()!=0?0.5:1.);
  int nk=nsimmax_1d();
  ostringstream crep_nom;
  crep_nom<<dir<<(dir.substr(dir.length()-1)=="/"?"":"/")<<"pcrep"<<bill.print_pars()<<"_"<<tag<<".txt";
  std::ifstream crep(crep_nom.str().c_str());
  m_crep_table=vector<vector<vector<vector<float> > > >(m_n_etabins,vector<vector<vector<float> > >(m_n_phibins,vector<vector<float> >(nk,vector<float>(3,0))));
  if(crep.good()){
    for(int i=0;i<m_n_etabins;i++){
      string estr=eta_str(i);
      for(int j=0;j<m_n_phibins;j++){
	string pstr=phi_str(i);
	for(int k=0;k<nk;k++){
	string title;double the,phi,dth;
	crep>>title;
	if(title!=estr+pstr+index_to_hit_str(k)){
	  ATH_MSG_WARNING("Something's wrong with your simulation-based correct read-in...you want entries for "<<estr+pstr+index_to_hit_str(k)<<", but you got "<<title<<" in "<<crep_nom.str());
	  exit(2);
	}
	crep>>the>>phi>>dth;
	m_crep_table[i][j][k][0]=the*fudge_factor;m_crep_table[i][j][k][1]=phi;m_crep_table[i][j][k][2]=dth*fudge_factor;
	}
      }
    }
  }
}

void MMT_Parameters::fill_yzmod(){
  vector<int> x_planes=q_planes("x");//this tells us which planes are x planes
  int nxp=x_planes.size();
  m_ymod=vector<vector<vector<float32fixed<m_yzdex> > > >(m_n_etabins,vector<vector<float32fixed<m_yzdex> > >(m_n_phibins,vector<float32fixed<m_yzdex> >(nxp,float32fixed<m_yzdex>(0.))));
  m_zmod=vector<vector<vector<float32fixed<m_yzdex> > > >(m_n_etabins,vector<vector<float32fixed<m_yzdex> > >(m_n_phibins,vector<float32fixed<m_yzdex> >(nxp,float32fixed<m_yzdex>(0.))));
  double pbump=0.5*(m_maximum_large_phi.getFixed()-m_minimum_large_phi.getFixed())/(m_n_phibins);
  for(int et=0;et<m_n_etabins;et++){
    double theta=2*atan(exp(-1.*m_etabins[et]));
    for(int ph=0;ph<m_n_phibins;ph++){
      double phi=m_phibins[ph]+pbump;
      for(int pl=0;pl<nxp;pl++){
        if(m_correct.type!=2||x_planes[pl]>3)continue;//if we don't correct or if it's the back multiplet, just leave these as zero

        double yadd=0.;
        double zadd=0.;
        double zflt=m_z_nominal[x_planes[pl]].getFixed();
        double x=zflt*tan(theta)*sin(phi);
        double yflt=zflt*tan(theta)*cos(phi);
        double yup=yflt-m_ybases[x_planes[pl]][0].getFixed();

        double alpha=m_correct.rotate.Z();
        double beta=m_correct.rotate.Y();
        // double gamma=correct.rotate.X();
        //gamma--do here if necessary...problematic if so

        //beta angle--rotation around the y axis; the x coordinate or phi taken care of in correction to slope road limits in x in constructor
        //z is changed, and so y must be scaled (should it?)
        zadd-=tan(beta)*tan(theta)*sin(phi)*zflt/MMTStructConst;
        yadd-=tan(beta)*tan(theta)*sin(phi)*yflt/MMTStructConst;

        //alpha angle--rotation around the z axis; z unchanged, y the expected rotation (final rotation taken care of at start)
        yadd+=((cos(alpha)-1.)*yup+x*sin(alpha))/MMTStructConst;

        //add the entries
        m_ymod[et][ph][pl]=float32fixed<m_yzdex>(yadd);m_zmod[et][ph][pl]=float32fixed<m_yzdex>(zadd);
      }
    }
  }
}

void MMT_Parameters::index_key_test(){
  for(int xdex=0;xdex<11;xdex++){
    vector<bool>xhits(lcl_int_to_xhits(xdex));
    int ntru=0;
    for(int k=0;k<(int)xhits.size();k++)ntru+=xhits[k];
    for(int ybin=0;ybin<m_ybins;ybin++){
      if(ybin==m_ybins-1)ntru=1;
      for(int which=0;which<ntru;which++){
	ATH_MSG_DEBUG("Let's start with ["<<xdex<<"]["<<ybin<<"]["<<which<<"] makes ");
	vector<int>key(indices_to_key(xdex,ybin,which));
	for(int i=0;i<(int)key.size();i++) ATH_MSG_DEBUG(key[i]<<" ");
	int x,y,w; key_to_indices(key,x,y,w);
	ATH_MSG_DEBUG("...and back to ["<<x<<"]["<<y<<"]["<<w<<"]  ");
      }
    }
  }
}


void MMT_Parameters::key_to_indices(const vector<int>& key,int& xdex,int& ybin,int& which)const{
  vector<bool> boogah;vector<int> was_hit;
  for(unsigned int i=0;i<key.size();i++){
    boogah.push_back(key[i]>=0&&key[i]<m_ybins);
    if(boogah.back())was_hit.push_back(key[i]);
  }
  bool even=true;int dev_pos=0,dev_bin=-1;
  for(unsigned int i=0;i<was_hit.size();i++){
    if(was_hit[i]!=was_hit.front()){
      if(even){
	dev_pos=i;
	if(was_hit[i]!=was_hit.front()+1){
	  dev_pos=-2;break;
	}
	else dev_bin=was_hit[i];
      }
      else if(was_hit[i]!=dev_bin){
	dev_pos=-2;break;
      }
      even=false;
    }
  }
  if(dev_pos<0||dev_pos>=(int)boogah.size())which=0;//if the track isn't upward going, make the single bin assumption
  else which=dev_pos;
  xdex=xhits_to_lcl_int(boogah);
  if(was_hit.size()==0) was_hit.push_back(-999);
  ybin=was_hit.front();
}

vector<int> MMT_Parameters::indices_to_key(int xdex,int ybin,int which)const{
  vector<bool> xhits=lcl_int_to_xhits(xdex);
  vector<int> key(xhits.size(),-1);
  int count=0;int raise_it=(which==0?xhits.size():which);
  for(unsigned int ix=0;ix<xhits.size();ix++){
    if(xhits[ix]){
      key[ix]=ybin;
      if(count>=raise_it)key[ix]++;
      count++;
    }
  }
  return key;
}

void MMT_Parameters::Local_Slope_A_B(){
  fill_full_Ak_Bk();
  fill_slims();
}

void MMT_Parameters::Slope_Components_ROI(){
  //these are mm/mm i.e. not units of strip #

  m_Slope_to_ROI=vector<vector<vector<float32fixed<4> > > >(m_n_y,vector<vector<float32fixed<4> > >(m_n_x,vector<float32fixed<4> >(2,float32fixed<4>(0.))));
  for(int ix=0;ix<m_n_x;ix++){
    for(int jy=0;jy<m_n_y;jy++){
      int xdex=ix,ydex=jy+1;
      if(xdex==0)xdex++;

      double m_x = ( m_x_min + m_h_mx * xdex ).getFixed();
      double m_y = ( m_y_min + m_h_my * ydex ).getFixed();

      double theta = atan(sqrt(m_x*m_x+m_y*m_y));
      double phi=atan2(m_y,m_x);
      if(m_minimum_large_phi>phi || m_maximum_large_phi<phi) phi=999;
      if(m_minimum_large_theta>theta || m_maximum_large_theta<theta) theta=0;
      m_Slope_to_ROI[jy][ix][0] = theta;
      m_Slope_to_ROI[jy][ix][1] = phi;
    }
  }
  ATH_MSG_DEBUG( "return Slope_Components_ROI" );
}

void MMT_Parameters::Delta_theta_optimization_LG(){
  m_DT_Factors.clear();
  double a=1.;   //sin(pi/2+TMath::DegToRad(28/4));  //sin(phi);
  int number_LG_regions = 256;
  double LG_min = 0;
  double LG_max = 0.5;
  double LG_region_width = (LG_max - LG_min)/number_LG_regions;//LG_min=0 means e no neg slopes

  for(int ilgr=0; ilgr<number_LG_regions; ilgr++){
    vector<float32fixed<2> > tmpVector;
    tmpVector.push_back(LG_min+LG_region_width *ilgr);//LG
    tmpVector.push_back(  float32fixed<2>(1.) / (tmpVector.front()/a  +  a));//mult_factor
    m_DT_Factors.push_back(tmpVector);
  }
  ATH_MSG_DEBUG( "return Delta_theta_optimization_LG" );
}

int MMT_Parameters::xhits_to_lcl_int(const vector<bool>& xhits) const{
  if(xhits.size()!=4){
    ATH_MSG_WARNING("There should be 4 xplanes, only "<<xhits.size()<<" in the xhit vector given to local_slope_index()\n"); exit(99);
  }
  if(xhits[0]&& xhits[1]&& xhits[2]&& xhits[3]) return 0;
  else if( xhits[0]&& xhits[1]&& xhits[2]&&!xhits[3]) return 1;
  else if( xhits[0]&& xhits[1]&&!xhits[2]&& xhits[3]) return 2;
  else if( xhits[0]&&!xhits[1]&& xhits[2]&& xhits[3]) return 3;
  else if(!xhits[0]&& xhits[1]&& xhits[2]&& xhits[3]) return 4;
  else if( xhits[0]&& xhits[1]&&!xhits[2]&&!xhits[3]) return 5;
  else if( xhits[0]&&!xhits[1]&& xhits[2]&&!xhits[3]) return 6;
  else if( xhits[0]&&!xhits[1]&&!xhits[2]&& xhits[3]) return 7;
  else if(!xhits[0]&& xhits[1]&& xhits[2]&&!xhits[3]) return 8;
  else if(!xhits[0]&& xhits[1]&&!xhits[2]&& xhits[3]) return 9;
  else if(!xhits[0]&&!xhits[1]&& xhits[2]&& xhits[3]) return 10;
  return -999;
}

int MMT_Parameters::nsimmax_1d()const{
  return pow(2.,m_setup.size());
}

int MMT_Parameters::bool_to_index(const vector<bool>&track)const{
  assert(track.size()==m_setup.size());
  int index=0;
  for(int plane=0;plane<(int)track.size();plane++){
    if(!track[plane])continue;
    index = index | int(pow(2.,plane));
  }
  return index;
}

vector<bool> MMT_Parameters::index_to_bool(int index)const{
  vector<bool>code(m_setup.size(),false);
  for(int i=0;i<(int)code.size();i++)code[i]=(index&int(pow(2.,i)));
  return code;
}

string MMT_Parameters::index_to_hit_str(int index)const{
  return bool_to_hit_str(index_to_bool(index));
}

string MMT_Parameters::bool_to_hit_str(const vector<bool>&track)const{
  ostringstream ok;ok<<"_ht";
  for(int i=0;i<(int)track.size();i++)ok<<track[i];
  return ok.str();
}

vector<bool> MMT_Parameters::lcl_int_to_xhits(int lcl_int)const{
  vector<bool> xhits(4,true);
  if(lcl_int<0||lcl_int>10){
    ATH_MSG_WARNING("Wherefore dost thou chooseth the hits of planes of X for thy lcl_int "<<lcl_int<<"?  'Tis not in [0,10]!"); exit(-99);
  }
  if(lcl_int==0) return xhits;
  if(lcl_int==1||lcl_int==5||lcl_int==6||lcl_int==8)xhits[3]=false;
  if(lcl_int==2||lcl_int==5||lcl_int==7||lcl_int==9)xhits[2]=false;
  if(lcl_int==3||lcl_int==6||lcl_int==7||lcl_int==10)xhits[1]=false;
  if(lcl_int==4||lcl_int==8||lcl_int==9||lcl_int==10)xhits[0]=false;
  return xhits;
}

string MMT_Parameters::lcl_int_to_xhit_str(int lcl_int)const{
  vector<bool>hits=lcl_int_to_xhits(lcl_int);
  ostringstream out;out<<"_xht";
  for(int i=0;i<(int)hits.size();i++)out<<hits[i];
  return out.str();
}

evInf_entry::evInf_entry(int event,int pdg,double e,double p,double ieta,double peta,double eeta,double iphi,double pphi,double ephi,double ithe,double pthe,double ethe,double dth,
			 int trn,int mun,const TVector3& tex,int troi,int antev,int postv,int nxh,int nuvh,int nxbg,int nuvbg,double adt,int difx,
			 int difuv,bool cut,bool bad)
  :athena_event(event),pdg_id(pdg),E(e),pt(p),eta_ip(ieta),eta_pos(peta),eta_ent(eeta),phi_ip(iphi),phi_pos(pphi),phi_ent(ephi),theta_ip(ithe),theta_pos(pthe),theta_ent(ethe),
   dtheta(dth),truth_n(trn),mu_n(mun),vertex(tex),truth_roi(troi),N_hits_preVMM(antev),N_hits_postVMM(postv),N_X_hits(nxh),N_UV_hits(nuvh),NX_bg_preVMM(nxbg),
   NUV_bg_preVMM(nuvbg),avg_drift_time(adt),max_less_min_hitsteps_X(difx),max_less_min_hitsteps_UV(difuv),pass_cut(cut),bad_wedge(bad) {}

void evInf_entry::print() const{
}

evFit_entry::evFit_entry(int event,float32fixed<4> fthe,float32fixed<4> fphi,float32fixed<2> fdth,int roi,int xhit,int uvhit,int bgx,int bguv,float32fixed<2> dth_nd,int hc,int tph,int bgph)
  :athena_event(event),fit_theta(fthe),fit_phi(fphi),fit_dtheta(fdth),fit_roi(roi),X_hits_in_fit(xhit),UV_hits_in_fit(uvhit),bg_X_fit(bgx),
   bg_UV_fit(bguv),dtheta_nodiv(dth_nd),hcode(hc),truth_planes_hit(tph),bg_planes_hit(bgph) {}

void evFit_entry::print()const{
}

hitData_key::hitData_key(int bct, double t, double gt, int vmm, int ev):BC_time(bct),time(t),gtime(gt),VMM_chip(vmm),event(ev) {}

bool hitData_key::operator==(const hitData_key& rhs) const{
  if(this->BC_time==rhs.BC_time&&this->time==rhs.time&&this->gtime==rhs.gtime&&this->VMM_chip==rhs.VMM_chip&&this->event==rhs.event) return true;
  return false;
}

bool hitData_key::operator!=(const hitData_key& rhs) const{
  return !(*this==rhs);
}
bool hitData_key::operator<(const hitData_key& rhs) const{
  if(this->BC_time<rhs.BC_time) return true;
  else if(this->BC_time==rhs.BC_time&&this->time<rhs.time) return true;
  else if(this->BC_time==rhs.BC_time&&this->time==rhs.time&&this->gtime<rhs.gtime) return true;
  else if(this->BC_time==rhs.BC_time&&this->time==rhs.time&&this->gtime==rhs.gtime&&this->VMM_chip<rhs.VMM_chip) return true;
  else if(this->BC_time==rhs.BC_time&&this->time==rhs.time&&this->gtime==rhs.gtime&&this->VMM_chip==rhs.VMM_chip&&this->event<rhs.event) return true;
  return false;
}

bool hitData_key::operator>(const hitData_key& rhs) const{
  if(this->BC_time>rhs.BC_time) return true;
  else if(this->BC_time==rhs.BC_time&&this->time>rhs.time) return true;
  else if(this->BC_time==rhs.BC_time&&this->time==rhs.time&&this->gtime>rhs.gtime) return true;
  else if(this->BC_time==rhs.BC_time&&this->time==rhs.time&&this->gtime==rhs.gtime&&this->VMM_chip>rhs.VMM_chip) return true;
  else if(this->BC_time==rhs.BC_time&&this->time==rhs.time&&this->gtime==rhs.gtime&&this->VMM_chip==rhs.VMM_chip&&this->event>rhs.event) return true;
  return false;
}

bool hitData_key::operator<=(const hitData_key& rhs) const{
  return(*this<rhs || *this==rhs);
}

bool hitData_key::operator>=(const hitData_key& rhs) const{
  return(*this>rhs || *this==rhs);
}

string hitData_key::hdr()const{
  ostringstream out;
  out<<setw(9)<<"BC_t"<<setw(9)<<"t"<<setw(9)<<"g_t"<<setw(9)<<"VMM"<<setw(9)<<"event";
  return out.str();
}
string hitData_key::str()const{
  ostringstream out;
  out<<setw(9)<<this->BC_time<<setw(9)<<this->time<<setw(9)<<this->gtime<<setw(9)<<this->VMM_chip<<setw(9)<<this->event;
  return out.str();
}
void hitData_key::print()const{
  // ATH_MSG_INFO("^^^^^^hitData_key"<<hdr());
}


hitData_info::hitData_info(int pl,int station_eta,int strip,MMT_Parameters *m_par,const TVector3&tru,double tpos,double ppos):plane(pl){
  (void) tru;
  //The idea here is to calculate/assign a y and a z to a given hit based on its pl/station/strip, the geometry of the detector (in m_par), and misalignment based on position.
  //We start by assigning the plane dependent strip width (the stereo planes come in skew and so get divided by cos(stereo_angle)
  char schar=m_par->m_setup[plane];
  bool horizontal=(schar=='x'||schar=='X');
  double swidth=m_par->m_strip_width.getFixed();
  double base=m_par->m_ybases[plane][station_eta-1].getFixed();
  if(!horizontal)swidth/=cos(TMath::DegToRad()*(m_par->m_stereo_degree.getFixed()));
  //Next, we initialize some constants--y will eventually be calculated as y=base+scale*(width*strip+delta_y(misalignment,correction)).
  //The correction portion of delta_y is done in in the calculations on the ybases object in m_par
  //yup, or "y up" is the portion of y above the base of the plane (i.e. in the detector)
  double delta_y=mis_dy(pl,m_par,tpos,ppos),yup_scale=1.;
  double yflt=-999;
  yflt=base+(strip*swidth+delta_y)*yup_scale;
  //Note that ybin should be calculated on the basis of a misaligned y, not a corrected one (to get the position on the wedge just right),
  //but the difference for this part of the calculation should be negligible (an effect of <~ misal/(bin length) ~ (5 mm/(3600 mm/15)) ~ %.
  int bin=m_par->ybin(yflt,plane);

  //We have here both the "perfect" and binned z calculations/lookups; the commented out one is there for reference
  double zflt=m_par->m_z_large[bin][plane].getFixed();
//   if(m_par->correct.type==2&&plane<4&&m_par->correct.rotate.X()!=0){
//     // double fltyup=(base+(strip*swidth+delta_y)-planebase);
//     double angle=m_par->correct.rotate.X();
// //     yflt=planebase+fltyup*cos(angle);
// //     zflt=zplane+0.5*fltyup*sin(angle)*cos(angle)*(1-tan(angle)*base/zplane)/(1-tan(angle)*1.*yflt/zplane);
// //   }
//   y = yflt / MMTStructConst;
//   z = zflt / MMTStructConst;

  slope =  (zflt!=0.) ? yflt / zflt : 0.;

}

double hitData_info::mis_dy(int plane,MMT_Parameters *m_par,double tpos,double ppos)const{
  if(m_par->m_misal.type!=1||plane>3)return 0.;
  double zplane=m_par->m_z_nominal[plane].getFixed();
  double base=m_par->m_ybases[plane].front().getFixed();

  double s_x0=zplane*tan(tpos)*sin(ppos);
  double s_y0=zplane*tan(tpos)*cos(ppos);//initial position

  double hats_z0=cos(tpos),hats_x0=sin(tpos)*sin(ppos),hats_y0=sin(tpos)*cos(ppos);//muon track unit vector
  double zeta_y0=s_y0-base;//height in y in the wedge local coordinates--this is what we have to compare in the end
  double alpha=m_par->m_misal.rotate.Z(),beta=m_par->m_misal.rotate.Y(),gamma=m_par->m_misal.rotate.X();//rotation angles
  double ds=m_par->m_misal.translate.X(),dz=m_par->m_misal.translate.Y(),dt=-1.*m_par->m_misal.translate.Z();//t comes in -z
  double O_bxf=ds,O_byf=base+dz,O_bzf=zplane+dt;//position of bottom of the wedge in global coordinates; subtract this from the final y position (s_yf) for final zeta (comparison to add to y position)
  double yhat_x=-1.*sin(alpha)*cos(beta),yhat_y=(cos(alpha)*cos(gamma)-sin(alpha)*sin(beta)*sin(gamma)),yhat_z=(cos(alpha)*sin(gamma)+sin(alpha)*sin(beta)*cos(gamma));//new y direction after rotations; horizontal case
  char schar=m_par->m_setup[plane];
  if(!(schar=='x'||schar=='X')){
    //if we're in a stereo plane, calculate different coefficients.
    double omega=TMath::DegToRad()*(m_par->m_stereo_degree.getFixed());
    double pm=(schar=='u'||schar=='U'?1.:-1.);
    yhat_x=pm*cos(alpha)*cos(beta)*sin(omega)-sin(alpha)*cos(beta)*cos(omega);
    yhat_y=pm*sin(omega)*(sin(alpha)*cos(gamma)+cos(alpha)*sin(beta)*sin(gamma))+cos(omega)*(cos(alpha)*cos(gamma)-sin(alpha)*sin(beta)*sin(gamma));
    yhat_z=pm*sin(omega)*(sin(alpha)*sin(gamma)-cos(alpha)*sin(beta)*cos(gamma))+cos(omega)*(cos(alpha)*sin(gamma)+sin(alpha)*sin(beta)*cos(gamma));
    zeta_y0=pm*sin(omega)*s_x0+cos(omega)*(s_y0-base);
  }
  double kprime=(sin(beta)*O_bxf-cos(beta)*sin(gamma)*O_byf+cos(beta)*cos(gamma)*O_bzf)/(sin(beta)*hats_x0-cos(beta)*sin(gamma)*hats_y0+cos(beta)*cos(gamma)*hats_z0);
  double zeta_xf=kprime*hats_x0-O_bxf,zeta_yf=kprime*hats_y0-O_byf,zeta_zf=kprime*hats_z0-O_bzf;
  double zetayf_yhatf=zeta_xf*yhat_x+zeta_yf*yhat_y+zeta_zf*yhat_z;
  return zetayf_yhatf-zeta_y0;
}

hitData_info::hitData_info(int pl,double _y,double _z):plane(pl),y(_y),z(_z){
  if(_z==0||_z==-999)slope=-999;
  else slope=_y/_z;
}

string hitData_info::hdr()const{
  ostringstream out;
  out<<setw(9)<<"plane"<<setw(9)<<"y"<<setw(9)<<"z"<<setw(9)<<"slope";
  return out.str();
}
string hitData_info::str()const{
  ostringstream out;
  out<<setprecision(4)<<setw(9)<<plane<<setw(9)<<y.getFixed()<<setw(9)<<z.getFixed()<<setw(9)<<slope.getFixed();
  return out.str();
}
void hitData_info::print()const{
  // ATH_MSG_INFO("------- hitData_info: "<<hdr() );
}

bool hitData_info::operator==(const hitData_info& rhs) const{
  if(this->plane==rhs.plane&&this->y==rhs.y&&this->z==rhs.z&&this->slope==rhs.slope)return true;
  return false;
}

Hit::Hit(const hitData_key& k,const hitData_info&i):key(k),info(i) {}

bool Hit::operator==(const Hit& rhs) const{
  if(this->key==rhs.key&&this->info==rhs.info)return true;
  return false;
}

void Hit::print_track(const vector<Hit>& track) const{
  (void) track;
  // ATH_MSG_INFO(track.front().key.hdr()<<track.front().info.hdr());
  // for(unsigned int i=0; i<track.size(); i++) ATH_MSG_INFO( track[i].key.hdr()<<track[i].info.hdr() ) ;
}

void Hit::print() const{
  // ATH_MSG_INFO( "%%%%%%HIT%%%%%%");
  // ATH_MSG_INFO( key.hdr()<<info.hdr());
  // ATH_MSG_INFO( key.str()<<info.str());
}

hitData_entry::hitData_entry(int ev, double gt, double q, int vmm, int pl, int st, int est, double tr_the, double tru_phi,
		       bool q_tbg, int bct, double t, const TVector3& tru, const TVector3& rec,
		       double fit_the, double fit_ph, double fit_dth, double tru_dth,// double tru_thl, double tru_thg,
		       double mxg, double mug, double mvg, double mxl, double _mx, double _my, int _roi):
  event(ev),gtime(gt),charge(q),VMM_chip(vmm),plane(pl),strip(st),station_eta(est),tru_theta_ip(tr_the),tru_phi_ip(tru_phi),truth_nbg(q_tbg),
  BC_time(bct),time(t),truth(tru),recon(rec),fit_theta(fit_the),fit_phi(fit_ph),fit_dtheta(fit_dth),tru_dtheta(tru_dth),
  /*tru_theta_local(tru_thl),tru_theta_global(tru_thg),*/M_x_global(mxg),M_u_global(mug),M_v_global(mvg),M_x_local(mxl),mx(_mx),my(_my),roi(_roi) {}

Hit hitData_entry::entry_hit(MMT_Parameters *m_par)const{
  return Hit(entry_key(),entry_info(m_par));
}
hitData_key hitData_entry::entry_key() const{
  return hitData_key(BC_time,time,gtime,VMM_chip,event);
}

hitData_info hitData_entry::entry_info(MMT_Parameters *m_par)const{
  hitData_info spade(plane,station_eta,strip,m_par,recon,tru_theta_ip,tru_phi_ip);//truth or recon? doesn't matter too much--it's for misalignment
//   spade.y=recon.Y();
  return spade;
}
void hitData_entry::fit_fill(float32fixed<4> fthe,float32fixed<4> fphi, float32fixed<2> fdth, float32fixed<2> mxg, float32fixed<2> mug, float32fixed<2> mvg, float32fixed<2> mxl, float32fixed<2> m_x, float32fixed<2> m_y, int king){
  this->fit_theta=fthe; this->fit_phi=fphi; this->fit_dtheta=fdth; this->M_x_global=mxg; this->M_u_global=mug; this->M_v_global=mvg; this->M_x_local=mxl; this->mx=m_x; this->my=m_y; this->roi=king;
}

void hitData_entry::print() const{
  // ATH_MSG_INFO( "%%%%%%%%%%%%%%%%HDST_ENTRY%%%%%%%%%%%%%%%%%%"<<endl
  //     <<"(Event,BC_time,time): ("<<event<<","<<BC_time<<","<<time<<"), "<<(truth_nbg?"truth":"bg")<<", charge: "<<charge<<endl
  //     <<"Wedge Coord---plane: "<<plane<<", strip: "<<strip<<", est: "<<station_eta<<", vmm: "<<VMM_chip<<", ip theta: "<<tru_theta_ip<<", ip phi: "<<tru_phi_ip<<endl
  //     <<"truth angles---dtheta: "<<tru_dtheta<<endl//", theta(loc): "<<tru_theta_local<<", theta(glo): "<<tru_theta_global<<endl
  //     <<"Truth vertex: \n");
  // truth.Print();
  // ATH_MSG_INFO( "Recon vertex: \n");
  // recon.Print();
  // ATH_MSG_INFO( "FIT---roi: "<<roi<<endl
  //     <<"        angles---theta: "<<fit_theta.getValue()<<", phi: "<<fit_phi.getValue()<<", dtheta: "<<fit_dtheta.getValue()<<endl
  //     <<"slopes, global---x: "<<M_x_global.getValue()<<", u: "<<M_u_global.getValue()<<", v: "<<M_v_global.getValue()<<endl
  //     <<"slopes,  other---x(loc): "<<M_x_local.getValue()<<", x(coord): "<<mx.getValue()<<", y(coord): "<<my.getValue()<<endl
  //     <<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
}

finder_entry::finder_entry(bool _is_hit, int _clock,const Hit& k):
  is_hit(_is_hit), clock(_clock), hit(k) {}


bool finder_entry::operator==(const finder_entry& merp) const{
  if(merp.is_hit==this->is_hit&&merp.clock==this->clock&&this->hit==merp.hit) return true;
  else return false;
}
bool finder_entry::operator!=(const finder_entry& merp) const{
  return !(*this!=merp);
}

ROI::ROI(double _theta, double _phi, double _m_x, double _m_y, int _roi):
  theta(_theta), phi(_phi), m_x(_m_x), m_y(_m_y), roi(_roi) {}


athena_header::athena_header(const TLorentzVector& par, int tpn, double etp, double ete, double php, double phe, int mun, const TVector3& ver):
  the_part(par),trupart_n(tpn),etapos(etp),etaent(ete),phipos(php),phient(phe),muent_n(mun),vertex(ver) {}

digitWrapper::digitWrapper(const MmDigit* digit,
                           double tmpGTime,
                           const TVector3& truthLPos,
                           const TVector3& stripLPos,
                           const TVector3& stripGPos
                           ):
  digit(digit),
  gTime(tmpGTime),
  truth_lpos(truthLPos),
  strip_lpos(stripLPos),
  strip_gpos(stripGPos){}

track_address::track_address(int bct,bool big,int wed,int pl,int sh,const TVector3& chr):
  BC(bct),islarge(big),wedge(wed),plane(pl),strip_hit(sh),cart_hit(chr) {}
