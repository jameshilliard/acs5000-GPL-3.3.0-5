#ifdef PMD
void cycPmcMountCommand(pmcInfo *pmc,int outlet, char *user, char *command);
int cycWaitAnswer (pmcInfo *pmc);
pmcInfo * cycGetPmc(UInt32 serialPort,int size);
#endif
