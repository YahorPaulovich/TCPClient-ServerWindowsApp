// C++ program to illustrate command line arguments
#include <iostream>
#include <windows.h>
#include <lmcons.h>
#include <atlbase.h>
#include <atlsecurity.h>
using namespace std;

// The function returns true if the user who is running the
// application is a member of the Administrators group,
// which does not necessarily mean the process has admin privileges.
bool IsAdministrator(HRESULT& rHr)
{
    bool bIsAdmin = false;

    try
    {
        // Open the access token of the current process.
        ATL::CAccessToken aToken;
        if (!aToken.GetProcessToken(TOKEN_QUERY))
        {
            throw MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32,
                ::GetLastError());
        }


        // Query for the access token's group information.
        ATL::CTokenGroups groups;
        if (!aToken.GetGroups(&groups))
        {
            throw MAKE_SCODE(SEVERITY_ERROR, FACILITY_WIN32,
                ::GetLastError());
        }

        // Iterate through the access token's groups
        // looking for a match against the builtin admins group.
        ATL::CSid::CSidArray groupSids;
        ATL::CAtlArray<DWORD> groupAttribs;
        groups.GetSidsAndAttributes(&groupSids, &groupAttribs);
        for (UINT i = 0; !bIsAdmin && i < groupSids.GetCount(); ++i)
        {
            bIsAdmin = groupSids.GetAt(i) == ATL::Sids::Admins();
        }
        rHr = S_OK;
    }
    catch (HRESULT hr)
    {
        rHr = hr;
    }

    return bIsAdmin;
}

BOOL IsRunAsAdministrator()
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdministratorsGroup))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:

    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = NULL;
    }

    if (ERROR_SUCCESS != dwError)
    {
        throw dwError;
    }

    return fIsRunAsAdmin;
}



int main(int argc, char* argv[])
{
    /*cout << "Program name is: " << argv[0] << endl;*/
    WCHAR path[500];
    DWORD size = GetModuleFileNameW(NULL, path, 500);

    //wcout << path << endl;
    wcout << "The current path to the program file: \n" << path << endl;
    //std::wcout << L"INSERT MULTIBYTE WCHAR* HERE" << std::endl;

    TCHAR username[UNLEN + 1];
    DWORD size2 = UNLEN + 1;
    GetUserName((TCHAR*)username, &size2);
    wcout << "The current user name: " << username << endl;

    HRESULT hr;
    if (IsAdministrator(hr))
    {
        cout << "You are an administrator, you will have access to all functions!" << endl;
    }
    else
    {
        cout << "You are not an administrator, some features won't be accessible!" << endl;
    }

    if (argc == 1) {
        cout << "No extra Command Line Argument passed "
            "other than program name"
            << endl;
    }
    if (argc > 1) {
        cout << "Number of arguments passed: " << argc
            << endl;
        cout << "----Following are the commnand line "
            "arguments passed----"
            << endl;
        for (int i = 0; i < argc; i++) {
            cout << "argv[" << i << "]: " << argv[i]
                << '\n';
        }
    }

    return 0;
}
