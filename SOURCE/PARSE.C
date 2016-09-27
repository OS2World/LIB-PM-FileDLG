/****************************************************************************
 * USHORT ParseFileName( PSZ pszSource,PSZ pszDest,PSZ pszSearch );         *
 * Purpose                  This function generates a fully                 *
 *                          qualified file name or search spec.             *
 *                          from the input value, and changes               *
 *                          the current drive and directory to              *
 *                          the drive/directory listed in the               *
 *                          input file name.                                *
 *                                                                          *
 * Parameters               pszSource points to the input file name         *
 *                          or search spec.                                 *
 *                                                                          *
 *                          pszDest points to the location where            *
 *                          the fully qualified file name or                *
 *                          search spec is to be stored.                    *
 *                                                                          *
 *                          pszSearch points to the current file search     *
 *                          specification. This is used when the input      *
 *                          string is just a drive or a drive:\directory    *
 *                          without a file name.                            *
 *                                                                          *
 * Return Value             The return value is zero if the function        *
 *                          is successful, otherwise it is an               *
 *                          error value. The contents of pszDest is         *
 *                          undefined if an error has occurred.             *
 *                                                                          *
 * Notes                    This function modifies the contents of          *
 *                          the string pointed to by pszSource.             *
 *                                                                          *
 *                          This function will not detect an illegal        *
 *                          filename or search specification.               *
 *                                                                          *
 *                                                                          *
 * Modifications -                                                          *
 *      17-Aug-1989 : Initial version.                                      *
 *                                                                          *
 * (c)Copyright 1989 Rick Yoder                                             *
 ****************************************************************************/

    #define INCL_DOSFILEMGR
    #define INCL_DOSMISC
    #define INCL_DOSERRORS
    #include <os2.h>
    #include <string.h>
    #include "tools.h"

/****************************************************************************/
    USHORT ParseFileName( PSZ pszSource,PSZ pszDest,PSZ pszSearch )
    {
        USHORT  usMaxPathLen;
        USHORT  usDrive;
        ULONG   ulMap;
        USHORT  cbBuf;
        USHORT  usResult;
        PCHAR   pcLastSlash,pcFileOnly;

    /* Get maximum path length */
        if ( usResult = DosQSysInfo(0,(PBYTE)&usMaxPathLen,sizeof(USHORT)) )
            return usResult;

    /* Convert input string to upper case */
        strupr( pszSource );

    /* Get drive from input string or current drive */
        if ( pszSource[1] == ':' && pszSource[0] != '\0' )
            {
            if ( usResult = DosSelectDisk(pszSource[0]-'@') ) return usResult;
            pszSource += 2;
            }
        if ( usResult = DosQCurDisk(&usDrive,&ulMap) ) return usResult;
        *pszDest++ = (CHAR)usDrive + '@';
        *pszDest++ = ':';
        *pszDest++ = '\\';
        *pszDest   = '\0';

    /* If rest of input string is empty, append curdir\search and return */
        if ( *pszSource == '\0' )
            {
            cbBuf = usMaxPathLen - 3;
            if ( !(usResult = DosQCurDir(0,pszDest,&cbBuf)) )
                {
                if ( *(pszDest + strlen(pszDest) - 1) != '\\' )
                    strcat( pszDest,"\\" );
                strcat( pszDest,pszSearch );
                }
            return usResult;
            }

    /* Search for last backslash. If none then source could be directory. */
        if ( NULL == (pcLastSlash = strrchr(pszSource,'\\')) )
            {
            switch ( usResult = DosChDir(pszSource,0L) ) {
                case NO_ERROR:
                    cbBuf = usMaxPathLen - 3;
                    if ( usResult = DosQCurDir(0,pszDest,&cbBuf) )
                        return usResult;
                    if ( *(pszDest + strlen(pszDest) - 1) != '\\' )
                        strcat( pszDest,"\\" );
                    strcat( pszDest,pszSearch );
                    return 0;

                case ERROR_PATH_NOT_FOUND:
                case ERROR_FILE_NOT_FOUND:
                case ERROR_ACCESS_DENIED:
                    cbBuf = usMaxPathLen - 3;
                    if ( usResult = DosQCurDir(0,pszDest,&cbBuf) )
                        return usResult;
                    if ( *(pszDest + strlen(pszDest) - 1) != '\\' )
                        strcat( pszDest,"\\" );
                    strcat( pszDest,pszSource );
                    return 0;

                default:
                    return usResult;
                }
            }

    /* If the only backslash is at beginning, pszSource contains either */
    /* \filename or \dirname or \                                       */
        if ( pszSource == pcLastSlash )
            {
            switch ( usResult = DosChDir(pszSource,0L) ) {
                case NO_ERROR:  // pszSource contains \dirname
                    cbBuf = usMaxPathLen - 3;
                    if ( !(usResult = DosQCurDir(0,pszDest,&cbBuf)) )
                        {
                        if ( *(pszDest + strlen(pszDest) - 1) != '\\' )
                            strcat( pszDest,"\\" );
                        strcat( pszDest,pszSearch );
                        }
                    return usResult;

                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                case ERROR_ACCESS_DENIED:   // pszSource contains \filename
                    if ( usResult = DosChDir("\\",0L) ) return usResult;
                    strcpy( pszDest,pszSource+1 );
                    return 0;

                default:
                    return usResult;
                }
            }

    /* Input has d:\dir\filename or d:\dir\dir or d:\dir\ */
        switch ( usResult = DosChDir(pszSource,0L) ) {
            case NO_ERROR:      // pszSource contains d:\dir\dir
                cbBuf = usMaxPathLen - 3;
                if ( !(usResult = DosQCurDir(0,pszDest,&cbBuf)) )
                    {
                    if ( *(pszDest + strlen(pszDest) - 1) != '\\' )
                        strcat( pszDest,"\\" );
                    strcat( pszDest,pszSearch );
                    }
                return usResult;

            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
            case ERROR_ACCESS_DENIED:   // d:\dir\filename or d:\dir\
                *pcLastSlash = '\0';
                cbBuf = usMaxPathLen - 3;
                if (   !(usResult = DosChDir(pszSource,0L))
                    && !(usResult = DosQCurDir(0,pszDest,&cbBuf)) )
                    {
                    /* Append input filename, if any */
                    pcFileOnly = pcLastSlash + 1;
                    if ( *(pszDest + strlen(pszDest) - 1) != '\\' )
                        strcat( pszDest,"\\" );
                    strcat( pszDest,
                            (*pcFileOnly=='\0') ? pszSearch : pcFileOnly );
                    }
                return usResult;

            default:
                return usResult;
            }
    }
/****************************************************************************/
