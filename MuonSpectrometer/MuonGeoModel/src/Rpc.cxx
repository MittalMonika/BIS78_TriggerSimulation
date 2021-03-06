/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/


#include "MuonGeoModel/RPC_Technology.h"
#include "MuonGeoModel/Rpc.h"
#include "MuonGeoModel/RpcLayer.h"
#include "MuonGeoModel/MYSQL.h"
#include "GeoModelKernel/GeoCutVolAction.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include <iomanip>
#include <cassert>
#include <string>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <array>
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeIntersection.h"

#define skip_rpc false

namespace {
    // make a const array holding all amdb RPC names corresponding to BIS RPCs
    const static std::array<std::string, 7> bisRpcs = {"RPC26", "RPC27", "RPC28", "RPC29", "RPC30", "RPC31", "RPC32"};
    static constexpr double const& rpc3GapLayerThickness = 11.8; // gas vol. + ( bakelite + graphite + PET )x2
    static constexpr double const& rpc3GapMaxThickness = 36.0; // min 35.4 (11.8x3) - max 40.0 mm (tolerance from design)
}

namespace MuonGM {


Rpc::Rpc(Component* ss): DetectorElement(ss->name)
{
   double tol = 1.e-3;
   RpcComponent* s = (RpcComponent*)ss;
   width = s->dx1;
   longWidth = s->dx2;
   thickness = s->GetThickness();
   length = s->dy - tol;
   m_component = s;
   idiv = s->ndivy;
   jdiv = s->ndivz;
   m_nlayers = 2;
   // the BIS RPCs are the only ones with 3 gas gaps
   if (std::find(std::begin(bisRpcs), std::end(bisRpcs), ss->name) != std::end(bisRpcs)) m_nlayers = 3;
}


GeoFullPhysVol* Rpc::build(int minimalgeo)
{
   std::vector<Cutout*> vcutdef;
   int cutoutson = 0;
   return build(minimalgeo, cutoutson, vcutdef);
}


GeoFullPhysVol* Rpc::build(int minimalgeo, int cutoutson,
                           std::vector<Cutout*> vcutdef)
{
   MYSQL* mysql = MYSQL::GetPointer();
   std::string geometry_version = mysql->getGeometryVersion();
   RPC* r = (RPC*)mysql->GetTechnology(name);

   // Retrieve geometrical information, these are for middle and outer alyers ("standard" RPCs)
   double thickness = r->maxThickness;
   width = width/idiv;
   longWidth = longWidth/idiv;
   length = length/jdiv;

  double extSupThick = r->externalSupPanelThickness;
  double extAlSupThick = r->externalAlSupPanelThickness;
  double rpcLayerThickness = r->rpcLayerThickness;
  double centSupThick = r->centralSupPanelThickness;
  double centAlSupThick = r->centralAlSupPanelThickness;

  // Geometrical information to be overwritten for BIS chambers (having 3 gas gaps)
  if (m_nlayers==3) {
     // width, longWidth, length are taken from geometry DB
     thickness = rpc3GapMaxThickness;
     rpcLayerThickness = rpc3GapLayerThickness;
  }

   if (RPCprint) std::cout << " RPC build: " << name << " has thickness = "
                           << thickness << " long width = " << longWidth
                           << " width = " << width << " length = " << length 
                           << std::endl;  
 
   const GeoShape* srpc = new GeoTrd(thickness/2, thickness/2,
                                     width/2, longWidth/2, length/2);
   // Apply cutouts to mother volume

   if (cutoutson && vcutdef.size() > 0) {
     Cutout* cut = 0;
     GeoShape* cutoutShape = 0;
     HepGeom::Transform3D cutTrans;
     for (unsigned i = 0; i < vcutdef.size(); i++) {
       cut = vcutdef[i];
       cutoutShape = new GeoTrd(thickness/2.+1., thickness/2.+1.,
                                cut->widthXs/2., cut->widthXl/2., cut->lengthY/2.);
       cutTrans = HepGeom::Translate3D(0.0, cut->dx, -length/2 + cut->dy + cut->lengthY/2.);
       srpc = &(srpc->subtract( (*cutoutShape) << cutTrans) ); 
     }
//     std::cout << " Cutouts for Rpc " << std::endl;
//     for (unsigned i = 0; i < vcutdef.size(); i++) std::cout
//         << " x = " << vcutdef[i]->dx << " y = " << vcutdef[i]->dy
//         << " width = " << vcutdef[i]->widthXs << " length = " << vcutdef[i]->lengthY
//         << std::endl;
   }

   const GeoMaterial* mrpc = matManager->getMaterial("std::Air");
   GeoLogVol* lrpc = new GeoLogVol(logVolName, srpc, mrpc);
   GeoFullPhysVol* prpc = new GeoFullPhysVol(lrpc);

   if (minimalgeo == 1) return prpc;
  
   if (geometry_version.substr(0,1) != "M") {
     // here layout P and following (hopefully!)
     if (idiv * jdiv != 1) assert(0);

     // Note: the standard RPC (iswap != -1) has DED at the top:
     // from bottom to top it consists of the external support panel  
     // and then the RPC doublet

     double newpos = -thickness/2.;

     // here the bottom/external/pre-bent support panel
     // shape of the al skin of the support panel
     GeoTrd* slpan = new GeoTrd(extSupThick/2, extSupThick/2,
                                width/2, longWidth/2, length/2);
     GeoTrd* sholpan = new GeoTrd(extSupThick/2 - extAlSupThick,
                                  extSupThick/2 - extAlSupThick,
                                  width/2 - extAlSupThick,
                                  longWidth/2 - extAlSupThick,
                                  length/2 - extAlSupThick);
     const GeoShape* sallpan = slpan;
     const GeoShape* sholpan2 = sholpan;
     const GeoMaterial* mallpan = matManager->getMaterial("std::Aluminium");
     GeoLogVol* lallpan = new GeoLogVol("RPC_AL_extsuppanel", sallpan, mallpan);
     GeoPhysVol* pallpan = new GeoPhysVol(lallpan);
     const GeoMaterial* mholpan = matManager->getMaterial("muo::RpcAlHonC");
     GeoLogVol* lholpan = new GeoLogVol("RPC_honeyc_extsuppanel", sholpan2, mholpan);
     GeoPhysVol* pholpan = new GeoPhysVol (lholpan);
     pallpan->add(pholpan);   // this way the honeycomb is a child of its al skin

     // Apply cutouts
     if (cutoutson && vcutdef.size() > 0) {
       GeoPhysVol* tempPhys = 0;
       Cutout* cut = 0;
       GeoShape* cutoutShape = 0;
       HepGeom::Transform3D cutTrans;
       for (unsigned i = 0; i < vcutdef.size(); i++) {
         cut = vcutdef[i];
         cutoutShape = new GeoTrd(thickness/2.+1., thickness/2.+1.,
                                  cut->widthXs/2., cut->widthXl/2., cut->lengthY/2.);
         cutTrans = HepGeom::Translate3D(0.0, cut->dx, -length/2 + cut->dy + cut->lengthY/2.);

         GeoCutVolAction cutAction(*cutoutShape, cutTrans);
         pallpan->apply(&cutAction);
         tempPhys = cutAction.getPV();
         pallpan->ref(); pallpan->unref();
         pallpan = tempPhys;
       }
     }

     if (m_nlayers==2) { // only to be done for standard (non-BIS) RPCs
       newpos += extSupThick/2.;

     GeoTransform* tlpan = new GeoTransform(HepGeom::TranslateX3D( newpos ));
     if (RPCprint) std::cout << " Rpc:: put ext.sup panel at " << newpos
                             << " from centre" << std::endl;
     if (!skip_rpc) {
       prpc->add(tlpan);
       prpc->add(pallpan);
     }
     // The first layer is support for RPCs with 2 gaps, is a layer for 3 gaps (BIS chambers, no supports)
       newpos += extSupThick/2.;
     }

     // bottom RpcLayer
     RpcLayer* rl = new RpcLayer(name,this);
     GeoVPhysVol* plowergg;
     if (cutoutson && vcutdef.size() > 0) {
       plowergg = rl->build(cutoutson, vcutdef);
     } else {
       plowergg = rl->build();
     }

     newpos += rpcLayerThickness/2.;
     GeoTransform* tlgg = new GeoTransform(HepGeom::TranslateX3D( newpos ));
     if (RPCprint) std::cout << " Rpc:: put lower RPC layer at " << newpos
                             << " from centre " << std::endl;
     if (!skip_rpc) {
       prpc->add(new GeoIdentifierTag(1));
       prpc->add(tlgg);
       prpc->add(plowergg);
     }

     // central support panel
     newpos += rpcLayerThickness/2.;
     GeoTrd* scpan = new GeoTrd(centSupThick/2, centSupThick/2,
                                width/2, longWidth/2, length/2);
     GeoTrd* shocpan = new GeoTrd(centSupThick/2 - centAlSupThick,
                                  centSupThick/2 - centAlSupThick,
                                  width/2 - centAlSupThick,
                                  longWidth/2 - centAlSupThick,
                                  length/2 - centAlSupThick);

     const GeoShape* salcpan = scpan;
     const GeoShape* shocpan2 = shocpan;
     GeoLogVol* lalcpan = new GeoLogVol("RPC_AL_midsuppanel", salcpan, mallpan);
     GeoPhysVol* palcpan = new GeoPhysVol(lalcpan);
     const GeoMaterial* mhocpan = matManager->getMaterial("muo::RpcPapHonC");
     GeoLogVol* lhocpan = new GeoLogVol("RPC_honeyc_midsuppanel", shocpan2, mhocpan);
     GeoPhysVol* phocpan = new GeoPhysVol (lhocpan);
     palcpan->add(phocpan);  // this way the honeycomb is a child of its al skin

     // Apply cutouts
     if (cutoutson && vcutdef.size() > 0) {
       GeoPhysVol* tempPhys = 0;
       Cutout* cut = 0;
       GeoShape* cutoutShape = 0;
       HepGeom::Transform3D cutTrans;
       for (unsigned i = 0; i < vcutdef.size(); i++) {
         cut = vcutdef[i];
         cutoutShape = new GeoTrd(thickness/2.+1., thickness/2.+1.,
                                  cut->widthXs/2., cut->widthXl/2., cut->lengthY/2.);
         cutTrans = HepGeom::Translate3D(0.0, cut->dx, -length/2 + cut->dy + cut->lengthY/2.);

         GeoCutVolAction cutAction(*cutoutShape, cutTrans);
         palcpan->apply(&cutAction);
         tempPhys = cutAction.getPV();
         palcpan->ref(); palcpan->unref();
         palcpan = tempPhys;
       }
     }

     if (m_nlayers==2) { // only to be done for standard (non-BIS) RPCs
       newpos += centSupThick/2.;
     GeoTransform* tcpan = new GeoTransform(HepGeom::TranslateX3D( newpos ));
     if (RPCprint) std::cout << " Rpc:: put central sup panel at " << newpos 
                             << " from centre" << std::endl;
     if (!skip_rpc) {
       prpc->add(tcpan);
       prpc->add(palcpan);
     }
     newpos += centSupThick/2.;
     }

     // top RpcLayer
     RpcLayer* ru = new RpcLayer(name,this);
     GeoVPhysVol* puppergg;
     if (cutoutson && vcutdef.size() > 0) {
       // This code required to take into account the various
       // 180 degree rotations of RPC panels in BMS chambers 
       int subtype = 0;
       int ijob = 0;
       for (unsigned int i = 0; i < vcutdef.size(); i++) {
         subtype = vcutdef[i]->subtype;
         ijob = vcutdef[i]->ijob;

         // For BMS2, BMS13
         if (name == "RPC06" && ijob == 3) {
           if (subtype == 2) {
             vcutdef[i]->dy = 0;
           } else if (subtype == 13) {
             vcutdef[i]->dy = this->length - vcutdef[i]->lengthY;
           }
         }
              
         // For BMS5, BMS9
         if (name == "RPC07") {
           if (subtype == 5) { // BMS5
             if (ijob == 24) vcutdef[i]->dy = this->length - vcutdef[i]->lengthY;
             if (ijob == 32) vcutdef[i]->dy = 0.;
           } else if (subtype == 9) { // BMS9
             if (ijob == 32) vcutdef[i]->dy = this->length - vcutdef[i]->lengthY;
             if (ijob == 24) vcutdef[i]->dy = 0.;
           }
         }

         // For BMS6, BMS11
         if (subtype == 6) {
           if (ijob == 29 && name == "RPC07") vcutdef[i]->dy = this->length - vcutdef[i]->lengthY;
           if (ijob == 21 && name == "RPC08") vcutdef[i]->dy = 0;
         } else if (subtype == 11) {
           if (ijob == 29 && name == "RPC07") vcutdef[i]->dy = 0;
           if (ijob == 21 && name == "RPC08") vcutdef[i]->dy = this->length - vcutdef[i]->lengthY - vcutdef[i]->dy;
         }
       }

       puppergg = ru->build(cutoutson, vcutdef);
     } else {
       puppergg = ru->build();
     }

     newpos += rpcLayerThickness/2.;
     GeoTransform* tugg = new GeoTransform(HepGeom::TranslateX3D(newpos) );
     if (RPCprint) std::cout<< " Rpc:: put upper RPC layer at " << newpos
                            << " from centre " << std::endl;
     GeoTransform* rugg = new GeoTransform(HepGeom::RotateY3D(180*CLHEP::deg) );
     if (!skip_rpc) {
       prpc->add(new GeoIdentifierTag(2));
       prpc->add(tugg);
       if (m_nlayers==2) prpc->add(rugg); // only to be done for standard (non-BIS) RPCs
       prpc->add(puppergg);            
     }

    // additional RpcLayer for BIS (3 gaps)
    if (m_nlayers==3) {
       newpos += rpcLayerThickness/2.;
       RpcLayer* rthird = new RpcLayer(name,this);
       GeoVPhysVol* pthirdgg;
       if (cutoutson && vcutdef.size() > 0) {
         pthirdgg = rthird->build(cutoutson, vcutdef);
       } else {
         pthirdgg = rthird->build();
       }

       newpos += rpcLayerThickness/2.;
       GeoTransform* tthirdgg = new GeoTransform(HepGeom::TranslateX3D(newpos) );
       if (!skip_rpc) {
          prpc->add(new GeoIdentifierTag(3));
          prpc->add(tthirdgg);
          prpc->add(pthirdgg);
       }
     }

     // release memory allocated for the builders 
     delete ru;
     delete rl;
     ru = 0;
     rl = 0;
   }

   return prpc;
}

void Rpc::print()
{
   std::cout <<"Rpc " << name.c_str() <<" :"<<std::endl;
}
} // namespace MuonGM
