SET echo OFF;
drop table ACUT_DATA2TAG; 
drop table ACUT_DATA;
drop table ALMN_DATA2TAG; 
drop table ALMN_DATA;
drop table ASMP_DATA2TAG; 
drop table ASMP_DATA;
drop table AMDC_DATA2TAG; 
drop table AMDC_DATA;
drop table ATYP_DATA2TAG; 
drop table ATYP_DATA;
drop table ALIN_DATA2TAG; 
drop table ALIN_DATA;
drop table APTP_DATA2TAG; 
drop table APTP_DATA;

purge user_recyclebin;
commit;
select * from tab;
select * from user_ts_quotas;
