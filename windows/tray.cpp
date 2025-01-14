#include "tray.h"

#include <iostream>

#include <strsafe.h>
#include <windowsx.h>

const static char kSystemTrayEventLButtnUp[] = "leftMouseUp";
const static char kSystemTrayEventLButtonDblClk[] = "leftMouseDblClk";
const static char kSystemTrayEventRButtnUp[] = "rightMouseUp";

// Converts the given UTF-8 string to UTF-16.
static std::wstring Utf16FromUtf8(const std::string& utf8_string) {
  if (utf8_string.empty()) {
    return std::wstring();
  }
  int target_length =
      ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_string.data(),
                            static_cast<int>(utf8_string.length()), nullptr, 0);
  if (target_length == 0) {
    return std::wstring();
  }
  std::wstring utf16_string;
  utf16_string.resize(target_length);
  int converted_length =
      ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_string.data(),
                            static_cast<int>(utf8_string.length()),
                            utf16_string.data(), target_length);
  if (converted_length == 0) {
    return std::wstring();
  }
  return utf16_string;
}

SystemTray::SystemTray(Delegate* delegate) : delegate_(delegate) {}

SystemTray::~SystemTray() {
  removeTrayIcon();
  destroyIcon();
  destroyMenu();
}

bool SystemTray::initSystemTray(HWND window,
                                const std::string* title,
                                const std::string* iconPath,
                                const std::string* toolTip) {
  bool ret = false;

  do {
    if (tray_icon_installed_) {
      ret = true;
      break;
    }

    tray_icon_installed_ = installTrayIcon(window, title, iconPath, toolTip);

    ret = tray_icon_installed_;
  } while (false);

  return ret;
}

bool SystemTray::setSystemTrayInfo(const std::string* title,
                                   const std::string* iconPath,
                                   const std::string* toolTip) {
  bool ret = false;

  do {
    if (!IsWindow(window_)) {
      break;
    }

    if (!tray_icon_installed_) {
      break;
    }

    if (toolTip) {
      nid_.uFlags |= NIF_TIP;
      std::wstring toolTip_u = Utf16FromUtf8(*toolTip);
      StringCchCopy(nid_.szTip, _countof(nid_.szTip), toolTip_u.c_str());
    }

    if (iconPath) {
      destroyIcon();

      nid_.uFlags |= NIF_ICON;
      std::wstring iconPath_u = Utf16FromUtf8(*iconPath);
      icon_ = static_cast<HICON>(
          LoadImage(nullptr, iconPath_u.c_str(), IMAGE_ICON,
                    GetSystemMetrics(SM_CXSMICON),
                    GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE));
      nid_.hIcon = icon_;
    }

    if (!Shell_NotifyIcon(NIM_MODIFY, &nid_)) {
      break;
    }

    ret = true;
  } while (false);

  return ret;
}

bool SystemTray::setContextMenu(HMENU context_menu) {
  destroyMenu();
  context_menu_ = context_menu;
  return true;
}

bool SystemTray::installTrayIcon(HWND window,
                                 const std::string* title,
                                 const std::string* iconPath,
                                 const std::string* toolTip) {
  bool ret = false;

  do {
    destroyIcon();

    std::wstring title_u = title ? Utf16FromUtf8(*title) : L"";
    std::wstring iconPath_u = iconPath ? Utf16FromUtf8(*iconPath) : L"";
    std::wstring toolTip_u = toolTip ? Utf16FromUtf8(*toolTip) : L"";

    icon_ = static_cast<HICON>(LoadImage(
        nullptr, iconPath_u.c_str(), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE));
    if (!icon_) {
      break;
    }

    window_ = window;

    nid_.uVersion = NOTIFYICON_VERSION_4;  // Windows Vista and later support
    nid_.hWnd = window_;
    nid_.hIcon = icon_;
    nid_.uCallbackMessage = tray_notify_callback_message_;
    StringCchCopy(nid_.szTip, _countof(nid_.szTip), toolTip_u.c_str());
    nid_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    if (!Shell_NotifyIcon(NIM_ADD, &nid_)) {
      break;
    }

    ret = true;
  } while (false);

  return ret;
}

bool SystemTray::removeTrayIcon() {
  if (tray_icon_installed_) {
    return Shell_NotifyIcon(NIM_DELETE, &nid_);
  }
  return false;
}

bool SystemTray::reinstallTrayIcon() {
  if (tray_icon_installed_) {
    tray_icon_installed_ = Shell_NotifyIcon(NIM_ADD, &nid_);
    return tray_icon_installed_;
  }
  return false;
}

void SystemTray::destroyIcon() {
  if (icon_) {
    DestroyIcon(icon_);
    icon_ = nullptr;
  }
}

void SystemTray::destroyMenu() {
  if (context_menu_) {
    DestroyMenu(context_menu_);
    context_menu_ = nullptr;
  }
}

std::optional<LRESULT> SystemTray::HandleWindowProc(HWND hwnd,
                                                    UINT message,
                                                    WPARAM wparam,
                                                    LPARAM lparam) {
  if (message == taskbar_created_message_) {
    reinstallTrayIcon();
    return 0;
  } else if (message == tray_notify_callback_message_) {
    UINT id = HIWORD(lparam);
    UINT notifyMsg = LOWORD(lparam);
    POINT pt = {GET_X_LPARAM(wparam), GET_Y_LPARAM(wparam)};
    return OnTrayIconCallback(id, notifyMsg, pt);
  }
  return std::nullopt;
}

std::optional<LRESULT> SystemTray::OnTrayIconCallback(UINT id,
                                                      UINT notifyMsg,
                                                      const POINT& pt) {
  do {
    switch (notifyMsg) {
      case WM_LBUTTONUP: {
        if (delegate_) {
          delegate_->OnSystemTrayEventCallback(kSystemTrayEventLButtnUp);
        }
      } break;
      case WM_LBUTTONDBLCLK: {
        if (delegate_) {
          delegate_->OnSystemTrayEventCallback(kSystemTrayEventLButtonDblClk);
        }
      } break;
      case WM_RBUTTONUP: {
        if (delegate_) {
          delegate_->OnSystemTrayEventCallback(kSystemTrayEventRButtnUp);
        }
        ShowPopupMenu();
      } break;
    }

  } while (false);
  return 0;
}

void SystemTray::ShowPopupMenu() {
  if (!context_menu_) {
    return;
  }

  POINT pt{};
  GetCursorPos(&pt);

  SetForegroundWindow(window_);
  TrackPopupMenu(context_menu_, TPM_LEFTBUTTON, pt.x, pt.y, 0, window_,
                 nullptr);
  PostMessage(window_, WM_NULL, 0, 0);
}