// emacs: this is -*- c++ -*-
//
//   @file    globals.h        
//                    
//
//   $Id: globals.h, v0.0   Sun 12 Mar 2017 15:21:08 CET sutt $
//
//   Copyright (C) 2017 M Sutton (sutt@cern.ch)    

#ifndef  GLOBALS_H
#define  GLOBALS_H

/// all these externals initialised in globals.cxx

extern TIDARoiDescriptor* groi; 
extern TIDA::Event*       gevent;

extern int Nvtxtracks; 
extern int NvtxCount; 

extern int NMod; 

extern bool hipt; 
extern bool dumpflag; 

extern int r; 
extern int lb;
extern int ts;
extern int ev;

extern double a0;

#endif  // GLOBALS_H 










