-- PROJECT: ATLAS MDT Calibration DB
-- AUTHOR: Elisabetta.Vilucchi@lnf.infn.it, celio@roma3.infn.it
-- DATE: May 08
-- VERSION v1r1

--USAGE:
--sqlplus <USER_NAME>/<USER_PASSWORD> @mdt_rt_map_t_table.sql <USER_NAME> <SITE_NAME>(2 characters like MU,MI,RM...) <USER_PASSOWRD> <TABLESPCE_NAME>

--DROP TABLE &1.."MDT_RT_MAP_T";
CREATE TABLE &1.."MDT_RT_MAP_T" 
("MDT_RT_MAP_T_ID"  NUMBER        NOT NULL ,
"MDT_RT_ID_START"   NUMBER        NOT NULL ,
"MDT_RT_ID_END"	    NUMBER        NOT NULL ,
"BINS"		    NUMBER(4)     NOT NULL ,
"SITE_NAME"         VARCHAR2(2)   DEFAULT UPPER('&2') ,
"INSERT_TIME"       VARCHAR2(19) ,
"T_1"		    NUMBER(9,6)   NOT NULL ,
"T_2"               NUMBER(9,6)   NOT NULL ,
"T_3"               NUMBER(9,6)   NOT NULL ,
"T_4"               NUMBER(9,6)   NOT NULL ,
"T_5"               NUMBER(9,6)   NOT NULL ,
"T_6"               NUMBER(9,6)   NOT NULL ,
"T_7"               NUMBER(9,6)   NOT NULL ,
"T_8"               NUMBER(9,6)   NOT NULL ,
"T_9"               NUMBER(9,6)   NOT NULL ,
"T_10"              NUMBER(9,6)   NOT NULL ,
"T_11"              NUMBER(9,6)   NOT NULL ,
"T_12"              NUMBER(9,6)   NOT NULL ,
"T_13"              NUMBER(9,6)   NOT NULL ,
"T_14"              NUMBER(9,6)   NOT NULL ,
"T_15"              NUMBER(9,6)   NOT NULL ,
"T_16"              NUMBER(9,6)   NOT NULL ,
"T_17"              NUMBER(9,6)   NOT NULL ,
"T_18"              NUMBER(9,6)   NOT NULL ,
"T_19"              NUMBER(9,6)   NOT NULL ,
"T_20"              NUMBER(9,6)   NOT NULL ,
"T_21"              NUMBER(9,6)   NOT NULL ,
"T_22"              NUMBER(9,6)   NOT NULL ,
"T_23"              NUMBER(9,6)   NOT NULL ,
"T_24"              NUMBER(9,6)   NOT NULL ,
"T_25"              NUMBER(9,6)   NOT NULL ,
"T_26"              NUMBER(9,6)   NOT NULL ,
"T_27"              NUMBER(9,6)   NOT NULL ,
"T_28"              NUMBER(9,6)   NOT NULL ,
"T_29"              NUMBER(9,6)   NOT NULL ,
"T_30"              NUMBER(9,6)   NOT NULL ,
"T_31"              NUMBER(9,6)   NOT NULL ,
"T_32"              NUMBER(9,6)   NOT NULL ,
"T_33"              NUMBER(9,6)   NOT NULL ,
"T_34"              NUMBER(9,6)   NOT NULL ,
"T_35"              NUMBER(9,6)   NOT NULL ,
"T_36"              NUMBER(9,6)   NOT NULL ,
"T_37"              NUMBER(9,6)   NOT NULL ,
"T_38"              NUMBER(9,6)   NOT NULL ,
"T_39"              NUMBER(9,6)   NOT NULL ,
"T_40"              NUMBER(9,6)   NOT NULL ,
"T_41"              NUMBER(9,6)   NOT NULL ,
"T_42"              NUMBER(9,6)   NOT NULL ,
"T_43"              NUMBER(9,6)   NOT NULL ,
"T_44"              NUMBER(9,6)   NOT NULL ,
"T_45"              NUMBER(9,6)   NOT NULL ,
"T_46"              NUMBER(9,6)   NOT NULL ,
"T_47"              NUMBER(9,6)   NOT NULL ,
"T_48"              NUMBER(9,6)   NOT NULL ,
"T_49"              NUMBER(9,6)   NOT NULL ,
"T_50"              NUMBER(9,6)   NOT NULL ,
"T_51"              NUMBER(9,6)   NOT NULL ,
"T_52"              NUMBER(9,6)   NOT NULL ,
"T_53"              NUMBER(9,6)   NOT NULL ,
"T_54"              NUMBER(9,6)   NOT NULL ,
"T_55"              NUMBER(9,6)   NOT NULL ,
"T_56"              NUMBER(9,6)   NOT NULL ,
"T_57"              NUMBER(9,6)   NOT NULL ,
"T_58"              NUMBER(9,6)   NOT NULL ,
"T_59"              NUMBER(9,6)   NOT NULL ,
"T_60"              NUMBER(9,6)   NOT NULL ,
"T_61"              NUMBER(9,6)   NOT NULL ,
"T_62"              NUMBER(9,6)   NOT NULL ,
"T_63"              NUMBER(9,6)   NOT NULL ,
"T_64"              NUMBER(9,6)   NOT NULL ,
"T_65"              NUMBER(9,6)   NOT NULL ,
"T_66"              NUMBER(9,6)   NOT NULL ,
"T_67"              NUMBER(9,6)   NOT NULL ,
"T_68"              NUMBER(9,6)   NOT NULL ,
"T_69"              NUMBER(9,6)   NOT NULL ,
"T_70"              NUMBER(9,6)   NOT NULL ,
"T_71"              NUMBER(9,6)   NOT NULL ,
"T_72"              NUMBER(9,6)   NOT NULL ,
"T_73"              NUMBER(9,6)   NOT NULL ,
"T_74"              NUMBER(9,6)   NOT NULL ,
"T_75"              NUMBER(9,6)   NOT NULL ,
"T_76"              NUMBER(9,6)   NOT NULL ,
"T_77"              NUMBER(9,6)   NOT NULL ,
"T_78"              NUMBER(9,6)   NOT NULL ,
"T_79"              NUMBER(9,6)   NOT NULL ,
"T_80"              NUMBER(9,6)   NOT NULL ,
"T_81"              NUMBER(9,6)   NOT NULL ,
"T_82"              NUMBER(9,6)   NOT NULL ,
"T_83"              NUMBER(9,6)   NOT NULL ,
"T_84"              NUMBER(9,6)   NOT NULL ,
"T_85"              NUMBER(9,6)   NOT NULL ,
"T_86"              NUMBER(9,6)   NOT NULL ,
"T_87"              NUMBER(9,6)   NOT NULL ,
"T_88"              NUMBER(9,6)   NOT NULL ,
"T_89"              NUMBER(9,6)   NOT NULL ,
"T_90"              NUMBER(9,6)   NOT NULL ,
"T_91"              NUMBER(9,6)   NOT NULL ,
"T_92"              NUMBER(9,6)   NOT NULL ,
"T_93"              NUMBER(9,6)   NOT NULL ,
"T_94"              NUMBER(9,6)   NOT NULL ,
"T_95"              NUMBER(9,6)   NOT NULL ,
"T_96"              NUMBER(9,6)   NOT NULL ,
"T_97"              NUMBER(9,6)   NOT NULL ,
"T_98"              NUMBER(9,6)   NOT NULL ,
"T_99"              NUMBER(9,6)   NOT NULL ,
"T_100"             NUMBER(9,6)   NOT NULL ,
CONSTRAINT "MDT_RT_MAP_T_RT_ID_START_REF" FOREIGN KEY ("MDT_RT_ID_START", "SITE_NAME") REFERENCES &1.."MDT_RT" ("MDT_RT_ID", "SITE_NAME") VALIDATE ,
CONSTRAINT "MDT_RT_MAP_T_RT_ID_END_REF" FOREIGN KEY ("MDT_RT_ID_END", "SITE_NAME") REFERENCES &1.."MDT_RT" ("MDT_RT_ID", "SITE_NAME") VALIDATE ,
CONSTRAINT "SITE_NAME_REF" FOREIGN KEY ("SITE_NAME") REFERENCES &1.."SITES" ("NAME") VALIDATE ,
CONSTRAINT "MDT_RT_MAP_T_ID_SITE_PR" PRIMARY KEY ("MDT_RT_MAP_T_ID", "SITE_NAME") VALIDATE ) 
TABLESPACE &4 PCTFREE 10 INITRANS 1 MAXTRANS 255 STORAGE ( INITIAL 64K BUFFER_POOL DEFAULT) LOGGING;
