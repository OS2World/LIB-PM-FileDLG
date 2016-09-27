/****************************************************************************
 * OPENDLG.C - Open File dialog box routines.                               *
 *                                                                          *
 *  Notes -                                                                 *
 *      This following functions will have to be modified in order to       *
 *      support OS/2 v1.2 long file names :                                 *
 *                                                                          *
 *              FillListBoxes()                                             *
 *                                                                          *
 *  Modifications -                                                         *
 *      11-Aug-1989 : Initial version.                                      *
 *                                                                          *
 * (c)Copyright 1989 Rick Yoder                                             *
 ****************************************************************************/

    #define INCL_WIN
    #define INCL_DOS
    #define INCL_DOSERRORS
    #include <os2.h>

    #include <string.h>

    #include "filedlg.h"
    #include "dialog.h"
    #include "tools.h"

/****************************************************************************
 *  Internal data structure definitions                                     *
 ****************************************************************************/
    typedef struct {
        PSZ     pszTitle;       // dialog box title
        PSZ     pszIns;         // dialog box instructions
        void (CALLBACK *pfnHelpProc)(HWND hDlg); // ptr to help procedure
        PSZ     pszFile;        // ptr to name of opened file
        PHFILE  phf;            // ptr to file handle
        ULONG   ulFileSize;     // initial file size
        PUSHORT pusAction;      // action taken on open
        USHORT  usAttribute;    // file attribute
        USHORT  fsOpenFlags;    // open flags
        USHORT  fsOpenMode;     // open mode
        ULONG   ulReserved;     // reserved
        PSZ     pszShowSpec;    // file spec of files to be listed
        USHORT  usShowAttr;     // attributes of files to be listed
        USHORT  usMaxPathLen;   // maximum path name length
        PSZ     pszScratch;     // ptr to scratch data area
        USHORT  usSelectDrive;
        USHORT  usSelectDir;
        USHORT  usSelectFile;
        } DATA;

    typedef DATA * PDATA;
/****************************************************************************/


/****************************************************************************
 *  Internal procedure declarations                                         *
 ****************************************************************************/
    MRESULT CALLBACK _OpenDlgProc( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2 );
    static void FillListBoxes( HWND hDlg,PDATA pData );
    static USHORT OpenFile( HWND hDlg,PDATA pData );
/****************************************************************************/


/****************************************************************************
 *  FileOpenDlg()                                                           *
 ****************************************************************************/
    USHORT CALLBACK FileOpenDlg( HWND hwndOwner,
                                 PSZ pszTitle,PSZ pszIns,
                                 PSZ pszShowSpec,USHORT usShowAttr,
                                 void (CALLBACK *pfnHelpProc)(HWND hDlg),
                                 PSZ pszFile,
                                 PHFILE phf,
                                 ULONG ulFileSize,
                                 PUSHORT pusAction,
                                 USHORT usAttribute,
                                 USHORT fsOpenFlags,
                                 USHORT fsOpenMode,
                                 ULONG ulReserved )
    {
        USHORT  usMaxPathLen;
        SEL     sel;
        PDATA   pData;
        USHORT  rc;
        HMODULE hmod;

    /* Set pszTitle and pszIns to default if NULL */
        if ( pszTitle == NULL ) pszTitle = "Open File";
        if ( pszIns == NULL ) pszIns = "Select file or type filename.";

    /* Get maximum pathname length */
        DosQSysInfo( 0,(PBYTE)&usMaxPathLen,sizeof(USHORT) );

    /* Get module handle for filedlg dynamic-link library */
        if ( (rc = DosGetModHandle("FILEDLG",&hmod)) )
            {
            ErrMessageBox( hwndOwner,pszTitle,rc );
            return FDLG_CANCEL;
            }

    /* Allocate memory for dialog data */
        if ( (rc = DosAllocSeg(sizeof(DATA),&sel,SEG_NONSHARED)) )
            {
            ErrMessageBox( hwndOwner,pszTitle,rc );
            return FDLG_CANCEL;
            }
        pData = MAKEP(sel,0);

    /* Allocate memory for search spec */
        if ( (rc = DosAllocSeg(usMaxPathLen,&sel,SEG_NONSHARED)) )
            {
            DosFreeSeg( SELECTOROF(pData) );
            ErrMessageBox( hwndOwner,pszTitle,rc );
            return FDLG_CANCEL;
            }
        pData->pszShowSpec = MAKEP(sel,0);

    /* Allocate scratch data area */
        if ( (rc = DosAllocSeg(usMaxPathLen,&sel,SEG_NONSHARED)) )
            {
            DosFreeSeg( SELECTOROF(pData->pszShowSpec) );
            DosFreeSeg( SELECTOROF(pData) );
            ErrMessageBox( hwndOwner,pszTitle,rc );
            return FDLG_CANCEL;
            }
        pData->pszScratch = MAKEP(sel,0);

    /* Set current drive and directory to drive and directory listed   */
    /* in show file specification, and store filename portion of spec. */
        if ( rc = ParseFileName(pszShowSpec,pData->pszScratch,NULL) )
            {
            ErrMessageBox( hwndOwner,pszTitle,rc );
            strcpy( pData->pszShowSpec,"*.*" );
            }
        else
            strcpy( pData->pszShowSpec,strrchr(pData->pszScratch,'\\')+1 );

    /* Initialize contents of dialog box data structure */
        pData->pszTitle     = pszTitle;
        pData->pszIns       = pszIns;
        pData->pfnHelpProc  = pfnHelpProc;
        pData->pszFile      = pszFile;
        pData->phf          = phf;
        pData->ulFileSize   = ulFileSize;
        pData->pusAction    = pusAction;
        pData->usAttribute  = usAttribute;
        pData->fsOpenFlags  = fsOpenFlags;
        pData->fsOpenMode   = fsOpenMode;
        pData->ulReserved   = ulReserved;
        strcpy( pData->pszShowSpec,pszShowSpec );
        pData->usShowAttr   = usShowAttr;
        pData->usMaxPathLen = usMaxPathLen;

    /* Activate open file dialog box */
        rc = WinDlgBox( HWND_DESKTOP,hwndOwner,_OpenDlgProc,
                        hmod,IDD_OPEN,pData );

    /* Free resources */
        DosFreeSeg( SELECTOROF(pData->pszShowSpec) );
        DosFreeSeg( SELECTOROF(pData->pszScratch) );
        DosFreeSeg( SELECTOROF(pData) );

        return rc;
    }
