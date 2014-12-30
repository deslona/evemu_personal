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

#ifndef EVEMU_EVESERVER_CONSOLECOMMANDS_H_
#define EVEMU_EVESERVER_CONSOLECOMMANDS_H_

#define BUFLEN 256
#define EVEMU_REVISION "0.7.54-allan"
#define EVEMU_BUILD_DATE "25 December 2014"

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "ServiceDB.h"
#include "inventory/ItemFactory.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


class ConsoleCommand
: public Singleton<ConsoleCommand>
{
  public:
    ConsoleCommand();
    virtual ~ConsoleCommand();

    void Init();
	bool Process();

	void GetUpTime(uint8*, uint8*, uint8*, uint8*, uint8*);
	void SendMessage(const char*);
	void Status(std::string*, int64*, float*, float*, float*, float*);
	void MemStatus(float*, float*);

  protected:
	char *buf;
	fd_set fds;
	struct timeval tv;

  private:
	ServiceDB m_db;
	ItemFactory *p_factory;
	SystemBubble *p_bubbles;
	SystemManager *p_systems;

};

//Singleton
#define sCommand \
    ( ConsoleCommand::get() )

#endif  // EVEMU_EVESERVER_CONSOLECOMMANDS_H_

