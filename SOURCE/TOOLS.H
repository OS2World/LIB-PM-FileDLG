/****************************************************************************
 * TOOLS.H                                                                  *
 *                                                                          *
 *  This header file contains the declarations of the common tool           *
 *  functions used internally by the open file and save file dialog         *
 *  box functions.                                                          *
 *                                                                          *
 *                                                                          *
 *  Modifications -                                                         *
 *      17-Aug-1989 : Initial version.                                      *
 *                                                                          *
 *  (c)Copyright 1989 Rick Yoder                                            *
 ****************************************************************************/

 extern USHORT ParseFileName( PSZ pszSource,PSZ pszDest,PSZ pszSearch );
    /* Purpose                  This function generates a fully
     *                          qualified file name or search spec.
     *                          from the input value, and changes
     *                          the current drive and directory to
     *                          the drive/directory listed in the
     *                          input file name.
     *
     * Parameters               pszSource points to the input file name
     *                          or search spec.
     *
     *                          pszDest points to the location where
     *                          the fully qualified file name or
     *                          search spec is to be stored.
     *
     *                          pszSearch points to the current file search
     *                          search specification. This string is used
     *                          when the input string is just a drive or
     *                          a drive:\directory without a file name.
     *
     * Return Value             The return value is zero if the function
     *                          is successful, otherwise it is an
     *                          error value. The contents of the buffer
     *                          pointed to by pszDest is undefined if
     *                          an error occurs.
     *
     * Notes                    This function modifies the contents of the
     *                          string pointed to by pszSource.
     *
     *                          This function will not detect an illegal
     *                          file name or search specification.
     */

 extern PSZ FitPathToBox( HWND hDlg,USHORT idText,PSZ pszPath );
    /* Purpose                  This function modifies the input
     *                          drive:\directory string so that it
     *                          fits within the bounds of the specified
     *                          static text control.
     *
     * Parameters               hDlg is a handle to the dialog box.
     *
     *                          idText is the static text control id.
     *
     *                          pszPath is a pointer to the drive:\directory
     *                          string that is to be modified.
     *
     * Return Value             The function returns a pointer to the
     *                          modified path string.
     */

 extern void ErrMessageBox( HWND hwndOwner,PSZ pszCaption,
                            USHORT usErrorCode );
    /* Purpose          The ErrMessageBox function displays in a message
     *                  box the error message associated with the given
     *                  error code.
     *
     * Parameters       hwndOwner identifies the owner window of the
     *                  message-box window. The owner window is activated
     *                  when ErrMessageBox returns.
     *
     *                  pszCaption points to the title of the message-box
     *                  window. If this parameter is NULL, "Error" (the
     *                  default title) is displayed. The maximum length
     *                  of the text is device-dependent. If the text is
     *                  too long, it will be clipped.
     *
     *                  usErrorCode specifies the error value returned
     *                  by an MS OS/2 function.
     *
     * Notes            If a message-box window is created as part of the
     *                  processing of a dialog window, the dialog window
     *                  should be made the owner of the message-box
     *                  window.
     */
/****************************************************************************/
