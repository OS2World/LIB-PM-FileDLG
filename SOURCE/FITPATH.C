/****************************************************************************
 * PSZ FitPathToBox( HWND hDlg,USHORT idText,PSZ pszPath );                 *
 * Purpose                  This function modifies the input                *
 *                          drive:\directory string so that it              *
 *                          fits within the bounds of the specified         *
 *                          static text control.                            *
 *                                                                          *
 * Parameters               hDlg is a handle to the dialog box.             *
 *                                                                          *
 *                          idText is the static text control id.           *
 *                                                                          *
 *                          pszPath is a pointer to the drive:\directory    *
 *                          string that is to be modified.                  *
 *                                                                          *
 * Return Value             The function returns a pointer to the           *
 *                          modified path string.                           *
 *                                                                          *
 *                                                                          *
 * Modifications -                                                          *
 *      17-Aug-1989 : Initial version.                                      *
 *                                                                          *
 * (c)Copyright 1989 Rick Yoder                                             *
 ****************************************************************************/

    #define INCL_WINWINDOWMGR
    #define INCL_WINMESSAGEMGR
    #define INCL_GPIPRIMITIVES
    #include <os2.h>
    #include <string.h>
    #include "tools.h"

/****************************************************************************
 * Internal function declarations                                           *
 ****************************************************************************/
    static LONG near GetTextExtent( HPS hps,PCH pchStr,USHORT cch );
/****************************************************************************/


/****************************************************************************/
    PSZ FitPathToBox( HWND hDlg,USHORT idText,PSZ pszPath )
    {
        RECTL   rc;
        LONG    cxField;
        HPS     hps;
        CHAR    chDrive;

    /* get length of static field */
        WinQueryWindowRect( WinWindowFromID(hDlg,idText),(PRECTL)&rc );
        cxField = rc.xRight - rc.xLeft;

        hps = WinGetPS(hDlg);
        if ( cxField < GetTextExtent(hps,pszPath,strlen(pszPath)) )
            {
            chDrive = *pszPath;

            /* chop characters off front of string until text is short enough */
            do
                {
                do
                    if ( *pszPath ) pszPath++;
                while ( pszPath[6] != '\\' && pszPath[6] != '\0' );
                *pszPath = chDrive;
                memcpy( pszPath+1,":\\...",5 );
                }
            while ( cxField < GetTextExtent(hps,pszPath,strlen(pszPath)) );
            }

        WinReleasePS(hps);
        return pszPath;
    }
/****************************************************************************/


/****************************************************************************
 * GetTextExtent() - Get width of the text box for the input string.        *
 ****************************************************************************/
    static LONG near GetTextExtent( HPS hps,PCH pchStr,USHORT cch )
    {
        POINTL aptl[TXTBOX_COUNT];

        if ( cch )
            {
            GpiQueryTextBox( hps,(LONG)cch,pchStr,TXTBOX_COUNT,aptl );
            return (aptl[TXTBOX_CONCAT].x - aptl[TXTBOX_BOTTOMLEFT].x);
            }
        else
            return 0L;
    }
/****************************************************************************/
