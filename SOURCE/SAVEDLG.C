/****************************************************************************
 * SAVEDLG.C - Save File dialog box routines.                               *
 *                                                                          *
 *  Modifications -                                                         *
 *      21-Aug-1989 : Initial version.                                      *
 *                                                                          *
 * (c)Copyright 1989 Rick Yoder                                             *
 ****************************************************************************/

    #define INCL_WIN
    #define INCL_DOS
    #include <os2.h>

    #include <string.h>
    #include <io.h>

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
        USHORT  usMaxPathLen;   // maximum path name length
        PSZ     pszScratch;     // ptr to scratch data area
        } DATA;

    typedef DATA * PDATA;
/****************************************************************************/


/****************************************************************************
 *  Internal procedure declarations                                         *
 ****************************************************************************/
    MRESULT CALLBACK _SaveDlgProc( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2 );
    static USHORT OpenFile( HWND hDlg,PDATA pData );
/****************************************************************************/


/****************************************************************************
 *  FileOpenDlg()                                                           *
 ****************************************************************************/
    USHORT CALLBACK FileSaveDlg( HWND hwndOwner,
                                 PSZ pszTitle,PSZ pszIns,
                                 void (CALLBACK *pfnHelpProc)(HWND hDlg),
                                 PSZ pszDefault,
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
        if ( pszTitle == NULL ) pszTitle = "Save File";
        if ( pszIns == NULL ) pszIns = "Type file name.";

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

    /* Allocate scratch data area */
        if ( (rc = DosAllocSeg(usMaxPathLen,&sel,SEG_NONSHARED)) )
            {
            DosFreeSeg( SELECTOROF(pData) );
            ErrMessageBox( hwndOwner,pszTitle,rc );
            return FDLG_CANCEL;
            }
        pData->pszScratch = MAKEP(sel,0);

    /* Set current drive and directory to drive and directory listed         */
    /* in default file name, and store filename portion in scratch data area */
        if ( rc = ParseFileName(pszDefault,pData->pszScratch,NULL) )
            {
            ErrMessageBox( hwndOwner,pszTitle,rc );
            *pData->pszScratch = '\0';
            }
        else
            strcpy( pData->pszScratch,strrchr(pData->pszScratch,'\\')+1 );

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
        pData->usMaxPathLen = usMaxPathLen;

    /* Activate open file dialog box */
        rc = WinDlgBox( HWND_DESKTOP,hwndOwner,_SaveDlgProc,
                        hmod,IDD_SAVE,pData );

    /* Free resources */
        DosFreeSeg( SELECTOROF(pData->pszScratch) );
        DosFreeSeg( SELECTOROF(pData) );

        return rc;
    }
/****************************************************************************/


/****************************************************************************
 * SaveDlgProc()                                                            *
 ****************************************************************************/
    MRESULT CALLBACK _SaveDlgProc( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2 )
    {
        PDATA   pData;
        USHORT  usDriveNum;
        ULONG   ulMap;
        USHORT  usCount;
        USHORT  usResult;

        switch ( msg ) {
            case WM_INITDLG:
                pData = PVOIDFROMMP( mp2 );
                WinSetWindowULong( hwnd,QWL_USER,(ULONG)pData );
                if ( pData->pfnHelpProc == NULL )
                    WinDestroyWindow( WinWindowFromID(hwnd,MBID_HELP) );
                WinSetDlgItemText( hwnd,SAVE_TITLE,pData->pszTitle );
                WinSetDlgItemText( hwnd,SAVE_HLPTEXT,pData->pszIns );
                WinSendDlgItemMsg( hwnd,SAVE_FNAME,EM_SETTEXTLIMIT,
                                   MPFROM2SHORT(pData->usMaxPathLen,0),NULL );
                WinSetDlgItemText( hwnd,SAVE_FNAME,pData->pszScratch );
                usCount = pData->usMaxPathLen-3;
                if (   (usResult=DosQCurDisk(&usDriveNum,&ulMap))
                    || (usResult=DosQCurDir(0,pData->pszScratch+3,&usCount)) )
                    {
                    WinSetDlgItemText( hwnd,SAVE_CURDIR,"" );
                    ErrMessageBox( hwnd,pData->pszTitle,usResult );
                    }
                else
                    {
                    pData->pszScratch[0] = (CHAR)usDriveNum + '@';
                    pData->pszScratch[1] = ':';
                    pData->pszScratch[2] = '\\';
                    WinSetDlgItemText(
                        hwnd,
                        SAVE_CURDIR,
                        FitPathToBox(hwnd,SAVE_CURDIR,pData->pszScratch) );
                    }
                return 0;

            case WM_CHAR:
                if ( CHARMSG(&msg)->fs & KC_ALT )
                    switch ( CHARMSG(&msg)->chr ) {
                        case 'f':
                        case 'F': WinSetFocus(HWND_DESKTOP,WinWindowFromID(hwnd,SAVE_FNAME));
                                  return 0;
                        }
                break;


            case WM_CONTROL:
                if (   SHORT1FROMMP(mp1) == SAVE_FNAME
                    && SHORT2FROMMP(mp1) == EN_SETFOCUS )
                    {
                    usResult = WinQueryDlgItemTextLength( hwnd,SAVE_FNAME );
                    WinSendDlgItemMsg( hwnd,SAVE_FNAME,EM_SETSEL,
                                       MPFROM2SHORT(0,usResult),0L );
                    return 0;
                    }
                break;

            case WM_COMMAND:
                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                switch (COMMANDMSG(&msg)->cmd) {
                    case DID_OK:
                        WinQueryDlgItemText( hwnd,SAVE_FNAME,
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
 * OpenFile() - This function attempts to open the file specified           *
 *              in the scratch data area.                                   *
 *                                                                          *
 *              This function returns a non-zero value if an error occured. *
 ****************************************************************************/
    static USHORT OpenFile( HWND hDlg,PDATA pData )
    {
        USHORT  usResult;

        usResult = ParseFileName( pData->pszScratch,
                                  pData->pszFile,
                                  "*.*" );
        if ( usResult )
            {
            ErrMessageBox( hDlg,pData->pszTitle,usResult );
            return 1;
            }

        if ( 0 == access(pData->pszFile,0) )
            {
            switch ( WinMessageBox(HWND_DESKTOP,hDlg,
                                   "File already exists. Overwrite?",
                                   pData->pszTitle,
                                   0,MB_YESNOCANCEL|MB_ICONQUESTION) )
                {
                case MBID_YES:      break;

                case MBID_CANCEL:   WinDismissDlg( hDlg,FDLG_CANCEL );
                                    return 1;

                default:            return 1;
                }
            }

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
/****************************************************************************/
