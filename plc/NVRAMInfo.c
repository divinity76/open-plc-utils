/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted (subject to the limitations
 *   in the disclaimer below) provided that the following conditions
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   signed NVRAMInfo (struct plc * plc);
 *
 *   plc.h
 *
 *   This plugin for program plc requests a device NVRAM parameter
 *   block using a VS_GET_NVM message; the information is displayed
 *   and can be used to determine the flash memory characteristics
 *   before downloading firmware;
 *
 *
 *   Contributor(s):
 *      Charles Maier
 *
 *--------------------------------------------------------------------*/

#ifndef NVRAMINFO_SOURCE
#define NVRAMINFO_SOURCE

#include <stdint.h>
#include <memory.h>

#include "../plc/plc.h"
#include "../tools/error.h"
#include "../tools/memory.h"
#include "../tools/symbol.h"
#include "../ram/nvram.h"

signed NVRAMInfo (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_get_nvm_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
	}
	* request = (struct vs_get_nvm_request *) (message);
	struct __packed vs_get_nvm_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		struct config_nvram config_nvram;
	}
	* confirm = (struct vs_get_nvm_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	struct config_nvram * config_nvram = (struct config_nvram *)(&confirm->config_nvram);
	Request (plc, "Fetch NVRAM Configuration");
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_GET_NVM | MMTYPE_REQ));
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_GET_NVM | MMTYPE_CNF)) > 0)
	{
		if (confirm->MSTATUS)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
		Display (plc, "TYPE=0x%02X (%s) PAGE=0x%04X (%d) BLOCK=0x%04X (%d) SIZE=0x%04X (%d)", LE32TOH (config_nvram->NVRAMTYPE), NVRAMName (LE32TOH (config_nvram->NVRAMTYPE)), LE32TOH (config_nvram->NVRAMPAGE), LE32TOH (config_nvram->NVRAMPAGE), LE32TOH (config_nvram->NVRAMBLOCK), LE32TOH (config_nvram->NVRAMBLOCK), LE32TOH (config_nvram->NVRAMSIZE), LE32TOH (config_nvram->NVRAMSIZE));
	}
	return (0);
}


#endif

