/****************************************************************************/
/* File: pmdnginfo.h 														*/
/* Description: Function for Get and Set of PMDng information				*/
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include <webApi.h>

#define PMDCONF "/etc/pmd.conf"
#define PMDUSER "/etc/pmd.users"
#define PMDGRPS "/etc/pmdgrp.conf"

#define PASSWDSIZE 30
#define USRNAMESIZE 50
#define TYPESIZE 30
#define SECTIONNAMESIZE 20
#define LINESIZE 512
#define CONFSECSIZE LINESIZE * 7
#define FILESIZE LINESIZE * 64
#define IPDUNAMESIZE 30
#define IPDUIDNAMESIZE 30
#define IPDUFIELDNAMESIZE 30
#define IPDUVALUENAMESIZE 30
#define VENDORNAMESIZE 30
#define DEVNAMESIZE 6
#define GRPNAMESIZE 20
#define MEMBERSIZE 20
#define PARAMNAMESIZE 15
#define VALUENAMESIZE 15
#define INFEEDNAMESIZE 20
#define IPLEN 15

#define NUMFIELD 7
#define FIELDNAMESIZE 20

#define kSuccess 1
#define kFailure 0

/****************************************************************************/
/* Check Functions															*/
/****************************************************************************/
char CheckConfSection(int fd);
char CheckVendor(int fd, char *Vendor);
char CheckField(int fd, char *Vendor, char *Field);
int StartWithIP(char *pLine);

/****************************************************************************/
/* Get Functions															*/
/****************************************************************************/
UInt32 GetSerialPortInfo(char *SPort, int Size);
PMDPortSpeed GetSerialPortSpeed(char *Speed, int Size);
CycPortBits GetSerialPortBits(char *Bits, int Size);
CycPortParityType GetSerialPortParity(char *Parity, int Size);
CycPortStopBits GetSerialPortStopBit(char *StopBit, int Size);
CycPortFlowType GetSerialPortFlowType(char *Flow, int Size);
IPDUvendor GetSerialPortVendor(char *Vendor, int Size);

/****************************************************************************/
/* Set Functions															*/
/****************************************************************************/
int SetSerialPortInfo(UInt32 Num, char *PortNum);
int SetSerialPortSpeed(PMDPortSpeed PortSpeed, char *Speed);
int SetSerialPortBits(CycPortBits PortBits, char *Bits);
int SetSerialPortParity(CycPortParityType PortParity, char *Parity);
int SetSerialPortStopBit(CycPortStopBits PortStopBit, char *StopBit);
int SetSerialPortFlowType(CycPortFlowType PortFlow, char *Flow);
int SetSerialPortVendor(IPDUvendor PortVendor, char *Vendor);
int SetGenInfo(int fd, IPDUvendor vnd, char *Field, char *Value);
int SetPortInfo(int fd, char *Line, Action act);
int SetIpduInfo(int fd, char *Line, Action act);
int SetGrpDef(int fd,char *Line, Action act);
int SetUserMgm(int fd,char *Line, Action act);
int setIpduInfoParam(PMDIpduInfo *pIpduInfo,char *Parameter, char *Value);

/****************************************************************************/
/* Miscellaneous Functions													*/
/****************************************************************************/
int power(int base, int exp);

/****************************************************************************/
/* Get Number of Items														*/
/****************************************************************************/
int GetConfNumPort(char *pFile);
int GetConfNumOutGrps(char *pFile);
int GetConfNumUsers(char *pFile);

/****************************************************************************/
/* Function: CheckConfSection												*/
/* Description: Check if [config] section exist in /etc/pmd.conf			*/
/* Return:																	*/
/*			0 - Section does not exist										*/
/*			1 - Section exist												*/
/****************************************************************************/
char CheckConfSection(int fd)
{
	int nread;
	char ch;
	char ConfSection;

	nread = 0;
	ConfSection = 0;

	/* Check if "[config]" section exist */	
	while ( (nread = read(fd,&ch,1)) != 0 ) {
		if ( ch == '[' ) {
			char Section[SECTIONNAMESIZE];
			int s;

			memset(Section,'\0',SECTIONNAMESIZE);
			s = 0;
			nread = read(fd,&ch,1);

			while ( (ch != ']') && (s < SECTIONNAMESIZE) ) {
				Section[s] = ch;
				s++;
				nread = read(fd,&ch,1);
			}
			if ( !strncmp("config",Section,strlen(Section)) ) {
				ConfSection = 1;
				break;
			}
		}
	}

	lseek(fd,0,SEEK_SET);
	return ConfSection;
}

/****************************************************************************/
/* Function: CheckVendor													*/
/* Description: Check if vendor exist inside [config] section				*/
/* Return:																	*/
/*			0 - Vendor does not exist										*/
/*			1 - Vendor exist												*/
/****************************************************************************/
char CheckVendor(int fd, char *Vendor)
{
	char VendorExist;
	int nread;
	char ch;
	int offset;

	VendorExist = 0;
	offset = 0;

	while ( (nread = read(fd,&ch,1)) != 0 ) {
		offset++;
		if ( ch == '[' ) {
			break;
		} 
		if ( ch == Vendor[0] ) {
			char VendorName[strlen(Vendor)];
			int v;

			memset(VendorName,'\0',strlen(Vendor));
			v = 0;

			while ( (ch != '.') && (v < strlen(Vendor)) ) {
				VendorName[v] = ch;
				v++;
				nread = read(fd,&ch,1);
				offset++;
			}
			if ( !strncmp(VendorName,Vendor,strlen(Vendor)) ) {
				VendorExist = 1;
				break;
			}
		}
	}

	offset--;
	lseek(fd,-offset,SEEK_CUR);
	return VendorExist;
}

/****************************************************************************/
/* Function: CheckField														*/
/* Description: Check if field exist for vendor inside [config] section		*/
/* Return:																	*/
/*			0 - Field does not exist										*/
/*			1 - Field exist													*/
/****************************************************************************/
char CheckField(int fd, char *Vendor, char *Field)
{
	char FieldExist;
	int nread;
	char ch;
	int offset;

	FieldExist = 0;
	nread = 0;
	offset = 0;

	while ( (nread = read(fd,&ch,1)) != 0 ) {
		offset++;
		if ( ch == Vendor[0] ) {
			char VendorName[strlen(Vendor)];
			int v;

			memset(VendorName,'\0',strlen(Vendor));
			v = 0;

			while ( (ch != '.') && (v < strlen(Vendor)) ) {
				VendorName[v] = ch;
				v++;
				nread = read(fd,&ch,1);
				offset++;
			}
			if ( !strncmp(VendorName,Vendor,strlen(Vendor)) ) {
				while ( ch != ' ' && ch != '\n' ) {
					nread = read(fd,&ch,1);
					offset++;

					if ( ch == Field[0] ) {
						char FieldName[strlen(Field)];
						int f;
		
						memset(FieldName,'\0',strlen(Field));
						f = 0;
	
						while ( (ch != ' ') && (f < strlen(Field)) ) {
							FieldName[f] = ch;
							f++;
							nread = read(fd,&ch,1);
							offset++;
						}
	
						if ( !strncmp(FieldName,Field,strlen(Field)) ) {
							FieldExist = 1;
							break;
						}
					}
				}

				if ( FieldExist ) {
					break;
				}
			}
		}
	} 

	 /* Point fd after "[config]" */
	if ( ! FieldExist ) {
		offset -= 7;
	}
	lseek(fd,-offset,SEEK_CUR);
	return FieldExist;
}

