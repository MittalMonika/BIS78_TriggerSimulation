/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

//1234567890123901234 123456789023456 123456789023456123123456789023456123 1234567890123 1234567890123 1234565 4444412344326567890
#ifndef STGCgeometry_h
#define STGCgeometry_h

// NSW STGC (c) Daniel.Lellouch@cern.ch April 11, 2013

static const float H1[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 1019.4, 1019.4, 1019.4, 1019.4} //D0
, { 1681.4, 1651.4, 1621.4, 1591.4} //D1
, { 2541.4, 2541.4, 2541.4, 2541.4} //D2
, { 3510.9, 3510.9, 3510.9, 3510.9} //D3
}
,
{ // SMALL_CONFIRM
 { 1019.4, 1019.4, 1019.4, 1019.4} //D0
, { 1681.4, 1651.4, 1621.4, 1591.4} //D1
, { 2541.4, 2541.4, 2541.4, 2541.4} //D2
, { 3510.9, 3510.9, 3510.9, 3510.9} //D3
}
,
{ // LARGE_PIVOT
 { 982.0, 982.0, 982.0, 982.0} //D0
, { 1678.0, 1648.0, 1618.0, 1588.0} //D1
, { 2538.0, 2538.0, 2538.0, 2538.0} //D2
, { 3598.0, 3598.0, 3598.0, 3598.0} //D3
}
,
{ // LARGE_CONFIRM
 { 982.0, 982.0, 982.0, 982.0} //D0
, { 1678.0, 1648.0, 1618.0, 1588.0} //D1
, { 2538.0, 2538.0, 2538.0, 2538.0} //D2
, { 3598.0, 3598.0, 3598.0, 3598.0} //D3
}
,
{ //TOY_PIVOT
 { -413.5, -413.5, -413.5, -413.5} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { -413.5, -413.5, -413.5, -413.5} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}

};

static const float H2[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 1681.4, 1651.4, 1621.4, 1591.4} //D0
, { 2531.4, 2531.4, 2531.4, 2531.4} //D1
, { 3500.9, 3500.9, 3500.9, 3500.9} //D2
, { 4470.4, 4470.4, 4470.4, 4470.4} //D3
}
,
{ // SMALL_CONFIRM
 { 1681.4, 1651.4, 1621.4, 1591.4} //D0
, { 2531.4, 2531.4, 2531.4, 2531.4} //D1
, { 3500.9, 3500.9, 3500.9, 3500.9} //D2
, { 4470.4, 4470.4, 4470.4, 4470.4} //D3
}
,
{ // LARGE_PIVOT
 { 1678.0, 1648.0, 1618.0, 1588.0} //D0
, { 2528.0, 2528.0, 2528.0, 2528.0} //D1
, { 3588.0, 3588.0, 3588.0, 3588.0} //D2
, { 4398.0, 4398.0, 4398.0, 4398.0} //D3
}
,
{ // LARGE_CONFIRM
 { 1678.0, 1648.0, 1618.0, 1588.0} //D0
, { 2528.0, 2528.0, 2528.0, 2528.0} //D1
, { 3588.0, 3588.0, 3588.0, 3588.0} //D2
, { 4398.0, 4398.0, 4398.0, 4398.0} //D3
}
,
{ //TOY_PIVOT
 { 413.5, 413.5, 413.5, 413.5} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { 413.5, 413.5, 413.5, 413.5} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};


static const float H3[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF } //D1
, { UNDEF , UNDEF , UNDEF , UNDEF } //D2
, { UNDEF , UNDEF , UNDEF , UNDEF } //D3
}
,
{ // SMALL_CONFIRM
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF } //D1
, { UNDEF , UNDEF , UNDEF , UNDEF } //D2
, { UNDEF , UNDEF , UNDEF , UNDEF } //D3
}
,
{ // LARGE_PIVOT
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF } //D1
, { UNDEF , UNDEF , UNDEF , UNDEF } //D2
, { 4648.0, 4648.0, 4648.0, 4648.0} //D3
}
,
{ // LARGE_CONFIRM
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF } //D1
, { UNDEF , UNDEF , UNDEF , UNDEF } //D2
, { 4648.0, 4648.0, 4648.0, 4648.0} //D3
}
,
{ //TOY_PIVOT
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};

