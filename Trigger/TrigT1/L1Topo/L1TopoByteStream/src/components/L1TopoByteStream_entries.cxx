#include "../L1TopoByteStreamCnv.h"
#include "../L1TopoByteStreamxAODCnv.h"
#include "../L1TopoByteStreamAuxCnv.h"
#include "../L1TopoByteStreamTool.h"

DECLARE_CONVERTER_FACTORY(L1TopoByteStreamCnv)
DECLARE_NAMESPACE_CONVERTER_FACTORY(LVL1BS, L1TopoByteStreamxAODCnv)
DECLARE_NAMESPACE_CONVERTER_FACTORY(LVL1BS, L1TopoByteStreamAuxCnv)
DECLARE_TOOL_FACTORY(L1TopoByteStreamTool)

