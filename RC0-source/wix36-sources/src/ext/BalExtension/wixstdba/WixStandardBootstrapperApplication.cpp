//-------------------------------------------------------------------------------------------------
// <copyright file="WixStandardBootstrapperApplication.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
//-------------------------------------------------------------------------------------------------


#include "precomp.h"

static const HRESULT E_WIXSTDBA_CONDITION_FAILED = MAKE_HRESULT(SEVERITY_ERROR, 500, 1);

static const LPCWSTR WIXSTDBA_WINDOW_CLASS = L"WixStdBA";
static const LPCWSTR WIXSTDBA_VARIABLE_INSTALL_FOLDER = L"InstallFolder";
static const LPCWSTR WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH = L"LaunchTarget";
static const DWORD WIXSTDBA_ACQUIRE_PERCENTAGE = 30;

enum WIXSTDBA_STATE
{
    WIXSTDBA_STATE_OPTIONS,
    WIXSTDBA_STATE_INITIALIZING,
    WIXSTDBA_STATE_INITIALIZED,
    WIXSTDBA_STATE_DETECTING,
    WIXSTDBA_STATE_DETECTED,
    WIXSTDBA_STATE_PLANNING,
    WIXSTDBA_STATE_PLANNED,
    WIXSTDBA_STATE_APPLYING,
    WIXSTDBA_STATE_CACHING,
    WIXSTDBA_STATE_CACHED,
    WIXSTDBA_STATE_EXECUTING,
    WIXSTDBA_STATE_EXECUTED,
    WIXSTDBA_STATE_APPLIED,
    WIXSTDBA_STATE_FAILED,
};

enum WM_WIXSTDBA
{
    WM_WIXSTDBA_DETECT_PACKAGES = WM_APP + 100,
    WM_WIXSTDBA_PLAN_PACKAGES,
    WM_WIXSTDBA_APPLY_PACKAGES,
    WM_WIXSTDBA_CHANGE_STATE,
};

// This enum must be kept in the same order as the vrgwzPageNames array.
enum WIXSTDBA_PAGE
{
    WIXSTDBA_PAGE_LOADING,
    WIXSTDBA_PAGE_HELP,
    WIXSTDBA_PAGE_INSTALL,
    WIXSTDBA_PAGE_OPTIONS,
    WIXSTDBA_PAGE_MODIFY,
    WIXSTDBA_PAGE_PROGRESS,
    WIXSTDBA_PAGE_PROGRESS_PASSIVE,
    WIXSTDBA_PAGE_SUCCESS,
    WIXSTDBA_PAGE_FAILURE,
    COUNT_WIXSTDBA_PAGE,
};

// This array must be kept in the same order as the WIXSTDBA_PAGE enum.
static LPCWSTR vrgwzPageNames[] = {
    L"Loading",
    L"Help",
    L"Install",
    L"Options",
    L"Modify",
    L"Progress",
    L"ProgressPassive",
    L"Success",
    L"Failure",
};

enum WIXSTDBA_CONTROL
{
    // Non-paged controls
    WIXSTDBA_CONTROL_CLOSE_BUTTON = THEME_FIRST_ASSIGN_CONTROL_ID,
    WIXSTDBA_CONTROL_MINIMIZE_BUTTON,

    // Welcome page
    WIXSTDBA_CONTROL_INSTALL_BUTTON,
    WIXSTDBA_CONTROL_OPTIONS_BUTTON,
    WIXSTDBA_CONTROL_EULA_RICHEDIT,
    WIXSTDBA_CONTROL_EULA_LINK,
    WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX,
    WIXSTDBA_CONTROL_WELCOME_CANCEL_BUTTON,

    // Options page
    WIXSTDBA_CONTROL_FOLDER_EDITBOX,
    WIXSTDBA_CONTROL_BROWSE_BUTTON,
    WIXSTDBA_CONTROL_OK_BUTTON,
    WIXSTDBA_CONTROL_CANCEL_BUTTON,

    // Modify page
    WIXSTDBA_CONTROL_REPAIR_BUTTON,
    WIXSTDBA_CONTROL_UNINSTALL_BUTTON,
    WIXSTDBA_CONTROL_MODIFY_CANCEL_BUTTON,

    // Progress page
    WIXSTDBA_CONTROL_CACHE_PROGRESS_PACKAGE_TEXT,
    WIXSTDBA_CONTROL_CACHE_PROGRESS_BAR,
    WIXSTDBA_CONTROL_CACHE_PROGRESS_TEXT,

    WIXSTDBA_CONTROL_EXECUTE_PROGRESS_PACKAGE_TEXT,
    WIXSTDBA_CONTROL_EXECUTE_PROGRESS_BAR,
    WIXSTDBA_CONTROL_EXECUTE_PROGRESS_TEXT,

    WIXSTDBA_CONTROL_OVERALL_PROGRESS_PACKAGE_TEXT,
    WIXSTDBA_CONTROL_OVERALL_PROGRESS_BAR,
    WIXSTDBA_CONTROL_OVERALL_CALCULATED_PROGRESS_BAR,
    WIXSTDBA_CONTROL_OVERALL_PROGRESS_TEXT,

    WIXSTDBA_CONTROL_PROGRESS_CANCEL_BUTTON,

    // Success page
    WIXSTDBA_CONTROL_LAUNCH_BUTTON,
    WIXSTDBA_CONTROL_SUCCESS_RESTART_TEXT,
    WIXSTDBA_CONTROL_SUCCESS_RESTART_BUTTON,
    WIXSTDBA_CONTROL_SUCCESS_CANCEL_BUTTON,

    // Failure page
    WIXSTDBA_CONTROL_FAILURE_LOGFILE_LINK,
    WIXSTDBA_CONTROL_FAILURE_MESSAGE_TEXT,
    WIXSTDBA_CONTROL_FAILURE_RESTART_TEXT,
    WIXSTDBA_CONTROL_FAILURE_RESTART_BUTTON,
    WIXSTDBA_CONTROL_FAILURE_CANCEL_BUTTON,
};

static THEME_ASSIGN_CONTROL_ID vrgInitControls[] = {
    { WIXSTDBA_CONTROL_CLOSE_BUTTON, L"CloseButton" },
    { WIXSTDBA_CONTROL_MINIMIZE_BUTTON, L"MinimizeButton" },

    { WIXSTDBA_CONTROL_INSTALL_BUTTON, L"InstallButton" },
    { WIXSTDBA_CONTROL_OPTIONS_BUTTON, L"OptionsButton" },
    { WIXSTDBA_CONTROL_EULA_RICHEDIT, L"EulaRichedit" },
    { WIXSTDBA_CONTROL_EULA_LINK, L"EulaHyperlink" },
    { WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX, L"EulaAcceptCheckbox" },
    { WIXSTDBA_CONTROL_WELCOME_CANCEL_BUTTON, L"WelcomeCancelButton" },

    { WIXSTDBA_CONTROL_FOLDER_EDITBOX, L"FolderEditbox" },
    { WIXSTDBA_CONTROL_BROWSE_BUTTON, L"BrowseButton" },
    { WIXSTDBA_CONTROL_OK_BUTTON, L"OptionsOkButton" },
    { WIXSTDBA_CONTROL_CANCEL_BUTTON, L"OptionsCancelButton" },

    { WIXSTDBA_CONTROL_REPAIR_BUTTON, L"RepairButton" },
    { WIXSTDBA_CONTROL_UNINSTALL_BUTTON, L"UninstallButton" },
    { WIXSTDBA_CONTROL_MODIFY_CANCEL_BUTTON, L"ModifyCancelButton" },

    { WIXSTDBA_CONTROL_CACHE_PROGRESS_PACKAGE_TEXT, L"CacheProgressPackageText" },
    { WIXSTDBA_CONTROL_CACHE_PROGRESS_BAR, L"CacheProgressbar" },
    { WIXSTDBA_CONTROL_CACHE_PROGRESS_TEXT, L"CacheProgressText" },
    { WIXSTDBA_CONTROL_EXECUTE_PROGRESS_PACKAGE_TEXT, L"ExecuteProgressPackageText" },
    { WIXSTDBA_CONTROL_EXECUTE_PROGRESS_BAR, L"ExecuteProgressbar" },
    { WIXSTDBA_CONTROL_EXECUTE_PROGRESS_TEXT, L"ExecuteProgressText" },
    { WIXSTDBA_CONTROL_OVERALL_PROGRESS_PACKAGE_TEXT, L"OverallProgressPackageText" },
    { WIXSTDBA_CONTROL_OVERALL_PROGRESS_BAR, L"OverallProgressbar" },
    { WIXSTDBA_CONTROL_OVERALL_CALCULATED_PROGRESS_BAR, L"OverallCalculatedProgressbar" },
    { WIXSTDBA_CONTROL_OVERALL_PROGRESS_TEXT, L"OverallProgressText" },
    { WIXSTDBA_CONTROL_PROGRESS_CANCEL_BUTTON, L"ProgressCancelButton" },

    { WIXSTDBA_CONTROL_LAUNCH_BUTTON, L"LaunchButton" },
    { WIXSTDBA_CONTROL_SUCCESS_RESTART_TEXT, L"SuccessRestartText" },
    { WIXSTDBA_CONTROL_SUCCESS_RESTART_BUTTON, L"SuccessRestartButton" },
    { WIXSTDBA_CONTROL_SUCCESS_CANCEL_BUTTON, L"SuccessCancelButton" },

    { WIXSTDBA_CONTROL_FAILURE_LOGFILE_LINK, L"FailureLogFileLink" },
    { WIXSTDBA_CONTROL_FAILURE_MESSAGE_TEXT, L"FailureMessageText" },
    { WIXSTDBA_CONTROL_FAILURE_RESTART_TEXT, L"FailureRestartText" },
    { WIXSTDBA_CONTROL_FAILURE_RESTART_BUTTON, L"FailureRestartButton" },
    { WIXSTDBA_CONTROL_FAILURE_CANCEL_BUTTON, L"FailureCloseButton" },
};

