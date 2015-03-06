/* This is a modified 'library version' of the 'units' command found in OpenBSD.
 * Changes include putting it in a namespace and defining the 'convert' function
 * instead of 'main' as in the original.
 */

/*	$OpenBSD: units.c,v 1.14 2007/03/29 20:13:57 jmc Exp $	*/
/*	$NetBSD: units.c,v 1.6 1996/04/06 06:01:03 thorpej Exp $	*/

/*
 * units.c   Copyright (c) 1993 by Adrian Mariano (adrian@cam.cornell.edu)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * Disclaimer:  This software is provided by the author "as is".  The author
 * shall not be liable for any damages caused in any way by this software.
 *
 * I would appreciate (though I do not require) receiving a copy of any
 * improvements you might make to this program.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "units.h"

#include <string>
#include <iostream>

#if defined(WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#define snprintf _snprintf
#endif

#ifndef strlcpy
#define strlcpy strncpy
#endif

using namespace std;

namespace units
{

#ifndef UNITSFILE
#define UNITSFILE "/usr/share/misc/units.lib"
#endif

const int num_unitsfile_locations = 6;
const char *unitsfile_locations[] = {
	"units.dat",
	"../units.dat",
	"data/units.dat",
	"../data/units.dat",
	"/usr/share/misc/units.dat",
	"/usr/share/misc/units.lib"
};


const char VERSION[] = "1.0 modified";

const int MAXUNITS = 3000;
const int MAXPREFIXES = 500;

const int MAXSUBUNITS = 1500;

const char PRIMITIVECHAR = '!';

const char *powerstring = "^";

struct untittabletype {
	char *uname;
	char *uval;
} unittable[MAXUNITS];

struct unittype {
	char *numerator[MAXSUBUNITS];
	char *denominator[MAXSUBUNITS];
	double factor;
};

struct prefixtabletype {
	char *prefixname;
	char *prefixval;
} prefixtable[MAXPREFIXES];


char *NULLUNIT = "";

#ifdef DOS
#define SEPERATOR ";";
#else
#define SEPERATOR ":";
#endif

int unitcount;
int prefixcount;

char *dupstr(const char *);
void readerror(int);
void readunits(char *);
void initializeunit(struct unittype *);
int addsubunit(char *[], char *);
void showunit(struct unittype *);
void zeroerror(void);
int addunit(struct unittype *, const char *, int);
int compare(const void *, const void *);
void sortunit(struct unittype *);
void cancelunit(struct unittype *);
char *lookupunit(char *);
int reduceproduct(struct unittype *, int);
int reduceunit(struct unittype *);
int compareproducts(char **, char **);
int compareunits(struct unittype *, struct unittype *);
int completereduce(struct unittype *);
void showanswer(struct unittype *, struct unittype *);
void usage(void);

char *
dupstr(const char *str)
{
	char *ret;

	ret = strdup(str);
	if (!ret) {
		fprintf(stderr, "Memory allocation error\n");
		exit(3);
	}
	return (ret);
}


void
readerror(int linenum)
{
//	fprintf(stderr, "Error in units file '%s' line %d\n", UNITSFILE,
//	    linenum);
}


void
readunits(char *userfile)
{
	char line[80], *lineptr;
	int len, linenum, i;
	FILE *unitfile;

	unitcount = 0;
	linenum = 0;

	if (userfile) {
		unitfile = fopen(userfile, "rt");
		if (!unitfile) {
			fprintf(stderr, "Unable to open units file '%s'\n",
			    userfile);
			exit(1);
		}
	} else {
		unitfile = fopen(UNITSFILE, "rt");
		if (!unitfile) {
			char filename[1000], separator[2] = SEPERATOR;
			char *direc, *env;

			env = getenv("PATH");
			if (env) {
				direc = strtok(env, separator);
				while (direc) {
					snprintf(filename, sizeof(filename),
					    "%s/%s", direc, UNITSFILE);
					unitfile = fopen(filename, "rt");
					if (unitfile)
						break;
					direc = strtok(NULL, separator);
				}
			}
			if (!unitfile) {
				fprintf(stderr, "Can't find units file '%s'\n",
				    UNITSFILE);
				exit(1);
			}
		}
	}
	while (!feof(unitfile)) {
		if (!fgets(line, sizeof(line), unitfile))
			break;
		linenum++;
		lineptr = line;
		if (*lineptr == '/')
			continue;
		lineptr += strspn(lineptr, " \n\t");
		len = strcspn(lineptr, " \n\t");
		lineptr[len] = 0;
		if (!strlen(lineptr))
			continue;
		if (lineptr[strlen(lineptr) - 1] == '-') { /* it's a prefix */
			if (prefixcount == MAXPREFIXES) {
				fprintf(stderr,
				    "Memory for prefixes exceeded in line %d\n",
				    linenum);
				continue;
			}

			lineptr[strlen(lineptr) - 1] = 0;
			for (i = 0; i < prefixcount; i++) {
				if (!strcmp(prefixtable[i].prefixname, lineptr))
					break;
			}
			if (i < prefixcount) {
				fprintf(stderr, "Redefinition of prefix '%s' "
				    "on line %d ignored\n", lineptr, linenum);
				continue;	/* skip duplicate prefix */
			}

			prefixtable[prefixcount].prefixname = dupstr(lineptr);
			lineptr += len + 1;
			if (!strlen(lineptr)) {
				readerror(linenum);
				free(prefixtable[prefixcount].prefixname);
				continue;
			}
			lineptr += strspn(lineptr, " \n\t");
			len = strcspn(lineptr, "\n\t");
			lineptr[len] = 0;
			prefixtable[prefixcount++].prefixval = dupstr(lineptr);
		} else {		/* it's not a prefix */
			if (unitcount == MAXUNITS) {
				fprintf(stderr,
				    "Memory for units exceeded in line %d\n",
				    linenum);
				continue;
			}

			for (i = 0; i < unitcount; i++) {
				if (!strcmp(unittable[i].uname, lineptr))
					break;
			}
			if (i < unitcount) {
				fprintf(stderr, "Redefinition of unit '%s' "
				    "on line %d ignored\n", lineptr, linenum);
				continue;	/* skip duplicate unit */
			}

			unittable[unitcount].uname = dupstr(lineptr);
			lineptr += len + 1;
			lineptr += strspn(lineptr, " \n\t");
			if (!strlen(lineptr)) {
				readerror(linenum);
				free(unittable[unitcount].uname);
				continue;
			}
			len = strcspn(lineptr, "\n\t");
			lineptr[len] = 0;
			unittable[unitcount++].uval = dupstr(lineptr);
		}
	}
	fclose(unitfile);
}

