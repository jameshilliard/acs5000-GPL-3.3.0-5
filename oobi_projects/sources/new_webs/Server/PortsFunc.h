
void portMountConnectPortsHtml(char *username, CycUserType userType,char * group_name);
void boxMountAppletHtml(char_t *mysid, char_t *myname, char_t *pass, webs_t wp);
void portMountAppletHtml(int portNum, char_t *mysid, char_t *myname, char_t *pass, webs_t wp);
void portMountPhysPortHtml(char * bufHtml);
void portGetPortConfiguration(void);
int portPhysChangeStatus(int);
int portPhysChangeGeneral(void);
int portPhysChangeSerial(void);
int portPhysChangeAccess(void);
int portPhysChangeDataBuf(void);
int portPhysChangeSysBuf(void);
#if defined(PMD) || defined(IPMI)
int portPhysChangePowerMgm(void);
#endif
int portPhysChangeOther(void);
int portPhysChangeMus(void);
int portPhysChangeCas(void);
int portPhysChangeTs(void);
int portPhysChangeDialIn(void);
int portPhysChangeAll(void);
#ifdef OLDPMD
void portPmFindUserlist(PhysPowerMgmInfo *power);
#endif
/*UserFunc.c:4808: warning: implicit declaration of function `portMountGrapherAppletHtml'*/
void portMountGrapherAppletHtml(int portNum, char_t *mysid, char_t *myname, char_t *pass, webs_t wp);
