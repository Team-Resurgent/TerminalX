#pragma once

#include "Integers.h"
#include <xtl.h>

#define FILE_SYNCHRONOUS_IO_NONALERT 0x00000020
#define FILE_NON_DIRECTORY_FILE 0x00000040
#define OBJ_CASE_INSENSITIVE 0x00000040L

typedef LONG NTSTATUS;

typedef struct STRING {
	WORD Length;
	WORD MaximumLength;
	PSTR Buffer;
} STRING;

typedef struct IO_STATUS_BLOCK
{
	NTSTATUS Status;
	ULONG Information;
} IO_STATUS_BLOCK;

typedef struct OBJECT_ATTRIBUTES
{
	HANDLE RootDirectory;
	STRING* ObjectName;
	ULONG Attributes;
} OBJECT_ATTRIBUTES;

typedef struct FILE_NETWORK_OPEN_INFORMATION {
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER AllocationSize;
	LARGE_INTEGER EndOfFile;
	ULONG FileAttributes;
} FILE_NETWORK_OPEN_INFORMATION;

typedef enum FILE_INFORMATION_CLASS
{
	FileDirectoryInformation = 1,
	FileFullDirectoryInformation,
	FileBothDirectoryInformation,
	FileBasicInformation,
	FileStandardInformation,
	FileInternalInformation,
	FileEaInformation,
	FileAccessInformation,
	FileNameInformation,
	FileRenameInformation,
	FileLinkInformation,
	FileNamesInformation,
	FileDispositionInformation,
	FilePositionInformation,
	FileFullEaInformation,
	FileModeInformation,
	FileAlignmentInformation,
	FileAllInformation,
	FileAllocationInformation,
	FileEndOfFileInformation,
	FileAlternateNameInformation,
	FileStreamInformation,
	FilePipeInformation,
	FilePipeLocalInformation,
	FilePipeRemoteInformation,
	FileMailslotQueryInformation,
	FileMailslotSetInformation,
	FileCompressionInformation,
	FileCopyOnWriteInformation,
	FileCompletionInformation,
	FileMoveClusterInformation,
	FileQuotaInformation,
	FileReparsePointInformation,
	FileNetworkOpenInformation,
	FileObjectIdInformation,
	FileTrackingInformation,
	FileOleDirectoryInformation,
	FileContentIndexInformation,
	FileInheritContentIndexInformation,
	FileOleInformation,
	FileMaximumInformation
} FILE_INFORMATION_CLASS;

extern "C" {

    NTSTATUS WINAPI NtClose(HANDLE Handle);
	NTSTATUS WINAPI NtOpenFile(
		OUT PHANDLE pHandle,
		IN ACCESS_MASK DesiredAccess,
		IN OBJECT_ATTRIBUTES* ObjectAttributes,
		OUT IO_STATUS_BLOCK* IOStatusBlock,
		IN ULONG ShareAccess,
		IN ULONG OpenOptions
	);
    NTSTATUS WINAPI NtQueryInformationFile(
		HANDLE FileHandle,
		IO_STATUS_BLOCK* IoStatusBlock,
		PVOID FileInformation,
		ULONG FileInformationLength,
		FILE_INFORMATION_CLASS FileInformationClass
	);
    NTSTATUS WINAPI NtReadFile(
		HANDLE Handle,
		HANDLE Event,
		PVOID pApcRoutine,
		PVOID pApcContext,
		PVOID pIoStatusBlock,
		PVOID pBuffer,
		ULONG Length,
		PLARGE_INTEGER pByteOffset
	);

    extern STRING* XeImageFileName;
    VOID WINAPI HalReturnToFirmware(unsigned int value);
    NTSTATUS WINAPI HalWriteSMBusValue(UCHAR devddress, UCHAR offset, UCHAR writedw, DWORD data);
    NTSTATUS WINAPI HalReadSMBusValue(UCHAR devddress, UCHAR offset, UCHAR readdw, DWORD* pdata);

}

#define HalReadSMBusByte(SlaveAddress, CommandCode, DataValue)                                                         \
    HalReadSMBusValue(SlaveAddress, CommandCode, FALSE, DataValue)
#define HalWriteSMBusByte(SlaveAddress, CommandCode, DataValue)                                                        \
    HalWriteSMBusValue(SlaveAddress, CommandCode, FALSE, DataValue)