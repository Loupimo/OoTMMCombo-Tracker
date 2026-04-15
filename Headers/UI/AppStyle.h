#pragma once

#include <QtWidgets/QApplication>
#include <QSettings>
#include <QDebug>

bool IsDarkMode();
void SetDarkPalette(QApplication& app);
QString GetDarkStyle();