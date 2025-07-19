#include <QtWidgets/QApplication>
#include "UI/AppStyle.h"
#include "UI/ObjectRenderer.h"
#include "main.h"

OoTMMComboTracker* MainWindow = nullptr;

OoTMMComboTracker* GetMainWindow()
{
    return MainWindow;
}

int main(int argc, char *argv[])
{
    // Create the application
    QApplication a(argc, argv);

    // Set up the icon
    a.setWindowIcon(QIcon("./Resources/Logo.ico"));
    
    if (IsDarkMode())
        SetDarkPalette(a);
    a.setStyleSheet(GetDarkStyle());

    // Create the main window
    MainWindow = new OoTMMComboTracker();
    MainWindow->showMaximized();

    // Start the main loop
    int ret = a.exec();

    delete MainWindow;
    return ret;
}
