#include "getopt.h"
#include <stdio.h>
#include <string.h>

int     opterr = 1,             /* if error message should be printed */
_optind = 1,             /* index into parent argv vector */
optopt,                 /* character checked for validity */
optreset;               /* reset getopt */
char    *_optarg;                /* argument associated with option */

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""


char *ADVPLAT_CALL getoptarg() { return _optarg; }
int ADVPLAT_CALL getoptind() { return _optind; }

/*
* getopt --
*      Parse argc/argv argument vector.
*/
int ADVPLAT_CALL getopt(int nargc, char * const nargv[], const char *ostr)
{
	static char *place = EMSG;              /* option letter processing */
	const char *oli;                        /* option letter list index */

	if (optreset || !*place) {              /* update scanning pointer */
		optreset = 0;
		if (_optind >= nargc || *(place = nargv[_optind]) != '-') {
			place = EMSG;
			return (-1);
		}
		if (place[1] && *++place == '-') {      /* found "--" */
			++_optind;
			place = EMSG;
			return (-1);
		}
	}                                       /* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
		!(oli = strchr(ostr, optopt))) {
		/*
		* if the user didn't specify '-' as an option,
		* assume it means -1.
		*/
		if (optopt == (int)'-')
			return (-1);
		if (!*place)
			++_optind;
		if (opterr && *ostr != ':')
			(void)printf("illegal option -- %c\n", optopt);
		return (BADCH);
	}
	if (*++oli != ':') {                    /* don't need argument */
		_optarg = NULL;
		if (!*place)
			++_optind;
	}
	else {                                  /* need an argument */
		if (*place)                     /* no white space */
			_optarg = place;
		else if (nargc <= ++_optind) {   /* no arg */
			place = EMSG;
			if (*ostr == ':')
				return (BADARG);
			if (opterr)
				(void)printf("option requires an argument -- %c\n", optopt);
			return (BADCH);
		}
		else                            /* white space */
			_optarg = nargv[_optind];
		place = EMSG;
		++_optind;
	}
	return (optopt);                        /* dump back option letter */
}
