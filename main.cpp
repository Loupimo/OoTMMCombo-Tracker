#include <QtWidgets/QApplication>
#include "UI/OoTMMComboTracker.h"
#include "UI/ObjectRenderer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OoTMMComboTracker w;
    w.showMaximized();
    return a.exec();
}
