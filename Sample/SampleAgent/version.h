//====================================================================================
// Revision History :
//---------------------------------------------------------------
// 2013.03.29 by Kevin.Ong
//	 First version
// 
//---------------------------------------------------------------
// Copyright (c) Advantech Co., Ltd. All Rights Reserved
//====================================================================================
#ifndef _SUSIACCESSAGENT_VERSION_H_
#define _SUSIACCESSAGENT_VERSION_H_

#if !defined(SVN_REVISION)
#include "../../Include/svnversion.h"
#endif

#define VER_MAJOR	MAIN_VERSION
#define VER_MINOR	SUB_VERSION
#define VER_BUILD	BUILD_VERSION
#define VER_FIX		SVN_REVISION

#define U32VER		(VER_MAJOR << 24 | VER_MINOR << 16 | VER_BUILD)

#define CREATE_XVER(maj,min,build,fix) 		maj ##, ## min ##, ## build ##, ## fix

#endif /* _DRV_VERSION_H_ */
