/*
 * CupsParser.h
 *
 *  Created on: 18.10.2009
 *      Author: amir
 */

#ifndef CUPSPARSER_H_
#define CUPSPARSER_H_

/** Default for debug mode. */
#define DEBUG (1)

/** Basename for files generated by the program. */
#define FILE_BASENAME "epilog"

/** Number of characters allowable for a filename. */
#define FILENAME_NCHARS (128)

/** Maximum allowable hostname characters. */
#define HOSTNAME_NCHARS (1024)

/** Temporary directory to store files. */
#define TMP_DIRECTORY "/tmp"

#define PRINTER_NAME "LaserCutter"

#define GTKLP_CONF_DIR "/home/amir/dev/Epilog/gtklpconf/"


class CupsParser {
public:
    CupsParser();
    virtual ~CupsParser();
};

#endif /* CUPSPARSER_H_ */