static const float A[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 384.0, 384.0, 384.0, 384.0} //D0
, { 677.5, 664.2, 650.9, 637.6} //D1
, { 1056.5, 1056.5, 1056.5, 1056.5} //D2
, { 1260.3, 1260.3, 1260.3, 1260.3} //D3
}
,
{ // SMALL_CONFIRM
 { 420.0, 420.0, 420.0, 420.0} //D0
, { 713.5, 700.2, 686.9, 673.6} //D1
, { 1092.5, 1092.5, 1092.5, 1092.5} //D2
, { 1296.3, 1296.3, 1296.3, 1296.3} //D3
}
,
{ // LARGE_PIVOT
 { 598.0, 598.0, 598.0, 598.0} //D0
, { 843.4, 832.8, 822.2, 811.7} //D1
, { 1149.1, 1149.1, 1149.1, 1149.1} //D2
, { 1777.1, 1777.1, 1777.1, 1777.1} //D3
}
,
{ // LARGE_CONFIRM
 { 634.0, 634.0, 634.0, 634.0} //D0
, { 879.4, 868.8, 858.2, 847.7} //D1
, { 1185.1, 1185.1, 1185.1, 1185.1} //D2
, { 1813.1, 1813.1, 1813.1, 1813.1} //D3
}
,
{ //TOY_PIVOT
 { 834.0, 834.0, 834.0, 834.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { 834.0, 834.0, 834.0, 834.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};


static const float B[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 677.5, 664.2, 650.9, 637.6} //D0
, { 1054.4, 1054.4, 1054.4, 1054.4} //D1
, { 1258.2, 1258.2, 1258.2, 1258.2} //D2
, { 1462.0, 1462.0, 1462.0, 1462.0} //D3
}
,
{ // SMALL_CONFIRM
 { 713.5, 700.2, 686.9, 673.6} //D0
, { 1090.4, 1090.4, 1090.4, 1090.4} //D1
, { 1294.2, 1294.2, 1294.2, 1294.2} //D2
, { 1498.0, 1498.0, 1498.0, 1498.0} //D3
}
,
{ // LARGE_PIVOT
 { 843.4, 832.8, 822.2, 811.7} //D0
, { 1143.2, 1143.2, 1143.2, 1143.2} //D1
, { 1771.1, 1771.1, 1771.1, 1771.1} //D2
, { 2251.0, 2251.0, 2251.0, 2251.0} //D3
}
,
{ // LARGE_CONFIRM
 { 879.4, 868.8, 858.2, 847.7} //D0
, { 1179.2, 1179.2, 1179.2, 1179.2} //D1
, { 1807.1, 1807.1, 1807.1, 1807.1} //D2
, { 2287.0, 2287.0, 2287.0, 2287.0} //D3
}
,
{ //TOY_PIVOT
 { 834.0, 834.0, 834.0, 834.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { 834.0, 834.0, 834.0, 834.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};



static const float H1_ACTIVE[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 1032.9, 1032.9, 1032.9, 1032.9} //D0
, { 1694.9, 1664.9, 1634.9, 1604.9} //D1
, { 2554.9, 2554.9, 2554.9, 2554.9} //D2
, { 3524.4, 3524.4, 3524.4, 3524.4} //D3
}
,
{ // SMALL_CONFIRM
 { 1032.9, 1032.9, 1032.9, 1032.9} //D0
, { 1694.9, 1664.9, 1634.9, 1604.9} //D1
, { 2554.9, 2554.9, 2554.9, 2554.9} //D2
, { 3524.4, 3524.4, 3524.4, 3524.4} //D3
}
,
{ // LARGE_PIVOT
 { 995.5, 995.5, 995.5, 995.5} //D0
, { 1691.5, 1661.5, 1631.5, 1601.5} //D1
, { 2551.5, 2551.5, 2551.5, 2551.5} //D2
, { 3611.5, 3611.5, 3611.5, 3611.5} //D3
}
,
{ // LARGE_CONFIRM
 { 995.5, 995.5, 995.5, 995.5} //D0
, { 1691.5, 1661.5, 1631.5, 1601.5} //D1
, { 2551.5, 2551.5, 2551.5, 2551.5} //D2
, { 3611.5, 3611.5, 3611.5, 3611.5} //D3
}
,
{ //TOY_PIVOT
 { -400.0, -400.0, -400.0, -400.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { -400.0, -400.0, -400.0, -400.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};

static const float H2_ACTIVE[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 1667.9, 1637.9, 1607.9, 1577.9} //D0
, { 2517.9, 2517.9, 2517.9, 2517.9} //D1
, { 3470.9, 3470.9, 3470.9, 3470.9} //D2
, { 4440.4, 4440.4, 4440.4, 4440.4} //D3
}
,
{ // SMALL_CONFIRM
 { 1667.9, 1637.9, 1607.9, 1577.9} //D0
, { 2517.9, 2517.9, 2517.9, 2517.9} //D1
, { 3470.9, 3470.9, 3470.9, 3470.9} //D2
, { 4440.4, 4440.4, 4440.4, 4440.4} //D3
}
,
{ // LARGE_PIVOT
 { 1664.5, 1634.5, 1604.5, 1574.5} //D0
, { 2514.5, 2514.5, 2514.5, 2514.5} //D1
, { 3558.0, 3558.0, 3558.0, 3558.0} //D2
, { 4368.0, 4368.0, 4368.0, 4368.0} //D3
}
,
{ // LARGE_CONFIRM
 { 1664.5, 1634.5, 1604.5, 1574.5} //D0
, { 2514.5, 2514.5, 2514.5, 2514.5} //D1
, { 3558.0, 3558.0, 3558.0, 3558.0} //D2
, { 4368.0, 4368.0, 4368.0, 4368.0} //D3
}
,
{ //TOY_PIVOT
 { 400.0, 400.0, 400.0, 400.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { 400.0, 400.0, 400.0, 400.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};

static const float H3_ACTIVE[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF } //D1
, { UNDEF , UNDEF , UNDEF , UNDEF } //D2
, { UNDEF , UNDEF , UNDEF , UNDEF } //D3
}
,
{ // SMALL_CONFIRM
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF } //D1
, { UNDEF , UNDEF , UNDEF , UNDEF } //D2
, { UNDEF , UNDEF , UNDEF , UNDEF } //D3
}
,
{ // LARGE_PIVOT
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF } //D1
, { UNDEF , UNDEF , UNDEF , UNDEF } //D2
, { 4618.0, 4618.0, 4618.0, 4618.0} //D3
}
,
{ // LARGE_CONFIRM
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF } //D1
, { UNDEF , UNDEF , UNDEF , UNDEF } //D2
, { 4618.0, 4618.0, 4618.0, 4618.0} //D3
}
,
{ //TOY_PIVOT
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { UNDEF , UNDEF , UNDEF , UNDEF } //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};

static const float A_ACTIVE[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 356.8, 356.8, 356.8, 356.8} //D0
, { 650.3, 637.0, 623.7, 610.4} //D1
, { 1025.5, 1025.5, 1025.5, 1025.5} //D2
, { 1229.3, 1229.3, 1229.3, 1229.3} //D3
}
,
{ // SMALL_CONFIRM
 { 392.8, 392.8, 392.8, 392.8} //D0
, { 686.3, 673.0, 659.7, 646.4} //D1
, { 1061.5, 1061.5, 1061.5, 1061.5} //D2
, { 1265.3, 1265.3, 1265.3, 1265.3} //D3
}
,
{ // LARGE_PIVOT
 { 569.2, 569.2, 569.2, 569.2} //D0
, { 814.7, 804.1, 793.5, 782.9} //D1
, { 1124.5, 1124.5, 1124.5, 1124.5} //D2
, { 1752.5, 1752.5, 1752.5, 1752.5} //D3
}
,
{ // LARGE_CONFIRM
 { 605.2, 605.2, 605.2, 605.2} //D0
, { 850.7, 840.1, 829.5, 818.9} //D1
, { 1160.5, 1160.5, 1160.5, 1160.5} //D2
, { 1788.5, 1788.5, 1788.5, 1788.5} //D3
}
,
{ //TOY_PIVOT
 { 800.0, 800.0, 800.0, 800.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { 800.0, 800.0, 800.0, 800.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};


static const float B_ACTIVE[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 638.3, 625.0, 611.7, 598.4} //D0
, { 1015.2, 1015.2, 1015.2, 1015.2} //D1
, { 1218.1, 1218.1, 1218.1, 1218.1} //D2
, { 1421.9, 1421.9, 1421.9, 1421.9} //D3
}
,
{ // SMALL_CONFIRM
{ 674.3, 661.0, 647.7, 634.4} //D0
, { 1051.2, 1051.2, 1051.2, 1051.2} //D1
, { 1254.1, 1254.1, 1254.1, 1254.1} //D2
, { 1457.9, 1457.9, 1457.9, 1457.9} //D3
}
,
{ // LARGE_PIVOT
 { 805.2, 794.6, 784.0, 773.4} //D0
, { 1104.9, 1104.9, 1104.9, 1104.9} //D1
, { 1720.8, 1720.8, 1720.8, 1720.8} //D2
, { 2200.6, 2200.6, 2200.6, 2200.6} //D3
}
,
{ // LARGE_CONFIRM
{ 841.2, 830.6, 820.0, 809.4} //D0
, { 1140.9, 1140.9, 1140.9, 1140.9} //D1
, { 1756.8, 1756.8, 1756.8, 1756.8} //D2
, { 2236.6, 2236.6, 2236.6, 2236.6} //D3
}
,
{ //TOY_PIVOT
 { 800.0, 800.0, 800.0, 800.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { 800.0, 800.0, 800.0, 800.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};



static const float A_ADAPTERS[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 428.0, 428.0, 428.0, 428.0} //D0
, { 787.5, 774.2, 760.9, 747.6} //D1
, { 1136.5, 1136.5, 1136.5, 1136.5} //D2
, { 1340.3, 1340.3, 1340.3, 1340.3} //D3
}
,
{ // SMALL_CONFIRM
 { 464.0, 464.0, 464.0, 464.0} //D0
, { 823.5, 810.2, 796.9, 783.6} //D1
, { 1172.5, 1172.5, 1172.5, 1172.5} //D2
, { 1376.3, 1376.3, 1376.3, 1376.3} //D3
}
,
{ // LARGE_PIVOT
 { 642.0, 642.0, 642.0, 642.0} //D0
, { 953.4, 942.8, 932.2, 921.7} //D1
, { 1229.1, 1229.1, 1229.1, 1229.1} //D2
, { 1857.1, 1857.1, 1857.1, 1857.1} //D3
}
,
{ // LARGE_CONFIRM
 { 678.0, 678.0, 678.0, 678.0} //D0
, { 989.4, 978.8, 968.2, 957.7} //D1
, { 1265.1, 1265.1, 1265.1, 1265.1} //D2
, { 1893.1, 1893.1, 1893.1, 1893.1} //D3
}
,
{ //TOY_PIVOT
 { 878.0, 878.0, 878.0, 878.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { 878.0, 878.0, 878.0, 878.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};


static const float B_ADAPTERS[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 721.5, 708.2, 694.9, 681.6} //D0
, { 1098.4, 1098.4, 1098.4, 1098.4} //D1
, { 1338.2, 1338.2, 1338.2, 1338.2} //D2
, { 1542.0, 1542.0, 1542.0, 1542.0} //D3
}
,
{ // SMALL_CONFIRM
{ 757.5, 744.2, 730.9, 717.6} //D0
, { 1134.4, 1134.4, 1134.4, 1134.4} //D1
, { 1374.2, 1374.2, 1374.2, 1374.2} //D2
, { 1578.0, 1578.0, 1578.0, 1578.0} //D3
}
,
{ // LARGE_PIVOT
 { 887.4, 876.8, 866.2, 855.7} //D0
, { 1187.2, 1187.2, 1187.2, 1187.2} //D1
, { 1851.1, 1851.1, 1851.1, 1851.1} //D2
, { 2331.0, 2331.0, 2331.0, 2331.0} //D3
}
,
{ // LARGE_CONFIRM
{ 923.4, 912.8, 902.2, 891.7} //D0
, { 1223.2, 1223.2, 1223.2, 1223.2} //D1
, { 1887.1, 1887.1, 1887.1, 1887.1} //D2
, { 2367.0, 2367.0, 2367.0, 2367.0} //D3
}
,
{ //TOY_PIVOT
 { 878.0, 878.0, 878.0, 878.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { 878.0, 878.0, 878.0, 878.0} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};


static const float PAD_PHI_SUBDIVISION = 1.5; // Module 1 wrt Modules 2 and 3
static const float PAD_PHI_DIVISION = 0.130900; // Modules 2 and 3 in radians


static const float H_PAD_ROW_0[STGC_TYPES][STGC_LAYERS]={
 { 922.5, 883.9, 920.3, 881.5} // SMALL_PIVOT
 ,{ 867.0, 829.9, 865.0, 827.7} // SMALL_CONFIRM
 ,{ 939.3, 899.9, 937.0, 897.5} // LARGE_PIVOT
 ,{ 954.4, 913.4, 951.8, 910.7} // LARGE_CONFIRM
 ,{ -397.5, -437.5, -402.5, -442.5} //TOY_PIVOT
 ,{ -417.5, -457.5, -422.5, -462.5} // TOY_CONFIRM
 };


static const float PAD_HEIGHT[STGC_TYPES][STGC_LAYERS]={
 { 80.000, 80.123, 80.246, 80.368} // SMALL_PIVOT
 ,{ 76.855, 76.978, 77.101, 77.224} // SMALL_CONFIRM
 ,{ 81.453, 81.576, 81.699, 81.821} // LARGE_PIVOT
 ,{ 84.598, 84.721, 84.843, 84.966} // LARGE_CONFIRM
 ,{ 80.000, 80.000, 80.000, 80.000} //TOY_PIVOT
 ,{ 80.000, 80.000, 80.000, 80.000} // TOY_CONFIRM
 };

static const float Z_DANIEL[STGC_TYPES][STGC_LAYERS]={
 { 7377.5, 7388.8, 7400.2, 7411.5} // SMALL_PIVOT
 ,{ 7087.5, 7098.8, 7110.2, 7121.5} // SMALL_CONFIRM
 ,{ 7511.5, 7522.8, 7534.2, 7545.5} // LARGE_PIVOT
 ,{ 7801.5, 7812.8, 7824.2, 7835.5} // LARGE_CONFIRM
 ,{ -1, -1, -1, -1} //TOY_PIVOT
 ,{ -1, -1, -1, -1} // TOY_CONFIRM
 };

static const float Z_CURRENT_LAYOUT[STGC_TYPES][STGC_LAYERS]={
 { 7323.1, 7334.2, 7345.3, 7356.4} // SMALL_PIVOT
 ,{ 7058.2, 7069.3, 7080.4, 7091.5} // SMALL_CONFIRM
 ,{ 7420.2, 7431.3, 7442.4, 7453.5} // LARGE_PIVOT
 ,{ 7685.1, 7696.2, 7707.3, 7718.4} // LARGE_CONFIRM
 ,{ -397.5, -437.5, -402.5, -442.5} //TOY_PIVOT
 ,{ -417.5, -457.5, -422.5, -462.5} // TOY_CONFIRM
};

static const int FIRST_PAD_ROW_DIVISION[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 2, 3, 2, 3} //D0
, { 10, 10, 10, 10} //D1
, { 21, 22, 21, 22} //D2
, { 33, 34, 33, 34} //D3
}
,
{ // SMALL_CONFIRM
{ 3, 3, 3, 3} //D0
, { 11, 12, 11, 11} //D1
, { 23, 23, 23, 23} //D2
, { 35, 36, 35, 36} //D3
}
,
{ // LARGE_PIVOT
 { 1, 2, 1, 2} //D0
, { 10, 10, 9, 9} //D1
, { 21, 21, 20, 21} //D2
, { 34, 34, 33, 34} //D3
}
,
{ // LARGE_CONFIRM
{ 1, 2, 1, 2} //D0
, { 9, 10, 9, 9} //D1
, { 20, 20, 20, 20} //D2
, { 32, 33, 32, 32} //D3
}
,
{ //TOY_PIVOT
 { 1, 1, 1, 1} //D0
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
}
,
{ // TOY_CONFIRM
 { 1, 1, 1, 1} //D0
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
}
};

static const int PAD_ROWS[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 9, 8, 8, 7} //D0
, { 11, 12, 11, 12} //D1
, { 12, 12, 12, 12} //D2
, { 12, 12, 12, 12} //D3
}
,
{ // SMALL_CONFIRM
{ 9, 9, 8, 8} //D0
, { 12, 11, 12, 12} //D1
, { 12, 13, 12, 13} //D2
, { 13, 12, 13, 12} //D3
}
,
{ // LARGE_PIVOT
 { 9, 8, 8, 8} //D0
, { 11, 11, 12, 12} //D1
, { 12, 13, 13, 13} //D2
, { 12, 13, 13, 13} //D3
}
,
{ // LARGE_CONFIRM
{ 9, 8, 8, 7} //D0
, { 11, 10, 11, 11} //D1
, { 12, 13, 12, 12} //D2
, { 13, 12, 13, 13} //D3
}
,
{ //TOY_PIVOT
 { 10, 11, 10, 11} //D0
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
}
,
{ // TOY_CONFIRM
 { 11, 11, 11, 11} //D0
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
}
};



static const float PAD_COL_PHI0[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 0.002717, -0.040916, -0.002717, -0.046351} //D0
, { 0.002717, -0.040916, -0.002717, -0.046351} //D1
, { 0.002717, -0.062732, -0.002717, -0.068167} //D2
, { 0.002717, -0.062732, -0.002717, -0.068167} //D3
}
,
{ // SMALL_CONFIRM
{ -0.019099, -0.062732, -0.024534, -0.068167} //D0
, { -0.019099, -0.062732, -0.024534, -0.068167} //D1
, { -0.030008, -0.095457, -0.035442, -0.100892} //D2
, { -0.030008, -0.095457, -0.035442, -0.100892} //D3
}
,
{ // LARGE_PIVOT
 { 0.002717, -0.040916, -0.002717, -0.046351} //D0
, { 0.002717, -0.040916, -0.002717, -0.046351} //D1
, { 0.002717, -0.062732, -0.002717, -0.068167} //D2
, { 0.002717, -0.062732, -0.002717, -0.068167} //D3
}
,
{ // LARGE_CONFIRM
{ -0.019099, -0.062732, -0.024534, -0.068167} //D0
, { -0.019099, -0.062732, -0.024534, -0.068167} //D1
, { -0.030008, -0.095457, -0.035442, -0.100892} //D2
, { -0.030008, -0.095457, -0.035442, -0.100892} //D3
}
,
{ //TOY_PIVOT
 { -317.5, -357.5, -322.5, -362.5} //D0 actually X (mm)
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
,
{ // TOY_CONFIRM
 { -337.5, -377.5, -342.5, -382.5} //D0
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
}
};

