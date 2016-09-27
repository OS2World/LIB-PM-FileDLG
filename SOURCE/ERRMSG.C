/****************************************************************************
 * void ErrMessageBox( HWND hwndOwner,PSZ pszCaption,                       *
 *                     USHORT usErrorCode );                                *
 * Purpose          The ErrMessageBox function displays in a message        *
 *                  box the error message associated with the given         *
 *                  error code.                                             *
 *                                                                          *
 * Parameters       hwndOwner identifies the owner window of the            *
 *                  message-box window. The owner window is activated       *
 *                  when ErrMessageBox returns.                             *
 *                                                                          *
 *                  pszCaption points to the title of the message-box       *
 *                  window. If this parameter is NULL, "Error" (the         *
 *                  default title) is displayed. The maximum length         *
 *                  of the text is device-dependent. If the text is         *
 *                  too long, it will be clipped.                           *
 *                                                                          *
 *                  usErrorCode specifies the error value returned          *
 *                  by an MS OS/2 function.                                 *
 *                                                                          *
 * Notes            If a message-box window is created as part of the       *
 *                  processing of a dialog window, the dialog window        *
 *                  should be made the owner of the message-box             *
 *                  window.                                                 *
 *                                                                          *
 *                                                                          *
 * Modifications -                                                          *
 *      17-Aug-1989 : Initial version.                                      *
 ****************************************************************************/

    #define INCL_WINDIALOGS
    #define INCL_DOSERRORS
    #define INCL_DOSMISC
    #define INCL_DOSSEMAPHORES
    #include <os2.h>
    #include <stdio.h>
    #include <string.h>
    #include "tools.h"

