/**
 *    IL-2 Graphics Extender
 *    Copyright (C) 2018 Jan Lepper
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "iat.h"

#include <stdio.h>
#include <assert.h>
#include <windows.h>

#ifndef _stricmp
#define _stricmp strcasecmp
#endif

static IMAGE_THUNK_DATA32 *findThunk(const void *function, const char *import_name, HMODULE module)
{
  char *base_addr = (char*)module;
  PIMAGE_DOS_HEADER dos_headers = (PIMAGE_DOS_HEADER)base_addr;

//   printf("base addr: %x\n", (unsigned)base_addr);

  if (dos_headers->e_magic != IMAGE_DOS_SIGNATURE) {
    printf("Error: e_magic is no valid DOS signature\n");
    exit(1);
  }

  PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)(base_addr + dos_headers->e_lfanew);

//   printf("nt_headers: %x\n", (unsigned)nt_headers);

  DWORD pe_signature = nt_headers->Signature;
  char pe_signature_str[4];
  assert(sizeof(DWORD) == sizeof(pe_signature_str));
  memcpy(&pe_signature_str, &pe_signature, sizeof(pe_signature_str));
  assert(pe_signature_str[0] == 'P');
  assert(pe_signature_str[1] == 'E');
  assert(pe_signature_str[2] == '\0');
  assert(pe_signature_str[3] == '\0');

//   printf("size of optional header: %d\n", (int)nt_headers->FileHeader.SizeOfOptionalHeader);

//   printf("magic: %x\n", (unsigned)nt_headers->OptionalHeader.Magic);
//   printf("import directory virtual addr: %x\n", (unsigned)nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

  PIMAGE_IMPORT_DESCRIPTOR import_descriptor = (PIMAGE_IMPORT_DESCRIPTOR)
    (base_addr + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

//   printf("import directory: %x\n", (unsigned)import_descriptor);

//   BOOL found = FALSE;
  while (import_descriptor->Name) {
    const char *importName = base_addr + import_descriptor->Name;
//     printf("importName: %s\n", importName);
    if (_stricmp (importName, import_name) == 0) {
      PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)(base_addr + import_descriptor->FirstThunk);
      while (thunk->u1.Function) {
//         printf("function: %x\n", (unsigned)thunk->u1.Function);
        if (thunk->u1.Function == (DWORD)function) {
//           printf("got it!\n");
          return thunk;
        }
        thunk++;
      }

    }
    import_descriptor++;
  }
//   if (!found)
//     return FALSE;

  return 0;
}

void patchIAT(const char *function_name, const char *import_name, void *new_function, void **orig_func_out, HMODULE module)
{
  HANDLE hImportModule = GetModuleHandle(import_name);
  assert(hImportModule);

  void *orig_func = GetProcAddress(hImportModule, function_name);
  assert(orig_func);


  if (orig_func_out)
    *orig_func_out = orig_func;

  IMAGE_THUNK_DATA32 *thunk = findThunk(orig_func, import_name, module);
  if (!thunk)
  {
    return;
  }
  assert(thunk);

  DWORD oldrights, newrights = PAGE_READWRITE;
  //Update the protection to READWRITE
  VirtualProtect(thunk, sizeof(LPVOID), newrights, &oldrights);

  thunk->u1.Function = (DWORD)new_function;

  //Restore the old memory protection flags.
  VirtualProtect(thunk, sizeof(LPVOID), oldrights, &newrights);
}