static const int INDEX_LEFTMOST_COL[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { -1, -1, -1, -1} //D0
, { -1, -1, -1, -1} //D1
, { -1, 0, -1, 0} //D2
, { -1, 0, -1, 0} //D3
}
,
{ // SMALL_CONFIRM
{ -1, -1, -1, -1} //D0
, { -1, -1, -1, -1} //D1
, { -1, 0, -1, 0} //D2
, { -1, 0, -1, 0} //D3
}
,
{ // LARGE_PIVOT
 { -2, -1, -2, -1} //D0
, { -2, -1, -2, -1} //D1
, { -1, 0, -1, 0} //D2
, { -1, 0, -1, 0} //D3
}
,
{ // LARGE_CONFIRM
{ -2, -1, -2, -1} //D0
, { -2, -1, -2, -1} //D1
, { -1, 0, -1, 0} //D2
, { -1, 0, -1, 0} //D3
}
,
{ //TOY_PIVOT
 { 1, 1, 1, 1} //D0 actually 1st column
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
}
,
{ // TOY_CONFIRM
 { 1, 1, 1, 1} //D0 actually 1st column
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
}
};



static const int INDEX_RIGHTMOST_COL[STGC_TYPES][STGC_DETECTORS][STGC_LAYERS]={
{ // SMALL_PIVOT
 { 1, 2, 1, 2} //D0
, { 1, 2, 1, 2} //D1
, { 1, 1, 1, 1} //D2
, { 1, 1, 1, 1} //D3
}
,
{ // SMALL_CONFIRM
{ 1, 2, 1, 2} //D0
, { 1, 2, 1, 2} //D1
, { 1, 1, 1, 1} //D2
, { 1, 1, 1, 1} //D3
}
,
{ // LARGE_PIVOT
 { 2, 2, 2, 2} //D0
, { 2, 2, 2, 2} //D1
, { 1, 1, 1, 1} //D2
, { 1, 1, 1, 1} //D3
}
,
{ // LARGE_CONFIRM
{ 2, 2, 2, 2} //D0
, { 2, 2, 2, 2} //D1
, { 1, 1, 1, 1} //D2
, { 1, 1, 1, 1} //D3
}
,
{ //TOY_PIVOT
 { 9, 10, 9, 10} //D0
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
}
,
{ // TOY_CONFIRM
 { 10, 10, 10, 10} //D0
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
, { UNDEFINED , UNDEFINED , UNDEFINED , UNDEFINED }
}
};