void
initializeunit(struct unittype *theunit)
{
	theunit->factor = 1.0;
	theunit->numerator[0] = theunit->denominator[0] = NULL;
}


int
addsubunit(char *product[], char *toadd)
{
	char **ptr;

	for (ptr = product; *ptr && *ptr != NULLUNIT; ptr++);
	if (ptr >= product + MAXSUBUNITS) {
		fprintf(stderr, "Memory overflow in unit reduction\n");
		return 1;
	}
	if (!*ptr)
		*(ptr + 1) = 0;
	*ptr = dupstr(toadd);
	return 0;
}


void
showunit(struct unittype *theunit)
{
	char **ptr;
	int printedslash;
	int counter = 1;

	printf("\t%.8g", theunit->factor);
	for (ptr = theunit->numerator; *ptr; ptr++) {
		if (ptr > theunit->numerator && **ptr &&
		    !strcmp(*ptr, *(ptr - 1)))
			counter++;
		else {
			if (counter > 1)
				printf("%s%d", powerstring, counter);
			if (**ptr)
				printf(" %s", *ptr);
			counter = 1;
		}
	}
	if (counter > 1)
		printf("%s%d", powerstring, counter);
	counter = 1;
	printedslash = 0;
	for (ptr = theunit->denominator; *ptr; ptr++) {
		if (ptr > theunit->denominator && **ptr &&
		    !strcmp(*ptr, *(ptr - 1)))
			counter++;
		else {
			if (counter > 1)
				printf("%s%d", powerstring, counter);
			if (**ptr) {
				if (!printedslash)
					printf(" /");
				printedslash = 1;
				printf(" %s", *ptr);
			}
			counter = 1;
		}
	}
	if (counter > 1)
		printf("%s%d", powerstring, counter);
	printf("\n");
}


