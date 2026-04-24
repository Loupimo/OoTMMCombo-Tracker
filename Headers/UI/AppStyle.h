#pragma once

#include <QtWidgets/QApplication>
#include <QSettings>
#include <QDebug>

/*
*   Detect if the current Windows system theme is using dark mode.
*
*   @return True if the system is in dark mode, false otherwise.
*/
bool IsDarkMode();

/*
*   Apply a custom dark palette on the given Qt application.
*
*   @param App    The Qt application to apply the dark palette on.
*/
void SetDarkPalette(QApplication& app);

/*
*   Load the dark QSS stylesheet from the resources folder.
*
*   @return The content of the dark stylesheet, or an empty string if the file could not be opened.
*/
QString GetDarkStyle();