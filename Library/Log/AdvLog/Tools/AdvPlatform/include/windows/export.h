/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2015/08/18 by Fred Chang									*/
/* Modified Date: 2015/08/18 by Fred Chang									*/
/* Abstract     :  					*/
/* Reference    : None														*/
/****************************************************************************/
#ifndef __export_h__
#define __export_h__

#if defined(WIN32)
	#pragma once
	#ifdef ADVPLAT_EXPORTS
		#define ADVPLAT_CALL __stdcall
		#define ADVPLAT_EXPORT __declspec(dllexport)
	#else
		#define ADVPLAT_CALL __stdcall
		#define ADVPLAT_EXPORT
	#endif
#else
	#define ADVPLAT_CALL
	#define ADVPLAT_EXPORT
#endif


#endif //__export_h__