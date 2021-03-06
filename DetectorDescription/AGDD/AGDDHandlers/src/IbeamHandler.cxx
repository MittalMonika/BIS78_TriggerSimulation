/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "AGDDHandlers/IbeamHandler.h"
#include "AGDDModel/AGDDIbeam.h"
#include <iostream>

IbeamHandler::IbeamHandler(std::string s):XMLHandler(s)
{
}

void IbeamHandler::ElementHandle()
{
	bool res;
	std::string name=getAttributeAsString("name",res);
	std::string material=getAttributeAsString("material",res);

	double l=getAttributeAsDouble("length",res);
	double w=getAttributeAsDouble("width",res);
	double sw=getAttributeAsDouble("small_width",res);
	double h=getAttributeAsDouble("heigth",res);
	double sh=getAttributeAsDouble("small_heigth",res);
	
	AGDDIbeam *b=new AGDDIbeam(name);
	b->SetMaterial(material);
	b->SetLength(l);
	b->SetWidth(w);
	b->SetSmallWidth(sw);
	b->SetHeigth(h);
	b->SetSmallHeigth(sh);
	
	std::string col=getAttributeAsString("color",res);
	if (!col.empty())
		b->SetColor(col);
}