void
zeroerror(void)
{
	fprintf(stderr, "Unit reduces to zero\n");
}

/*
   Adds the specified string to the unit.
   Flip is 0 for adding normally, 1 for adding reciprocal.

   Returns 0 for successful addition, nonzero on error.
*/

int
addunit(struct unittype *theunit, const char *toadd, int flip)
{
	char *scratch, *savescr;
	char *item;
	char *divider, *slash;
	int doingtop;

	savescr = scratch = dupstr(toadd);
	for (slash = scratch + 1; *slash; slash++)
		if (*slash == '-' &&
		    (tolower(*(slash - 1)) != 'e' ||
		    !strchr(".0123456789", *(slash + 1))))
			*slash = ' ';
	slash = strchr(scratch, '/');
	if (slash)
		*slash = 0;
	doingtop = 1;
	do {
		item = strtok(scratch, " *\t\n/");
		while (item) {
			if (strchr("0123456789.", *item)) { /* item is a number */
				double num;

				divider = strchr(item, '|');
				if (divider) {
					*divider = 0;
					num = atof(item);
					if (!num) {
						zeroerror();
						free(savescr);
						return 1;
					}
					if (doingtop ^ flip)
						theunit->factor *= num;
					else
						theunit->factor /= num;
					num = atof(divider + 1);
					if (!num) {
						zeroerror();
						free(savescr);
						return 1;
					}
					if (doingtop ^ flip)
						theunit->factor /= num;
					else
						theunit->factor *= num;
				} else {
					num = atof(item);
					if (!num) {
						zeroerror();
						free(savescr);
						return 1;
					}
					if (doingtop ^ flip)
						theunit->factor *= num;
					else
						theunit->factor /= num;

				}
			} else {	/* item is not a number */
				int repeat = 1;

				if (strchr("23456789",
				    item[strlen(item) - 1])) {
					repeat = item[strlen(item) - 1] - '0';
					item[strlen(item) - 1] = 0;
				}
				for (; repeat; repeat--)
					if (addsubunit(doingtop ^ flip
					    ? theunit->numerator
					    : theunit->denominator, item)) {
						free(savescr);
						return 1;
					}
			}
			item = strtok(NULL, " *\t/\n");
		}
		doingtop--;
		if (slash) {
			scratch = slash + 1;
		} else
			doingtop--;
	} while (doingtop >= 0);
	free(savescr);
	return 0;
}


int
compare(const void *item1, const void *item2)
{
	return strcmp(*(char **) item1, *(char **) item2);
}


void
sortunit(struct unittype *theunit)
{
	char **ptr;
	int count;

	for (count = 0, ptr = theunit->numerator; *ptr; ptr++, count++);
	qsort(theunit->numerator, count, sizeof(char *), compare);
	for (count = 0, ptr = theunit->denominator; *ptr; ptr++, count++);
	qsort(theunit->denominator, count, sizeof(char *), compare);
}


