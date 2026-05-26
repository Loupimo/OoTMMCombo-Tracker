#include <QtWidgets/QApplication>
#include "UI/AppStyle.h"
#include "UI/ObjectRenderer.h"
#include "UI/SceneEntranceUpdate.h"
#include "Combo/Entrances.h"
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

    // Register SceneEntranceUpdate so Qt's queued connections can copy it
    // across threads (the EntranceHelper emits from the MemoryReader thread
    // to slots living in the main thread). Without this, queued delivery
    // would fail at runtime with "Cannot queue arguments of type ...".
    qRegisterMetaType<SceneEntranceUpdate>("SceneEntranceUpdate");

    // Force the Fusion style so the QSS is fully honored on every platform.
    // Without this, Windows 11's native theme bleeds through some controls
    // (notably QTabBar selection indicators), producing accent colors that
    // do not match the stylesheet.
    QApplication::setStyle("Fusion");

    // Set up the icon
    a.setWindowIcon(QIcon("./Resources/Logo.ico"));

   /* if (IsDarkMode())
        SetDarkPalette(a);*/
    a.setStyleSheet(GetDarkStyle());

    // Populate the default intra-scene travel costs on every OoT / MM entrance
    // entry so the GPS pathfinder can rely on EntranceMetaInfo::Cost from now on.
    InitializeEntranceCosts();

    // Create the main window
    MainWindow = new OoTMMComboTracker();
    MainWindow->showMaximized();

    // Start the main loop
    int ret = a.exec();

    delete MainWindow;
    return ret;
}