static const float Z_CENTER_STGC_DETECTOR[STGC_TYPES][STGC_LAYERS]={
 { 7377.5, 7388.8, 7400.2, 7411.5} // SMALL_PIVOT
, { 7087.5, 7098.8, 7110.2, 7121.5} // SMALL_CONFIRM
, { 7511.5, 7522.8, 7534.2, 7545.5} // LARGE_PIVOT
, { 7801.5, 7812.8, 7824.2, 7835.5} // LARGE_CONFIRM
, { 18.0, 29.3, 40.7, 52.0} // TOY_PIVOT
, { 308.0, 319.3, 330.7, 342.0} // TOY_CONFIRM
};

static const std::string STGC_LAYER_NAMES[STGC_LAYERS]={
 "Layer 1"
, "Layer 2"
, "Layer 3"
, "Layer 4"
};

static const std::string STGC_DETECTOR_NAMES[STGC_DETECTORS]={
 "STGC Detector 0"
, "STGC Detector 1"
, "STGC Detector 2"
, "STGC Detector 3"
};

static const std::string STGC_MODULE_NAMES[STGC_MODULES]={
 "STGC Module 1"
, "STGC Module 2"
, "STGC Module 3"
};

static const std::string PIVOT_CONFIRM_NAMES[STGC_WEDGES]={
 "Pivot"
, "Confirm"
};