void
cancelunit(struct unittype *theunit)
{
	char **den, **num;
	int comp;

	den = theunit->denominator;
	num = theunit->numerator;

	while (*num && *den) {
		comp = strcmp(*den, *num);
		if (!comp) {
#if 0
			if (*den!=NULLUNIT)
				free(*den);
			if (*num!=NULLUNIT)
				free(*num);
#endif
			*den++ = NULLUNIT;
			*num++ = NULLUNIT;
		} else if (comp < 0)
			den++;
		else
			num++;
	}
}




/*
   Looks up the definition for the specified unit.
   Returns a pointer to the definition or a null pointer
   if the specified unit does not appear in the units table.
*/

static char buffer[100];	/* buffer for lookupunit answers with
				   prefixes */

char *
lookupunit(char *unit)
{
	size_t len;
	int i;
	char *copy;

	for (i = 0; i < unitcount; i++) {
		if (!strcmp(unittable[i].uname, unit))
			return unittable[i].uval;
	}

	len = strlen(unit);
	if (len == 0)
		return NULL;
	if (unit[len - 1] == '^') {
		copy = dupstr(unit);
		copy[len - 1] = '\0';
		for (i = 0; i < unitcount; i++) {
			if (!strcmp(unittable[i].uname, copy)) {
				strlcpy(buffer, copy, sizeof(buffer));
				free(copy);
				return buffer;
			}
		}
		free(copy);
	}
	if (unit[len - 1] == 's') {
		copy = dupstr(unit);
		copy[len - 1] = '\0';
		--len;
		for (i = 0; i < unitcount; i++) {
			if (!strcmp(unittable[i].uname, copy)) {
				strlcpy(buffer, copy, sizeof(buffer));
				free(copy);
				return buffer;
			}
		}
		if (len != 0 && copy[len - 1] == 'e') {
			copy[len - 1] = 0;
			for (i = 0; i < unitcount; i++) {
				if (!strcmp(unittable[i].uname, copy)) {
					strlcpy(buffer, copy, sizeof(buffer));
					free(copy);
					return buffer;
				}
			}
		}
		free(copy);
	}
	for (i = 0; i < prefixcount; i++) {
		len = strlen(prefixtable[i].prefixname);
		if (!strncmp(prefixtable[i].prefixname, unit, len)) {
			unit += len;
			if (!strlen(unit) || lookupunit(unit)) {
				snprintf(buffer, sizeof(buffer),
				    "%s %s", prefixtable[i].prefixval, unit);
				return buffer;
			}
		}
	}
	return NULL;
}



/*
   reduces a product of symbolic units to primitive units.
   The three low bits are used to return flags:

     bit 0 (1) set on if reductions were performed without error.
     bit 1 (2) set on if no reductions are performed.
     bit 2 (4) set on if an unknown unit is discovered.
*/


int
reduceproduct(struct unittype *theunit, int flip)
{
	char *toadd, **product;
	int didsomething = 2;

	if (flip)
		product = theunit->denominator;
	else
		product = theunit->numerator;

	for (; *product; product++) {

		for (;;) {
			if (!strlen(*product))
				break;
			toadd = lookupunit(*product);
			if (!toadd) {
//				printf("unknown unit '%s'\n", *product);
				return 0;
			}
			if (strchr(toadd, PRIMITIVECHAR))
				break;
			didsomething = 1;
			if (*product != NULLUNIT) {
				free(*product);
				*product = NULLUNIT;
			}
			if (addunit(theunit, toadd, flip))
				return 0;
		}
	}
	return didsomething;
}


/*
   Reduces numerator and denominator of the specified unit.
   Returns 0 on success, or 1 on unknown unit error.
*/

int
reduceunit(struct unittype *theunit)
{
	int ret;

	ret = 1;
	while (ret & 1) {
		ret = reduceproduct(theunit, 0) | reduceproduct(theunit, 1);
		if (ret & 4)
			return 1;
	}
	return 0;
}


