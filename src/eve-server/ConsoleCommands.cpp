/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:			Allan
    Thanks to:		avianrr  for the idea
*/
/*
#include <time.h> // for clock
#include <math.h> // for fmod
#include <cstdlib> //for system
#include <stdio.h> //for delay
*/
#include <ios>
#include <iostream>
#include <fstream>

#include "ConsoleCommands.h"

ConsoleCommand::ConsoleCommand() {}

ConsoleCommand::~ConsoleCommand() {}

void ConsoleCommand::Init()
{
    sLog.Log( "       ServerInit", "Console Commands initialized." );
    sLog.Warning( "   ConsoleCommand", "Enter 'h' for current list of supported commands." );
}

bool ConsoleCommand::Process() {
	tv.tv_sec = 1;
	tv.tv_usec = 1;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
	if (!FD_ISSET(STDIN_FILENO, &fds)) {
	    return true;
	} else {
		buf = (char*) malloc (BUFLEN);
	    if (fgets(buf, BUFLEN, stdin)) {
			if (strncmp(buf, "x", 1) == 0) {
				sLog.Log("  Alasiya's EvEMu", "EXIT called.  Setting RunLoops to false.");
				free (buf);
				return false;
			} else if (strncmp(buf, "h", 1) == 0) {
				sLog.Warning("  Alasiya's EvEMu", "Current Console Commands and Descriptions: ");
				sLog.Log("","");
				sLog.Log("           (h)elp", " Displays this dialog.");
				sLog.Log("           e(x)it", " Exits the server, saving all loaded items and loging out all connected clients.");
				sLog.Log("        (c)lients", " Displays connected clients, showing account, character, and ip.");
				sLog.Log("         (s)tatus", " Displays Server's System Status, showing threads, memory, and cpu time.");
				sLog.Log("        (v)ersion", " Displays server version.");
				sLog.Log("    (i)nformation", " Displays server information: version, memory, uptime, clients, items, systems, bubbles.  *InComplete*");
				sLog.Log("           s(a)ve", " Immediatly saves all loaded items.  *Broken*");
				sLog.Log("      (b)roadcast", " Broadcasts a message to all clients thru the LocalChat window.  *Not Implemented*");
				sLog.Log("           (n)ote", " Broadcasts a message to all clients thru a notification window.  *Not Implemented*");
			} else if (strncmp(buf, "c", 1) == 0) {
				sLog.Warning("  Alasiya's EvEMu", "Server Client List: ");
				uint8 clients = sEntityList.GetClientCount();
				sLog.Log("Connected Clients", " %u", clients);
				if(clients) {
					std::list<Client *> list;
					sEntityList.GetClients(list);
					std::list<Client *>::const_iterator cur, end;
					cur = list.begin();
					end = list.end();
					for(; cur != end; cur++) {
						sLog.Log("Connected Clients", " [(%u)%u] %s in %s(%u).  %u Minutes Online.", (*cur)->GetAccountID(), (*cur)->GetCharacterID(), (*cur)->GetName(),(*cur)->GetSystemName().c_str(), (*cur)->GetLocationID(), (*cur)->GetChar()->logonMinutes() );
					}
				}
			} else if (strncmp(buf, "s", 1) == 0) {
			    std::string state = "";
				int64 threads = 0;
				float vm = 0.0f, rss = 0.0f, user = 0.0f, kernel = 0.0f;
				Status(&state, &threads, &vm, &rss, &user, &kernel);
				sLog.Log("    Server Status", "  S: %s | T: %d | RSS: %.3fMb | VM: %.3fMb | U: %.2f | K: %.2f", \
                         state.c_str(), threads, rss, vm, user, kernel );
			} else if (strncmp(buf, "v", 1) == 0) {
			    sLog.Warning("  Alasiya's EvEMu", "Server Version:");
				sLog.Log("  Server Revision", " " EVEMU_REVISION );
				sLog.Log("         Based on", " " EVEMU_VERSION );
				sLog.Log("       Build Date", " " EVEMU_BUILD_DATE );
			} else if (strncmp(buf, "i", 1) == 0) {
			    sLog.Warning("  Alasiya's EvEMu", "Server Information:");
				sLog.Log("  Server Revision", " " EVEMU_REVISION );
				sLog.Log("         Based on", " " EVEMU_VERSION );
				sLog.Log("       Build Date", " " EVEMU_BUILD_DATE );
				sLog.Log("      This Source", " " EVEMU_REPOSITORY );
                //  memory
                std::string state = "";
                int64 threads = 0;
                float vm = 0.0f, rss = 0.0f, user = 0.0f, kernel = 0.0f;
                Status(&state, &threads, &vm, &rss, &user, &kernel);
				sLog.Log("     Memory Usage", " RSS: %.3fMb  VM: %.3fMb", rss, vm );
                sLog.Log("    Server Status", "  S: %s | T: %d | U: %.2f | K: %.2f", \
                         state.c_str(), threads, rss, vm, user, kernel );
                uint8 w = 0, d = 0, h = 0, m = 0, s = 0;
				GetUpTime(&w, &d, &h, &m, &s);
				if(w)
				    sLog.Log("    Server UpTime", " %u W, %u D, %u H, %u M, %u S.", w, d, h, m, s );
				else if(d)
				    sLog.Log("    Server UpTime", " %u D, %u H, %u M, %u S.", d, h, m, s );
				else if(h)
				    sLog.Log("    Server UpTime", " %u H, %u M, %u S.", h, m, s );
				else if(m)
				    sLog.Log("    Server UpTime", " %u M, %u S.", m, s );
				else
				    sLog.Log("    Server UpTime", " %u S.", s );
				uint8 clients = sEntityList.GetClientCount();
				sLog.Log("Connected Clients", " %u", clients);
				//  loaded items
				sLog.Log("     Loaded Items", " %u", p_factory->Count() );		// TODO: this is wrong...
				//  loaded systems
				//sLog.Log("   Active Systems", " %u", p_systems->Count());		// TODO: this is wrong...
				//  loaded bubbles
				sLog.Log("   Active Bubbles", " %u", p_bubbles->Count());
				m_db.SaveServerStats(threads, rss, vm, user, kernel, p_factory->Count() , 3 /*systems*/, p_bubbles->Count());
			} else if (strncmp(buf, "a", 1) == 0) {
			    sLog.Warning("  Alasiya's EvEMu", "Server SaveAll:");
				//p_factory->SaveItems();
			} else if (strncmp(buf, "b", 1) == 0) {
			    sLog.Warning("  Alasiya's EvEMu", "Server Broadcast:");
				sLog.Log("     Chat Message", " Not Avalible Yet." );
				//buf.erase (0, 4);  <-- this does not work...std::string::erase...NOT char* erase
				//SendMessage(buf);
			} else if (strncmp(buf, "n", 1) == 0) {
			    sLog.Warning("  Alasiya's EvEMu", "Server Notify:");
				sLog.Log("   Notify Message", " Not Avalible Yet." );
			  /**    buf is char*, but erase is std::string::erase().
			   *   having problems switching between char* and string (to erase "n "), then back to char*.
			   *   will put this off for later.
				std::string str;
				str.push_back(buf);
				str.erase (0, 5);
				strcpy(buf, str.c_str());
				std::list<Client *> list;
				sEntityList.GetClients(list);
				std::list<Client *>::const_iterator cur, end;
				cur = list.begin();
				end = list.end();
				for(; cur != end; cur++) {
					(*cur)->SendNotifyMsg( buf );
				}
				*/
				sLog.Log("Console Command", "Notification sent to all online clients." );
			} else {
				sLog.Error("  Alasiya's EvEMu", "Command not recognized: %s", buf);
			}
		}
		free (buf);
		return true;
	}
}

