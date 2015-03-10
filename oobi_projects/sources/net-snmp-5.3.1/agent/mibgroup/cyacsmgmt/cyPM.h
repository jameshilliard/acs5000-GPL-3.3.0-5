#ifndef CYPM_H
#define CYPM_H

#define CY_ID_SHM 0xFACBADEC
#define CY_ADDR_SHM     0x3FF00000
#define SHM_PERM	0666

/*
 * function declarations 
 */
void init_cyPM(void);

unsigned char *var_cyPMTable(struct variable *vp, oid * name, size_t * length,
		 int exact, size_t * var_len, WriteMethod ** write_method);

unsigned char *var_cyPMUnitTable(struct variable *vp, oid * name, 
	size_t * length, int exact, size_t * var_len, 
	WriteMethod ** write_method);

unsigned char *var_cyOutletTable(struct variable *vp, oid * name, 
	size_t * length, int exact, size_t * var_len, 
	WriteMethod ** write_method);

unsigned char *var_cyPMElecPduTable(struct variable *vp,oid * name,
        size_t * length,int exact,size_t * var_len,
        WriteMethod ** write_method);

unsigned char *var_cyPMElecPhaseTable(struct variable *vp,oid * name,
        size_t * length,int exact,size_t * var_len,
        WriteMethod ** write_method);

unsigned char *var_cyPMElecBankTable(struct variable *vp,oid * name,
        size_t * length,int exact,size_t * var_len,
        WriteMethod ** write_method);

unsigned char *var_cyPMElecOutletTable(struct variable *vp,oid * name,
        size_t * length,int exact,size_t * var_len,
        WriteMethod ** write_method);

unsigned char *var_cyPMEnvMonTable(struct variable *vp,oid * name,
	size_t * length,int exact,size_t * var_len,
	WriteMethod ** write_method);

int write_cyPM_command(int action, u_char * var_val, u_char var_val_type,
	   size_t var_val_len, u_char * statP, oid * name, size_t name_len);

int write_cyPM_save(int action, u_char * var_val, u_char var_val_type,
	   size_t var_val_len, u_char * statP, oid * name, size_t name_len);

int do_cyNumberOfPM(netsnmp_mib_handler *handler,
	netsnmp_handler_registration *reginfo,
	netsnmp_agent_request_info *reqinfo,
	netsnmp_request_info *requests);

int write_cyOutlet_name(int action, u_char * var_val, u_char var_val_type,
	size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyOutlet_lock(int action, u_char * var_val, u_char var_val_type,
   size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyOutlet_power(int action, u_char * var_val, u_char var_val_type,
   size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyOutlet_interval(int action, u_char * var_val, u_char var_val_type,
   size_t var_val_len, u_char * statP, oid * name, size_t name_len);

int write_cyPMUnit_current(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMUnit_temp(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);

int write_cyPMEnvMonTable_ValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);

int write_cyPMElecPhaseTable_CurrValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecPhaseTable_PowValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecPhaseTable_VoltValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecPhaseTable_PowfacValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);

int write_cyPMElecBankTable_CurrValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecBankTable_PowValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecBankTable_VoltValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecBankTable_PowfacValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);

int write_cyPMElecOutletTable_CurrValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecOutletTable_PowValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecOutletTable_VoltValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecOutletTable_PowfacValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);

int write_cyPMElecPduTable_CurrValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecPduTable_PowValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecPduTable_VoltValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
int write_cyPMElecPduTable_PowfacValRst(int action, u_char * var_val, u_char var_val_type,
      size_t var_val_len, u_char * statP, oid * name, size_t name_len);
#endif	/* CYPM_H */
