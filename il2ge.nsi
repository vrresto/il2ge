!macro BackupFile FILE BACKUP_DIR
  ClearErrors
  IfFileExists "$INSTDIR\${FILE}" 0 +2
    Rename "$INSTDIR\${FILE}" "${BACKUP_DIR}\${FILE}"
  IfErrors 0 +3
    DetailPrint "Failed to backup file: ${FILE}"
    Abort
!macroend

!define SRC_DIR "$%SRC_DIR%"
!define DEST_DIR "$%DEST_DIR%"
!define BUILD "$%BUILD%"

SetCompressor /SOLID /FINAL lzma

!include "FileFunc.nsh"
!insertmacro GetTime

!include "MUI2.nsh"

#!define MUI_ABORTWARNING # This will warn the user if they exit from the installer.
; !define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_DIRECTORYPAGE_TEXT_TOP "Select the folder of your IL-2 installation."
; !define MUI_PAGE_HEADER_TEXT

; !insertmacro MUI_PAGE_WELCOME # Welcome to the installer page.
; !insertmacro MUI_PAGE_LICENSE "LICENSE"
; !define MUI_PAGE_HEADER_TEXT
!insertmacro MUI_PAGE_DIRECTORY # In which folder install page.
!insertmacro MUI_PAGE_INSTFILES # Installing page.
; !insertmacro MUI_PAGE_FINISH # Finished installation page.
!insertmacro MUI_LANGUAGE "English"

Name "IL-2 Graphics Extender (build ${BUILD})" # Name of the installer (usually the name of the application to install).
OutFile "${DEST_DIR}/il2ge-installer.exe" # Name of the installer's file.
; InstallDir "" # Default installing folder ($PROGRAMFILES is Program Files folder).
ShowInstDetails show # This will always show the installation details.


Section "il2ge" # In this section add your files or your folders.

  ${GetTime} "" "l" $0 $1 $2 $3 $4 $5 $6
  !define TIMESTAMP "$2.$1.$0-$4.$5.$6"

  !define BACKUP_DIR "$INSTDIR\il2ge-installer-backup\${TIMESTAMP}"

  CreateDirectory ${BACKUP_DIR}

  IfErrors 0 create_backup_dir_success
  DetailPrint "Failed to create folder: ${BACKUP_DIR}"
  Abort

create_backup_dir_success:
  !insertmacro BackupFile "il2ge.dll" "${BACKUP_DIR}"
  !insertmacro BackupFile "il2ge" "${BACKUP_DIR}"

  SetOutPath "$INSTDIR\il2ge"
  File /r "${SRC_DIR}/il2ge/*"

SectionEnd