class CWixStandardBootstrapperApplication : public CBalBaseBootstrapperApplication
{
public: // IBootstrapperApplication
    virtual STDMETHODIMP OnStartup()
    {
        HRESULT hr = S_OK;
        DWORD dwUIThreadId = 0;

        // create UI thread
        m_hUiThread = ::CreateThread(NULL, 0, UiThreadProc, this, 0, &dwUIThreadId);
        if (!m_hUiThread)
        {
            ExitWithLastError(hr, "Failed to create UI thread.");
        }

    LExit:
        return hr;
    }


    virtual STDMETHODIMP_(int) OnShutdown()
    {
        int nResult = IDNOACTION;

        // wait for UI thread to terminate
        if (m_hUiThread)
        {
            ::WaitForSingleObject(m_hUiThread, INFINITE);
            ReleaseHandle(m_hUiThread);
        }

        // If a restart was required.
        if (m_fRestartRequired)
        {
            if (m_fAllowRestart)
            {
                nResult = IDRESTART;
            }

            if (m_sczPrereqPackage)
            {
                BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, m_fAllowRestart ? "The prerequisites scheduled a restart. The bootstrapper application will be reloaded after the computer is restarted."
                                                                        : "A restart is required by the prerequisites but the user delayed it. The bootstrapper application will be reloaded after the computer is restarted.");
            }
        }
        else if (m_fPrereqInstalled)
        {
            BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "The prerequisites were successfully installed. The bootstrapper application will be reloaded.");
            nResult = IDRELOAD_BOOTSTRAPPER;
        }
        else if (m_fPrereqAlreadyInstalled)
        {
            BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "The prerequisites were already installed. The bootstrapper application will not be reloaded to prevent an infinite loop.");
        }
        else if (m_fPrereq)
        {
            BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "The prerequisites were not successfully installed, error: 0x%x. The bootstrapper application will be not reloaded.", m_hrFinal);
        }

        return nResult;
    }


    virtual STDMETHODIMP_(int) OnDetectRelatedBundle(
        __in LPCWSTR /*wzBundleId*/,
        __in LPCWSTR /*wzBundleTag*/,
        __in BOOL /*fPerMachine*/,
        __in DWORD64 /*dw64Version*/,
        __in BOOTSTRAPPER_RELATED_OPERATION operation
        )
    {
        // If we're not doing a pre-req install, remember when our bundle would cause a downgrade.
        if (!m_sczPrereqPackage && BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE == operation)
        {
            m_fDowngrading = TRUE;
        }

        return CheckCanceled() ? IDCANCEL : IDOK;
    }


    virtual STDMETHODIMP_(void) OnDetectPackageComplete(
        __in LPCWSTR wzPackageId,
        __in HRESULT /*hrStatus*/,
        __in BOOTSTRAPPER_PACKAGE_STATE state
        )
    {
        // If the prereq package is already installed, remember that.
        if (m_sczPrereqPackage && BOOTSTRAPPER_PACKAGE_STATE_PRESENT == state &&
            CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzPackageId, -1, m_sczPrereqPackage, -1))
        {
            m_fPrereqAlreadyInstalled = TRUE;
        }
    }


    virtual STDMETHODIMP_(void) OnDetectComplete(
        __in HRESULT hrStatus
        )
    {
        if (SUCCEEDED(hrStatus))
        {
            hrStatus = EvaluateConditions();
        }

        SetState(WIXSTDBA_STATE_DETECTED, hrStatus);

        // If we're not interacting with the user or we're doing a layout then automatically
        // start planning
        if (BOOTSTRAPPER_DISPLAY_FULL > m_command.display || BOOTSTRAPPER_ACTION_LAYOUT == m_command.action)
        {
            if (SUCCEEDED(hrStatus))
            {
                ::PostMessageW(m_hWnd, WM_WIXSTDBA_PLAN_PACKAGES, 0, m_command.action);
            }
        }
    }


    virtual STDMETHODIMP_(int) OnPlanRelatedBundle(
        __in_z LPCWSTR /*wzBundleId*/,
        __inout_z BOOTSTRAPPER_REQUEST_STATE* pRequestedState
        )
    {
        // If we're only installing prereq, do not touch related bundles.
        if (m_sczPrereqPackage)
        {
            *pRequestedState = BOOTSTRAPPER_REQUEST_STATE_NONE;
        }

        return CheckCanceled() ? IDCANCEL : IDOK;
    }


    virtual STDMETHODIMP_(int) OnPlanPackageBegin(
        __in_z LPCWSTR wzPackageId,
        __inout BOOTSTRAPPER_REQUEST_STATE *pRequestState
        )
    {
        // If we're planning to install a pre-req, install it. The pre-req needs to be installed
        // in all cases (even uninstall!) so the BA can load next.
        if (m_sczPrereqPackage)
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzPackageId, -1, m_sczPrereqPackage, -1))
            {
                *pRequestState = BOOTSTRAPPER_REQUEST_STATE_PRESENT;
            }
            else // skip everything else.
            {
                *pRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
            }
        }
        // else, not doing a pre-req install so just do the default behavior.

        return CheckCanceled() ? IDCANCEL : IDOK;
    }


    virtual STDMETHODIMP_(void) OnPlanComplete(
        __in HRESULT hrStatus
        )
    {
        SetState(WIXSTDBA_STATE_PLANNED, hrStatus);

        if (SUCCEEDED(hrStatus))
        {
            ::PostMessageW(m_hWnd, WM_WIXSTDBA_APPLY_PACKAGES, 0, 0);
        }

        m_dwCalculatedCacheProgress = 0;
        m_dwCalculatedExecuteProgress = 0;
    }


    virtual STDMETHODIMP_(int) OnCachePackageBegin(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cCachePayloads,
        __in DWORD64 dw64PackageCacheSize
        )
    {
        if (wzPackageId && *wzPackageId)
        {
            BAL_INFO_PACKAGE* pPackage = NULL;
            HRESULT hr = BalInfoFindPackageById(&m_Bundle.packages, wzPackageId, &pPackage);
            LPCWSTR wz = (SUCCEEDED(hr) && pPackage->sczDisplayName) ? pPackage->sczDisplayName : wzPackageId;

            ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_CACHE_PROGRESS_PACKAGE_TEXT, wz);
            ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_PROGRESS_PACKAGE_TEXT, wz);
        }

        return __super::OnCachePackageBegin(wzPackageId, cCachePayloads, dw64PackageCacheSize);
    }


    virtual STDMETHODIMP_(int) OnCacheAcquireProgress(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in DWORD64 dw64Progress,
        __in DWORD64 dw64Total,
        __in DWORD dwOverallPercentage
        )
    {
        WCHAR wzProgress[5] = { };

#ifdef DEBUG
        BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "WIXSTDBA: OnCacheAcquireProgress() - container/package: %ls, payload: %ls, progress: %I64u, total: %I64u, overall progress: %u%%", wzPackageOrContainerId, wzPayloadId, dw64Progress, dw64Total, dwOverallPercentage);
#endif

        ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallPercentage);
        ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_CACHE_PROGRESS_TEXT, wzProgress);

        ThemeSetProgressControl(m_pTheme, WIXSTDBA_CONTROL_CACHE_PROGRESS_BAR, dwOverallPercentage);

        m_dwCalculatedCacheProgress = dwOverallPercentage * WIXSTDBA_ACQUIRE_PERCENTAGE / 100;
        ThemeSetProgressControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_CALCULATED_PROGRESS_BAR, m_dwCalculatedCacheProgress + m_dwCalculatedExecuteProgress);

        return __super::OnCacheAcquireProgress(wzPackageOrContainerId, wzPayloadId, dw64Progress, dw64Total, dwOverallPercentage);
    }


    virtual STDMETHODIMP_(void) OnCacheComplete(
        __in HRESULT /*hrStatus*/
        )
    {
        ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_CACHE_PROGRESS_PACKAGE_TEXT, L"");
        SetState(WIXSTDBA_STATE_CACHED, S_OK); // we always return success here and let OnApplyComplete() deal with the error.
    }


    virtual STDMETHODIMP_(int) OnError(
        __in BOOTSTRAPPER_ERROR_TYPE /*errorType*/,
        __in LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint,
        __in DWORD /*cData*/,
        __in_ecount_z_opt(cData) LPCWSTR* /*rgwzData*/,
        __in int nRecommendation
        )
    {
        int nResult = IDNOACTION;
        LPWSTR sczError = NULL;

        if (BOOTSTRAPPER_DISPLAY_EMBEDDED == m_command.display)
        {
             HRESULT hr = m_pEngine->SendEmbeddedError(dwCode, wzError, dwUIHint, &nResult);
             if (FAILED(hr))
             {
                 nResult = IDERROR;
             }
        }
        else if (BOOTSTRAPPER_DISPLAY_FULL == m_command.display)
        {
            BalRetryErrorOccurred(wzPackageId, dwCode);

            // If no error message was provided, use the error code to try and get an error message.
            if (!wzError || !*wzError)
            {
                HRESULT hr = StrAllocFromError(&sczError, dwCode, NULL);
                if (FAILED(hr) || !sczError || !*sczError)
                {
                    StrAllocFormatted(&sczError, L"0x%x", dwCode);
                }
            }

            nResult = ::MessageBoxW(m_hWnd, sczError ? sczError : wzError, m_pTheme->sczCaption, dwUIHint);
        }
        else // just take note of the error code and let things continue.
        {
            BalRetryErrorOccurred(wzPackageId, dwCode);
        }

        ReleaseStr(sczError);
        return nResult;
    }


    virtual STDMETHODIMP_(int) OnProgress(
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        HRESULT hr = S_OK;
        WCHAR wzProgress[5] = { };
        int nResult = IDNOACTION;

#ifdef DEBUG
        BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "WIXSTDBA: OnProgress() - progress: %u%%, overall progress: %u%%", dwProgressPercentage, dwOverallProgressPercentage);
