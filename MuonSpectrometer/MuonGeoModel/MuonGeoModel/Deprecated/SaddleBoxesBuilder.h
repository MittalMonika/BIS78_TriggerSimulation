/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SaddleBoxesBuilder_H
#define SaddleBoxesBuilder_H


class StoreGateSvc;
class IRDBAccessSvc;
class GeoPhysVol;
class IRDBRecordset;
class IMessageSvc;

#include <string>

namespace MuonGM {
	
class SaddleBoxesBuilder 
{ 
 public:

  SaddleBoxesBuilder(StoreGateSvc  *pDetStore,
		   IRDBAccessSvc *pRDBAccess, std::string geoTag, std::string geoNode);
  ~SaddleBoxesBuilder();
    
    bool unavailableData() {return _unavailableData;}
  void buildSaddleBoxes    ( GeoPhysVol* container );

 private:
  std::string getMaterial( std::string materialName );

  IRDBAccessSvc* m_pRDBAccess;
  StoreGateSvc*  m_pDetStore;

    bool _unavailableData;

  const IRDBRecordset* m_Saddle;
  const IRDBRecordset* m_Saddlebovp;
  const IRDBRecordset* m_Saddlebovs;
  const IRDBRecordset* m_Saddlebovn;
  const IRDBRecordset* m_Saddlebovsbo;
  const IRDBRecordset* m_Saddlebovstu;
  const IRDBRecordset* m_Saddlbovsbr;
  const IRDBRecordset* m_Saddleeboxes;
  const IRDBRecordset* m_Saddleetubes;
  const IRDBRecordset* m_Saddleebreps;
  const IRDBRecordset* m_Saddleebrepsxy;
  
  IMessageSvc*    m_msgSvc;
};

} // namespace MuonGM

#endif
