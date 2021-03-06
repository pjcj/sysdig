/*
Copyright (C) 2013-2014 Draios inc.

This file is part of sysdig.

sysdig is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

sysdig is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with sysdig.  If not, see <http://www.gnu.org/licenses/>.
*/

//
// Variuos helper functions to render stuff on the screen
//
#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <algorithm> 

#include <sinsp.h>
#include "sysdig.h"
#include "chisel.h"

#define DESCRIPTION_TEXT_START 18
#define CONSOLE_LINE_LEN 79
#define PRINTF_WRAP_CPROC(x)  #x
#define PRINTF_WRAP(x) PRINTF_WRAP_CPROC(x)

void list_fields(bool verbose)
{
	uint32_t j, l, m;
	int32_t k;

	vector<const filter_check_info*> fc_plugins;
	sinsp::get_filtercheck_fields_info(&fc_plugins);

	for(j = 0; j < fc_plugins.size(); j++)
	{
		const filter_check_info* fci = fc_plugins[j];

		printf("\n----------------------\n");
		printf("Field Class: %s\n\n", fci->m_name.c_str());

		for(k = 0; k < fci->m_nfiedls; k++)
		{
			const filtercheck_field_info* fld = &fci->m_fields[k];

			printf("%s", fld->m_name);
			uint32_t namelen = strlen(fld->m_name);

			ASSERT(namelen < DESCRIPTION_TEXT_START);

			for(l = 0; l < DESCRIPTION_TEXT_START - namelen; l++)
			{
				printf(" ");
			}
				
			string desc;

			if(verbose)
			{
				desc = string(fld->m_description) + " Type:" + param_type_to_string(fld->m_type) + ".";
			}
			else
			{
				desc = string(fld->m_description);
			}

			size_t desclen = desc.size();

			for(l = 0; l < desclen; l++)
			{
				if(l % (CONSOLE_LINE_LEN - DESCRIPTION_TEXT_START) == 0 && l != 0)
				{
					printf("\n");

					for(m = 0; m < DESCRIPTION_TEXT_START; m++)
					{
						printf(" ");
					}
				}

				printf("%c", desc[l]);
			}

			printf("\n");
		}
	}
}

const char* param_type_to_string(ppm_param_type pt)
{
	switch(pt)
	{
	case PT_NONE:
		return "NONE";
		break;
	case PT_INT8:
		return "INT8";
		break;
	case PT_INT16:
		return "INT16";
		break;
	case PT_INT32:
		return "INT32";
		break;
	case PT_INT64:
		return "INT64";
		break;
	case PT_UINT8:
		return "UINT8";
		break;
	case PT_UINT16:
		return "UINT16";
		break;
	case PT_UINT32:
		return "UINT32";
		break;
	case PT_UINT64:
		return "UINT64";
		break;
	case PT_CHARBUF:
		return "CHARBUF";
		break;
	case PT_BYTEBUF:
		return "BYTEBUF";
		break;
	case PT_ERRNO:
		return "ERRNO";
		break;
	case PT_SOCKADDR:
		return "SOCKADDR";
		break;
	case PT_SOCKTUPLE:
		return "SOCKTUPLE";
		break;
	case PT_FD:
		return "FD";
		break;
	case PT_PID:
		return "PID";
		break;
	case PT_FDLIST:
		return "FDLIST";
		break;
	case PT_FSPATH:
		return "FSPATH";
		break;
	case PT_SYSCALLID:
		return "SYSCALLID";
		break;
	case PT_SIGTYPE:
		return "SIGTYPE";
		break;
	case PT_RELTIME:
		return "RELTIME";
		break;
	case PT_ABSTIME:
		return "ABSTIME";
		break;
	case PT_PORT:
		return "PORT";
		break;
	case PT_L4PROTO:
		return "L4PROTO";
		break;
	case PT_SOCKFAMILY:
		return "SOCKFAMILY";
		break;
	case PT_BOOL:
		return "BOOL";
		break;
	case PT_IPV4ADDR:
		return "IPV4ADDR";
		break;
	case PT_DYN:
		return "DYNAMIC";
		break;
	case PT_FLAGS8:
		return "FLAGS8";
		break;
	case PT_FLAGS16:
		return "FLAGS16";
		break;
	case PT_FLAGS32:
		return "FLAGS32";
		break;
	default:
		ASSERT(false);
		return "<NA>";
	}
}