#endif

        if (BOOTSTRAPPER_DISPLAY_EMBEDDED == m_command.display)
        {
            hr = m_pEngine->SendEmbeddedProgress(dwProgressPercentage, dwOverallProgressPercentage, &nResult);
            BalExitOnFailure(hr, "Failed to send embedded progress.");
        }
        else
        {
            ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallProgressPercentage);
            ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_PROGRESS_TEXT, wzProgress);

            ThemeSetProgressControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_PROGRESS_BAR, dwOverallProgressPercentage);
        }

    LExit:
        return FAILED(hr) ? IDERROR : CheckCanceled() ? IDCANCEL : nResult;
    }


    virtual STDMETHODIMP_(int) OnExecutePackageBegin(
        __in_z LPCWSTR wzPackageId,
        __in BOOL fExecute
        )
    {
        if (wzPackageId && *wzPackageId)
        {
            BAL_INFO_PACKAGE* pPackage = NULL;
            HRESULT hr = BalInfoFindPackageById(&m_Bundle.packages, wzPackageId, &pPackage);
            LPCWSTR wz = (SUCCEEDED(hr) && pPackage->sczDisplayName) ? pPackage->sczDisplayName : wzPackageId;

            ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_EXECUTE_PROGRESS_PACKAGE_TEXT, wz);
            ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_PROGRESS_PACKAGE_TEXT, wz);
        }

        return __super::OnExecutePackageBegin(wzPackageId, fExecute);
    }


    virtual int __stdcall  OnExecuteProgress(
        __in_z LPCWSTR wzPackageId,
        __in DWORD dwProgressPercentage,
        __in DWORD dwOverallProgressPercentage
        )
    {
        WCHAR wzProgress[5] = { };

#ifdef DEBUG
        BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "WIXSTDBA: OnExecuteProgress() - package: %ls, progress: %u%%, overall progress: %u%%", wzPackageId, dwProgressPercentage, dwOverallProgressPercentage);
#endif

        ::StringCchPrintfW(wzProgress, countof(wzProgress), L"%u%%", dwOverallProgressPercentage);
        ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_EXECUTE_PROGRESS_TEXT, wzProgress);

        ThemeSetProgressControl(m_pTheme, WIXSTDBA_CONTROL_EXECUTE_PROGRESS_BAR, dwOverallProgressPercentage);

        m_dwCalculatedExecuteProgress = dwOverallProgressPercentage * (100 - WIXSTDBA_ACQUIRE_PERCENTAGE) / 100;
        ThemeSetProgressControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_CALCULATED_PROGRESS_BAR, m_dwCalculatedCacheProgress + m_dwCalculatedExecuteProgress);

        return __super::OnExecuteProgress(wzPackageId, dwProgressPercentage, dwOverallProgressPercentage);
    }


    virtual STDMETHODIMP_(int) OnExecutePackageComplete(
        __in_z LPCWSTR wzPackageId,
        __in HRESULT hrExitCode,
        __in BOOTSTRAPPER_APPLY_RESTART restart,
        __in int nRecommendation
        )
    {
        int nResult = __super::OnExecutePackageComplete(wzPackageId, hrExitCode, restart, nRecommendation);

        if (m_sczPrereqPackage && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzPackageId, -1, m_sczPrereqPackage, -1))
        {
            m_fPrereqInstalled = SUCCEEDED(hrExitCode);

            // If the pre-req required a restart (any restart) then do an immediate
            // restart to ensure that the bundle will get launched again post reboot.
            if (BOOTSTRAPPER_APPLY_RESTART_NONE != restart)
            {
                nResult = IDRESTART;
            }
        }

        return nResult;
    }


    virtual STDMETHODIMP_(void) OnExecuteComplete(
        __in HRESULT /*hrStatus*/
        )
    {
        ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_EXECUTE_PROGRESS_PACKAGE_TEXT, L"");
        ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_OVERALL_PROGRESS_PACKAGE_TEXT, L"");
        ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_PROGRESS_CANCEL_BUTTON, FALSE); // no more cancel.

        SetState(WIXSTDBA_STATE_EXECUTED, S_OK); // we always return success here and let OnApplyComplete() deal with the error.
    }


    virtual STDMETHODIMP_(int) OnResolveSource(
        __in_z LPCWSTR wzPackageOrContainerId,
        __in_z_opt LPCWSTR wzPayloadId,
        __in_z LPCWSTR wzLocalSource,
        __in_z_opt LPCWSTR wzDownloadSource
        )
    {
        int nResult = IDERROR; // assume we won't resolve source and that is unexpected.

        if (BOOTSTRAPPER_DISPLAY_FULL == m_command.display)
        {
            if (wzDownloadSource)
            {
                nResult = IDDOWNLOAD;
            }
            else // prompt to change the source location.
            {
                OPENFILENAMEW ofn = { };
                WCHAR wzFile[MAX_PATH] = { };

                ::StringCchCopyW(wzFile, countof(wzFile), wzLocalSource);

                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = m_hWnd;
                ofn.lpstrFile = wzFile;
                ofn.nMaxFile = countof(wzFile);
                ofn.lpstrFilter = L"All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                ofn.lpstrTitle = m_pTheme->sczCaption;

                if (::GetOpenFileNameW(&ofn))
                {
                    HRESULT hr = m_pEngine->SetLocalSource(wzPackageOrContainerId, wzPayloadId, ofn.lpstrFile);
                    nResult = SUCCEEDED(hr) ? IDRETRY : IDERROR;
                }
                else
                {
                    nResult = IDCANCEL;
                }
            }
        }
        else if (wzDownloadSource)
        {
            // If doing a non-interactive install and download source is available, let's try downloading the package silently
            nResult = IDDOWNLOAD;
        }
        // else there's nothing more we can do in non-interactive mode

        return CheckCanceled() ? IDCANCEL : nResult;
    }


    virtual STDMETHODIMP_(int) OnApplyComplete(
        __in HRESULT hrStatus,
        __in BOOTSTRAPPER_APPLY_RESTART restart
        )
    {
        // If a restart was encountered and we are not suppressing restarts, then restart is required.
        m_fRestartRequired = (BOOTSTRAPPER_APPLY_RESTART_NONE != restart && BOOTSTRAPPER_RESTART_NEVER < m_command.restart);
        // If a restart is required and we're not displaying a UI or we are not supposed to prompt for restart then allow the restart.
        m_fAllowRestart = m_fRestartRequired && (BOOTSTRAPPER_DISPLAY_FULL > m_command.display || BOOTSTRAPPER_RESTART_PROMPT < m_command.restart);

        // If we are showing UI, wait a beat before moving to the final screen.
        if (BOOTSTRAPPER_DISPLAY_NONE < m_command.display)
        {
            ::Sleep(250);
        }

        SetState(WIXSTDBA_STATE_APPLIED, hrStatus);

        return IDNOACTION;
    }


