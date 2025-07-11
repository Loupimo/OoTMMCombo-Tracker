#include "UI/ObjectRenderer.h"
#include "UI/AppStyle.h"

bool IsDarkMode()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    QVariant value = settings.value("AppsUseLightTheme");
    if (value.isValid() && value.toInt() == 0)
        return true; // Dark mode
    return false;    // Light mode
}


void SetDarkPalette(QApplication& app)
{
    QPalette darkPalette;
    
    darkPalette.setColor(QPalette::WindowText, Qt::white);

    app.setPalette(darkPalette);
}


QString GetDarkStyle()
{

    // Charger le fichier QSS
    QFile file("./Resources/Styles/darkstyle.qss");

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        return stream.readAll();
    }
    else {
        qDebug() << "Impossible de charger le fichier QSS";
        return QString();
    }

    /*
    QString darkStyle = R"(
    QWidget {
        background-color: #353535;
        color: #ffffff;
    }

    QLineEdit {
        background-color: #2d2d2d;
        color: #ffffff;
        border: 1px solid #555555;
        padding: 4px;
        selection-background-color: #555555;
        selection-color: #ffffff;
    }

    QTextEdit, QPlainTextEdit {
        background-color: #2d2d2d;
        color: #ffffff;
        border: 1px solid #555555;
        padding: 4px;
        selection-background-color: #555555;
        selection-color: #ffffff;
    }

    QComboBox {
        background-color: #2d2d2d;
        color: #ffffff;
        border: 1px solid #555555;
    }

    QComboBox QAbstractItemView {
        background-color: #2d2d2d;
        color: #ffffff;
        selection-background-color: #555555;
    }

    QPushButton {
        background-color: #444444;
        border: 1px solid #555555;
        padding: 5px;
    }
    QPushButton:hover {
        background-color: #555555;
    }
    QPushButton:pressed {
        background-color: #666666;
    }

    QHeaderView::section {
        background-color: #444444;
        color: #ffffff;
        padding: 4px;
        border: 1px solid #555555;
    }

    QTabWidget::pane {
        border: 1px solid #555555;
        top: -1px;
        background-color: #353535;
    }
    QTabBar::tab {
        background: #444444;
        border: 1px solid #555555;
        padding: 6px;
    }
    QTabBar::tab:selected {
        background: #555555;
        margin-bottom: -1px;
    }
    QTabBar::tab:hover {
        background: #666666;
    }

    QGroupBox {
        border: 1px solid #555555;
        margin-top: 6px;
    }
    QGroupBox:title {
        subcontrol-origin: margin;
        subcontrol-position: top center;
        padding: 0 3px;
    }

    QFrame {
        border: 1px solid #555555;
    }

    QScrollBar:vertical {
        background: #2d2d2d;
        width: 12px;
        margin: 15px 3px 15px 3px;
        border: 1px solid #555555;
    }
    QScrollBar::handle:vertical {
        background: #555555;
        min-height: 20px;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        background: #444444;
        height: 15px;
        subcontrol-origin: margin;
    }
    QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {
        width: 10px;
        height: 10px;
        background: white;
    }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
        background: none;
    }

    QTreeView::branch:has-children:!has-siblings:closed,
    QTreeView::branch:closed:has-children:has-siblings {
        border-image: none;
        image: url(:/icons/arrow-right-white.png);
    }

    QTreeView::branch:open:has-children:!has-siblings,
    QTreeView::branch:open:has-children:has-siblings  {
        border-image: none;
        image: url(:/icons/arrow-down-white.png);
    }

    QToolTip {
        background-color: #2d2d2d; 
        color: #f0f0f0;            
        border: 1px solid #555555;
        padding: 4px;
        border-radius: 4px;
    }

)";
    return darkStyle;*/
}