/****************************************************************************/


/****************************************************************************
 * OpenDlgProc()                                                            *
 ****************************************************************************/
    MRESULT CALLBACK _OpenDlgProc( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2 )
    {
        PDATA   pData;
        USHORT  usSelect;
        USHORT  usResult;
        USHORT  usControl,usEvent;

        switch ( msg ) {
            case WM_INITDLG:
                pData = PVOIDFROMMP( mp2 );
                WinSetWindowULong( hwnd,QWL_USER,(ULONG)pData );
                if ( pData->pfnHelpProc == NULL )
                    WinDestroyWindow( WinWindowFromID(hwnd,MBID_HELP) );
                WinSetDlgItemText( hwnd,OPEN_TITLE,pData->pszTitle );
                WinSetDlgItemText( hwnd,OPEN_HLPTEXT,pData->pszIns );
                WinSendDlgItemMsg( hwnd,OPEN_FNAME,EM_SETTEXTLIMIT,
                                   MPFROM2SHORT(pData->usMaxPathLen,0),NULL );
                FillListBoxes( hwnd,pData );
                return 0;

            case WM_CHAR:
                if ( CHARMSG(&msg)->fs & KC_ALT )
                    switch ( CHARMSG(&msg)->chr ) {
                        case 'n':
                        case 'N': WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwnd,OPEN_FNAME));
                                  return 0;

                        case 'r':
                        case 'R': WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwnd,OPEN_DRIVES));
                                  return 0;

                        case 'd':
                        case 'D': WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwnd,OPEN_DIRLIST));
                                  return 0;

                        case 'f':
                        case 'F': WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwnd,OPEN_FILELIST));
                                  return 0;
                        }
                break;

            case WM_CONTROL:
                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                usControl = SHORT1FROMMP( mp1 );
                usEvent   = SHORT2FROMMP( mp1 );

                switch ( usControl ) {
                    case OPEN_DRIVES:
                        switch ( usEvent ) {
                            case LN_ENTER:
                                WinSendDlgItemMsg( hwnd,OPEN_DRIVES,
                                                   LM_QUERYITEMTEXT,
                                                   MPFROM2SHORT(pData->usSelectDrive,pData->usMaxPathLen),
                                                   MPFROMP(pData->pszScratch) );
                                usResult = DosSelectDisk(pData->pszScratch[0]-'@');
                                if ( usResult )
                                    ErrMessageBox( hwnd,pData->pszTitle,usResult );
                                else
                                    FillListBoxes( hwnd,pData );
                                WinSendDlgItemMsg( hwnd,OPEN_DRIVES,LM_SELECTITEM,
                                                   MPFROMSHORT(pData->usSelectDrive),
                                                   MPFROMSHORT(TRUE) );
                                return 0;

                            case LN_SETFOCUS:
                                usResult = (USHORT)WinSendDlgItemMsg( hwnd,OPEN_DRIVES,
                                                                      LM_QUERYTOPINDEX,
                                                                      0L,0L );
                                if ( usResult != LIT_NONE )
                                    {
                                    if ( pData->usSelectDrive < usResult )
                                        pData->usSelectDrive = usResult;
                                    else if (pData->usSelectDrive > usResult+5)
                                        pData->usSelectDrive = usResult+5;

                                    WinSendDlgItemMsg( hwnd,OPEN_DRIVES,LM_SELECTITEM,
                                                       MPFROMSHORT(pData->usSelectDrive),
                                                       MPFROMSHORT(TRUE) );
                                    }
                                return 0;

                            case LN_KILLFOCUS:
                                WinSendDlgItemMsg( hwnd,OPEN_DRIVES,LM_SELECTITEM,
                                                   MPFROMSHORT(pData->usSelectDrive),
                                                   MPFROMSHORT(FALSE) );
                                return 0;

                            case LN_SELECT:
                                usSelect = (USHORT)WinSendDlgItemMsg( hwnd,OPEN_DRIVES,
                                                                      LM_QUERYSELECTION,
                                                                      0L,0L );
                                if ( usSelect != LIT_NONE )
                                    pData->usSelectDrive = usSelect;
                                return 0;
                            }
                        break;

                    case OPEN_DIRLIST:
                        switch ( usEvent ) {
                            case LN_ENTER:
                                WinSendDlgItemMsg( hwnd,OPEN_DIRLIST,
                                                   LM_QUERYITEMTEXT,
                                                   MPFROM2SHORT(pData->usSelectDir,pData->usMaxPathLen),
                                                   MPFROMP(pData->pszScratch) );
                                usResult = DosChDir(pData->pszScratch,0L);
                                if ( usResult )
                                    ErrMessageBox( hwnd,pData->pszTitle,usResult );
                                else
                                    FillListBoxes( hwnd,pData );
                                WinSendDlgItemMsg( hwnd,OPEN_DIRLIST,LM_SELECTITEM,
                                                   MPFROMSHORT(pData->usSelectDir),
                                                   MPFROMSHORT(TRUE) );
                                return 0;

                            case LN_SETFOCUS:
                                usResult = (USHORT)WinSendDlgItemMsg( hwnd,OPEN_DIRLIST,
                                                                      LM_QUERYTOPINDEX,
                                                                      0L,0L );
                                if ( usResult != LIT_NONE )
                                    {
                                    if ( pData->usSelectDir < usResult )
                                        pData->usSelectDir = usResult;
                                    else if (pData->usSelectDir > usResult+5)
                                        pData->usSelectDir = usResult+5;

                                    WinSendDlgItemMsg( hwnd,OPEN_DIRLIST,LM_SELECTITEM,
                                                       MPFROMSHORT(pData->usSelectDir),
                                                       MPFROMSHORT(TRUE) );
                                    }
                                return 0;

                            case LN_KILLFOCUS:
                                WinSendDlgItemMsg( hwnd,OPEN_DIRLIST,LM_SELECTITEM,
                                                   MPFROMSHORT(pData->usSelectDir),
                                                   MPFROMSHORT(FALSE) );
                                return 0;

                            case LN_SELECT:
                                usSelect = (USHORT)WinSendDlgItemMsg( hwnd,OPEN_DIRLIST,
                                                                      LM_QUERYSELECTION,
                                                                      0L,0L );
                                if ( usSelect != LIT_NONE )
                                    pData->usSelectDir = usSelect;
                                return 0;
                            }
                        break;

                    case OPEN_FILELIST:
                        switch ( usEvent ) {
                            case LN_SELECT:
                                usSelect = (USHORT)WinSendDlgItemMsg( hwnd,OPEN_FILELIST,
                                                                      LM_QUERYSELECTION,
                                                                      0L,0L );
                                if ( usSelect != LIT_NONE )
                                    pData->usSelectFile = usSelect;
                                WinSendDlgItemMsg( hwnd,OPEN_FILELIST,
                                                   LM_QUERYITEMTEXT,
                                                   MPFROM2SHORT(pData->usSelectFile,pData->usMaxPathLen),
                                                   MPFROMP(pData->pszScratch) );
                                WinSetDlgItemText( hwnd,OPEN_FNAME,
                                                   pData->pszScratch );
                                return 0;

                            case LN_ENTER:
                                WinSendDlgItemMsg( hwnd,OPEN_FILELIST,
                                                   LM_QUERYITEMTEXT,
                                                   MPFROM2SHORT(pData->usSelectFile,pData->usMaxPathLen),
                                                   MPFROMP(pData->pszScratch) );
                                WinSetDlgItemText( hwnd,OPEN_FNAME,
                                                   pData->pszScratch );
                                if ( !OpenFile(hwnd,pData) )
                                    WinDismissDlg( hwnd,FDLG_OK );
                                return 0;

                            case LN_SETFOCUS:
                                usResult = (USHORT)WinSendDlgItemMsg( hwnd,OPEN_FILELIST,
                                                                      LM_QUERYTOPINDEX,
                                                                      0L,0L );
                                if ( usResult != LIT_NONE )
                                    {
                                    if ( pData->usSelectFile < usResult )
                                        pData->usSelectFile = usResult;
                                    else if (pData->usSelectFile > usResult+8)
                                        pData->usSelectFile = usResult+8;

                                    WinSendDlgItemMsg( hwnd,OPEN_FILELIST,LM_SELECTITEM,
                                                       MPFROMSHORT(pData->usSelectFile),
                                                       MPFROMSHORT(TRUE) );
                                    }
                                return 0;

                            case LN_KILLFOCUS:
                                WinSendDlgItemMsg( hwnd,OPEN_FILELIST,LM_SELECTITEM,
                                                   MPFROMSHORT(pData->usSelectFile),
                                                   MPFROMSHORT(FALSE) );
                                return 0;
                            }
                        break;

                    case OPEN_FNAME:
                        if ( usEvent == EN_SETFOCUS )
                            {
                            usResult = WinQueryDlgItemTextLength( hwnd,
                                                                  OPEN_FNAME );
                            WinSendDlgItemMsg( hwnd,OPEN_FNAME,EM_SETSEL,
                                               MPFROM2SHORT(0,usResult),0L );
                            return 0;
                            }
                        break;
                    }
                break;

            case WM_COMMAND:
                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                switch (COMMANDMSG(&msg)->cmd) {
                    case DID_OK:
                        WinQueryDlgItemText( hwnd,OPEN_FNAME,
                                             pData->usMaxPathLen,
                                             pData->pszScratch );
                        if ( !OpenFile(hwnd,pData) )
                            WinDismissDlg( hwnd,FDLG_OK );
                        return 0;

                    case DID_CANCEL:
                        WinDismissDlg( hwnd,FDLG_CANCEL );
                        return 0;
                    }
                break;

            case WM_HELP:
                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                if ( pData->pfnHelpProc != NULL )
                    {
                    (*pData->pfnHelpProc)( hwnd );
                    return 0;
                    }
                break;
            }
        return WinDefDlgProc( hwnd,msg,mp1,mp2 );
    }
