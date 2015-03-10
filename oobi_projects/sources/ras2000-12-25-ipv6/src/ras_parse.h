/*
 *  Module:  2way-raw.h (raw data socket client/server application header file)
 *
 *  Author:  Marcelo Peccin <marcelo.peccin@cyclades.com>
 *
 *  Copyright:  (c) 2004 Cyclades Corp.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Date:  October 8, 2004
 *
 *  Comments:  The first release of this module is intended to
 *             Telia's project. The project involves a main Application
 *             that pools and receive asynchronous alarms from
 *             a number of remote serial devices.
 *             This software is based on modem-replace.c.
 */

#ifndef RAS_PARSE_H
#define RAS_PARSE_H
typedef enum _ras_tty_regex_index
{
	RAS_TTY_NONE = -1,
	RAS_TTY_DEVICE,
	RAS_TTY_VIRTUAL_IPv4_PORT,
	RAS_TTY_VIRTUAL_IPv6_PORT,
	RAS_TTY_VIRTUAL_IPv4,
	RAS_TTY_VIRTUAL_IPv6,
	RAS_TTY_HOSTNAME_PORT,
	RAS_TTY_HOSTNAME,
	RAS_TTY_TOTAL
} TTtyRegexIndex;

typedef enum _ras_match_field {
	FIELD_0,
	FIELD_1,
	FIELD_2,
	FIELD_3,
	FIELD_4,
	FIELD_5,
	FIELD_6,
	FIELD_7,
	FIELD_8,
	FIELD_9,
	FIELD_10
} TRasMatchField;

char *g_ttyRegexTable[RAS_TTY_TOTAL] =
{
	"ttyS[0-9][0-9]*",
	"([0-9]+[.][0-9]+[.][0-9]+[.][0-9]+):([0-9]+)$",
	"\\[([0-9a-fA-F:]+([0-9.]*)?(%.*)?)\\]:([0-9]+$)",
	"([0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)$",
	"\\[([0-9a-fA-F:]+([0-9.]*)?(%.*)?)\\]",
	"([a-zA-Z0-9.-_]+):([0-9]+)",
	"([a-zA-Z0-9.-_]+)",
};

#define RAS_MAX_MATCHES 10

struct ras_parse {
	regex_t pattern[64];
	regmatch_t match[RAS_MAX_MATCHES];
};

#endif //RAS_PARSE_H
