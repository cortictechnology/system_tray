import 'dart:io';
import 'dart:async';

import 'package:flutter/material.dart';

import 'package:bitsdojo_window/bitsdojo_window.dart';

import 'package:system_tray/system_tray.dart' as system_tray;

void main() async {
  runApp(const MyApp());

  doWhenWindowReady(() {
    final win = appWindow;
    const initialSize = Size(600, 450);
    win.minSize = initialSize;
    win.size = initialSize;
    win.alignment = Alignment.center;
    win.title = "How to use system tray with Flutter";
    win.show();
  });
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final system_tray.SystemTray _systemTray = system_tray.SystemTray();
  final system_tray.AppWindow _appWindow = system_tray.AppWindow();

  Timer? _timer;
  bool _toogleTrayIcon = true;

  @override
  void initState() {
    super.initState();
    initSystemTray();
  }

  @override
  void dispose() {
    super.dispose();
    _timer?.cancel();
  }

  Future<void> initSystemTray() async {
    final path =
        Platform.isWindows ? 'assets/app_icon.ico' : 'assets/app_icon.png';
    final menu = [
      system_tray.MenuItem(label: 'Show', onClicked: _appWindow.show),
      system_tray.MenuItem(label: 'Hide', onClicked: _appWindow.hide),
      system_tray.MenuItem(
        label: 'Start flash tray icon',
        onClicked: () {
          debugPrint("Start flash tray icon");

          _timer ??= Timer.periodic(
            const Duration(milliseconds: 500),
            (timer) {
              _toogleTrayIcon = !_toogleTrayIcon;
              _systemTray.setSystemTrayInfo(
                iconPath: _toogleTrayIcon ? "" : path,
              );
            },
          );
        },
      ),
      system_tray.MenuItem(
        label: 'Stop flash tray icon',
        onClicked: () {
          debugPrint("Stop flash tray icon");

          _timer?.cancel();
          _timer = null;

          _systemTray.setSystemTrayInfo(
            iconPath: path,
          );
        },
      ),
      system_tray.MenuSeparator(),
      system_tray.SubMenu(
        label: "SubMenu",
        children: [
          system_tray.MenuItem(
            label: 'SubItem1',
            enabled: false,
            onClicked: () {
              debugPrint("click SubItem1");
            },
          ),
          system_tray.MenuItem(
            label: 'SubItem2',
            onClicked: () {
              debugPrint("click SubItem2");
            },
          ),
          system_tray.MenuItem(
            label: 'SubItem3',
            onClicked: () {
              debugPrint("click SubItem3");
            },
          ),
        ],
      ),
      system_tray.MenuSeparator(),
      system_tray.MenuItem(
        label: 'Exit',
        onClicked: _appWindow.close,
      ),
    ];

    // We first init the systray menu and then add the menu entries
    await _systemTray.initSystemTray(
      title: "system tray",
      iconPath: path,
      toolTip: "How to use system tray with Flutter",
    );

    await _systemTray.setContextMenu(menu);

    // handle system tray event
    _systemTray.registerSystemTrayEventHandler((eventName) {
      debugPrint("eventName: $eventName");
      if (eventName == "leftMouseUp") {
        _appWindow.show();
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: Scaffold(
        body: WindowBorder(
          color: const Color(0xFF805306),
          width: 1,
          child: Row(
            children: const [
              LeftSide(),
              RightSide(),
            ],
          ),
        ),
      ),
    );
  }
}

const backgroundStartColor = Color(0xFFFFD500);
const backgroundEndColor = Color(0xFFF6A00C);

class LeftSide extends StatelessWidget {
  const LeftSide({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 200,
      child: Container(
        color: const Color(0xFFFFFFFF),
        child: Column(
          children: [
            WindowTitleBarBox(
              child: Container(
                decoration: const BoxDecoration(
                  gradient: LinearGradient(
                      begin: Alignment.topCenter,
                      end: Alignment.bottomCenter,
                      colors: [backgroundStartColor, backgroundEndColor],
                      stops: [0.0, 1.0]),
                ),
                child: MoveWindow(),
              ),
            ),
            Expanded(
              child: Container(),
            )
          ],
        ),
      ),
    );
  }
}

class RightSide extends StatelessWidget {
  const RightSide({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Expanded(
      child: Container(
        color: const Color(0xFFFFFFFF),
        child: Column(
          children: [
            WindowTitleBarBox(
              child: Container(
                decoration: const BoxDecoration(
                  gradient: LinearGradient(
                      begin: Alignment.topCenter,
                      end: Alignment.bottomCenter,
                      colors: [backgroundStartColor, backgroundEndColor],
                      stops: [0.0, 1.0]),
                ),
                child: Row(
                  children: [
                    Expanded(
                      child: MoveWindow(),
                    ),
                    const WindowButtons()
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

final buttonColors = WindowButtonColors(
    iconNormal: const Color(0xFF805306),
    mouseOver: const Color(0xFFF6A00C),
    mouseDown: const Color(0xFF805306),
    iconMouseOver: const Color(0xFF805306),
    iconMouseDown: const Color(0xFFFFD500));

final closeButtonColors = WindowButtonColors(
    mouseOver: const Color(0xFFD32F2F),
    mouseDown: const Color(0xFFB71C1C),
    iconNormal: const Color(0xFF805306),
    iconMouseOver: Colors.white);

class WindowButtons extends StatelessWidget {
  const WindowButtons({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        MinimizeWindowButton(colors: buttonColors),
        MaximizeWindowButton(colors: buttonColors),
        CloseWindowButton(colors: closeButtonColors),
      ],
    );
  }
}