void list_events(sinsp* inspector)
{
	uint32_t j, k;
	string tstr;

	sinsp_evttables* einfo = inspector->get_event_info_tables();
	const struct ppm_event_info* etable = einfo->m_event_info;

	for(j = 0; j < PPM_EVENT_MAX; j++)
	{
		const struct ppm_event_info ei = etable[j];
		char dir = (PPME_IS_ENTER(j))? '>' : '<';

		if(ei.flags & EF_UNUSED)
		{
			continue;
		}

		printf("%c %s(", dir, ei.name);

		for(k = 0; k < ei.nparams; k++)
		{
			if(k != 0)
			{
				printf(", ");
			}

			printf("%s %s", param_type_to_string(ei.params[k].type), 
				ei.params[k].name);
		}
				
		printf(")\n");
	}
}

struct summary_chisel_comparer
{
    bool operator() (const chisel_desc& first, const chisel_desc& second) const 
	{
		return (first.m_category == second.m_category) 
		? first.m_name < second.m_name 
		: first.m_category < second.m_category;
	}
};

void print_chisel_info(chisel_desc* cd)
{
	// First we create a single list composed of
	// just this chisel and then run the short_description
	// over it in order to get those fields for free.
	std::vector<chisel_desc> chlist;
	chlist.push_back(cd[0]);

	list_chisels(&chlist);

	// Now we have to do the real work
	printf("\n");

	uint32_t l;
	string astr;

	string desc = cd->m_description;
	size_t desclen = desc.size();

	for(l = 0; l < desclen; l++)
	{
		if(l % (CONSOLE_LINE_LEN - DESCRIPTION_TEXT_START) == 0 && l != 0)
		{
			printf("\n");
		}

		printf("%c", desc[l]);
	}

	printf("\n");

	astr +=	"\nArgs:\n";

	if(cd->m_args.size() != 0)
	{

		for(l = 0; l < cd->m_args.size(); l++)
		{
			astr += "[" + cd->m_args[l].m_type + "] " + cd->m_args[l].m_name + " - ";
			astr +=	cd->m_args[l].m_description + "\n";
		}

	}
	else
	{
		astr +=	"(None)";
	}

	size_t astrlen = astr.size();

	for(l = 0; l < astrlen; l++)
	{
		if(l % (CONSOLE_LINE_LEN - DESCRIPTION_TEXT_START) == 0 && l != 0)
		{
			printf("\n%" PRINTF_WRAP(DESCRIPTION_TEXT_START) "s", "");
		}

		printf("%c", astr[l]);
	}

	// just for good meaure
	printf("\n");
}

void list_chisels(vector<chisel_desc>* chlist)
{
	uint32_t j, l;

	//
	// Sort the list by name
	//
	sort(chlist->begin(), chlist->end(), summary_chisel_comparer());
	string last_category;

	//
	// Print the list to the screen
	//
	for(j = 0; j < chlist->size(); j++)
	{
		chisel_desc* cd = &(chlist->at(j));

		string category = cd->m_category;

		if(category != last_category) 
		{
			string fullcatstr = "Category: " + category;

			printf("\n%s\n", fullcatstr.c_str());
			for(l = 0; l < fullcatstr.size(); l++)
			{
				putchar('-');
			}

			printf("\n");
			last_category = category;
		}

		printf("%s", cd->m_name.c_str());
		uint32_t namelen = cd->m_name.size();

		ASSERT(namelen < (DESCRIPTION_TEXT_START - 2));

		for(l = 0; l < (DESCRIPTION_TEXT_START - namelen - 2); l++)
		{
			printf(" ");
		}

		string desc = cd->m_shortdesc;
		size_t desclen = desc.size();

		for(l = 0; l < desclen; l++)
		{
			if(l % (CONSOLE_LINE_LEN - DESCRIPTION_TEXT_START) == 0 && l != 0)
			{
				printf("\n%" PRINTF_WRAP(DESCRIPTION_TEXT_START) "s", "");
			}

			printf("%c", desc[l]);
		}

		printf("\n");
	}

	printf("\nUse the -i flag to get detailed information about a specific chisel\n");
}
