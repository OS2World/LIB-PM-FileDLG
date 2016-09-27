/*
 * SAVETEST.C -- Program to test operation of FileSaveDlg function.
 *               Compile using compact memory model.
 */

#define INCL_DOSFILEMGR
#define INCL_WINWINDOWMGR
#define INCL_WINDIALOGS
#define INCL_DOSMISC
#include <os2.h>
#include <string.h>
#include <malloc.h>
#include "filedlg.h"

void main( void )
{
    HAB     hab = WinInitialize( 0 );
    HMQ     hmq = WinCreateMsgQueue( hab,DEFAULT_QUEUE_SIZE );
    USHORT  usPathLen;
    PSZ     pszFile,pszBuf;
    HFILE   hf;
    USHORT  usAction;
    USHORT  usResult;

    DosQSysInfo( 0,(PBYTE)&usPathLen,sizeof(USHORT) );
    pszFile = malloc( usPathLen );
    pszBuf  = malloc( usPathLen );

    usResult = FileSaveDlg( HWND_DESKTOP,
                            NULL,
                            NULL,
                            NULL,
                            "unnamed",
                            pszFile,
                            &hf,
                            0L,
                            &usAction,
                            FILE_NORMAL,
                            FILE_OPEN,
                            OPEN_ACCESS_READONLY|OPEN_SHARE_DENYWRITE,
                            0L );

    switch ( usResult ) {
        case FDLG_OK:
            strcpy( pszBuf,pszFile );
            break;

        case FDLG_CANCEL:
            strcpy( pszBuf,"Save file dialog box was cancelled." );
            break;

        default:
            strcpy( pszBuf,"Internal error in FileSaveDlg procedure." );
            break;
        }

    WinMessageBox( HWND_DESKTOP,HWND_DESKTOP,
                   pszBuf,"Debugging information",
                   0,MB_OK|MB_NOICON );

    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
}
