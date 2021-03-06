LOAD DATA
INFILE './load_data/mdt_rt_map_t.dat'
BADFILE './load_data/mdt_rt_map_t.bad'
DISCARDFILE './load_data/mdt_rt_map_t.dsc'
CONTINUEIF NEXT(1)='+'
INTO TABLE mdt_rt_map_t  APPEND
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"'
(MDT_RT_MAP_T_ID,
MDT_RT_ID_START, 
MDT_RT_ID_END,	 
BINS,		
T_1,	
T_2,
T_3,
T_4,
T_5,
T_6,
T_7,
T_8,
T_9,
T_10,
T_11,
T_12,
T_13,
T_14,
T_15,
T_16,
T_17,
T_18,
T_19,
T_20,
T_21,
T_22,
T_23,
T_24,
T_25,
T_26,
T_27,
T_28,
T_29,
T_30,
T_31,
T_32,
T_33,
T_34,
T_35,
T_36,
T_37,
T_38,
T_39,
T_40,
T_41,
T_42,
T_43,
T_44,
T_45,
T_46,
T_47,
T_48,
T_49,
T_50,
T_51,
T_52,
T_53,
T_54,
T_55,
T_56,
T_57,
T_58,
T_59,
T_60,
T_61,
T_62,
T_63,
T_64,
T_65,
T_66,
T_67,
T_68,
T_69,
T_70,
T_71,
T_72,
T_73,
T_74,
T_75,
T_76,
T_77,
T_78,
T_79,
T_80,
T_81,
T_82,
T_83,
T_84,
T_85,
T_86,
T_87,
T_88,
T_89,
T_90,
T_91,
T_92,
T_93,
T_94,
T_95,
T_96,
T_97,
T_98,
T_99,
T_100)
