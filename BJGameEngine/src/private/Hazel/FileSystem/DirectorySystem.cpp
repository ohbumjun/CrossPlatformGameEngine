﻿#include "hzpch.h"
#include "Hazel/FileSystem/DirectorySystem.h"
#include "Hazel/Utils/StringUtil.h"

namespace Hazel
{

char a_executablePath[2048] = {
    0,
};
std::string s_executableDirectoryPath;


const char *DirectorySystem::PrintError(int error_code)
{
    switch (error_code)
    {
    case EPERM:
        return "Operation not permitted";
    case ENOENT:
        return "No such file or directory";
    // Insufficient Info for file access ? 해당 파일 경로가 존재 x 등등
    case ESRCH:
        return "No such process || Insufficient Info for file access";
    case EINTR:
        return "Interrupted system call";
    case EIO:
        return "Error I/O";
    case ENXIO:
        return "No such device or address";
    case E2BIG:
        return "Argument list too long";
    case ENOEXEC:
        return "Exec format error";
    case EBADF:
        return "Bad file number";
    case ECHILD:
        return "No child processes";
    case EAGAIN:
        return "Try again";
    case ENOMEM:
        return "Out of memory";
    case EACCES:
        return "Access Permission denied";
    case EFAULT:
        return "Bad address";
    case EBUSY:
        return "Block device required";
    case EEXIST:
        return "File exists";
    case EXDEV:
        return "Cross-device link";
    case ENODEV:
        return "No such device";
    case ENOTDIR:
        return "Not a directory";
    case EISDIR:
        return "Is a directory";
    case EINVAL:
        return "Invalid argument";
    case ENFILE:
        return "File table overflow";
    case EMFILE:
        return "Too many open files";
    case ENOTTY:
        return "Not a typewriter";
    case ETXTBSY:
        return "Text file busy";
    case EFBIG:
        return "File too large";
    case ENOSPC:
        return "No space left on device";
    case ESPIPE:
        return "Illegal seek";
    case EROFS:
        return "Read-only file system";
    case EMLINK:
        return "Too many links";
    case EPIPE:
        return "Broken pipe, The process cannot access the file because it is "
               "being used by another process.";
    case EDOM:
        return "Math argument out of domain of func";
    case ERANGE:
        return "Math result not representable";
    case EDEADLK:
        return "Resource deadlock would occur";
    case ENAMETOOLONG:
        return "File name too long";
    case ENOLCK:
        return "No record locks available";
    case ENOSYS:
        return "Function not implemented";
    case ENOTEMPTY:
        return "Directory not empty";
    case ERROR_INVALID_PARAMETER:
        return "The parameter is incorrect";
    }
    return "Unknown error, you should check "
           "https://man7.org/linux/man-pages/man3/errno.3.html page";
};

bool DirectorySystem::SeekFile(HANDLE file, int64 offset, FilePosMode mode)
{
    LARGE_INTEGER largeOffset;

    //QuadPart : 해당 byte 만큼 position 을 움직여라
    largeOffset.QuadPart = offset;

    DWORD method = 0;

    switch (mode)
    {
    case FilePosMode::BEGIN:
        method = FILE_BEGIN;
        break;
    case FilePosMode::CURRENT:
        method = FILE_CURRENT;
        break;
    case FilePosMode::END:
        method = FILE_END;
        break;
    }

    return SetFilePointerEx(file, largeOffset, nullptr, method);
}

bool DirectorySystem::GetFilePos(HANDLE file, int64 &pos)
{
    // 인자 pos : 현재 파일에서의 pos 정보 저장

    LARGE_INTEGER largeOffset;

    // QuadPart 가 0 이라는 것은, 사실상 움직이지 말라는 의미이다.
    largeOffset.QuadPart = 0;

    LARGE_INTEGER newOffset;

    // 실제 file pointer 를 변경하는 것 없이 현재 위치를 얻어오고 싶어하는 것 뿐이다.
    if (SetFilePointerEx(file, largeOffset, &newOffset, FILE_CURRENT))
    {
        pos = newOffset.QuadPart;
        return true;
    }

    return false;
}

bool DirectorySystem::SetFilePos(HANDLE file, const int64 pos)
{
    LARGE_INTEGER largeOffset;
    largeOffset.QuadPart = pos;

    return SetFilePointerEx(file, largeOffset, nullptr, FILE_BEGIN);
}

HANDLE DirectorySystem::CreateFiile(const char *dest)
{
    char conv[INIT_STR_LENGTH] = {0};
    ToWinSystemPath(conv, dest);

    // WBC  로 변환한다.
    wchar_t wc[INIT_STR_LENGTH];
    FromUTF8ToSystemPath((void *)wc, conv, true);

    HANDLE newFileHandle = CreateFileW(wc,
                                       GENERIC_READ | GENERIC_WRITE,
                                       0,
                                       nullptr,
                                       CREATE_NEW,
                                       FILE_ATTRIBUTE_NORMAL,
                                       nullptr);

    if (newFileHandle == INVALID_HANDLE)
    {
        const int error_code = GetLastError();

        THROW("lv_file_create Code = %i : %s (%s)",
              error_code,
              DirectorySystem::PrintError(error_code),
              dest);
    }

    return newFileHandle;
}

HANDLE DirectorySystem::OpenFile(const char *path,
                                 FileAccessMode accessFlag,
                                 FileOpenMode modeFlag)
{
    char conv[INIT_STR_LENGTH] = {0};
    ToWinSystemPath(conv, path);

    wchar_t wc[INIT_STR_LENGTH];
    FromUTF8ToSystemPath((void *)wc, conv, true);

    DWORD desiredAccess = 0;
    DWORD shareMode = 0;
    DWORD disp = 0;
    constexpr DWORD attrs = FILE_ATTRIBUTE_NORMAL;

    switch (accessFlag)
    {
    case FileAccessMode::READ_ONLY:
    {
        desiredAccess = GENERIC_READ;
        shareMode = FILE_SHARE_READ; // 다른 프로세스도 읽기 가능
    }
    break;
    case FileAccessMode::WRITE_ONLY:
    {
        desiredAccess = GENERIC_WRITE;
        shareMode = 0; // 어떠한 조작도 불가능
    }
    break;
    case FileAccessMode::READ_WRITE:
    {
        desiredAccess = GENERIC_READ | GENERIC_WRITE;
        shareMode = 0; // 어떠한 조작도 불가능
    }
    break;
    }

    switch (modeFlag)
    {
    case FileOpenMode::OPEN_CREATE:
        disp = OPEN_ALWAYS;
        break;
    case FileOpenMode::OPEN:
        disp = OPEN_EXISTING;
        break;
    case FileOpenMode::CREATE:
        disp = CREATE_ALWAYS;
        break;
    case FileOpenMode::APPEND:
        desiredAccess |= FILE_APPEND_DATA;
        disp = OPEN_ALWAYS;
        break;
    case FileOpenMode::NEW:
        disp = CREATE_NEW;
        break;
    }

    HANDLE fileHandle;

    fileHandle = CreateFileW(
        wc,
        desiredAccess, // 파일 또는 디바이스에 대한 요청된 액세스 권한으로, 읽기, 쓰기, 둘 다 또는 0
        shareMode, // 열려 있는 각 핸들에 대한 공유 옵션. 즉, 해당 파일이 지금 이미 열려 있는데
        // 그 때 다른 프로세스가 해당 파일에 대해 어떤 작업까지 허용할 것인가.
        NULL,
        disp, // 존재하거나 존재하지 않는 파일 또는 디바이스에 대해 수행할 작업
        attrs,
        NULL);

    if (fileHandle == INVALID_HANDLE)
    {
        bool h = true;
    }


    if (fileHandle != INVALID_HANDLE)
    {
        return fileHandle;
    }

    const DWORD errorCode = GetLastError();

    // 다른 스레드(프로세스)에서 쓰고 있는 파일의 경우. 풀릴 때까지 대기
    if (errorCode == ERROR_SHARING_VIOLATION)
    {
        // 최대 대기 시간
        size_t totalWaitTime = 0;

        while ((fileHandle = CreateFileW(wc,
                                         desiredAccess,
                                         shareMode,
                                         nullptr,
                                         disp,
                                         attrs,
                                         nullptr)) == INVALID_HANDLE)
        {
            constexpr size_t maxWaitTime = 20000;

            size_t delay = 10;

            Sleep((DWORD)delay);

            totalWaitTime += delay;

            if (delay < 3000)
            {
                delay <<= 1;
            }

            if (totalWaitTime > maxWaitTime)
            {
                break;
            }
        }

        if (fileHandle != INVALID_HANDLE)
        {
            return fileHandle;
        }
    }

    Hazel::Log::GetCoreLogger()->warn(DirectorySystem::PrintError(errorCode));

    // 여기에 걸리면 정상적으로  file 조작을 실패한 것이다.
    assert(false);

    return fileHandle;
}

bool DirectorySystem::CloseFile(HANDLE handle)
{
    return CloseHandle(handle);
}

int64 DirectorySystem::ReadFromFile(HANDLE file, void *buffer, uint64 nbytes)
{
    DWORD readBytes = 0;

    // buffer 쪽에 file 혹은 하드웨어로부터 읽은 데이터를 넣어준다.
    return ReadFile(file,
                    buffer,
                    static_cast<DWORD>(nbytes),
                    &readBytes,
                    nullptr)
               ? static_cast<int64>(readBytes)
               : -1;
}

int64 DirectorySystem::WriteToFile(HANDLE file,
                                   const void *buffer,
                                   uint64 nbytes)
{
    DWORD writeBytes = 0;

    // buffer 포인터 시작점으로부터 nbytes 만큼의 데이터를 실제 file 에 쓰는 것이다.
    // 리턴값은 실제 파일에 쓴 바이트 숫자이다.
    return WriteFile(file,
                     buffer,
                     static_cast<DWORD>(nbytes),
                     &writeBytes,
                     nullptr)
               ? static_cast<int64>(writeBytes)
               : -1;
}

bool DirectorySystem::FlushFile(HANDLE file)
{
    // 실제 file.nativeHandle 에 대응되는 파일로 쓰여진 data 를, 실제 하드웨어에 쓰는 것은
    // Ensure 하는 역할
    // 즉, 실제 파일에 쓰는 것은 WriteFile 로 진행하고
    // WriteFile 을 호출한 이후, 아래 함수를 호출하여
    // Ensure 를 하는 것이다.
    return FlushFileBuffers(file);
}

bool DirectorySystem::IsFileEOF(HANDLE fileHandle)
{
    LARGE_INTEGER zero, pos, size;
    zero.QuadPart = 0;

    // pos 라는 변수에 실제 현재 File Pos 를 얻어온다.
    if (!SetFilePointerEx(fileHandle, zero, &pos, FILE_CURRENT))
    {
        THROW("Error!");
    }

    // File 의 Size 를 얻어오는 Windows 함수
    if (!GetFileSizeEx(fileHandle, &size))
    {
        THROW("Error!");
    }

    return (pos.QuadPart >= size.QuadPart);
}

std::string DirectorySystem::CombinePath(const char *a, const char *b)
{
    int32 aLen = a ? static_cast<int32>(Utils::pr_strlen(a)) : -1;
    int32 bLen = b ? static_cast<int32>(Utils::pr_strlen(b)) : -1;
    const char *pa = aLen == -1 ? nullptr : a + aLen - 1;
    const char *pb = bLen == -1 ? nullptr : b + bLen - 1;

    while (aLen >= 0 && *pa == DIRECTORY_SEPARATOR_CHAR)
    {
        --pa;
        --aLen;
    }

    while (bLen >= 0 && *pb == DIRECTORY_SEPARATOR_CHAR)
    {
        --pb;
        --bLen;
    }

    int32 newCapacity =
        aLen + bLen + 2; // 1 : DIRECTORY_SEPARATOR_CHAR, 2 : null limiter

    std::string s;
    s.reserve(newCapacity);
    if (aLen >= 0)
    {
        s.append(a, 0, aLen);
        s += DIRECTORY_SEPARATOR_CHAR;
    }

    if (bLen >= 0)
    {
        s.append(b, 0, bLen);
    }

    Utils::StringUtil::ReplaceStr(s, '\\\\', '\\');

    return s;
}

bool DirectorySystem::CopyFilePath(const char *dst, const char *src)
{// 	return lv_file_copy(lv_path_normalize(dst).c_str(), lv_path_normalize(src).c_str());
    const char ds[2] = {DIRECTORY_SEPARATOR_CHAR, 0};

    char cdst[CHAR_INIT_LENGTH] = {0};
    char csrc[CHAR_INIT_LENGTH] = {0};

    Utils::StringUtil::ReplaceStrOpt(cdst, dst, "/", ds);
    Utils::StringUtil::ReplaceStrOpt(csrc, src, "/", ds);

    wchar_t wcdst[CHAR_INIT_LENGTH] = {0};
    wchar_t wcsrc[CHAR_INIT_LENGTH] = {0};

    FromUTF8ToSystemPath(wcdst, cdst, true);
    FromUTF8ToSystemPath(wcsrc, csrc, true);

    FILE *in, *out;
    char buf[4096];
    size_t len;

    if (strcmp((char *)csrc, (char *)cdst) == 0)
        return false;

    if ((in = _wfopen(wcsrc, L"rb")) == NULL)
    {
        const int error_code = GetLastError();
        printf("lv_file_create Code = %i : %s\n",
               error_code,
               PrintError(error_code));
        return false;
    }

    if ((out = _wfopen(wcdst, L"wb")) == NULL)
    {
        const int error_code = GetLastError();
        printf("lv_file_create Code = %i : %s\n",
               error_code,
               PrintError(error_code));

        fclose(in);
        return false;
    }

    while ((len = fread(buf, sizeof(char), sizeof(buf), in)) != 0)
    {
        if (fwrite(buf, sizeof(char), len, out) == 0)
        {
            fclose(in);
            fclose(out);
            free(buf);
            remove((char *)cdst);
            //_unlink(cdst); // 에러난 파일 지우고 종료
            return false;
        }
    }

    SetFileAttributes(wcdst, GetFileAttributes(wcsrc));

    fclose(in);
    fclose(out);

    return true;
}

int DirectorySystem::DeleteFilePath(const char *dst)
{
    wchar_t wcdst[CHAR_INIT_LENGTH] = {0};
    FromUTF8ToSystemPath(wcdst, dst, true);
    return _wremove(wcdst);
}

bool DirectorySystem::ExistFilePath(const char *path)
{
    if (strlen(path) >= CHAR_INIT_LENGTH)
        THROW("Template stack char buffer is not enough");

    wchar_t wc[CHAR_INIT_LENGTH];
    FromUTF8ToSystemPath((void *)wc, path, true);
    DWORD dwAttrib = GetFileAttributes(wc);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES) &&
           !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

bool DirectorySystem::CreateDirectoryPath(const char *path)
{
    const wchar_t ds[2] = {DIRECTORY_SEPARATOR_CHAR, 0};
    wchar_t wc[CHAR_INIT_LENGTH];

    FromUTF8ToSystemPath((void *)wc, path, true);

    wchar_t *ptr = Utils::pr_strtok(wc, ds);
    wchar_t acc[CHAR_INIT_LENGTH] = {0};

    while (ptr != NULL)
    {
        Utils::pr_strcat(acc, ptr);
        Utils::pr_strcat(acc, ds);

        DWORD dwAttrib = GetFileAttributes(acc);
        bool existsDir = (dwAttrib != INVALID_FILE_ATTRIBUTES &&
                          (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

        if (!existsDir)
        {
            if (CreateDirectoryW((LPCWSTR)acc, NULL) == false)
            {
                return false;
            }
        }
        ptr = Utils::pr_strtok<wchar_t>(NULL, ds);
    }

    return true;
}

bool DirectorySystem::ExistDirectoryPath(const char *path)
{
    if (strlen(path) >= CHAR_INIT_LENGTH)
        THROW("Template stack char buffer is not enough");

    wchar_t wc[CHAR_INIT_LENGTH] = {
        0,
    };

    FromUTF8ToSystemPath((void *)wc, path, true);

    DWORD dwAttrib = GetFileAttributes(wc);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool DirectorySystem::CopyDirectoryPath(const char *dst, const char *src)
{
    wchar_t wc[CHAR_INIT_LENGTH];
    FromUTF8ToSystemPath((void *)wc, src, true);

    std::wstring new_sf = wc;
    new_sf += L"\\*";

    WCHAR sf[MAX_PATH + 1];
    WCHAR tf[MAX_PATH + 1];

    FromUTF8ToSystemPath((void *)wc, dst, true);

    wcscpy_s(sf, MAX_PATH, new_sf.c_str());
    wcscpy_s(tf, MAX_PATH, wc);

    sf[lstrlenW(sf) + 1] = 0;
    tf[lstrlenW(tf) + 1] = 0;

    SHFILEOPSTRUCTW s = {0};
    s.wFunc = FO_COPY;
    s.pTo = tf;
    s.pFrom = sf;
    s.fFlags = FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION |
               FOF_NOERRORUI | FOF_NO_UI;
    const int res = SHFileOperationW(&s);

    return res == 0;
}

bool DirectorySystem::RemoveDirectoryPath(const char *inputPath)
{
    if (strlen(inputPath) >= CHAR_INIT_LENGTH)
        THROW("Template stack char buffer is not enough");

    const char ds[2] = {DIRECTORY_SEPARATOR_CHAR, 0};
    wchar_t wc[CHAR_INIT_LENGTH];

    FromUTF8ToSystemPath((void *)wc, inputPath, true);

    // LvFixedWString<CHAR_INIT_LENGTH> path(wc);
    std::wstring path(wc);

    // path.append(L'\0');
    // path.append(L'\0');
    path += L"\0";
    path += L"\0";

    SHFILEOPSTRUCTW fileOperation;
    fileOperation.wFunc = FO_DELETE;
    // fileOperation.pFrom = path.GetCharArray();

    fileOperation.pFrom = path.c_str();
    fileOperation.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION;

    const int result = ::SHFileOperationW(&fileOperation);
    if (result != 0)
        return false;

    return true;
}

std::string DirectorySystem::GetPathParent(const char *path)
{
    std::string result("");
    const char ds[2] = {DIRECTORY_SEPARATOR_CHAR, 0};
    char conv[CHAR_INIT_LENGTH] = {0};

    ToWinSystemPath(conv, path);

    const char *ptr = Utils::pr_strrstr(conv, ds);

    //strcpy(conv, (const char*)(ptr - conv));

    if (ptr ==
        nullptr) //@yuiena : parent folder 가 없는 path를 넣으면 에러가 발생. 그러므로 해당 코드 추가.
    {
        return result;
    }
    else
    {
        result.append(conv, 0, static_cast<int>(ptr - conv));
        return result;
    }
}

// 순소 폴더 경로 리턴
std::string DirectorySystem::GetPathDirectory(const char* path)
{
    return std::string();
}

// 확장자 리턴
std::string DirectorySystem::GetExtenstion(const char* path)
{
    std::string filename(path);

    // Find the position of the last dot (.)
    size_t pos = filename.rfind('.');

    // If no dot is found, return an empty string
    if (pos == std::string::npos)
    {
        return "";
    }

    // Extract the extension (everything after the last dot)
    return filename.substr(pos + 1);
}

std::string DirectorySystem::GetPathame(const char *path)
{
    std::string result;
    if (nullptr == path)
    {
        throw("path is nullptr.");
        return result;
    }

    size_t len = strlen(path);

    if (len == 0)
    {
        return path;
    }

    const char *p = &path[len - 1];
    const char *s = path;

    while (p != s)
    {
        if (*p == '\\' || *p == '/')
        {
            result.append(p + 1, 0, static_cast<int>(len - (p + 1 - s)));
            return result;
        }

        if (*p == ':')
        {
            return result;
        }
        p--;
    }

    result.append(path);
    return result;
}

std::string DirectorySystem::ToWinSystemPath(const std::string &str)
{
    std::string returnStr = str;
    Utils::StringUtil::ReplaceStr(returnStr, '/', '\\');
    return returnStr;
}

void DirectorySystem::ToWinSystemPath(char *dest, const char *src)
{
    const char sper[2] = {DIRECTORY_SEPARATOR_CHAR, 0};
    Utils::StringUtil::ReplaceStrToDst(dest, src, "/", sper);
}

void DirectorySystem::FromSystemToUTF8Path(char *dest_utf8, const void *src_sys)
{
    // Casting the source system path to LPCWSTR (Wide character string)
    const LPCWSTR inStr = static_cast<const wchar_t *>(src_sys);

    wchar_t buf[INIT_STR_LENGTH] = {
        0,
    };

    /*
	Normalizing the input string to a specified form (NormalizationC) and storing it in 'buf'

	역할 :  Windows programming to convert a string to a specified "Unicode normalization form"
	
	NormalizationC: Presumably a constant or enumeration specifying the normalization form.
	
	inStr : The input system path to be normalized.

	- 1 : Indicates to process the entire string until the null terminator.

	buf : Buffer to store the normalized string.

	INIT_STR_LENGTH : The length of the buffer.

	즉, 해당 함수 호출 결과
	normalized wide character 가 된다.
	*/
    NormalizeString(NormalizationC, inStr, -1, buf, INIT_STR_LENGTH);

    /*
	normalized wide character 을 UTF8 형태로 바꿔야 한다.
	이 과정에서 필요한 byte 개수를 계산한다.
	*/
    const int size = WideCharToMultiByte(CP_UTF8,
                                         WC_NO_BEST_FIT_CHARS,
                                         buf,
                                         -1,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);

    if (size > 0)
    {
        // 실제 UTF-8 코드 형태로 변환해준다.
        WideCharToMultiByte(CP_UTF8,
                            WC_NO_BEST_FIT_CHARS,
                            buf,
                            -1,
                            dest_utf8,
                            size,
                            NULL,
                            NULL);
    }
}

void DirectorySystem::FromUTF8ToSystemPath(void *dest_sys,
                                           const char *src_utf8,
                                           bool need_safe_conversion)
{
    char src[INIT_STR_LENGTH] = {
        0,
    };
    const LPWSTR dest = static_cast<wchar_t *>(dest_sys);

    ToWinSystemPath(src, src_utf8);

    const int size = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);

    if (size > 0)
    {
        // MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, -1, dest, size + pr_path_utf8_extra_length(src_utf8));
        MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            src,
            -1,
            dest,
            size + Utils::StringUtil::CalculateUTF8ExtraLength(src_utf8));
    }

    // Check for invalid characters in the converted wide character string
    if (need_safe_conversion)
    {
        bool isANSI = false;
        const size_t destLen = wcslen(dest);
        for (size_t i = 0; i < destLen; ++i)
        {
            WCHAR w = dest[i];
            if (w ==
                0xFFFD) // 0xFFFD represents a Unicode replacement character (�) indicating an invalid character
            {
                isANSI = true;
                break;
            }
        }

        // If invalid characters are found, consider converting using the default system code page (ACP)
        if (isANSI)
        {
            const int size = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);

            if (size > 0)
            {
                // Convert the source string using the default system code page and store it in 'dest'
                MultiByteToWideChar(
                    CP_ACP,
                    MB_ERR_INVALID_CHARS,
                    src,
                    -1,
                    dest,
                    size +
                        Utils::StringUtil::CalculateUTF8ExtraLength(src_utf8));
            }
        }
    }
}
} // namespace Hazel
