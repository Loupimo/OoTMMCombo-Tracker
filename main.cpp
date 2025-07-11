#include <QtWidgets/QApplication>
#include "UI/AppStyle.h"
#include "UI/OoTMMComboTracker.h"
#include "UI/ObjectRenderer.h"



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
    OoTMMComboTracker w;
    w.showMaximized();

    // Start the main loop
    return a.exec();
}
