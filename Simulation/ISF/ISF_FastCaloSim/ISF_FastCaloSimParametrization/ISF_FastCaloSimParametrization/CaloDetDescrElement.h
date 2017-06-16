/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOGEODETECTORELEMENT_H
#define CALOGEODETECTORELEMENT_H

class CaloGeometry;

#include "Identifier/Identifier.h"
#include <iostream>
#include <iomanip>
#include <cmath>

class CaloGeoDetDescrElement
{
 friend class CaloGeometry;
 public:
  CaloGeoDetDescrElement() {
    m_identify = 0;
    m_hash_id = 0;
    m_calosample = 0;
    m_eta = 0;
    m_phi = 0;
    m_deta = 0;
    m_dphi = 0;
    m_r = 0;
    m_eta_raw = 0;
    m_phi_raw = 0;
    m_r_raw = 0;
    m_dr = 0;
    m_x = 0;
    m_y = 0;
    m_z = 0;
    m_x_raw = 0;
    m_y_raw = 0;
    m_z_raw = 0;
    m_dx = 0;
    m_dy = 0;
    m_dz = 0;
  };

  /** @brief virtual destructor
   */
  virtual ~CaloGeoDetDescrElement() {};

  /** @brief cell eta
   */
  float eta() const;
  /** @brief cell phi
   */
  float phi() const;
  /** @brief cell r
   */
  float r() const;
  /** @brief cell eta_raw
   */
  float eta_raw() const;
  /** @brief cell phi_raw
   */
  float phi_raw() const;
  /** @brief cell r_raw
   */
  float r_raw() const;
  /** @brief cell dphi
   */
  float dphi() const;
  /** @brief cell deta
   */
  float deta() const;
  /** @brief cell dr
   */
  float dr() const;

  /** @brief cell x
   */
  float x() const;
  /** @brief cell y
   */
  float y() const;
  /** @brief cell z
   */
  float z() const;
  /** @brief cell x_raw
   */
  float x_raw() const;
  /** @brief cell y_raw
   */
  float y_raw() const;
  /** @brief cell z_raw
   */
  float z_raw() const;
  /** @brief cell dx
   */
  float dx() const;
  /** @brief cell dy
   */
  float dy() const;
  /** @brief cell dz
   */
  float dz() const;

  /** @brief cell identifier
   */
  Identifier identify() const;

  unsigned long long calo_hash() const;

  int getSampling() const ;

 private:
 //ACH protected:
 //
  long long m_identify;
  long long m_hash_id;

  int m_calosample;

  /** @brief cylindric coordinates : eta
   */
  float m_eta;
  /** @brief cylindric coordinates : phi
   */
  float m_phi;

 /** @brief this one is cached for algorithm working in transverse Energy
    */
  float m_sinTh;
   /** @brief this one is cached for algorithm working in transverse Energy
   */
  float m_cosTh;

  /** @brief cylindric coordinates : delta eta
   */
  float m_deta;
  /** @brief cylindric coordinates : delta phi
   */
  float m_dphi;

  /** @brief cylindric coordinates : r
   */

  float m_volume;

  /** @brief cache to allow fast px py pz computation
   */
  float m_sinPhi;

  /** @brief cache to allow fast px py pz computation
  */
  float m_cosPhi;

  /** @brief cylindric coordinates : r
   */
  float m_r;
  /** @brief cylindric coordinates : eta_raw
   */
  float m_eta_raw;
  /** @brief cylindric coordinates : phi_raw
   */
  float m_phi_raw;
  /** @brief cylindric coordinates : r_raw
   */
  float m_r_raw;
  /** @brief cylindric coordinates : delta r
   */
  float m_dr;

  /** @brief cartesian coordinates : X
   */
  float m_x;
  /** @brief cartesian coordinates : Y
   */
  float m_y;
  /** @brief cartesian coordinates : Z
   */
  float m_z;
  /** @brief cartesian coordinates : X raw
   */
  float m_x_raw;
  /** @brief cartesian coordinates : Y raw
   */
  float m_y_raw;
  /** @brief cartesian coordinates : Z raw
   */
  float m_z_raw;
  /** @brief cartesian coordinates : delta X
   */
  float m_dx;
  /** @brief cartesian coordinates : delta Y
   */
  float m_dy;
  /** @brief cartesian coordinates : delta Z
   */
  float m_dz;

};

inline Identifier CaloGeoDetDescrElement::identify() const
{
	Identifier id((unsigned long long) m_identify);
	return id;
}

inline unsigned long long CaloGeoDetDescrElement::calo_hash() const
{
	return m_hash_id;
}

inline int CaloGeoDetDescrElement::getSampling() const
{ return m_calosample;}
inline float CaloGeoDetDescrElement::eta() const
{ return m_eta;}
inline float CaloGeoDetDescrElement::phi() const
{ return m_phi;}
inline float CaloGeoDetDescrElement::r() const
{ return m_r;}
inline float CaloGeoDetDescrElement::eta_raw() const
{ return m_eta_raw;}
inline float CaloGeoDetDescrElement::phi_raw() const
{ return m_phi_raw;}
inline float CaloGeoDetDescrElement::r_raw() const
{ return m_r_raw;}
inline float CaloGeoDetDescrElement::deta() const
{ return m_deta;}
inline float CaloGeoDetDescrElement::dphi() const
{ return m_dphi;}
inline float CaloGeoDetDescrElement::dr() const
{ return m_dr;}

inline float CaloGeoDetDescrElement::x() const
{ return m_x;}
inline float CaloGeoDetDescrElement::y() const
{ return m_y;}
inline float CaloGeoDetDescrElement::z() const
{ return m_z;}
inline float CaloGeoDetDescrElement::x_raw() const
{ return m_x_raw;}
inline float CaloGeoDetDescrElement::y_raw() const
{ return m_y_raw;}
inline float CaloGeoDetDescrElement::z_raw() const
{ return m_z_raw;}
inline float CaloGeoDetDescrElement::dx() const
{ return m_dx;}
inline float CaloGeoDetDescrElement::dy() const
{ return m_dy;}
inline float CaloGeoDetDescrElement::dz() const
{ return m_dz;}

#endif
