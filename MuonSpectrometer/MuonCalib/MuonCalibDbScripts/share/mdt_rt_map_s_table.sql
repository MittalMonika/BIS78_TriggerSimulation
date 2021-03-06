-- PROJECT: ATLAS MDT Calibration DB
-- AUTHOR: Elisabetta.Vilucchi@lnf.infn.it, celio@roma3.infn.it
-- DATE: May 08
-- VERSION v1r1

--USAGE:
--sqlplus <USER_NAME>/<USER_PASSWORD> @mdt_rt_map_s_table.sql <USER_NAME> <SITE_NAME>(2 characters like MU,MI,RM...) <USER_PASSOWRD> <TABLESPCE_NAME>

--DROP TABLE &1.."MDT_RT_MAP_S";
CREATE TABLE &1.."MDT_RT_MAP_S" 
("MDT_RT_MAP_S_ID"  NUMBER        NOT NULL ,
"MDT_RT_ID"	    NUMBER        NOT NULL ,
"BINS"		    NUMBER(4)     NOT NULL ,
"SITE_NAME"         VARCHAR2(2) DEFAULT UPPER('&2') ,
"INSERT_TIME"       VARCHAR2(19),
"S_1"		    NUMBER(3,2)   NOT NULL ,
"S_2"               NUMBER(3,2)   NOT NULL ,
"S_3"               NUMBER(3,2)   NOT NULL ,
"S_4"               NUMBER(3,2)   NOT NULL ,
"S_5"               NUMBER(3,2)   NOT NULL ,
"S_6"               NUMBER(3,2)   NOT NULL ,
"S_7"               NUMBER(3,2)   NOT NULL ,
"S_8"               NUMBER(3,2)   NOT NULL ,
"S_9"               NUMBER(3,2)   NOT NULL ,
"S_10"              NUMBER(3,2)   NOT NULL ,
"S_11"              NUMBER(3,2)   NOT NULL ,
"S_12"              NUMBER(3,2)   NOT NULL ,
"S_13"              NUMBER(3,2)   NOT NULL ,
"S_14"              NUMBER(3,2)   NOT NULL ,
"S_15"              NUMBER(3,2)   NOT NULL ,
"S_16"              NUMBER(3,2)   NOT NULL ,
"S_17"              NUMBER(3,2)   NOT NULL ,
"S_18"              NUMBER(3,2)   NOT NULL ,
"S_19"              NUMBER(3,2)   NOT NULL ,
"S_20"              NUMBER(3,2)   NOT NULL ,
"S_21"              NUMBER(3,2)   NOT NULL ,
"S_22"              NUMBER(3,2)   NOT NULL ,
"S_23"              NUMBER(3,2)   NOT NULL ,
"S_24"              NUMBER(3,2)   NOT NULL ,
"S_25"              NUMBER(3,2)   NOT NULL ,
"S_26"              NUMBER(3,2)   NOT NULL ,
"S_27"              NUMBER(3,2)   NOT NULL ,
"S_28"              NUMBER(3,2)   NOT NULL ,
"S_29"              NUMBER(3,2)   NOT NULL ,
"S_30"              NUMBER(3,2)   NOT NULL ,
"S_31"              NUMBER(3,2)   NOT NULL ,
"S_32"              NUMBER(3,2)   NOT NULL ,
"S_33"              NUMBER(3,2)   NOT NULL ,
"S_34"              NUMBER(3,2)   NOT NULL ,
"S_35"              NUMBER(3,2)   NOT NULL ,
"S_36"              NUMBER(3,2)   NOT NULL ,
"S_37"              NUMBER(3,2)   NOT NULL ,
"S_38"              NUMBER(3,2)   NOT NULL ,
"S_39"              NUMBER(3,2)   NOT NULL ,
"S_40"              NUMBER(3,2)   NOT NULL ,
"S_41"              NUMBER(3,2)   NOT NULL ,
"S_42"              NUMBER(3,2)   NOT NULL ,
"S_43"              NUMBER(3,2)   NOT NULL ,
"S_44"              NUMBER(3,2)   NOT NULL ,
"S_45"              NUMBER(3,2)   NOT NULL ,
"S_46"              NUMBER(3,2)   NOT NULL ,
"S_47"              NUMBER(3,2)   NOT NULL ,
"S_48"              NUMBER(3,2)   NOT NULL ,
"S_49"              NUMBER(3,2)   NOT NULL ,
"S_50"              NUMBER(3,2)   NOT NULL ,
"S_51"              NUMBER(3,2)   NOT NULL ,
"S_52"              NUMBER(3,2)   NOT NULL ,
"S_53"              NUMBER(3,2)   NOT NULL ,
"S_54"              NUMBER(3,2)   NOT NULL ,
"S_55"              NUMBER(3,2)   NOT NULL ,
"S_56"              NUMBER(3,2)   NOT NULL ,
"S_57"              NUMBER(3,2)   NOT NULL ,
"S_58"              NUMBER(3,2)   NOT NULL ,
"S_59"              NUMBER(3,2)   NOT NULL ,
"S_60"              NUMBER(3,2)   NOT NULL ,
"S_61"              NUMBER(3,2)   NOT NULL ,
"S_62"              NUMBER(3,2)   NOT NULL ,
"S_63"              NUMBER(3,2)   NOT NULL ,
"S_64"              NUMBER(3,2)   NOT NULL ,
"S_65"              NUMBER(3,2)   NOT NULL ,
"S_66"              NUMBER(3,2)   NOT NULL ,
"S_67"              NUMBER(3,2)   NOT NULL ,
"S_68"              NUMBER(3,2)   NOT NULL ,
"S_69"              NUMBER(3,2)   NOT NULL ,
"S_70"              NUMBER(3,2)   NOT NULL ,
"S_71"              NUMBER(3,2)   NOT NULL ,
"S_72"              NUMBER(3,2)   NOT NULL ,
"S_73"              NUMBER(3,2)   NOT NULL ,
"S_74"              NUMBER(3,2)   NOT NULL ,
"S_75"              NUMBER(3,2)   NOT NULL ,
"S_76"              NUMBER(3,2)   NOT NULL ,
"S_77"              NUMBER(3,2)   NOT NULL ,
"S_78"              NUMBER(3,2)   NOT NULL ,
"S_79"              NUMBER(3,2)   NOT NULL ,
"S_80"              NUMBER(3,2)   NOT NULL ,
"S_81"              NUMBER(3,2)   NOT NULL ,
"S_82"              NUMBER(3,2)   NOT NULL ,
"S_83"              NUMBER(3,2)   NOT NULL ,
"S_84"              NUMBER(3,2)   NOT NULL ,
"S_85"              NUMBER(3,2)   NOT NULL ,
"S_86"              NUMBER(3,2)   NOT NULL ,
"S_87"              NUMBER(3,2)   NOT NULL ,
"S_88"              NUMBER(3,2)   NOT NULL ,
"S_89"              NUMBER(3,2)   NOT NULL ,
"S_90"              NUMBER(3,2)   NOT NULL ,
"S_91"              NUMBER(3,2)   NOT NULL ,
"S_92"              NUMBER(3,2)   NOT NULL ,
"S_93"              NUMBER(3,2)   NOT NULL ,
"S_94"              NUMBER(3,2)   NOT NULL ,
"S_95"              NUMBER(3,2)   NOT NULL ,
"S_96"              NUMBER(3,2)   NOT NULL ,
"S_97"              NUMBER(3,2)   NOT NULL ,
"S_98"              NUMBER(3,2)   NOT NULL ,
"S_99"              NUMBER(3,2)   NOT NULL ,
"S_100"             NUMBER(3,2)   NOT NULL ,
CONSTRAINT "MDT_RT_MAP_S_MDT_RT_ID_REF" FOREIGN KEY ("MDT_RT_ID", "SITE_NAME") REFERENCES &1.."MDT_RT" ("MDT_RT_ID", "SITE_NAME") VALIDATE ,
CONSTRAINT "MDT_RT_MAP_S_ID_SITE_PR" PRIMARY KEY ("MDT_RT_MAP_S_ID", "SITE_NAME") VALIDATE )
TABLESPACE &4 PCTFREE 10 INITRANS 1 MAXTRANS 255 STORAGE ( INITIAL 64K BUFFER_POOL DEFAULT) LOGGING;