/****************************************************************************/


/****************************************************************************
 * FillListBoxes() - Fill drive, directory, and file list boxes, and        *
 *                   display the name of the current drive and directory    *
 *                   in the OPEN_CURDIR control.                            *
 *                                                                          *
 *                   This function uses the scratch data area.              *
 *                                                                          *
 *                   Currently, this function does not support OS/2 v1.2    *
 *                   long file names. To fix this, recompile using the      *
 *                   OS/2 1.2 header files so as to use the updated         *
 *                   definition of the FILEFINDBUF structure.               *
 ****************************************************************************/
    static void FillListBoxes( HWND hDlg,PDATA pData )
    {
        USHORT      usDriveNum;
        ULONG       ulMap;
        USHORT      usResult;
        HDIR        hdir;
        FILEFINDBUF findbuf;
        USHORT      usCount;

    /* Clear current contents of list boxes and text controls */
        WinSendDlgItemMsg( hDlg,OPEN_DRIVES,LM_DELETEALL,NULL,NULL );
        WinSendDlgItemMsg( hDlg,OPEN_DIRLIST,LM_DELETEALL,NULL,NULL );
        WinSendDlgItemMsg( hDlg,OPEN_FILELIST,LM_DELETEALL,NULL,NULL );
        WinSetDlgItemText( hDlg,OPEN_CURDIR,"" );
        WinSetDlgItemText( hDlg,OPEN_FNAME,pData->pszShowSpec );

        pData->usSelectDrive = 0;
        pData->usSelectDir   = 0;
        pData->usSelectFile  = 0;

    /* Fill in disk drive list box */
        if ( usResult = DosQCurDisk(&usDriveNum,&ulMap) )
            {
            ErrMessageBox( hDlg,pData->pszTitle,usResult );
            return;
            }

        pData->pszScratch[1] = ':';
        pData->pszScratch[2] = '\0';
        for ( usCount = 0; usCount < 26; usCount++ )
            if ( ulMap & 1L << usCount )
                {
                pData->pszScratch[0] = (CHAR)usCount + 'A';

                WinSendDlgItemMsg( hDlg,OPEN_DRIVES,LM_INSERTITEM,
                                   MPFROM2SHORT(LIT_END,0),
                                   MPFROMP(pData->pszScratch) );
                }

    /* Set OPEN_CURDIR static text control to current drive/directory */
       usCount = pData->usMaxPathLen-3;
       if ( usResult = DosQCurDir(0,pData->pszScratch+3,&usCount) )
            {
            ErrMessageBox( hDlg,pData->pszTitle,usResult );
            return;
            }
        pData->pszScratch[0] = (CHAR)usDriveNum + '@';
        pData->pszScratch[1] = ':';
        pData->pszScratch[2] = '\\';
        WinSetDlgItemText( hDlg,
                           OPEN_CURDIR,
                           FitPathToBox(hDlg,OPEN_CURDIR,pData->pszScratch) );

    /* Fill list box with subdirectories of current directory */
        hdir    = HDIR_CREATE;
        usCount = 1;
        usResult = DosFindFirst( "*.*",&hdir,FILE_DIRECTORY,&findbuf,
                                  sizeof(findbuf),&usCount,0L );

        while ( !usResult )
            {
            if (   (findbuf.attrFile & FILE_DIRECTORY)
                && (findbuf.achName[0] != '.' || findbuf.achName[1]) )
                {
                WinSendDlgItemMsg( hDlg,OPEN_DIRLIST,LM_INSERTITEM,
                                   MPFROM2SHORT(LIT_END,0),
                                   MPFROMP(findbuf.achName) );
                }
            usResult = DosFindNext( hdir,&findbuf,sizeof(findbuf),&usCount );
            }

        if ( usResult != ERROR_NO_MORE_SEARCH_HANDLES ) DosFindClose(hdir);
        if ( usResult && usResult != ERROR_NO_MORE_FILES )
            {
            ErrMessageBox( hDlg,pData->pszTitle,usResult );
            return;
            }

    /* Fill file list box with list of files that match search specs. */
        hdir    = HDIR_CREATE;
        usCount = 1;
        usResult = DosFindFirst( pData->pszShowSpec,&hdir,
                                 pData->usShowAttr,&findbuf,
                                 sizeof(findbuf),&usCount,0L );

        while ( !usResult )
            {
            WinSendDlgItemMsg( hDlg,OPEN_FILELIST,LM_INSERTITEM,
                               MPFROM2SHORT(LIT_END,0),
                               MPFROMP(findbuf.achName) );

            usResult = DosFindNext( hdir,&findbuf,sizeof(findbuf),&usCount );
            }

        if ( usResult != ERROR_NO_MORE_SEARCH_HANDLES ) DosFindClose(hdir);
        if ( usResult && usResult != ERROR_NO_MORE_FILES )
            ErrMessageBox( hDlg,pData->pszTitle,usResult );

    /* Done. Return to caller. */
        return;
    }