void ConsoleCommand::GetUpTime(uint8 *w, uint8 *d, uint8 *h, uint8 *m, uint8 *s) {
    uint32 seconds = m_db.GetServerUpTime();
    float minutes = seconds/60;
    float hours = minutes/60;
	float days = hours/24;
	float weeks = days/7;

    *s = fmod(seconds,60);
    *m = fmod(minutes,60);
    *h = fmod(hours,24);
	*d = fmod(days,7);
	*w = fmod(weeks,4);
}

void ConsoleCommand::SendMessage(const char *msg) {
	// LSCChannel::SendMessage(Client * c, const char * message, bool self)
}

void ConsoleCommand::Status(std::string *state, int64 *threads, float *vm_usage, float *resident_set, float *user, float *kernel)
{
    // the fields we want
    std::string ignore = "", run_state = "";
    int64 num_threads = 0;  //this is saved from OS as long decimal....*sigh*  gotta allocate long int for it or weird shit happens.
    uint64 vsize = 0;      //in bytes
    int64 rss = 0;			//in pages
	float utime = 0.0f, stime = 0.0f;

    // stat seems to give the most reliable results
    std::ifstream ifs ("/proc/self/stat", std::ios_base::in);
    ifs >> ignore >> ignore >> run_state >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> ignore >> utime >> stime >> ignore >> ignore >> ignore >> ignore >> num_threads
        >> ignore >> ignore >> vsize >> rss;
	ifs.close();

	*state = run_state;
	/*	state = One character from the string "RSDZTW" where
			  R is running,
			  S is sleeping in an interruptible wait,
			  D is waiting in uninterruptible disk sleep,
			  Z is zombie,
			  T is traced or stopped (on a signal),
			  W is paging
        */
	*threads = num_threads;

	*user = utime/sysconf(_SC_CLK_TCK)/100.0;
	*kernel = stime/sysconf(_SC_CLK_TCK)/100.0;

    *vm_usage     = (((vsize / 1024.0) / 1024.0) / 26);	//  weird math here.....just go with it.
	//rss in pages, x page_size in bytes, converted to k, then convert to Mb.
    *resident_set = (rss * (sysconf(_SC_PAGE_SIZE) / 1024.0) / 1024.0);

}

void ConsoleCommand::MemStatus(float *vm_usage, float *resident_set)
{
    std::string ignore = "";
    // the fields we want
    uint64 vsize = 0;      //in bytes
    int64 rss = 0;			//in pages

    // stat seems to give the most reliable results
    std::ifstream ifs ("/proc/self/stat", std::ios_base::in);
    ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> vsize >> rss;
	ifs.close();

    *vm_usage     = (((vsize / 1024.0) / 1024.0) / 26);	//now in Mb
	//rss in pages, x page_size(in bytes converted to k), then convert to Mb.
    *resident_set = (rss * (sysconf(_SC_PAGE_SIZE) / 1024.0) / 1024.0);

}