int
compareproducts(char **one, char **two)
{
	while (*one || *two) {
		if (!*one && *two != NULLUNIT)
			return 1;
		if (!*two && *one != NULLUNIT)
			return 1;
		if (*one == NULLUNIT)
			one++;
		else if (*two == NULLUNIT)
			two++;
		else if (strcmp(*one, *two))
			return 1;
		else
			one++, two++;
	}
	return 0;
}


/* Return zero if units are compatible, nonzero otherwise */

int
compareunits(struct unittype *first, struct unittype *second)
{
	return compareproducts(first->numerator, second->numerator) ||
	    compareproducts(first->denominator, second->denominator);
}


int
completereduce(struct unittype *unit)
{
	if (reduceunit(unit))
		return 1;
	sortunit(unit);
	cancelunit(unit);
	return 0;
}


void
showanswer(struct unittype *have, struct unittype *want)
{
	if (compareunits(have, want)) {
		printf("conformability error\n");
		showunit(have);
		showunit(want);
	} else
		printf("\t* %.8g\n\t/ %.8g\n", have->factor / want->factor,
		    want->factor / have->factor);
}


void
usage(void)
{
	fprintf(stderr,
	    "usage: units [-qv] [-f filename] [from-unit to-unit]\n");
	exit(3);
}

/*
int
main(int argc, char **argv)
{

	struct unittype have, want;
	char havestr[81], wantstr[81];
	int optchar;
	char *userfile = 0;
	int quiet = 0;

	extern char *optarg;
	extern int optind;

	while ((optchar = getopt(argc, argv, "vqf:")) != -1) {
		switch (optchar) {
		case 'f':
			userfile = optarg;
			break;
		case 'q':
			quiet = 1;
			break;
		case 'v':
			fprintf(stderr,
			    "units version %s Copyright (c) 1993 by Adrian Mariano\n",
			    VERSION);
			fprintf(stderr,
			    "This program may be freely distributed\n");
			usage();
		default:
			usage();
			break;
		}
	}

	if (optind != argc - 2 && optind != argc)
		usage();

	readunits(userfile);

	if (optind == argc - 2) {
		strlcpy(havestr, argv[optind], sizeof(havestr));
		strlcpy(wantstr, argv[optind + 1], sizeof(wantstr));
		initializeunit(&have);
		addunit(&have, havestr, 0);
		completereduce(&have);
		initializeunit(&want);
		addunit(&want, wantstr, 0);
		completereduce(&want);
		showanswer(&have, &want);
	} else {
		if (!quiet)
			printf("%d units, %d prefixes\n", unitcount,
			    prefixcount);
		for (;;) {
			do {
				initializeunit(&have);
				if (!quiet)
					printf("You have: ");
				if (!fgets(havestr, sizeof(havestr), stdin)) {
					if (!quiet)
						putchar('\n');
					exit(0);
				}
			} while (addunit(&have, havestr, 0) ||
			    completereduce(&have));
			do {
				initializeunit(&want);
				if (!quiet)
					printf("You want: ");
				if (!fgets(wantstr, sizeof(wantstr), stdin)) {
					if (!quiet)
						putchar('\n');
					exit(0);
				}
			} while (addunit(&want, wantstr, 0) ||
			    completereduce(&want));
			showanswer(&have, &want);
		}
	}
	return (0);
}
*/


bool initialized = false;