/****************************************************************************/


/****************************************************************************
 * OpenFile() - This function attempts to open the file specified           *
 *              in the scratch data area, or if the file is a search        *
 *              specification, updates the contents of the list boxes.      *
 *                                                                          *
 *              This function returns a non-zero value if an error occured  *
 *              or the input string was a search specification.             *
 ****************************************************************************/
    static USHORT OpenFile( HWND hDlg,PDATA pData )
    {
        USHORT  usResult;

        usResult = ParseFileName( pData->pszScratch,
                                  pData->pszFile,
                                  pData->pszShowSpec );
        if ( usResult )
            {
            ErrMessageBox( hDlg,pData->pszTitle,usResult );
            return 1;
            }

        if ( NULL != strpbrk(pData->pszFile,"*?") )
            {
            strcpy( pData->pszShowSpec,strrchr(pData->pszFile,'\\')+1 );
            FillListBoxes( hDlg,pData );
            return 1;
            }
        else
            {
            usResult = DosOpen( pData->pszFile,
                                pData->phf,
                                pData->pusAction,
                                pData->ulFileSize,
                                pData->usAttribute,
                                pData->fsOpenFlags,
                                pData->fsOpenMode,
                                pData->ulReserved );
            if ( usResult )
                {
                ErrMessageBox( hDlg,pData->pszTitle,usResult );
                return 1;
                }
            else
                return 0;
            }
    }
/****************************************************************************/
