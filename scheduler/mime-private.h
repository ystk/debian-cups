/*
 * "$Id: mime-private.h 9750 2011-05-06 22:53:53Z mike $"
 *
 *   Private MIME type/conversion database definitions for CUPS.
 *
 *   Copyright 2011 by Apple Inc.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Apple Inc. and are protected by Federal copyright
 *   law.  Distribution and use rights are outlined in the file "LICENSE.txt"
 *   which should have been included with this file.  If this file is
 *   file is missing or damaged, see the license at "http://www.cups.org/".
 */

#ifndef _CUPS_MIME_PRIVATE_H_
#  define _CUPS_MIME_PRIVATE_H_

#  include "mime.h"


/*
 * C++ magic...
 */

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Prototypes...
 */

extern void	_mimeError(mime_t *mime, const char *format, ...)
#ifdef __GNUC__
__attribute__ ((__format__ (__printf__, 2, 3)))
#endif /* __GNUC__ */
;


#  ifdef __cplusplus
}
#  endif /* __cplusplus */
#endif /* !_CUPS_MIME_PRIVATE_H_ */

/*
 * End of "$Id: mime-private.h 9750 2011-05-06 22:53:53Z mike $".
 */