private: // privates
    //
    // UiThreadProc - entrypoint for UI thread.
    //
    static DWORD WINAPI UiThreadProc(
        __in LPVOID pvContext
        )
    {
        HRESULT hr = S_OK;
        CWixStandardBootstrapperApplication* pThis = (CWixStandardBootstrapperApplication*)pvContext;
        BOOL fComInitialized = FALSE;
        BOOL fRet = FALSE;
        MSG msg = { };

        // Initialize COM and theme.
        hr = ::CoInitialize(NULL);
        BalExitOnFailure(hr, "Failed to initialize COM.");
        fComInitialized = TRUE;

        hr = ThemeInitialize(pThis->m_hModule);
        BalExitOnFailure(hr, "Failed to initialize theme manager.");

        hr = pThis->InitializeData();
        BalExitOnFailure(hr, "Failed to initialize data in bootstrapper application.");

        // Create main window.
        hr = pThis->CreateMainWindow();
        BalExitOnFailure(hr, "Failed to create main window.");

        // Okay, we're ready for packages now.
        pThis->SetState(WIXSTDBA_STATE_INITIALIZED, hr);
        ::PostMessageW(pThis->m_hWnd, WM_WIXSTDBA_DETECT_PACKAGES, 0, 0);

        // message pump
        while (0 != (fRet = ::GetMessageW(&msg, NULL, 0, 0)))
        {
            if (-1 == fRet)
            {
                hr = E_UNEXPECTED;
                BalExitOnFailure(hr, "Unexpected return value from message pump.");
            }
            else if (!ThemeHandleKeyboardMessage(pThis->m_pTheme, msg.hwnd, &msg))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessageW(&msg);
            }
        }

        // Succeeded thus far, check to see if anything went wrong while actually
        // executing changes.
        if (FAILED(pThis->m_hrFinal))
        {
            hr = pThis->m_hrFinal;
        }
        else if (pThis->CheckCanceled())
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
        }

    LExit:
        // destroy main window
        pThis->DestroyMainWindow();

        // initiate engine shutdown
        pThis->m_pEngine->Quit(hr);

        ReleaseTheme(pThis->m_pTheme);
        ThemeUninitialize();

        // uninitialize COM
        if (fComInitialized)
        {
            ::CoUninitialize();
        }

        return hr;
    }


    //
    // InitializeData - initializes all the package and prereq information.
    //
    HRESULT InitializeData()
    {
        HRESULT hr = S_OK;
        LPWSTR sczModulePath = NULL;
        IXMLDOMDocument *pixdManifest = NULL;

        hr = ProcessCommandLine(&m_sczLanguage);
        ExitOnFailure(hr, "Unknown commandline parameters.");

        hr = PathRelativeToModule(&sczModulePath, NULL, m_hModule);
        BalExitOnFailure(hr, "Failed to get module path.");

        hr = LoadLocalization(sczModulePath, m_sczLanguage);
        ExitOnFailure(hr, "Failed to load localization.");

        hr = LoadTheme(sczModulePath, m_sczLanguage);
        ExitOnFailure(hr, "Failed to load theme.");

        hr = BalManifestLoad(m_hModule, &pixdManifest);
        BalExitOnFailure(hr, "Failed to load bootstrapper application manifest.");

        hr = BalInfoParseFromXml(&m_Bundle, pixdManifest);
        BalExitOnFailure(hr, "Failed to load bundle information.");

        hr = BalConditionsParseFromXml(&m_Conditions, pixdManifest, m_pWixLoc);
        BalExitOnFailure(hr, "Failed to load conditions from XML.");

        if (m_fPrereq)
        {
            hr = ParsePrerequisiteInformationFromXml(pixdManifest);
            BalExitOnFailure(hr, "Failed to read prerequisite information.");
        }
        else
        {
            hr = ParseBootrapperApplicationDataFromXml(pixdManifest);
            BalExitOnFailure(hr, "Failed to read bootstrapper application data.");
        }

    LExit:
        ReleaseObject(pixdManifest);
        ReleaseStr(sczModulePath);

        return hr;
    }


    //
    // ProcessCommandLine - process the provided command line arguments.
    //
    HRESULT ProcessCommandLine(
        __inout LPWSTR* psczLanguage
        )
    {
        HRESULT hr = S_OK;
        int argc = 0;
        LPWSTR* argv = NULL;

        if (m_command.wzCommandLine && *m_command.wzCommandLine)
        {
            argv = ::CommandLineToArgvW(m_command.wzCommandLine, &argc);
            ExitOnNullWithLastError(argv, hr, "Failed to get command line.");

            for (int i = 0; i < argc; ++i)
            {
                if (argv[i][0] == L'-' || argv[i][0] == L'/')
                {
                    if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"lang", -1))
                    {
                        if (i + 1 >= argc)
                        {
                            hr = E_INVALIDARG;
                            BalExitOnFailure(hr, "Must specify a language.");
                        }

                        ++i;

                        hr = StrAllocString(psczLanguage, &argv[i][0], 0);
                        BalExitOnFailure(hr, "Failed to copy language.");
                    }
                }
            }
        }

    LExit:
        if (argv)
        {
            ::LocalFree(argv);
        }

        return hr;
    }

    HRESULT LoadLocalization(
        __in_z LPCWSTR wzModulePath,
        __in_z_opt LPCWSTR wzLanguage
        )
    {
        HRESULT hr = S_OK;
        LPWSTR sczLocPath = NULL;
        LPCWSTR wzLocFileName = m_fPrereq ? L"mbapreq.wxl" : L"thm.wxl";

        hr = LocProbeForFile(wzModulePath, wzLocFileName, wzLanguage, &sczLocPath);
        BalExitOnFailure2(hr, "Failed to probe for loc file: %ls in path: %ls", wzLocFileName, wzModulePath);

        hr = LocLoadFromFile(sczLocPath, &m_pWixLoc);
        BalExitOnFailure1(hr, "Failed to load loc file from path: %ls", sczLocPath);

        hr = StrAllocString(&m_sczConfirmCloseMessage, L"#(loc.ConfirmCancelMessage)", 0);
        ExitOnFailure(hr, "Failed to initialize confirm message loc identifier.");

        hr = LocLocalizeString(m_pWixLoc, &m_sczConfirmCloseMessage);
        BalExitOnFailure1(hr, "Failed to localize confirm close message: %ls", m_sczConfirmCloseMessage);

    LExit:
        ReleaseStr(sczLocPath);

        return hr;
    }


    HRESULT LoadTheme(
        __in_z LPCWSTR wzModulePath,
        __in_z_opt LPCWSTR wzLanguage
        )
    {
        HRESULT hr = S_OK;
        LPWSTR sczThemePath = NULL;
        LPCWSTR wzThemeFileName = m_fPrereq ? L"mbapreq.thm" : L"thm.xml";
        LPWSTR sczCaption = NULL;

        hr = LocProbeForFile(wzModulePath, wzThemeFileName, wzLanguage, &sczThemePath);
        BalExitOnFailure2(hr, "Failed to probe for theme file: %ls in path: %ls", wzThemeFileName, wzModulePath);

        hr = ThemeLoadFromFile(sczThemePath, &m_pTheme);
        BalExitOnFailure1(hr, "Failed to load theme from path: %ls", sczThemePath);

        hr = ThemeLocalize(m_pTheme, m_pWixLoc);
        BalExitOnFailure1(hr, "Failed to localize theme: %ls", sczThemePath);

        // Update the caption if there are any formatted strings in it.
        hr = BalFormatString(m_pTheme->sczCaption, &sczCaption);
        if (SUCCEEDED(hr))
        {
            ThemeUpdateCaption(m_pTheme, sczCaption);
        }

    LExit:
        ReleaseStr(sczCaption);
        ReleaseStr(sczThemePath);

        return hr;
    }


    HRESULT ParsePrerequisiteInformationFromXml(
        __in IXMLDOMDocument* pixdManifest
        )
    {
        HRESULT hr = S_OK;
        IXMLDOMNode* pNode = NULL;

        hr = XmlSelectSingleNode(pixdManifest, L"/BootstrapperApplicationData/WixMbaPrereqInformation", &pNode);
        if (S_FALSE == hr)
        {
            hr = E_INVALIDARG;
        }
        BalExitOnFailure(hr, "BootstrapperApplication.xml manifest is missing prerequisite information.");

        hr = XmlGetAttributeEx(pNode, L"PackageId", &m_sczPrereqPackage);
        BalExitOnFailure(hr, "Failed to get prerequisite package identifier.");

        hr = XmlGetAttributeEx(pNode, L"LicenseUrl", &m_sczLicenseUrl);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        BalExitOnFailure(hr, "Failed to get prerequisite license URL.");

        hr = XmlGetAttributeEx(pNode, L"LicenseFile", &m_sczLicenseFile);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        BalExitOnFailure(hr, "Failed to get prerequisite license file.");

    LExit:
        ReleaseObject(pNode);
        return hr;
    }


    HRESULT ParseBootrapperApplicationDataFromXml(
        __in IXMLDOMDocument* pixdManifest
        )
    {
        HRESULT hr = S_OK;
        IXMLDOMNode* pNode = NULL;
        DWORD dwBool = 0;

        hr = XmlSelectSingleNode(pixdManifest, L"/BootstrapperApplicationData/WixStdbaInformation", &pNode);
        if (S_FALSE == hr)
        {
            hr = E_INVALIDARG;
        }
        BalExitOnFailure(hr, "BootstrapperApplication.xml manifest is missing wixstdba information.");

        hr = XmlGetAttributeEx(pNode, L"LicenseFile", &m_sczLicenseFile);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        BalExitOnFailure(hr, "Failed to get license file.");

        hr = XmlGetAttributeEx(pNode, L"LicenseUrl", &m_sczLicenseUrl);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        BalExitOnFailure(hr, "Failed to get license URL.");

        ReleaseObject(pNode);

        hr = XmlSelectSingleNode(pixdManifest, L"/BootstrapperApplicationData/WixStdbaOptions", &pNode);
        if (S_FALSE == hr)
        {
            ExitFunction1(hr = S_OK);
        }
        BalExitOnFailure(hr, "Failed to read wixstdba options from BootstrapperApplication.xml manifest.");

        hr = XmlGetAttributeNumber(pNode, L"SuppressOptionsUI", &dwBool);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        else if (SUCCEEDED(hr))
        {
            m_fSuppressOptionsUI = 0 < dwBool;
        }
        BalExitOnFailure(hr, "Failed to get SuppressOptionsUI value.");

        dwBool = 0;
        hr = XmlGetAttributeNumber(pNode, L"SuppressDowngradeFailure", &dwBool);
        if (E_NOTFOUND == hr)
        {
            hr = S_OK;
        }
        else if (SUCCEEDED(hr))
        {
            m_fSuppressDowngradeFailure = 0 < dwBool;
        }
        BalExitOnFailure(hr, "Failed to get SuppressDowngradeFailure value.");

    LExit:
        ReleaseObject(pNode);
        return hr;
    }


    //
    // CreateMainWindow - creates the main install window.
    //
    HRESULT CreateMainWindow()
    {
        HRESULT hr = S_OK;
        WNDCLASSW wc = { };
        DWORD dwWindowStyle = 0;

        // Register the window class and create the window.
        wc.lpfnWndProc = CWixStandardBootstrapperApplication::WndProc;
        wc.hInstance = m_hModule;
        wc.hIcon = reinterpret_cast<HICON>(m_pTheme->hIcon);
        wc.hCursor = ::LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
        wc.hbrBackground = m_pTheme->rgFonts[m_pTheme->dwFontId].hBackground;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = WIXSTDBA_WINDOW_CLASS;
        if (!::RegisterClassW(&wc))
        {
            ExitWithLastError(hr, "Failed to register window.");
        }

        m_fRegistered = TRUE;

        // Calculate the window style based on the theme style and command display value.
        dwWindowStyle = m_pTheme->dwStyle;
        if (BOOTSTRAPPER_DISPLAY_NONE >= m_command.display)
        {
            dwWindowStyle &= ~WS_VISIBLE;
        }

        m_hWnd = ::CreateWindowExW(0, wc.lpszClassName, m_pTheme->sczCaption, dwWindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, m_pTheme->nWidth, m_pTheme->nHeight, HWND_DESKTOP, NULL, m_hModule, this);
        ExitOnNullWithLastError(m_hWnd, hr, "Failed to create window.");

        hr = S_OK;

    LExit:
        return hr;
    }


    //
    // DestroyMainWindow - clean up all the window registration.
    //
    void DestroyMainWindow()
    {
        if (::IsWindow(m_hWnd))
        {
            ::DestroyWindow(m_hWnd);
            m_hWnd = NULL;
        }

        if (m_fRegistered)
        {
            ::UnregisterClassW(WIXSTDBA_WINDOW_CLASS, m_hModule);
            m_fRegistered = FALSE;
        }
    }


    //
    // WndProc - standard windows message handler.
    //
    static LRESULT CALLBACK WndProc(
        __in HWND hWnd,
        __in UINT uMsg,
        __in WPARAM wParam,
        __in LPARAM lParam
        )
    {
#pragma warning(suppress:4312)
        CWixStandardBootstrapperApplication* pBA = reinterpret_cast<CWixStandardBootstrapperApplication*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));

        switch (uMsg)
        {
        case WM_NCCREATE:
            {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pBA = reinterpret_cast<CWixStandardBootstrapperApplication*>(lpcs->lpCreateParams);
#pragma warning(suppress:4244)
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pBA));
            }
            break;

        case WM_NCDESTROY:
            {
            LRESULT lres = ThemeDefWindowProc(pBA ? pBA->m_pTheme : NULL, hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
            return lres;
            }

        case WM_CREATE:
            if (!pBA->OnCreate(hWnd))
            {
                return -1;
            }
            break;

        case WM_QUERYENDSESSION:
            return IDCANCEL != pBA->OnSystemShutdown(static_cast<DWORD>(lParam), IDCANCEL);

        case WM_CLOSE:
            // If the user chose not to close, do *not* let the default window proc handle the message.
            if (!pBA->OnClose())
            {
                return 0;
            }
            break;

        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;

        case WM_WIXSTDBA_DETECT_PACKAGES:
            pBA->OnDetect();
            return 0;

        case WM_WIXSTDBA_PLAN_PACKAGES:
            pBA->OnPlan(static_cast<BOOTSTRAPPER_ACTION>(lParam));
            return 0;

        case WM_WIXSTDBA_APPLY_PACKAGES:
            pBA->OnApply();
            return 0;

        case WM_WIXSTDBA_CHANGE_STATE:
            pBA->OnChangeState(static_cast<WIXSTDBA_STATE>(lParam));
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX:
                pBA->OnClickAcceptCheckbox();
                return 0;

            case WIXSTDBA_CONTROL_OPTIONS_BUTTON:
                pBA->OnClickOptionsButton();
                return 0;

            case WIXSTDBA_CONTROL_BROWSE_BUTTON:
                pBA->OnClickOptionsBrowseButton();
                return 0;

            case WIXSTDBA_CONTROL_OK_BUTTON:
                pBA->OnClickOptionsOkButton();
                return 0;

            case WIXSTDBA_CONTROL_CANCEL_BUTTON:
                pBA->OnClickOptionsCancelButton();
                return 0;

            case WIXSTDBA_CONTROL_INSTALL_BUTTON:
                pBA->OnClickInstallButton();
                return 0;

            case WIXSTDBA_CONTROL_REPAIR_BUTTON:
                pBA->OnClickRepairButton();
                return 0;

            case WIXSTDBA_CONTROL_UNINSTALL_BUTTON:
                pBA->OnClickUninstallButton();
                return 0;

            case WIXSTDBA_CONTROL_LAUNCH_BUTTON:
                pBA->OnClickLaunchButton();
                return 0;

            case WIXSTDBA_CONTROL_SUCCESS_RESTART_BUTTON: __fallthrough;
            case WIXSTDBA_CONTROL_FAILURE_RESTART_BUTTON:
                pBA->OnClickRestartButton();
                return 0;

            case WIXSTDBA_CONTROL_WELCOME_CANCEL_BUTTON: __fallthrough;
            case WIXSTDBA_CONTROL_MODIFY_CANCEL_BUTTON: __fallthrough;
            case WIXSTDBA_CONTROL_PROGRESS_CANCEL_BUTTON: __fallthrough;
            case WIXSTDBA_CONTROL_SUCCESS_CANCEL_BUTTON: __fallthrough;
            case WIXSTDBA_CONTROL_FAILURE_CANCEL_BUTTON: __fallthrough;
            case WIXSTDBA_CONTROL_CLOSE_BUTTON:
                pBA->OnClickCloseButton();
                return 0;
            }
            break;

        case WM_NOTIFY:
            if (lParam)
            {
                LPNMHDR pnmhdr = reinterpret_cast<LPNMHDR>(lParam);
                switch (pnmhdr->code)
                {
                case NM_CLICK: __fallthrough;
                case NM_RETURN:
                    switch (static_cast<DWORD>(pnmhdr->idFrom))
                    {
                    case WIXSTDBA_CONTROL_EULA_LINK:
                        pBA->OnClickEulaLink();
                        return 1;
                    case WIXSTDBA_CONTROL_FAILURE_LOGFILE_LINK:
                        pBA->OnClickLogFileLink();
                        return 1;
                    }
                }
            }
            break;
        }

        return ThemeDefWindowProc(pBA ? pBA->m_pTheme : NULL, hWnd, uMsg, wParam, lParam);
    }


    //
    // OnCreate - finishes loading the theme.
    //
    BOOL OnCreate(
        __in HWND hWnd
        )
    {
        HRESULT hr = S_OK;
        LPWSTR sczText = NULL;
        LPWSTR sczLicenseFormatted = NULL;
        LPWSTR sczLicensePath = NULL;
        LPWSTR sczLicenseDirectory = NULL;
        LPWSTR sczLicenseFilename = NULL;

        hr = ThemeLoadControls(m_pTheme, hWnd, vrgInitControls, countof(vrgInitControls));
        BalExitOnFailure(hr, "Failed to load theme controls.");

        C_ASSERT(COUNT_WIXSTDBA_PAGE == countof(vrgwzPageNames));
        C_ASSERT(countof(m_rgdwPageIds) == countof(vrgwzPageNames));

        ThemeGetPageIds(m_pTheme, vrgwzPageNames, m_rgdwPageIds, countof(m_rgdwPageIds));

        // Initialize the text on all "application" (non-page) controls.
        for (DWORD i = 0; i < m_pTheme->cControls; ++i)
        {
            THEME_CONTROL* pControl = m_pTheme->rgControls + i;
            if (!pControl->wPageId && pControl->sczText && *pControl->sczText)
            {
                HRESULT hrFormat = BalFormatString(pControl->sczText, &sczText);
                if (SUCCEEDED(hrFormat))
                {
                    ThemeSetTextControl(m_pTheme, pControl->wId, sczText);
                }
            }
        }

        // Ensure the EULA link points at something if the control exists.
        if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_EULA_LINK))
        {
            BOOL fEulaLink = (m_sczLicenseUrl && *m_sczLicenseUrl);
            ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_EULA_LINK, fEulaLink);
            ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX, fEulaLink);
        }

        // Load the RTF EULA control with text if the control exists.
        if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_EULA_RICHEDIT))
        {
            hr = (m_sczLicenseFile && *m_sczLicenseFile) ? S_OK : E_INVALIDDATA;
            if (SUCCEEDED(hr))
            {
                hr = StrAllocString(&sczLicenseFormatted, m_sczLicenseFile, 0);
                if (SUCCEEDED(hr))
                {
                    hr = LocLocalizeString(m_pWixLoc, &sczLicenseFormatted);
                    if (SUCCEEDED(hr))
                    {
                        hr = BalFormatString(sczLicenseFormatted, &sczLicenseFormatted);
                        if (SUCCEEDED(hr))
                        {
                            hr = PathRelativeToModule(&sczLicensePath, sczLicenseFormatted, m_hModule);
                            if (SUCCEEDED(hr))
                            {
                                hr = PathGetDirectory(sczLicensePath, &sczLicenseDirectory);
                                if (SUCCEEDED(hr))
                                {
                                    hr = StrAllocString(&sczLicenseFilename, PathFile(sczLicenseFormatted), 0);
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = LocProbeForFile(sczLicenseDirectory, sczLicenseFilename, m_sczLanguage, &sczLicensePath);
                                        if (SUCCEEDED(hr))
                                        {
                                            hr = ThemeLoadRichEditFromFile(m_pTheme, WIXSTDBA_CONTROL_EULA_RICHEDIT, sczLicensePath, m_hModule);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (FAILED(hr))
            {
                BalLog(BOOTSTRAPPER_LOG_LEVEL_ERROR, "Failed to load file into license richedit control from path '%ls' manifest value: %ls", sczLicensePath, m_sczLicenseFile);
                hr = S_OK;
            }
        }

    LExit:
        ReleaseStr(sczLicenseFilename);
        ReleaseStr(sczLicenseDirectory);
        ReleaseStr(sczLicensePath);
        ReleaseStr(sczLicenseFormatted);
        ReleaseStr(sczText);

        return SUCCEEDED(hr);
    }


    //
    // OnDetect - start the processing of packages.
    //
    void OnDetect()
    {
        HRESULT hr = S_OK;

        SetState(WIXSTDBA_STATE_DETECTING, hr);

        m_pEngine->CloseSplashScreen();

        // Tell the core we're ready for the packages to be processed now.
        hr = m_pEngine->Detect();
        BalExitOnFailure(hr, "Failed to start detecting chain.");

    LExit:
        if (FAILED(hr))
        {
            SetState(WIXSTDBA_STATE_DETECTING, hr);
        }

        return;
    }


    //
    // OnPlan - plan the detected changes.
    //
    void OnPlan(
        __in BOOTSTRAPPER_ACTION action
        )
    {
        HRESULT hr = S_OK;

        // If we are going to apply a downgrade, bail.
        if (m_fDowngrading && BOOTSTRAPPER_ACTION_UNINSTALL < action)
        {
            if (m_fSuppressDowngradeFailure)
            {
                BalLog(BOOTSTRAPPER_LOG_LEVEL_STANDARD, "A newer version of this product is installed but downgrade failure has been suppressed; continuing...");
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_PRODUCT_VERSION);
                BalExitOnFailure(hr, "Cannot install a product when a newer version is installed.");
            }
        }

        SetState(WIXSTDBA_STATE_PLANNING, hr);

        hr = m_pEngine->Plan(action);
        BalExitOnFailure(hr, "Failed to start planning packages.");

    LExit:
        if (FAILED(hr))
        {
            SetState(WIXSTDBA_STATE_PLANNING, hr);
        }

        return;
    }


    //
    // OnApply - apply the packages.
    //
    void OnApply()
    {
        HRESULT hr = S_OK;

        SetState(WIXSTDBA_STATE_APPLYING, hr);

        hr = m_pEngine->Apply(m_hWnd);
        BalExitOnFailure(hr, "Failed to start applying packages.");

        ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_PROGRESS_CANCEL_BUTTON, TRUE); // ensure the cancel button is enabled before starting.

    LExit:
        if (FAILED(hr))
        {
            SetState(WIXSTDBA_STATE_APPLYING, hr);
        }

        return;
    }


    //
    // OnChangeState - change state.
    //
    void OnChangeState(
        __in WIXSTDBA_STATE state
        )
    {
        WIXSTDBA_STATE stateOld = m_state;
        DWORD dwOldPageId = 0;
        DWORD dwNewPageId = 0;
        LPWSTR sczText = NULL;
        LPWSTR sczUnformattedText = NULL;

        m_state = state;

        // If our install is at the end (success or failure) and we're not showing full UI or
        // we successfully installed the prerequisite then exit (prompt for restart if required).
        if ((WIXSTDBA_STATE_APPLIED <= m_state && BOOTSTRAPPER_DISPLAY_FULL > m_command.display) ||
            (WIXSTDBA_STATE_APPLIED == m_state && m_fPrereq))
        {
            // If a restart was required but we were not automatically allowed to
            // accept the reboot then do the prompt.
            if (m_fRestartRequired && !m_fAllowRestart)
            {
                StrAllocFromError(&sczUnformattedText, HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED), NULL);

                int nResult = ::MessageBoxW(m_hWnd, sczUnformattedText ? sczUnformattedText : L"The requested operation is successful. Changes will not be effective until the system is rebooted.", m_pTheme->sczCaption, MB_ICONEXCLAMATION | MB_OKCANCEL);
                m_fAllowRestart = (IDOK == nResult);
            }

            // Quietly exit.
            ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);
        }
        else // try to change the pages.
        {
            DeterminePageId(stateOld, &dwOldPageId);
            DeterminePageId(m_state, &dwNewPageId);

            if (dwOldPageId != dwNewPageId)
            {
                // Enable disable controls per-page.
                if (m_rgdwPageIds[WIXSTDBA_PAGE_INSTALL] == dwNewPageId) // on the "Install" page, ensure the install button is enabled/disabled correctly.
                {
                    BOOL fAcceptedLicense = !ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX) || !ThemeControlEnabled(m_pTheme, WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX) || ThemeIsControlChecked(m_pTheme, WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX);
                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_INSTALL_BUTTON, fAcceptedLicense);

                    // If there is an "Options" page, the "Options" button exists, and it hasn't been suppressed, then enable the button.
                    BOOL fOptionsEnabled = m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS] && ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_OPTIONS_BUTTON) && !m_fSuppressOptionsUI;
                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_OPTIONS_BUTTON, fOptionsEnabled);
                }
                else if (m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS] == dwNewPageId)
                {
                    HRESULT hr = BalGetStringVariable(WIXSTDBA_VARIABLE_INSTALL_FOLDER, &sczUnformattedText);
                    if (SUCCEEDED(hr))
                    {
                        BalFormatString(sczUnformattedText, &sczText);
                        ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_FOLDER_EDITBOX, sczText);
                    }
                }
                else if (m_rgdwPageIds[WIXSTDBA_PAGE_SUCCESS] == dwNewPageId) // on the "Success" page, check if the restart or launch button should be enabled.
                {
                    BOOL fShowRestartButton = FALSE;
                    BOOL fLaunchTargetExists = FALSE;
                    if (m_fRestartRequired)
                    {
                        if (BOOTSTRAPPER_RESTART_PROMPT == m_command.restart)
                        {
                            fShowRestartButton = TRUE;
                        }
                    }
                    else if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_LAUNCH_BUTTON))
                    {
                        fLaunchTargetExists = BalStringVariableExists(WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH);
                    }

                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_LAUNCH_BUTTON, fLaunchTargetExists);
                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_SUCCESS_RESTART_TEXT, fShowRestartButton);
                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_SUCCESS_RESTART_BUTTON, fShowRestartButton);
                }
                else if (m_rgdwPageIds[WIXSTDBA_PAGE_FAILURE] == dwNewPageId) // on the "Failure" page, show error message and check if the restart button should be enabled.
                {
                    BOOL fShowLogLink = (m_Bundle.sczLogVariable && *m_Bundle.sczLogVariable); // if there is a log file variable then we'll assume the log file exists.
                    BOOL fShowErrorMessage = FALSE;
                    BOOL fShowRestartButton = FALSE;

                    if (FAILED(m_hrFinal))
                    {
                        // If we know the failure message, use that.
                        if (m_sczFailedMessage && *m_sczFailedMessage)
                        {
                            StrAllocString(&sczUnformattedText, m_sczFailedMessage, 0);
                        }
                        else // try to get the error message from the error code.
                        {
                            StrAllocFromError(&sczUnformattedText, m_hrFinal, NULL);
                            if (!sczUnformattedText || !*sczUnformattedText)
                            {
                                StrAllocFromError(&sczUnformattedText, E_FAIL, NULL);
                            }
                        }

                        StrAllocFormatted(&sczText, L"0x%08x - %ls", m_hrFinal, sczUnformattedText);
                        ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_FAILURE_MESSAGE_TEXT, sczText);
                        fShowErrorMessage = TRUE;
                    }

                    if (m_fRestartRequired)
                    {
                        if (BOOTSTRAPPER_RESTART_PROMPT == m_command.restart)
                        {
                            fShowRestartButton = TRUE;
                        }
                    }

                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_FAILURE_LOGFILE_LINK, fShowLogLink);
                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_FAILURE_MESSAGE_TEXT, fShowErrorMessage);
                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_FAILURE_RESTART_TEXT, fShowRestartButton);
                    ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_FAILURE_RESTART_BUTTON, fShowRestartButton);
                }

                // Process each control for special handling in the new page.
                THEME_PAGE* pPage = ThemeGetPage(m_pTheme, dwNewPageId);
                if (pPage)
                {
                    for (DWORD i = 0; i < pPage->cControlIndices; ++i)
                    {
                        THEME_CONTROL* pControl = m_pTheme->rgControls + pPage->rgdwControlIndices[i];

                        // If we are on the options page and this is a named checkbox control, try to set its default
                        // state to the state of a matching named Burn variable.
                        if (m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS] == dwNewPageId && THEME_CONTROL_TYPE_CHECKBOX == pControl->type && pControl->sczName && *pControl->sczName)
                        {
                            LONGLONG llValue = 0;
                            HRESULT hr = m_pEngine->GetVariableNumeric(pControl->sczName, &llValue);

                            ThemeSendControlMessage(m_pTheme, pControl->wId, BM_SETCHECK, SUCCEEDED(hr) && llValue ? BST_CHECKED : BST_UNCHECKED, 0);
                        }

                        // Format the text in each of the new page's controls (if they have any text).
                        if (pControl->sczText && *pControl->sczText)
                        {
                            HRESULT hr = BalFormatString(pControl->sczText, &sczText);
                            if (SUCCEEDED(hr))
                            {
                                ThemeSetTextControl(m_pTheme, pControl->wId, sczText);
                            }
                        }
                    }
                }

                ThemeShowPage(m_pTheme, dwOldPageId, SW_HIDE);
                ThemeShowPage(m_pTheme, dwNewPageId, SW_SHOW);
            }
        }

        ReleaseStr(sczText);
        ReleaseStr(sczUnformattedText);
    }


    //
    // OnClose - called when the window is trying to be closed.
    //
    BOOL OnClose()
    {
        BOOL fClose = FALSE;

        // If we've already succeeded or failed, just close (prompts are annoying if the bootstrapper is done).
        if (WIXSTDBA_STATE_APPLIED <= m_state)
        {
            fClose = TRUE;
        }
        else // prompt the user or force the cancel if there is no UI.
        {
            fClose = PromptCancel(m_hWnd, BOOTSTRAPPER_DISPLAY_FULL != m_command.display, m_sczConfirmCloseMessage ? m_sczConfirmCloseMessage : L"Are you sure you want to cancel?", m_pTheme->sczCaption);
        }

        // If we're doing progress then we never close, we just cancel to let rollback occur.
        if (WIXSTDBA_STATE_APPLYING <= m_state && WIXSTDBA_STATE_APPLIED > m_state)
        {
            // If we canceled disable cancel button since clicking it again is silly.
            if (fClose)
            {
                ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_PROGRESS_CANCEL_BUTTON, FALSE);
            }

            fClose = FALSE;
        }

        return fClose;
    }


    //
    // OnClickAcceptCheckbox - allow the install to continue.
    //
    void OnClickAcceptCheckbox()
    {
        BOOL fAcceptedLicense = ThemeIsControlChecked(m_pTheme, WIXSTDBA_CONTROL_EULA_ACCEPT_CHECKBOX);
        ThemeControlEnable(m_pTheme, WIXSTDBA_CONTROL_INSTALL_BUTTON, fAcceptedLicense);
    }


    //
    // OnClickOptionsButton - show the options page.
    //
    void OnClickOptionsButton()
    {
        m_stateBeforeOptions = m_state;
        SetState(WIXSTDBA_STATE_OPTIONS, S_OK);
    }


    //
    // OnClickOptionsBrowseButton - browse for install folder on the options page.
    //
    void OnClickOptionsBrowseButton()
    {
        WCHAR wzPath[MAX_PATH] = { };
        BROWSEINFOW browseInfo = { };
        PIDLIST_ABSOLUTE pidl = NULL;

        browseInfo.hwndOwner = m_hWnd;
        browseInfo.pszDisplayName = wzPath;
        browseInfo.lpszTitle = m_pTheme->sczCaption;
        browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
        pidl = ::SHBrowseForFolderW(&browseInfo);
        if (pidl && ::SHGetPathFromIDListW(pidl, wzPath))
        {
            ThemeSetTextControl(m_pTheme, WIXSTDBA_CONTROL_FOLDER_EDITBOX, wzPath);
        }

        if (pidl)
        {
            ::CoTaskMemFree(pidl);
        }

        return;
    }

    //
    // OnClickOptionsOkButton - accept the changes made by the options page.
    //
    void OnClickOptionsOkButton()
    {
        HRESULT hr = S_OK;
        LPWSTR sczPath = NULL;
        THEME_PAGE* pPage = NULL;

        if (ThemeControlExists(m_pTheme, WIXSTDBA_CONTROL_FOLDER_EDITBOX))
        {
            hr = ThemeGetTextControl(m_pTheme, WIXSTDBA_CONTROL_FOLDER_EDITBOX, &sczPath);
            ExitOnFailure(hr, "Failed to get text from folder edit box.");

            // TODO: verify the path is valid.

            hr = m_pEngine->SetVariableString(WIXSTDBA_VARIABLE_INSTALL_FOLDER, sczPath);
            ExitOnFailure(hr, "Failed to set the install folder.");
        }

        // Loop through all the checkbox controls with names and set a Burn variable
        // with that name to true or false.
        pPage = ThemeGetPage(m_pTheme, m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS]);
        if (pPage)
        {
            for (DWORD i = 0; i < pPage->cControlIndices; ++i)
            {
                THEME_CONTROL* pControl = m_pTheme->rgControls + pPage->rgdwControlIndices[i];
                if (THEME_CONTROL_TYPE_CHECKBOX == pControl->type && pControl->sczName && *pControl->sczName)
                {
                    BOOL bChecked = ThemeIsControlChecked(m_pTheme, pControl->wId);
                    m_pEngine->SetVariableNumeric(pControl->sczName, bChecked ? 1 : 0);
                }
            }
        }

    LExit:
        SetState(m_stateBeforeOptions, S_OK);
        return;
    }


    //
    // OnClickOptionsCancelButton - discard the changes made by the options page.
    //
    void OnClickOptionsCancelButton()
    {
        SetState(m_stateBeforeOptions, S_OK);
        return;
    }


    //
    // OnClickInstallButton - start the install by planning the packages.
    //
    void OnClickInstallButton()
    {
        this->OnPlan(BOOTSTRAPPER_ACTION_INSTALL);
    }


    //
    // OnClickRepairButton - start the repair.
    //
    void OnClickRepairButton()
    {
        this->OnPlan(BOOTSTRAPPER_ACTION_REPAIR);
    }


    //
    // OnClickUninstallButton - start the uninstall.
    //
    void OnClickUninstallButton()
    {
        this->OnPlan(BOOTSTRAPPER_ACTION_UNINSTALL);
    }


    //
    // OnClickCloseButton - close the application.
    //
    void OnClickCloseButton()
    {
        ::SendMessageW(m_hWnd, WM_CLOSE, 0, 0);
    }


    //
    // OnClickEulaLink - show the end user license agreement.
    //
    void OnClickEulaLink()
    {
        HRESULT hr = S_OK;
        LPWSTR sczLicenseUrl = NULL;
        LPWSTR sczLicensePath = NULL;
        URI_PROTOCOL protocol = URI_PROTOCOL_UNKNOWN;

        hr = StrAllocString(&sczLicenseUrl, m_sczLicenseUrl, 0);
        BalExitOnFailure1(hr, "Failed to copy license URL: %ls", m_sczLicenseUrl);

        hr = LocLocalizeString(m_pWixLoc, &sczLicenseUrl);
        BalExitOnFailure1(hr, "Failed to localize license URL: %ls", m_sczLicenseUrl);

        hr = BalFormatString(sczLicenseUrl, &sczLicenseUrl);
        BalExitOnFailure1(hr, "Failed to get formatted license URL: %ls", m_sczLicenseUrl);

        hr = UriProtocol(sczLicenseUrl, &protocol);
        if (FAILED(hr) || URI_PROTOCOL_UNKNOWN == protocol)
        {
            hr = PathRelativeToModule(&sczLicensePath, sczLicenseUrl, m_hModule);
        }

        hr = ShelExec(sczLicensePath ? sczLicensePath : sczLicenseUrl, NULL, L"open", NULL, SW_SHOWDEFAULT, m_hWnd, NULL);
        BalExitOnFailure(hr, "Failed to launch URL to EULA.");

    LExit:
        ReleaseStr(sczLicensePath);
        ReleaseStr(sczLicenseUrl);
        return;
    }


    //
    // OnClickLaunchButton - launch the app from the success page.
    //
    void OnClickLaunchButton()
    {
        HRESULT hr = S_OK;
        LPWSTR sczUnformattedLaunchTarget = NULL;
        LPWSTR sczLaunchTarget = NULL;

        hr = BalGetStringVariable(WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH, &sczUnformattedLaunchTarget);
        BalExitOnFailure1(hr, "Failed to get launch target variable '%ls'.", WIXSTDBA_VARIABLE_LAUNCH_TARGET_PATH);

        hr = BalFormatString(sczUnformattedLaunchTarget, &sczLaunchTarget);
        BalExitOnFailure1(hr, "Failed to format launch target variable: %ls", sczUnformattedLaunchTarget);

        hr = ShelExec(sczLaunchTarget, NULL, L"open", NULL, SW_SHOWDEFAULT, m_hWnd, NULL);
        BalExitOnFailure1(hr, "Failed to launch target: %ls", sczLaunchTarget);

        ::PostMessageW(m_hWnd, WM_CLOSE, 0, 0);

    LExit:
        ReleaseStr(sczLaunchTarget);
        ReleaseStr(sczUnformattedLaunchTarget);

        return;
    }


    //
    // OnClickRestartButton - allows the restart and closes the app.
    //
    void OnClickRestartButton()
    {
        AssertSz(m_fRestartRequired, "Restart must be requested to be able to click on the restart button.");

        m_fAllowRestart = TRUE;
        ::SendMessageW(m_hWnd, WM_CLOSE, 0, 0);

        return;
    }


    //
    // OnClickLogFileLink - show the log file.
    //
    void OnClickLogFileLink()
    {
        HRESULT hr = S_OK;
        LPWSTR sczLogFile = NULL;

        hr = BalGetStringVariable(m_Bundle.sczLogVariable, &sczLogFile);
        BalExitOnFailure1(hr, "Failed to get log file variable '%ls'.", m_Bundle.sczLogVariable);

        hr = ShelExec(L"notepad.exe", sczLogFile, L"open", NULL, SW_SHOWDEFAULT, m_hWnd, NULL);
        BalExitOnFailure1(hr, "Failed to open log file target: %ls", sczLogFile);

    LExit:
        ReleaseStr(sczLogFile);

        return;
    }


    //
    // SetState
    //
    void SetState(
        __in WIXSTDBA_STATE state,
        __in HRESULT hrStatus
        )
    {
        if (FAILED(hrStatus))
        {
            m_hrFinal = hrStatus;
        }

        if (FAILED(m_hrFinal))
        {
            state = WIXSTDBA_STATE_FAILED;
        }

        if (WIXSTDBA_STATE_OPTIONS == state || m_state < state)
        {
            ::PostMessageW(m_hWnd, WM_WIXSTDBA_CHANGE_STATE, 0, state);
        }
    }


    void DeterminePageId(
        __in WIXSTDBA_STATE state,
        __out DWORD* pdwPageId
        )
    {
        if (BOOTSTRAPPER_DISPLAY_PASSIVE == m_command.display)
        {
            switch (state)
            {
            case WIXSTDBA_STATE_INITIALIZED: __fallthrough;
            case WIXSTDBA_STATE_DETECTING:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_LOADING] ? m_rgdwPageIds[WIXSTDBA_PAGE_LOADING] : m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS_PASSIVE] ? m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS_PASSIVE] : m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS];
                break;

            case WIXSTDBA_STATE_DETECTED: __fallthrough;
            case WIXSTDBA_STATE_PLANNING: __fallthrough;
            case WIXSTDBA_STATE_PLANNED: __fallthrough;
            case WIXSTDBA_STATE_APPLYING: __fallthrough;
            case WIXSTDBA_STATE_CACHING: __fallthrough;
            case WIXSTDBA_STATE_CACHED: __fallthrough;
            case WIXSTDBA_STATE_EXECUTING: __fallthrough;
            case WIXSTDBA_STATE_EXECUTED:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS_PASSIVE] ? m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS_PASSIVE] : m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS];
                break;

            default:
                *pdwPageId = 0;
                break;
            }
        }
        else if (BOOTSTRAPPER_DISPLAY_FULL == m_command.display)
        {
            switch (state)
            {
            case WIXSTDBA_STATE_INITIALIZING:
                *pdwPageId = 0;
                break;

            case WIXSTDBA_STATE_INITIALIZED: __fallthrough;
            case WIXSTDBA_STATE_DETECTING:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_LOADING];
                break;

            case WIXSTDBA_STATE_DETECTED:
                switch (m_command.action)
                {
                case BOOTSTRAPPER_ACTION_HELP:
                    *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_HELP] ? m_rgdwPageIds[WIXSTDBA_PAGE_HELP] : m_rgdwPageIds[WIXSTDBA_PAGE_INSTALL];
                    break;

                case BOOTSTRAPPER_ACTION_INSTALL:
                    *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_INSTALL];
                    break;

                case BOOTSTRAPPER_ACTION_MODIFY: __fallthrough;
                case BOOTSTRAPPER_ACTION_REPAIR: __fallthrough;
                case BOOTSTRAPPER_ACTION_UNINSTALL:
                    *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_MODIFY];
                    break;
                }
                break;

            case WIXSTDBA_STATE_OPTIONS:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_OPTIONS];
                break;

            case WIXSTDBA_STATE_PLANNING: __fallthrough;
            case WIXSTDBA_STATE_PLANNED: __fallthrough;
            case WIXSTDBA_STATE_APPLYING: __fallthrough;
            case WIXSTDBA_STATE_CACHING: __fallthrough;
            case WIXSTDBA_STATE_CACHED: __fallthrough;
            case WIXSTDBA_STATE_EXECUTING: __fallthrough;
            case WIXSTDBA_STATE_EXECUTED:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_PROGRESS];
                break;

            case WIXSTDBA_STATE_APPLIED:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_SUCCESS];
                break;

            case WIXSTDBA_STATE_FAILED:
                *pdwPageId = m_rgdwPageIds[WIXSTDBA_PAGE_FAILURE];
                break;
            }
        }
    }


    HRESULT EvaluateConditions()
    {
        HRESULT hr = S_OK;
        BOOL fResult = FALSE;

        for (DWORD i = 0; i < m_Conditions.cConditions; ++i)
        {
            BAL_CONDITION* pCondition = m_Conditions.rgConditions + i;

            hr = BalConditionEvaluate(pCondition, m_pEngine, &fResult, &m_sczFailedMessage);
            BalExitOnFailure(hr, "Failed to evaluate condition.");

            if (!fResult)
            {
                hr = E_WIXSTDBA_CONDITION_FAILED;
                BalExitOnFailure1(hr, "Bundle condition evaluated to false: %ls", pCondition->sczCondition);
            }
        }

        ReleaseNullStr(m_sczFailedMessage);

    LExit:
        return hr;
    }


