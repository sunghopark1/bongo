/****************************************************************************
 * <Novell-copyright>
 * Copyright (c) 2001 Novell, Inc. All Rights Reserved.
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
 |***************************************************************************/

#ifndef XPL_H
#define XPL_H

// this define should be used to mark function parameters that are currently 
// unused but ought to be used / will be used in the future.
#define UNUSED_PARAMETER(x)	if(x) {}

// this define should be used to mark function parameters that are unused
// and possibly should be removed.
#define UNUSED_PARAMETER_REFACTOR(x)	if(x) {}

#include "xplutil.h"
#include "xplthread.h"
#include "xplservice.h"
#include "xplschema.h"
#include "xplhash.h"
#include "xplold.h"
#include "memmgr.h"

#endif
