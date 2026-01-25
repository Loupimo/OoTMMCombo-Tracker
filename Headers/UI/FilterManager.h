#pragma once

#include <QToolButton>
#include <QCheckBox>
#include "Combo/Objects.h"
#include "UI/GameTab.h"


struct ObjectTypeEntry {
    ObjectType type;
    QString name;
};


class FilterItemWidget : public QCheckBox {
    Q_OBJECT


#pragma region Attributes

public:
   /*Label* IconLabel;
    QLabel* TextLabel;
    QLabel* CheckLabel;*/
    ObjectType AssociatedType;

#pragma endregion

public:
    explicit FilterItemWidget(const QIcon& Icon, const QString& Text, ObjectType ObjType, QWidget* Parent = nullptr) : QCheckBox(Parent) {

       /* this->setStyleSheet(R"(
            QCheckBox::indicator {
                subcontrol-position: right center;
            }
        )");*/
        
        this->setAttribute(Qt::WA_NoMousePropagation);
        this->setText(Text);
        this->setIcon(Icon);
        this->AssociatedType = ObjType;
        this->setCheckable(true);
        this->setChecked(true);

        this->setCursor(Qt::PointingHandCursor);
    }
};

class FilterManager : public QToolButton
{
    Q_OBJECT

#pragma region Attributes

public:

    GameTab * TabOwner = nullptr;                   // A reference to the owning game tab
    const QSet<ObjectType>* FilterTypes = nullptr;  // The list of associated filtering types
    QSet<ObjectType> ActiveFilter;                  // The list of all active object types

public:

    FilterManager(GameTab * TabRef = nullptr);

    void ToggleActiveType(ObjectType Target);

signals:
    void filterChanged();

};