public:
    //
    // Constructor - intitialize member variables.
    //
    CWixStandardBootstrapperApplication(
        __in HMODULE hModule,
        __in BOOL fPrereq,
        __in IBootstrapperEngine* pEngine,
        __in const BOOTSTRAPPER_COMMAND* pCommand
        ) : CBalBaseBootstrapperApplication(pEngine, pCommand->restart, 3, 3000)
    {
        m_hModule = hModule;
        memcpy_s(&m_command, sizeof(m_command), pCommand, sizeof(BOOTSTRAPPER_COMMAND));

        // Pre-req BA shouldn't be doing layout
        if (fPrereq && BOOTSTRAPPER_ACTION_LAYOUT == m_command.action)
        {
            m_command.action = BOOTSTRAPPER_ACTION_INSTALL;
        }
        else // maybe modify the action state if the bundle is or is not already installed.
        {
            LONGLONG llInstalled = 0;
            HRESULT hr = pEngine->GetVariableNumeric(L"WixBundleInstalled", &llInstalled);
            if (SUCCEEDED(hr) && 0 < llInstalled && BOOTSTRAPPER_ACTION_INSTALL == m_command.action)
            {
                m_command.action = BOOTSTRAPPER_ACTION_MODIFY;
            }
            else if (0 == llInstalled && (BOOTSTRAPPER_ACTION_MODIFY == m_command.action || BOOTSTRAPPER_ACTION_REPAIR == m_command.action))
            {
                m_command.action = BOOTSTRAPPER_ACTION_INSTALL;
            }
        }

        m_pWixLoc = NULL;
        memset(&m_Bundle, 0, sizeof(m_Bundle));
        memset(&m_Conditions, 0, sizeof(m_Conditions));
        m_sczConfirmCloseMessage = NULL;
        m_sczFailedMessage = NULL;

        m_sczLanguage = NULL;
        m_pTheme = NULL;
        memset(m_rgdwPageIds, 0, sizeof(m_rgdwPageIds));
        m_hUiThread = NULL;
        m_fRegistered = FALSE;
        m_hWnd = NULL;

        m_state = WIXSTDBA_STATE_INITIALIZING;
        m_hrFinal = S_OK;

        m_fDowngrading = FALSE;
        m_fRestartRequired = FALSE;
        m_fAllowRestart = FALSE;

        m_sczLicenseFile = NULL;
        m_sczLicenseUrl = NULL;
        m_fSuppressOptionsUI = FALSE;
        m_fSuppressDowngradeFailure = FALSE;

        m_fPrereq = fPrereq;
        m_sczPrereqPackage = NULL;
        m_fPrereqInstalled = FALSE;
        m_fPrereqAlreadyInstalled = FALSE;

        pEngine->AddRef();
        m_pEngine = pEngine;
    }


    //
    // Destructor - release member variables.
    //
    ~CWixStandardBootstrapperApplication()
    {
        AssertSz(!::IsWindow(m_hWnd), "Window should have been destroyed before destructor.");
        AssertSz(!m_pTheme, "Theme should have been released before destuctor.");

        ReleaseStr(m_sczFailedMessage);
        ReleaseStr(m_sczConfirmCloseMessage);
        BalConditionsUninitialize(&m_Conditions);
        BalInfoUninitialize(&m_Bundle);
        LocFree(m_pWixLoc);

        ReleaseStr(m_sczLanguage);
        ReleaseStr(m_sczLicenseFile);
        ReleaseStr(m_sczLicenseUrl);
        ReleaseStr(m_sczPrereqPackage);
        ReleaseNullObject(m_pEngine);
    }

