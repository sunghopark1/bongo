/****************************************************************************
 * <Novell-copyright>
 * Copyright (c) 2006 Novell, Inc. All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, contact Novell, Inc.
 * 
 * To contact Novell about this file by physical or electronic mail, you 
 * may find current contact information at www.novell.com.
 * </Novell-copyright>
 ****************************************************************************/

#ifndef CALENDAR_H
#define CALENDAR_H
    
#include "stored.h"

XPL_BEGIN_C_LINKAGE

const char * StoreProcessIncomingEvent(StoreClient *client, StoreObject *event, uint64_t linkGuid);

CCode StoreSetAlarm(StoreClient *client, StoreObject *event, const char *alarmtext);

XPL_END_C_LINKAGE

#endif