/****************************************************************************/
/* Function: SetGenInfo														*/
/* Description: Set the value for vendor.field in /etc/pmd.conf				*/
/* Parameters:																*/
/*				fd - File descriptor for /etc/pmd.conf						*/
/*				vnd - IPDU Vendor											*/
/*				Field - Pointer for username or password					*/
/*				Value - Pointer for the value to be assigned				*/
/****************************************************************************/
int SetGenInfo(int fd, IPDUvendor vnd, char *Field, char *Value)
{
	char Buff[LINESIZE];
	char ConfSecBuff[CONFSECSIZE];
	char *pBuff;
	int BuffSize;
	char ConfSection;
	char VendorExist;
	char FieldExist;
	int nread, nwrite, totalwrite;
	char ch;
	int newfd;
	char Vendor[VENDORNAMESIZE];

	memset(Buff,'\0',LINESIZE);
	memset(ConfSecBuff,'\0',CONFSECSIZE);
	pBuff = ConfSecBuff;
	BuffSize = 0;
	ConfSection = 0;
	VendorExist = 0;
	FieldExist = 0;
	nread = 0;
	nwrite = 0;
	totalwrite = 0;
	memset(Vendor,'\0',VENDORNAMESIZE);

	/* Get Vendor Name */
	switch (vnd) {
		case cyclades:
			strncpy(Vendor,"cyclades",8);
			break;
		case spc:
			strncpy(Vendor,"spc",3);
			break;
		case servertech:
			strncpy(Vendor,"servertech",10);
			break;
	}

	/* Check if [config] section exist */
	ConfSection = CheckConfSection(fd);

	if ( ConfSection ) {
		while ( (nread = read(fd,&ch,1)) != 0 ) {
			if ( ch == '[' ) {
				char Section[SECTIONNAMESIZE];
				int s;

				*pBuff = ch;
				pBuff++;
				memset(Section,'\0',SECTIONNAMESIZE);
				s = 0;
				nread = read(fd,&ch,1);
				*pBuff = ch;
				pBuff++;

				while ( (ch != ']') && (s < SECTIONNAMESIZE) ) {
					Section[s] = ch;
					s++;
					nread = read(fd,&ch,1);
					*pBuff = ch;
					pBuff++;
				}

				if ( !strncmp(Section,"config",6) ) {
					/* Check if Vendor exist */
					VendorExist = CheckVendor(fd,Vendor);

					if ( VendorExist ) {
						/* Check if Field exist */
						FieldExist = CheckField(fd,Vendor, Field);
		
						if ( FieldExist ) {
							/* Change value of this field */
							nread = read(fd,&ch,1);
							*pBuff = '\n';
							pBuff++;
							while ( ch != '[' && (nread) ) {
								/* The whole "[config]" section is treated here */
								if ( ch == Vendor[0] ) {
									char VendorName[strlen(Vendor)];
									int v;

									memset(VendorName,'\0',strlen(Vendor));
									v = 0;

									while ( (ch != '.') && (v < strlen(Vendor)) ) {
										VendorName[v] = ch;
										v++;
										nread = read(fd,&ch,1);
									}

									if ( !strncmp(VendorName,Vendor,strlen(Vendor)) ) {
										sprintf(pBuff,"%s",Vendor);
										pBuff += strlen(Vendor);
										while (ch != '\n'){
											if ( ch == Field[0] ) {
												char FieldName[strlen(Field)];
												int f;

												memset(FieldName,'\0',strlen(Field));
												f = 0;

												while ( (ch != ' ') && (f < strlen(Field)) ){
													FieldName[f] = ch;
													f++;
													nread = read(fd,&ch,1);
												}

												if ( !strncmp(FieldName,Field,strlen(Field)) ) {
													sprintf(pBuff,"%s",Field);
													pBuff += strlen(Field);
													while ( ch != '\n' ) {
														nread = read(fd,&ch,1);
													}

													sprintf(pBuff," %s",Value);
													pBuff += 1 + strlen(Value);
												}
												else {
													f++;
													lseek(fd,-f,SEEK_CUR);
													nread = read(fd,&ch,1);
												}
											}
											*pBuff = ch;
											pBuff++;
											nread = read(fd,&ch,1);
										}
									}
									else {
										v++;
										lseek(fd,-v,SEEK_CUR);
										nread = read(fd,&ch,1);
									}
								}
								*pBuff = ch;
								pBuff++;
								nread = read(fd,&ch,1);
							}
							if ( ch == '[' ) {
								strncpy(pBuff,"\n[",2);
								pBuff += 2;
							}
						}
						else {
							*pBuff = '\n';
							pBuff++;
							sprintf(pBuff,"%s.%s %s\n",Vendor,Field,Value);
							pBuff += (strlen(Vendor) + 1 + strlen(Field) + 1 + strlen(Value) + 1);
						}
					}
					else { /* Vendor does not exist */
						*pBuff = '\n';
						pBuff++;
						sprintf(pBuff,"%s.%s %s\n",Vendor,Field,Value);
						pBuff += (strlen(Vendor) + 1 + strlen(Field) + 1 + strlen(Value) + 1);
					}
				}
			}
			else {
				*pBuff = ch;
				pBuff++;
			}
		}
	}
	else {
		strncpy(pBuff,"[config]\n",9);
		pBuff += 9;
		sprintf(pBuff,"%s.%s %s\n",Vendor,Field,Value);
		pBuff += (strlen(Vendor) + 1 + strlen(Field) + 1 + strlen(Value) + 1);

		/* Get the whole file */
		while ( (nread = read(fd,&ch,1)) != 0 ) {
			*pBuff = ch;
			pBuff++;
		}
	}

	close(fd);

	/* Get new file size */
	BuffSize = strlen(Buff);

	newfd = open("/tmp/geninfo.tmp",O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	if ( newfd == -1 ) {
		return kFailure;
	}

	/* Write new buff to new file */
	while ( totalwrite < BuffSize ) {
		nwrite = 0;
		nwrite = write(newfd,Buff + totalwrite,BuffSize - totalwrite);
		totalwrite += nwrite;
	}
	close(newfd);

	/* Remove /etc/pmd.conf */
	unlink ("/etc/pmd.conf");

	/* Link new file to /etc/pmd.conf */
	link("/tmp/geninfo.tmp","/etc/pmd.conf");
	unlink("/tmp/geninfo.tmp");

	fd = open("/etc/pmd.conf",O_RDWR);

	return kSuccess;
}

/****************************************************************************/
/* Function: GetSerialPortInfo												*/
/* Description: Translate Sport into an int value							*/ 
/* Parameters:																*/
/*				Sport - Port number in char format							*/
/*				Size - Size of Sport										*/
/****************************************************************************/
UInt32 GetSerialPortInfo(char *SPort, int Size)
{
	char PortNum;
	UInt32 Num;

	PortNum = *(SPort + (Size - 1));
	PortNum = PortNum - 0x30;
	Num = (UInt32) PortNum;

	return 0;
}

/****************************************************************************/
/* Function: GetSerialPortSpeed												*/
/* Description: Translate Speed into a PMDPortSpeed enum value				*/ 
/* Parameters:																*/
/*				Speed - Speed number in char format							*/
/*				Size - Size of Speed										*/
/****************************************************************************/
PMDPortSpeed GetSerialPortSpeed(char *Speed, int Size)
{
	PMDPortSpeed Num;

	if ( Size == 0 ) {
		Num = pmdAuto;
	}
	else {
		if ( !strncmp(Speed,"1200",4) ) {
			Num = pmd1200;
		}
		else {
			if ( !strncmp(Speed,"2400",4) ) {
				Num = pmd2400;
			}
			else {
				if ( !strncmp(Speed,"4800",4) ) {
					Num = pmd4800;
				}
				else {
					if ( !strncmp(Speed,"9600",4) ) {
						Num = pmd9600;
					}
					else {
						if ( !strncmp(Speed,"19200",5) ){
							Num = pmd19200;
						}
						else {
							if ( !strncmp(Speed,"38400",5) ) {
								Num = pmd38400;
							}
							else {
								if ( !strncmp(Speed,"57600",5) ) {
									Num = pmd57600;
								}
								else {
									if ( !strncmp(Speed,"115200",6) ) {
										Num = pmd115200;
									}
									else {
										Num = pmdAuto;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return Num;
}

/****************************************************************************/
/* Function: GetSerialPortBits												*/
/* Description: Translate Bits into a CycPortBits enum value				*/ 
/* Parameters:																*/
/*				Bits - Bits number in char format							*/
/*				Size - Size of Bits											*/
/****************************************************************************/
CycPortBits GetSerialPortBits(char *Bits, int Size)
{
	CycPortBits Num;

	if ( Size == 0 ) {
		Num = kPb5;
	}
	else {
		if ( *Bits == '5' ) {
			Num = kPb5;
		}
		else {
			if ( *Bits == '6' ) {
				Num = kPb6;
			}
			else {
				if ( *Bits == '7' ) {
					Num = kPb7;
				}
				else {
					if ( *Bits == '8' ) {
						Num = kPb8;
					}
					else {
						Num = kPb5; 
					}
				}
			}
		}
	}

	return Num;
}

/****************************************************************************/
/* Function: GetSerialPortParity											*/
/* Description: Translate Parity into a CycPortParityType enum value		*/ 
/* Parameters:																*/
/*				Parity - Parity number in char format						*/
/*				Size - Size of Parity										*/
/****************************************************************************/
CycPortParityType GetSerialPortParity(char *Parity, int Size)
{
	CycPortParityType Num;

	if ( Size == 0 ) {
		Num = kPptNone;
	}
	else {
		if ( *Parity == 'O' ) {
			Num = kPptOdd;
		}
		else {
			if ( *Parity == 'E' ) {
				Num = kPptEven;
			}
			else {
				Num = kPptNone;
			}
		}
	}

	return Num;
}

/****************************************************************************/
/* Function: GetSerialPortStopBit											*/
/* Description: Translate StopBit into a CycPortStopBits enum value			*/ 
/* Parameters:																*/
/*				StopBit - StopBit number in char format						*/
/*				Size - Size of StopBit										*/
/****************************************************************************/
CycPortStopBits GetSerialPortStopBit(char *StopBit, int Size)
{
	CycPortStopBits Num;

	if ( Size == 0 ) {
		Num = kPsb1;
	}
	else {
		if ( *StopBit == '1' ) {
			Num = kPsb1;
		}
		else {
			if ( *StopBit == '2' ) {
				Num = kPb2;
			}
			else {
				Num = kPsb1;
			}
		}
	}

	return Num;
}

/****************************************************************************/
/* Function: GetSerialPortFlowType											*/
/* Description: Translate FlowType into a CycPortFlowType enum value		*/ 
/* Parameters:																*/
/*				Flow - Flow number in char format							*/
/*				Size - Size of Flow											*/
/****************************************************************************/
CycPortFlowType GetSerialPortFlowType(char *Flow, int Size)
{
	CycPortFlowType Num;

	if ( Size == 0 ) {
		Num = kPftNone;
	}
	else {
		if ( !strncmp(Flow,"none",4) ) {
			Num = kPftNone;
		}
		else {
			if ( !strncmp(Flow,"hard",4) ) {
				Num = kPftHard;
			}
			else {
				if ( !strncmp(Flow,"soft",4) ) {
					Num = kPftSoft;
				}
				else {
					Num = kPftNone;
				}
			}
		}
	}

	return Num;
}

/****************************************************************************/
/* Function: GetSerialPortVendor											*/
/* Description: Translate Vendor into an IPDUvendor enum value				*/ 
/* Parameters:																*/
/*				Vendor - Flow number in char format							*/
/*				Size - Size of Vendor										*/
/****************************************************************************/
IPDUvendor GetSerialPortVendor(char *Vendor, int Size)
{
	IPDUvendor Num;

	if ( Size == 0 ) {
		Num = cyclades;
	}
	else {
		if ( !strncmp(Vendor,"cyclades",8) ) {
			Num = cyclades;
		}
		else {
			if ( !strncmp(Vendor,"spc",3) ) {
				Num = spc;
			}
			else {
				if ( !strncmp(Vendor,"servertech",10) ) {
					Num = servertech;
				}
				else {
					Num = cyclades;
				}
			}
		}
	}

	return Num;
}

/****************************************************************************/
/* Function: SetSerialPortInfo												*/
/* Description: Translate Num into a char value								*/ 
/* Parameters:																*/
/*				Num - Port number in int 32 format							*/
/*				PortNum - Port number in char format						*/
/****************************************************************************/
int SetSerialPortInfo(UInt32 Num, char *PortNum)
{
	int ret;

	ret = 0;
	*PortNum = (char) Num + 0x30;
	ret = strlen(PortNum);

	return ret;
}

/****************************************************************************/
/* Function: SetSerialPortSpeed												*/
/* Description: Translate PortSpeed into a char value						*/ 
/* Parameters:																*/
/*				PortSpeed - Port number in PMDPortSpeed enum value			*/
/*				PortNum - Port number in char format						*/
/****************************************************************************/
int SetSerialPortSpeed(PMDPortSpeed PortSpeed, char *Speed)
{
	int ret;

	ret = 0;

	switch ( PortSpeed ) {
		case pmdAuto:
			strncpy(Speed,"::",2);
			ret = 6;
			break;

		case pmd1200:
			strncpy(Speed,":1200:",6);
			ret = 6;
			break;

		case pmd2400:
			strncpy(Speed,":2400:",6);
			ret = 6;
			break;

		case pmd4800:
			strncpy(Speed,":4800:",6);
			ret = 6;
			break;

		case pmd9600:
			strncpy(Speed,":9600:",6);
			ret = 6;
			break;

		case pmd19200:
			strncpy(Speed,":19200:",7);
			ret = 7;
			break;

		case pmd38400:
			strncpy(Speed,":38400:",7);
			ret = 7;
			break;

		case pmd57600:
			strncpy(Speed,":57600:",7);
			ret = 7;
			break;

		case pmd115200:
			strncpy(Speed,":115200:",8);
			ret = 8;
			break;

		default:
			strncpy(Speed,"::",2);
			break;
	}

	return ret;
}

/****************************************************************************/
/* Function: SetSerialPortBits												*/
/* Description: Translate PortBits into a char value						*/ 
/* Parameters:																*/
/*				PortBits - Port number in CycPortBits enum value			*/
/*				PortNum - Port number in char format						*/
/****************************************************************************/
int SetSerialPortBits(CycPortBits PortBits, char *Bits)
{
	int ret;

	switch ( PortBits ) {
		case kPb5:
			*Bits = '5';
			ret = 1;
			break;

		case kPb6:
			*Bits = '6';
			ret = 1;
			break;

		case kPb7:
			*Bits = '7';
			ret = 1;
			break;

		case kPb8:
			*Bits = '8';
			ret = 1;
			break;

		default:
			*Bits = '5';
			ret = 1;
	}

	return ret;
}

/****************************************************************************/
/* Function: SetSerialPortParity											*/
/* Description: Translate PortParity into a char value						*/ 
/* Parameters:																*/
/*				PortParity - Port parity in CycPortParityType enum value	*/
/*				Parity - Parity number in char format						*/
/****************************************************************************/
int SetSerialPortParity(CycPortParityType PortParity, char *Parity)
{
	int ret;

	switch ( PortParity ) {
		case kPptNone:
			*Parity = 'O';
			ret = 1;
			break;

		case kPptEven:
			*Parity = 'E';
			ret = 1;
			break;

		default:
			*Parity = 'N';
			ret = 1;
	}

	return ret;
}

/****************************************************************************/
/* Function: SetSerialPortStopBit											*/
/* Description: Translate PortStopBit into a char value						*/ 
/* Parameters:																*/
/*				PortStopBit - Port stopbit in CycPortStopBits enum value	*/
/*				StopBit - StopBit number in char format						*/
/****************************************************************************/
int SetSerialPortStopBit(CycPortStopBits PortStopBit, char *StopBit)
{
	int ret;

	switch ( PortStopBit ) {
		case kPsb1:
			*StopBit = '1';
			ret = 1;
			break;

		case kPb2:
			*StopBit = '2';
			ret = 1;
			break;

		default:
			*StopBit = '1';
			ret = 1;
			break;
	}

	return ret;
}

/****************************************************************************/
/* Function: SetSerialPortFlowType											*/
/* Description: Translate FlowType into a char value						*/ 
/* Parameters:																*/
/*				FlowType - Port Flow in CycPortFlowType enum value			*/
/*				Flow - Flow type number in char format						*/
/****************************************************************************/
int SetSerialPortFlowType(CycPortFlowType PortFlow, char *Flow)
{
	int ret;

	switch ( PortFlow ) {
		case kPftHard:
			strncpy(Flow,"hard",4);
			ret = 4;
			break;

		case kPftSoft:
			strncpy(Flow,"soft",4);
			ret = 4;
			break;

		default: 
			strncpy(Flow,"none",4);
			ret = 4;
			break;
	}

	return ret;
}

/****************************************************************************/
/* Function: SetSerialPortVendor											*/
/* Description: Translate PortVendor into a char value						*/ 
/* Parameters:																*/
/*				PortVendor - Port Vendor in IPDUvendor enum value			*/
/*				Vendor - Vendor type in char format							*/
/****************************************************************************/
int SetSerialPortVendor(IPDUvendor PortVendor, char *Vendor)
{
	int ret;

	switch ( PortVendor ) {
		case cyclades:
			strncpy(Vendor,"cyclades",8);
			ret = 8;
			break;

		case spc:
			strncpy(Vendor,"spc",3);
			ret = 3;
			break;

		case servertech:
			strncpy(Vendor,"servertech",10);
			ret = 10;
			break;

		default:
			strncpy(Vendor,"cyclades",8);
			ret = 8;
			break;
	}

	return ret;
}

/****************************************************************************/
/* Function: SetPortInfo													*/
/* Description: Set all values for a port in /etc/pmd.conf	 				*/
/* Parameters:																*/
/*				fd - File descriptor for /etc/pmd.conf						*/
/*				Line - A line with port values								*/
/*				Action - Action enum value									*/
/****************************************************************************/
int SetPortInfo(int fd, char *Line, Action act)
{
	char Buff[FILESIZE];
	char *pBuff;
	int BuffSize;
	int nread, nwrite;
	char ch;
	char SectionExist, LineExist;
	char Field[NUMFIELD][FIELDNAMESIZE];
	char fieldsize;
	char *pch;
	int i, newfd, totalwrite;

	memset(Buff,'\0',FILESIZE);
	pBuff = Buff;
	BuffSize = 0;
	SectionExist = 0;
	LineExist = 0;
	fieldsize = 0;
	totalwrite = 0;

	/* Go to "[port]" section */
	while ( (nread = read(fd,&ch,1)) != 0 ) {
		*pBuff = ch;
		pBuff++;
		if ( ch == '[' ) {
			char Section[SECTIONNAMESIZE];
			int s;

			memset(Section,'\0',SECTIONNAMESIZE);
			s = 0;
			nread = read(fd,&ch,1);

			while ( (ch != ']') && (s < SECTIONNAMESIZE) ) {
				Section[s] = ch;
				s++;
				nread = read(fd,&ch,1);
			}

			if ( !strncmp(Section,"port",4) ) {
				SectionExist = 1;
				strncpy(pBuff,Section,strlen(Section));
				pBuff += strlen(Section);
				break;
			}
			else {
				lseek(fd,-s,SEEK_CUR);
			}
		}
	}

	if ( ! SectionExist ) {
		/* Create section "[port]" */
		strncpy(pBuff,"\n[port]\n",8);
		pBuff += 8;
	}

	/* Get all fields */
	pch = Line;
	i = 0;

	while ( *pch != '\n' ) {
		if ( *pch == ':' ) {
			memset(Field[i],'\0',FIELDNAMESIZE);
			strncpy(Field[i],pch - fieldsize,fieldsize);
			fieldsize = 0;
			pch++;
			i++;
		}
		else {
			fieldsize++;
			pch++;
		}
	}

	switch ( act ) {
		case kDelete:
			if ( SectionExist ) {
				if ( *pch == Field[0][0] ) {
					char Device[DEVNAMESIZE];
					int d;

					memset(Device,'\0',DEVNAMESIZE);
					d = 0;

					while ( (ch != ':') && (d < DEVNAMESIZE) ) {
						Device[d] = ch;
						d++;
						nread = read(fd,&ch,1);
					}

					if ( !strncmp(Device,Field[0],strlen(Field[0])) ) {
						/* That is the line - Do not put it in the Buffer */
						while ( ch != '\n' ) {
							nread = read(fd,&ch,1);
						} 
					}
					else {
						lseek(fd,-d,SEEK_CUR);
					}
				}
				else {
					while ( ch != '\n' ) {
						nread = read(fd,&ch,1);
						*pBuff = ch;
						pBuff++;
					}
				}
			}
			break;

		case kAdd:
		case kEdit:
			if ( !SectionExist ) {
				strncpy(pBuff,Line,strlen(Line));
				pBuff += strlen(Line);
			}
			else {
				if ( SectionExist ) {
					if ( *pch == Field[0][0] ) {
						char Device[DEVNAMESIZE];
						int d;

						memset(Device,'\0',DEVNAMESIZE);
						d = 0;

						while ( (ch != ':') && (d < DEVNAMESIZE) ) {
							Device[d] = ch;
							d++;
							nread = read(fd,&ch,1);
						}

						if ( !strncmp(Device,Field[0],strlen(Field[0])) ) {
							/* That is the line - Do not put it in the Buffer */
							while ( ch != '\n' ) {
								nread = read(fd,&ch,1);
							} 
							strncpy(pBuff,Line,strlen(Line));
							pBuff += strlen(Line);
						}
						else {
							lseek(fd,-d,SEEK_CUR);
						}
					}
					else {
						while ( ch != '\n' ) {
							nread = read(fd,&ch,1);
							*pBuff = ch;
							pBuff++;
						}
					}
				}
			}
			break;

		case kActionIgnore:
			break;
	}

	close(fd);

	/* Get new file size */
	BuffSize = strlen(Buff);

	newfd = open("/tmp/pmd.tmp",O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	if ( newfd == -1 ) {
		return kFailure;
	}

	/* Write new buff to new file */
	while ( totalwrite < BuffSize ) {
		nwrite = 0;
		nwrite = write(newfd,Buff + totalwrite,BuffSize - totalwrite);
		totalwrite += nwrite;
	}
	close(newfd);

	/* Remove /etc/pmd.conf */
	unlink ("/etc/pmd.conf");

	/* Link new file to /etc/pmd.conf */
	link("/tmp/pmd.tmp","/etc/pmd.conf");
	unlink("/tmp/pmd.tmp");

	fd = open("/etc/pmd.conf",O_RDWR);

	return 0;
}

/****************************************************************************/
/* Function: SetIpduInfo													*/
/* Description: Set all values for an IPDU in /etc/pmd.conf	 				*/
/* Parameters:																*/
/*				fd - File descriptor for /etc/pmd.conf						*/
/*				Line - A line with IPDU values								*/
/*				Action - Action enum value									*/
/****************************************************************************/
int SetIpduInfo(int fd, char *Line, Action act)
{
	char Buff[FILESIZE];
	char *pBuff;
	int BuffSize;
	int nread, nwrite;
	char ch;
	char SectionExist, LineExist;
	char IPDUid[IPDUIDNAMESIZE], Parameter[PARAMNAMESIZE], Value[VALUENAMESIZE];
	char *pch;
	int i, p, v;
	int newfd, totalwrite;

	memset(Buff,'\0',FILESIZE);
	pBuff = Buff;
	BuffSize = 0;
	SectionExist = 0;
	LineExist = 0;
	memset(IPDUid,'\0',IPDUIDNAMESIZE);
	memset(Parameter,'\0',PARAMNAMESIZE);
	memset(Value,'\0',VALUENAMESIZE);
	totalwrite = 0;

	/* Go to "[idpu]" section */
	while ( (nread = read(fd,&ch,1)) != 0 ) {
		*pBuff = ch;
		pBuff++;
		if ( ch == '[' ) {
			char Section[SECTIONNAMESIZE];
			int s;

			memset(Section,'\0',SECTIONNAMESIZE);
			s = 0;
			nread = read(fd,&ch,1);

			while ( (ch != ']') && (s < SECTIONNAMESIZE) ) {
				Section[s] = ch;
				s++;
				nread = read(fd,&ch,1);
			}

			if ( !strncmp(Section,"idpu",4) ) {
				SectionExist = 1;
				strncpy(pBuff,Section,strlen(Section));
				pBuff += strlen(Section);
				break;
			}
			else {
				lseek(fd,-s,SEEK_CUR);
			}
		}
	}

	if ( ! SectionExist ) {
		/* Create section "[idpu]" */
		strncpy(pBuff,"\n[idpu]\n",8);
		pBuff += 8;
	}

	/* Get all fields */
	pch = Line;

	/* Get IPDU id */
	i = 0;
	while ( *pch != '.' ) {
		IPDUid[i] = *pch;
		i++;
		pch++;
	}

	/* Get Parameter */
	pch++; /* Jump the '.' */
	p = 0;
	while ( *pch != ' ' ) {
		Parameter[p] = *pch;
		p++;
		pch++;
	}

	/* Get Value */
	pch++; /* Jump the ' ' */
	v = 0;
	while ( *pch != '\n' ) {
		Value[v] = *pch;
		v++;
		pch++;
	}

	switch ( act ) {
		case kDelete:
			if ( SectionExist ) {
				while ( (nread = read(fd,&ch,1)) != 0 ) {
					if ( ch == IPDUid[0] ) {
						/* Check IPDU id */
						char IPDU[IPDUIDNAMESIZE];
						int i;

						memset(IPDU,'\0',IPDUIDNAMESIZE);
						i = 0;

						while ( (ch != '.') && (i < IPDUIDNAMESIZE)) {
							IPDU[i] = ch;
							i++;
							nread = read(fd,&ch,1);
						}

						if ( !strncmp(IPDU,IPDUid,strlen(IPDUid)) ) {
							/* Check Parameter */
							char Param[PARAMNAMESIZE];
							int p;

							memset(Param,'\0',PARAMNAMESIZE);
							p = 0;

							nread = read(fd,&ch,1); /* Jump '.' */
							while ( (ch != ' ') && (p < PARAMNAMESIZE) ) {
								Param[p] = ch;
								nread = read(fd,&ch,1);
							}

							if ( !strncmp(Param,Parameter,strlen(Parameter)) ) {
								/* Remove line */
								while ( ch != '\n' ) {
									nread = read(fd,&ch,1);
								}
							}
							else {
								lseek(fd,-(i+p),SEEK_CUR);
							}
						}
						else{
							lseek(fd,-i,SEEK_CUR);
						}
					}
					else {
						*pBuff = ch;
						pBuff++;
					}
				}
			}
			break;

		case kAdd:
		case kEdit:
			if ( !SectionExist ) {
				strncpy(pBuff,Line,strlen(Line));
				pBuff += strlen(Line);
			}
			else {
				while ( (nread = read(fd,&ch,1)) != 0 ) {
					if ( ch == IPDUid[0] ) {
						/* Check IPDU id */
						char IPDU[IPDUIDNAMESIZE];
						int i;

						memset(IPDU,'\0',IPDUIDNAMESIZE);
						i = 0;

						while ( (ch != '.') && (i < IPDUIDNAMESIZE)) {
							IPDU[i] = ch;
							i++;
							nread = read(fd,&ch,1);
						}

						if ( !strncmp(IPDU,IPDUid,strlen(IPDUid)) ) {
							/* Check Parameter */
							char Param[PARAMNAMESIZE];
							int p;

							memset(Param,'\0',PARAMNAMESIZE);
							p = 0;

							nread = read(fd,&ch,1); /* Jump '.' */
							while ( (ch != ' ') && (p < PARAMNAMESIZE) ) {
								Param[p] = ch;
								nread = read(fd,&ch,1);
							}

							if ( !strncmp(Param,Parameter,strlen(Parameter)) ) {
								/* Replace line */
								while ( ch != '\n' ) {
									nread = read(fd,&ch,1);
								}
								strncpy(pBuff,Line,strlen(Line));
								pBuff += strlen(Line);
							}
							else {
								lseek(fd,-(i+p),SEEK_CUR);
							}
						}
						else{
							lseek(fd,-i,SEEK_CUR);
						}
					}
					else {
						*pBuff = ch;
						pBuff++;
					}
				}
			}
			break;

		case kActionIgnore:
			break;
	}

	close(fd);

	/* Get new file size */
	BuffSize = strlen(Buff);

	newfd = open("/tmp/pmd.tmp",O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	if ( newfd == -1 ) {
		return kFailure;
	}

	/* Write new buff to new file */
	while ( totalwrite < BuffSize ) {
		nwrite = 0;
		nwrite = write(newfd,Buff + totalwrite,BuffSize - totalwrite);
		totalwrite += nwrite;
	}
	close(newfd);

	/* Remove /etc/pmd.conf */
	unlink ("/etc/pmd.conf");

	/* Link new file to /etc/pmd.conf */
	link("/tmp/pmd.tmp","/etc/pmd.conf");
	unlink("/tmp/pmd.tmp");

	fd = open("/etc/pmd.conf",O_RDWR);

	return 0;
}

/****************************************************************************/
/* Function: SetGrpDef														*/
/* Description: Set all values for an Group in /etc/pmdgrp.conf				*/
/* Parameters:																*/
/*				fd - File descriptor for /etc/pmdgrp.conf					*/
/*				Line - A line with group and list of users					*/
/*				Action - Action enum value									*/
/****************************************************************************/
int SetGrpDef(int fd,char *Line, Action act)
{
	char Buff[FILESIZE];
	char *pBuff;
	int BuffSize;
	int nread, nwrite;
	char ch;
	char LineExist;
	char GroupName[GRPNAMESIZE];
	char totalwrite;
	char *gch;
	int g, newfd;

	memset(Buff,'\0',FILESIZE);
	pBuff = Buff;
	BuffSize = 0;
	LineExist = 0;
	gch = Line;
	g = 0;
	totalwrite = 0;

	/* Get Group Name */
	memset(GroupName,'\0',GRPNAMESIZE);
	while ( *gch != ' ' ) {
		GroupName[g] = *gch;
		g++;
		gch++;
	}

	switch ( act ) {
		case kDelete:
			while ( (nread = read(fd,&ch,1)) != 0 ) {
				/* Check line */
				char FLine[LINESIZE];
				int l;

				memset(FLine,'\0',LINESIZE);
				l = 0;

				while ( ch != '\n' ) {
					FLine[l] = ch;
					l++;
					nread = read(fd,&ch,1);
				}

				/* Copy Lines with different group names */
				if ( strncmp(FLine,GroupName,strlen(GroupName)) ) {
					strncpy(pBuff,FLine,strlen(FLine));
					pBuff += strlen(FLine);
				}
			}
			break;

		case kAdd:
			while ( (nread = read(fd,&ch,1)) != 0 ) {
				/* Copy All Lines */
				*pBuff = ch;
				pBuff++;
			}
			strncpy(pBuff,Line,strlen(Line));
			pBuff += strlen(Line);
			break;

		case kEdit:
			while ( (nread = read(fd,&ch,1)) != 0 ) {
				/* Check line */
				char FLine[LINESIZE];
				int l;

				memset(FLine,'\0',LINESIZE);
				l = 0;

				while ( ch != '\n' ) {
					FLine[l] = ch;
					l++;
					nread = read(fd,&ch,1);
				}

				/* Copy new Line instead of File Line */
				if ( !strncmp(FLine,GroupName,strlen(GroupName)) ) {
					strncpy(pBuff,Line,strlen(Line));
					pBuff += strlen(Line);
				}
				else { /* Copy Lines with different group names */
					strncpy(pBuff,FLine,strlen(FLine));
					pBuff += strlen(FLine);
				}
			}
			break;

		case kActionIgnore:
			break;
	}

	close(fd);

	/* Get new file size */
	BuffSize = strlen(Buff);

	newfd = open("/tmp/pmdgrp.tmp",O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	if ( newfd == -1 ) {
		return kFailure;
	}

	/* Write new buff to new file */
	while ( totalwrite < BuffSize ) {
		nwrite = 0;
		nwrite = write(newfd,Buff + totalwrite,BuffSize - totalwrite);
		totalwrite += nwrite;
	}
	close(newfd);

	/* Remove /etc/pmdgrp.conf */
	unlink ("/etc/pmdgrp.conf");

	/* Link new file to /etc/pmdgrp.conf */
	link("/tmp/pmdgrp.tmp","/etc/pmdgrp.conf");
	unlink("/tmp/pmdgrp.tmp");

	fd = open("/etc/pmdgrp.conf",O_RDWR);

	return 0;
}

/****************************************************************************/
/* Function: SetUserMgm														*/
/* Description: Set all values for a User in /etc/pmd.users					*/
/* Parameters:																*/
/*				fd - File descriptor for /etc/pmd.users						*/
/*				Line - A line with user and list of IPDU					*/
/*				Action - Action enum value									*/
/****************************************************************************/
int SetUserMgm(int fd,char *Line, Action act)
{
	char Buff[FILESIZE];
	char *pBuff;
	int BuffSize;
	int nread, nwrite;
	char ch;
	char LineExist;
	char UserName[USRNAMESIZE];
	char totalwrite;
	char *uch;
	int u, newfd;

	memset(Buff,'\0',FILESIZE);
	pBuff = Buff;
	BuffSize = 0;
	LineExist = 0;
	uch = Line;
	u = 0;
	totalwrite = 0;

	/* Get User Name */
	memset(UserName,'\0',USRNAMESIZE);
	while ( *uch != ' ' ) {
		UserName[u] = *uch;
		u++;
		uch++;
	}

	switch ( act ) {
		case kDelete:
			while ( (nread = read(fd,&ch,1)) != 0 ) {
				/* Check line */
				char FLine[LINESIZE];
				int l;

				memset(FLine,'\0',LINESIZE);
				l = 0;

				while ( ch != '\n' ) {
					FLine[l] = ch;
					l++;
					nread = read(fd,&ch,1);
				}

				/* Copy Lines with different user names */
				if ( strncmp(FLine,UserName,strlen(UserName)) ) {
					strncpy(pBuff,FLine,strlen(FLine));
					pBuff += strlen(FLine);
				}
			}
			break;

		case kAdd:
			while ( (nread = read(fd,&ch,1)) != 0 ) {
				/* Copy All Lines */
				*pBuff = ch;
				pBuff++;
			}
			strncpy(pBuff,Line,strlen(Line));
			pBuff += strlen(Line);
			break;

		case kEdit:
			while ( (nread = read(fd,&ch,1)) != 0 ) {
				/* Check line */
				char FLine[LINESIZE];
				int l;

				memset(FLine,'\0',LINESIZE);
				l = 0;

				while ( ch != '\n' ) {
					FLine[l] = ch;
					l++;
					nread = read(fd,&ch,1);
				}

				/* Copy new Line instead of File Line */
				if ( !strncmp(FLine,UserName,strlen(UserName)) ) {
					strncpy(pBuff,Line,strlen(Line));
					pBuff += strlen(Line);
				}
				else { /* Copy Lines with different group names */
					strncpy(pBuff,FLine,strlen(FLine));
					pBuff += strlen(FLine);
				}
			}
			break;

		case kActionIgnore:
			break;
	}

	close(fd);

	/* Get new file size */
	BuffSize = strlen(Buff);

	newfd = open("/tmp/pmdusers.tmp",O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	if ( newfd == -1 ) {
		return kFailure;
	}

	/* Write new buff to new file */
	while ( totalwrite < BuffSize ) {
		nwrite = 0;
		nwrite = write(newfd,Buff + totalwrite,BuffSize - totalwrite);
		totalwrite += nwrite;
	}
	close(newfd);

	/* Remove /etc/pmd.users */
	unlink ("/etc/pmd.users");

	/* Link new file to /etc/pmd.users */
	link("/tmp/pmdusers.tmp","/etc/pmd.users");
	unlink("/tmp/pmdusers.tmp");

	fd = open("/etc/pmd.users",O_RDWR);

	return 0;
}

/****************************************************************************/
/* Function: power															*/
/* Description: Raise base to exponent exp									*/ 
/****************************************************************************/
int power(int base, int exp)
{
	int result;
	int i;

	result = 0;

	if ( exp == 0 ) {
		result = 1;
	}
	else {
		result = 1;
		for( i = 0; i < exp; i++) {
			result *= base;
		}
	}
	return result;
}

/****************************************************************************/
/* Function: SetIpduInfoParam												*/
/* Description: Set all values for an IPDU that will be written in 			*/
/*				/etc/pmd.conf												*/
/* Parameters:																*/
/*				pIpduInfo - PMDIpduInfo structure							*/
/*				Parameter - Field to be used								*/
/*				Value - Value assigned for field							*/
/****************************************************************************/
int setIpduInfoParam(PMDIpduInfo *pIpduInfo,char *Parameter, char *Value)
{
	PMDInletLoad *pload;

	if ( !strncmp(Parameter,"Pollrate",8) ) {
		/* Turn Value into a UInt32 */
		char Num[strlen(Value)];
		int size, i, j, total;

		total = 0; 

		size = strlen(Value);
		for (i = (size -1), j = 0; i >= 0; i--, j++){
			Num[j] = (Value[i] & 0x0F);
		}

		for ( i = 0; i < size; i++ ) {
			total += (Num[i] * (power(10,i)));
		}

		/* Set pIpduInfo->pollRate */
		pIpduInfo->pollRate = (UInt32) total;
	}
	else {
		if ( !strncmp(Parameter,"loadmax",7) ) {
			int v;

			v = *Value & 0x0F;
			switch ( v ) {
				case 1:
					pIpduInfo->inlets = PMDsingleFeed;
					break;

				case 2:
					pIpduInfo->inlets = PMDdualFeed;
					break;

				case 3:
					pIpduInfo->inlets = PMDtriFeed;
					break;
			}
		}
		else {
			if ( !strncmp(Parameter,"load",4) ) {
				int v;

				v = *Value & 0x0F;
				pload = pIpduInfo->load;

				switch ( v ) {
					case 0:
						pload->min = 0;
						break;

					case 1:
						pload->max = 1;
						break;
				}
			}
		}
	}

	return 0;
}

/****************************************************************************/
/* Function: GetConfNumPort 												*/
/* Description: Get the number of configured ports inside section [config]	*/
/*				in file /etc/pmd.conf										*/ 
/* Return:																	*/
/*			NumPort - The number of ports									*/
/****************************************************************************/
int GetConfNumPort(char *pFile)
{
	int NumPort;
	int fd;
	int nread;
	char ch;

	NumPort = 0;
	ch = '\0';
	nread = 0;

	fd = open(pFile,O_RDONLY);

	/* Check how many port exists */
	if ( fd != -1 ) {
		while ( (nread = read(fd,&ch,1)) != 0 ) {
			if ( ch == '[' ) {
				char Section[SECTIONNAMESIZE];
				int s;

				memset(Section,'\0',SECTIONNAMESIZE);
				s = 0;

				nread = read(fd,&ch,1);

				while ( (ch != ']') && (s < SECTIONNAMESIZE) ) {
					Section[s] = ch;
					s++;
					nread = read(fd,&ch,1);
				}

				/* Check if Section is "ports" */
				if ( !strncmp(Section,"ports",5) ) {
					/* Count the number of ports */
					char Line[LINESIZE];
					int l;

					nread = read(fd,&ch,1);

					while ( nread && (ch != '[') ) {
						memset(Line,'\0',LINESIZE);
						l = 0;

						/* Get line */
						while ( (ch != '\n') && (l < LINESIZE) ) {
							Line[l] = ch;
							l++;
							nread = read(fd,&ch,1);
						}

						/* Does Line begin with tty */
						if ( strncmp(Line,"tty",3) ) {
							NumPort++;
						}
						else {
							/* Does Line begin with an IP address */
							if ( StartWithIP(Line) ) {
								NumPort++;
							}
						}
					}
				}
				else {
					lseek(fd,-s,SEEK_CUR);
				}
			}
		}

		close(fd);
	}
	
	return NumPort;
}

/****************************************************************************/
/* Function: GetConfNumOutGrps 												*/
/* Description: Get the number of configured groupd in /etc/pmdgrp.conf		*/
/* Return:																	*/
/*			NumOutGrps - The number of groups								*/
/****************************************************************************/
int GetConfNumOutGrps(char *pFile)
{
	int NumOutGrps;
	int fd;
	int nread;
	char ch;

	NumOutGrps = 0;
	ch = '\0';
	nread = 0;

	fd = open(pFile,O_RDONLY);

	if ( fd != -1 ) {
		while ( (nread = read(fd,&ch,1)) != 0 ) {
			/* Count the number of groups */
			char Line[LINESIZE];
			int l;

			memset(Line,'\0',LINESIZE);
			l = 0;

			/* Get a whole line */
			while ( (ch != '\n') && (l < LINESIZE) ) {
				Line[l] = ch;
				l++;
				nread = read(fd,&ch,1);
			}

			if ( Line[0] == '$' ) {
				NumOutGrps++;
			}
		}
		close(fd);
	}
	
	return NumOutGrps;
}

/****************************************************************************/
/* Function: GetConfNumUsers 												*/
/* Description: Get the number of configured users in /etc/pmd.users		*/
/* Return:																	*/
/*			NumUsers - The number of users									*/
/****************************************************************************/
int GetConfNumUsers(char *pFile)
{
	int NumUsers;
	int fd;
	int nread;
	char ch;

	NumUsers = 0;
	ch = '\0';
	nread = 0;

	fd = open(pFile,O_RDONLY);

	if ( fd != -1 ) {
		while ( (nread = read(fd,&ch,1)) != 0 ) {
			/* Count the number of users */
			char Line[LINESIZE];
			int l;

			memset(Line,'\0',LINESIZE);
			l = 0;

			/* Get a whole line */
			while ( (ch != '\n') && (l < LINESIZE) ) {
				Line[l] = ch;
				l++;
				nread = read(fd,&ch,1);
			}

			/* Each line represents a user */
			NumUsers++;
		}
		close(fd);
	}
	
	return NumUsers;
}

/****************************************************************************/
/* Function: StartWithIP													*/
/* Description: Check if line starts with an IP address						*/
/* Return:																	*/
/*			0 - Line does not start with an IP address						*/
/*			1 - Line starts with an IP address								*/
/****************************************************************************/
int StartWithIP(char *pLine)
{
	int o, i;
	char *pbyte;
	char Oct1[4];
	char Oct2[4];
	char Oct3[4];
	char Oct4[4];

	pbyte = pLine;
	memset(Oct1,'\0',4);
	memset(Oct2,'\0',4);
	memset(Oct3,'\0',4);
	memset(Oct4,'\0',4);

	/* Fill Oct 1 */
	o = 0;
	while ( (*pbyte != '.') && (o < 4) ) {
		Oct1[o] = *pbyte;
		o++;
		pbyte++;
	}
	/* Is Oct in the range of 0-255 */
	for ( i = 0; i < o; i++) {
		if ( !((Oct1[i] <= '0') && (Oct1[i] >= '9')) )  {
			return 0;
		}
	}
	if ( *pbyte == '.' ){
		pbyte++;
	}

	/* Fill Oct 2 */
	o = 0;
	while ( (*pbyte != '.') && (o < 4) ) {
		Oct2[o] = *pbyte;
		o++;
		pbyte++;
	}
	/* Is Oct in the range of 0-255 */
	for ( i = 0; i < o; i++) {
		if ( !((Oct2[i] <= '0') && (Oct2[i] >= '9')) )  {
			return 0;
		}
	}
	if ( *pbyte == '.' ){
		pbyte++;
	}

	/* Fill Oct 3 */
	o = 0;
	while ( (*pbyte != '.') && (o < 4) ) {
		Oct3[o] = *pbyte;
		o++;
		pbyte++;
	}
	/* Is Oct in the range of 0-255 */
	for ( i = 0; i < o; i++) {
		if ( !((Oct3[i] <= '0') && (Oct3[i] >= '9')) )  {
			return 0;
		}
	}
	if ( *pbyte == '.' ){
		pbyte++;
	}

	/* Fill Oct 4 */
	o = 0;
	while ( (*pbyte != '\0') && (o < 4) ) {
		Oct3[o] = *pbyte;
		o++;
		pbyte++;
	}
	/* Is Oct in the range of 0-255 */
	for ( i = 0; i < o; i++) {
		if ( !((Oct4[i] <= '0') && (Oct4[i] >= '9')) )  {
			return 0;
		}
	}

	return 1;
}

/* End of File */