static const std::string SECTOR_NAMES[SECTOR_TYPES]={
 "Small"
, "Large"
, "Toy"
};

static const std::string WEDGE_NAMES[STGC_TYPES]={
 "SmallPivot"
, "SmallConfirm"
, "LargePivot"
, "LargeConfirm"
, "ToyPivot"
, "ToyConfirm"
};

static const std::string SHORT_WEDGE_NAMES[STGC_TYPES]={ "SP" , "SC"
 , "LP" , "LC"
 , "TP" , "TC" };
static const int MM_TYPE[SECTORS_IN_OCTANT][MM_MODULES]={
 { REGULAR , CORNERED , REGULAR , REGULAR }
, { REGULAR , REGULAR , REGULAR , EXTRUDED }
};

static const float MM_H0[SECTORS_IN_OCTANT][MM_MODULES]={
 { UNDEF , 1966.0, UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
};

static const float MM_H1[SECTORS_IN_OCTANT][MM_MODULES]={
 { 1016.0, 2056.3, 2816.0, 3666.0}
, { 982.0, 1932.0, 2882.0, 3832.0}
};

static const float MM_H2[SECTORS_IN_OCTANT][MM_MODULES]={
 { 1946.0, 2796.0, 3646.0, 4471.0}
, { 1912.0, 2862.0, 3812.0, 4287.0}
};

static const float MM_H3[SECTORS_IN_OCTANT][MM_MODULES]={
 { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , 4742.0}
};

static const float MM_D[SECTORS_IN_OCTANT][MM_MODULES]={
 { UNDEF , 1158.4, UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , UNDEF }
};

static const float MM_A[SECTORS_IN_OCTANT][MM_MODULES]={
 { 371.4, 1233.2, 1392.9, 1571.6}
, { 582.3, 1145.1, 1707.9, 2270.7}
};

static const float MM_B[SECTORS_IN_OCTANT][MM_MODULES]={
 { 1141.8, 1388.7, 1567.4, 1740.8}
, { 1133.3, 1696.1, 2258.9, 2540.3}
};

static const float MM_C[SECTORS_IN_OCTANT][MM_MODULES]={
 { UNDEF , UNDEF , UNDEF , UNDEF }
, { UNDEF , UNDEF , UNDEF , 2220.0}
};

static const float MM_ZMIN[SECTORS_IN_OCTANT] ={ 7104.5, 7528.5};
static const float MM_ZMAX[SECTORS_IN_OCTANT] ={ 7394.5, 7818.5};

static const std::string MM_MODULE_NAMES[MM_MODULES]={
 "MM Module 1"
, "MM Module 2"
, "MM Module 3"
, "MM Module 4"
};

static const std::string MM_TYPE_NAMES[MM_TYPES]={
 "Regular"
, "Cornered"
, "Extruded"
};

static const float MDT_LOST = 72.0;
static const float MDT_ZMIN[SECTORS_IN_OCTANT] ={ 14458.0, 14049.0};
static const float MDT_ZMAX[SECTORS_IN_OCTANT] ={ 14124.0, 13715.0};

static const float MDT_HMIN[SECTORS_IN_OCTANT][MDT_MODULES] ={
{ 1770.0, 3725.0, 5680.0, 7635.0}
,{ 1770.0, 3485.0, 5440.0, 7395.0}
};

static const float MDT_HMAX[SECTORS_IN_OCTANT][MDT_MODULES] ={
{ 3690.0, 5645.0, 7600.0, 9555.0}
,{ 3450.0, 5405.0, 7360.0, 9315.0}
};

static const float MDT_HALF_B[SECTORS_IN_OCTANT] ={ 417.5, 593.3};
static const float MDT_HALF_OPENING[SECTORS_IN_OCTANT] ={ 8.5, 14.0};




#endif