/****************************************************************************
 * Static data                                                              *
 ****************************************************************************/
    typedef struct {
        USHORT  usCode;
        PSZ pszMsg;
        } ERRORMSG;

    static ERRORMSG errorMsg[] = {
        { 1,"Incorrect function." },
        { 2,"The system cannot find the file specified." },
        { 3,"The system cannot find the path specified." },
        { 4,"The system cannot open the file." },
        { 5,"The system will not allow access to the file specified." },
        { 6,"Incorrect internal file identifier." },
        { 7,"The storage control blocks were destroyed." },
        { 8,"Not enough memory." },
        { 9,"The memory control block address is invalid." },
        { 12,"The access code is invalid." },
        { 13,"The data is invalid." },
        { 15,"The system cannot find the drive specified." },
        { 18,"There are no more files." },
        { 19,"The diskette is write protected." },
        { 20,"The system cannot find the device specified." },
        { 21,"The drive is not ready." },
        { 22,"The device does not recognize the command." },
        { 23,"Data error (cyclic redundancy check)." },
        { 24,"The program issued a command but the command length is incorrect." },
        { 25,"The drive cannot locate a specific area or track on the disk." },
        { 26,"The disk or diskette cannot be accessed because it is not properly formatted for Microsoft Operating System/2." },
        { 27,"The drive cannot find the sector requested." },
        { 29,"The system cannot write to the specified device." },
        { 30,"The system cannot read from the specified device." },
        { 31,"A device attached to the system is not functioning." },
        { 32,"The process cannot access the file because it is being used by another process." },
        { 33,"The process cannot access the file because another process has locked a portion of the file." },
        { 34,"The wrong diskette is in the drive." },
        { 35,"Duplicate FCB found." },
        { 36,"The system has detected an overflow in the sharing buffer." },
        { 50,"The network request is not supported." },
        { 51,"The remote computer is not on line." },
        { 52,"A duplicate name is on the network." },
        { 53,"The network path was not found." },
        { 54,"The network is busy." },
        { 55,"The network device is no longer installed." },
        { 56,"The network BIOS command limit was reached." },
        { 57,"A network adapter hardware error occurred." },
        { 58,"The response from the network was incorrect." },
        { 59,"An unexpected network error occurred." },
        { 60,"The remote adapter is incompatible." },
        { 64,"The network name was deleted." },
        { 65,"Network access is denied." },
        { 66,"The network device type is incorrect." },
        { 67,"The network name cannot be found." },
        { 68,"The network name limit was exceeded." },
        { 69,"The network BIOS session limit was exceeded." },
        { 70,"The remote server is paused." },
        { 71,"The network request was not accepted." },
        { 72,"Print or disk redirection is paused." },
        { 80,"The file already exists." },
        { 81,"Duplicate FCB found." },
        { 82,"The directory or file cannot be created." },
        { 83,"Fail on INT 24." },
        { 84,"Insufficient storage is available to process the request." },
        { 85,"The network redirection already exists." },
        { 86,"The network password entered is incorrect." },
        { 87,"The parameter is incorrect." },
        { 88,"A write operation to a network file failed. Check the network error log for more information." },
        { 95,"System call interrupted." },
        { 107,"Diskette must be changed." },
        { 108,"The disk accessed is in use or locked by another process." },
        { 110,"The system cannot open the device or file specified." },
        { 112,"There is not enough space on the disk." },
        { 113,"No more search handles are available." },
        { 123,"A filename or volume label contains an incorrect character." },
        { 125,"The disk has no volume label." },
        { 130,"Invalid handle for direct disk access." },
        { 154,"Volume label is too long." },
        { 161,"Invalid path name." },
        { 163,"Disk medium is uncertain." },
        { 166,"The network has not been installed." },
        { 206,"The filename or extension is too long." },
        { 208,"The global filename characters, * or ? are entered incorrectly or too many global filename characters are specified." },
        { 240,"The network connection is disconnected." } };

    #define ERRORCNT  (sizeof(errorMsg) / sizeof(ERRORMSG))

    static PSZ pszClass[]   = { "","ERRCLASS_OUTRES","ERRCLASS_TEMPSIT",
                                "ERRCLASS_AUTH","ERRCLASS_INTRN",
                                "ERRCLASS_HRDFAIL","ERRCLASS_SYSFAIL",
                                "ERRCLASS_APPERR","ERRCLASS_NOTFND",
                                "ERRCLASS_BADFMT","ERRCLASS_LOCKED",
                                "ERRCLASS_MEDIA","ERRCLASS_ALREADY",
                                "ERRCLASS_UNK","ERRCLASS_CANT",
                                "ERRCLASS_TIME"
                              };
    static PSZ pszAction[]  = { "","ERRACT_RETRY","ERRACT_DLYRET",
                                "ERRACT_USER","ERRACT_ABORT","ERRACT_PANIC",
                                "ERRACT_IGNORE","ERRACT_INTRET"
                              };
    static PSZ pszLocus[]   = { "","ERRLOC_UNK","ERRLOC_DISK","ERRLOC_NET",
                                "ERRLOC_SERDEV","ERRLOC_MEM" };
    static CHAR szFormat[] = "Internal error\n    class = %s\n    action = %s\n    locus = %s\n    code = %u";
    static ULONG near ulSem = 0L;
/****************************************************************************/


/****************************************************************************/
    void ErrMessageBox( HWND hwndOwner,
                        PSZ pszCaption,
                        USHORT usErrorCode )
    {
        CHAR    szBuf[160];
        USHORT  usClass,flAction,usLocus;
        USHORT  flStyle;
        USHORT  usCnt;

        DosErrClass( usErrorCode,&usClass,&flAction,&usLocus );
        if ( usClass == ERRCLASS_ALREADY ) return;

        flStyle = (usErrorCode == ERROR_NOT_ENOUGH_MEMORY)
                    ? MB_OK|MB_ICONHAND
                    : MB_OK|MB_ICONEXCLAMATION;

        for ( usCnt = 0; usCnt < ERRORCNT; usCnt++ )
            if ( errorMsg[usCnt].usCode == usErrorCode )
                {
                strcpy( szBuf,errorMsg[usCnt].pszMsg );
                break;
                }

        if ( usCnt == ERRORCNT )
            {
            DosSemRequest( &ulSem,SEM_INDEFINITE_WAIT );
            sprintf( szBuf,szFormat,pszClass[usClass],
                     pszAction[flAction],pszLocus[usLocus],usErrorCode );
            DosSemClear( &ulSem );
            }

        WinMessageBox( HWND_DESKTOP,hwndOwner,
                       szBuf,pszCaption,
                       0,flStyle );
        return;
    }
/****************************************************************************/