private:
    HMODULE m_hModule;
    BOOTSTRAPPER_COMMAND m_command;
    IBootstrapperEngine* m_pEngine;

    WIX_LOCALIZATION* m_pWixLoc;
    BAL_INFO_BUNDLE m_Bundle;
    BAL_CONDITIONS m_Conditions;
    LPWSTR m_sczFailedMessage;
    LPWSTR m_sczConfirmCloseMessage;

    LPWSTR m_sczLanguage;
    THEME* m_pTheme;
    DWORD m_rgdwPageIds[countof(vrgwzPageNames)];
    HANDLE m_hUiThread;
    BOOL m_fRegistered;
    HWND m_hWnd;

    WIXSTDBA_STATE m_state;
    WIXSTDBA_STATE m_stateBeforeOptions;
    HRESULT m_hrFinal;

    DWORD m_dwCalculatedCacheProgress;
    DWORD m_dwCalculatedExecuteProgress;

    BOOL m_fDowngrading;
    BOOL m_fRestartRequired;
    BOOL m_fAllowRestart;

    LPWSTR m_sczLicenseFile;
    LPWSTR m_sczLicenseUrl;
    BOOL m_fSuppressOptionsUI;
    BOOL m_fSuppressDowngradeFailure;

    BOOL m_fPrereq;
    LPWSTR m_sczPrereqPackage;
    BOOL m_fPrereqInstalled;
    BOOL m_fPrereqAlreadyInstalled;
};


//
// CreateUserExperience - creates a new IBurnUserExperience object.
//
HRESULT CreateBootstrapperApplication(
    __in HMODULE hModule,
    __in BOOL fPrereq,
    __in IBootstrapperEngine* pEngine,
    __in const BOOTSTRAPPER_COMMAND* pCommand,
    __out IBootstrapperApplication** ppApplication
    )
{
    HRESULT hr = S_OK;
    CWixStandardBootstrapperApplication* pApplication = NULL;

    pApplication = new CWixStandardBootstrapperApplication(hModule, fPrereq, pEngine, pCommand);
    ExitOnNull(pApplication, hr, E_OUTOFMEMORY, "Failed to create new standard bootstrapper application object.");

    *ppApplication = pApplication;
    pApplication = NULL;

LExit:
    ReleaseObject(pApplication);
    return hr;
}