void initialize(const string& userfilestr)
{
	const char* userfile = userfilestr.c_str();
	char line[80], *lineptr;
	int len, linenum, i;
	FILE* unitfile;
	bool skip = false;

	unitcount = 0;
	prefixcount = 0;
	linenum = 0;

	unitfile = fopen(userfile, "rt");
	if (!unitfile)
		throw UnitsException(string("Unable to open units file '") + userfile + "'");
	while (!feof(unitfile)) {
		if (!fgets(line, sizeof(line), unitfile))
			break;
		linenum++;
		lineptr = line;
		if (*lineptr == '/')
			continue;
		// Allow '#' comments as well
		char *cp = strchr(lineptr,'#');
		if (cp != NULL)
			*cp = 0;
		// Skip locale-specific units (TODO fix)
		if (strncmp(lineptr,"!locale",7)==0)
			skip = true;
		if (strncmp(lineptr,"!endlocale",10)==0)
			skip = false;
		if (skip)
			continue;

		lineptr += strspn(lineptr, " \n\t");
		len = strcspn(lineptr, " \n\t");
		lineptr[len] = 0;
		if (!strlen(lineptr))
			continue;
		if (lineptr[strlen(lineptr) - 1] == '-') { /* it's a prefix */
			if (prefixcount == MAXPREFIXES) {
//				fprintf(stderr,
//				    "Memory for prefixes exceeded in line %d\n",
//				    linenum);
				continue;
			}

			lineptr[strlen(lineptr) - 1] = 0;
			for (i = 0; i < prefixcount; i++) {
				if (!strcmp(prefixtable[i].prefixname, lineptr))
					break;
			}
			if (i < prefixcount) {
//				fprintf(stderr, "Redefinition of prefix '%s' "
//				    "on line %d ignored\n", lineptr, linenum);
				continue;	/* skip duplicate prefix */
			}

			prefixtable[prefixcount].prefixname = dupstr(lineptr);
			lineptr += len + 1;
			if (!strlen(lineptr)) {
				readerror(linenum);
				free(prefixtable[prefixcount].prefixname);
				continue;
			}
			lineptr += strspn(lineptr, " \n\t");
			len = strcspn(lineptr, "\n\t");
			lineptr[len] = 0;
			prefixtable[prefixcount++].prefixval = dupstr(lineptr);
		} else {		/* it's not a prefix */
			if (unitcount == MAXUNITS) {
//				fprintf(stderr,
//				    "Memory for units exceeded in line %d\n",
//				    linenum);
				continue;
			}

			for (i = 0; i < unitcount; i++) {
				if (!strcmp(unittable[i].uname, lineptr))
					break;
			}
			if (i < unitcount) {
//				fprintf(stderr, "Redefinition of unit '%s' "
//				    "on line %d ignored\n", lineptr, linenum);
				continue;	/* skip duplicate unit */
			}

			unittable[unitcount].uname = dupstr(lineptr);
			lineptr += len + 1;
			lineptr += strspn(lineptr, " \n\t");
			if (!strlen(lineptr)) {
				readerror(linenum);
				free(unittable[unitcount].uname);
				continue;
			}
			len = strcspn(lineptr, "\n\t");
			lineptr[len] = 0;
			unittable[unitcount++].uval = dupstr(lineptr);
		}
	}
	fclose(unitfile);
	initialized = true;
}

double convert(const double value, const string& haves, const string& wants)
{
	const char* havestr = haves.c_str();
	const char* wantstr = wants.c_str();
	if (!initialized) {
		for (int i = 0; i < num_unitsfile_locations; i++) {
			try {
				initialize(unitsfile_locations[i]);
			} catch (exception& e) {
			}
			if (initialized)
				break;
		}
		if (!initialized)
			throw(UnitsException("No units file (units.dat) found in default locations"));
	}

	struct unittype have, want;

	//printf("%d units, %d prefixes\n", unitcount, prefixcount);
	initializeunit(&have);
	if (addunit(&have, havestr, 0))
		throw(UnitsException(string("Error in unit '") + havestr + "'"));
	if (completereduce(&have))
		throw(UnitsException(string("Error in unit '") + havestr + "'"));
	initializeunit(&want);
	if (addunit(&want, wantstr, 0))
		throw(UnitsException(string("Error in unit '") + wantstr + "'"));
	if (completereduce(&want))
		throw(UnitsException(string("Error in unit '") + wantstr + "'"));
	if (compareunits(&have, &want)) {
		throw(UnitsException(string("Conformability error converting '") + havestr + "' to '" + wantstr));
	} else
		return value*(have.factor / want.factor);
}

} // namespace units