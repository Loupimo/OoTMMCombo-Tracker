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
    /*
    *   Constructs the filter item widget with the given appearance and associated object type.
    *
    *   @param Icon      The icon shown next to the filter label.
    *   @param Text      The text label of the filter entry.
    *   @param ObjType   The object type associated with this filter entry.
    *   @param Parent    The potential parent to attach this class to.
    */
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
    QMap<uint32_t, QSet<ObjectInfo*>> ExcludedObj;  // The list of all excluded object ID per scene

public:

    /*
    *   Constructs the filter manager for the given game tab.
    *
    *   @param TabRef    The owning game tab, used to pick the right filter type list (OoT or MM).
    */
    FilterManager(GameTab * TabRef = nullptr);

    /*
    *   Default destructor. Clears active filters and excluded objects.
    */
    ~FilterManager();

    /*
    *   Toggle the active state of the given object type in the filter.
    *
    *   @param Target    The object type to toggle (removed if active, added otherwise).
    */
    void ToggleActiveType(ObjectType Target);

    /*
    *   Exclude the given object from the tracker view.
    *
    *   @param ToExclude    The object to exclude from the tracker view.
    */
    void ExcludeNewObject(ObjectInfo* ToExclude);

    /*
    *   Clear the list of excluded objects for all scenes.
    */
    void ResetExcludedObject();

    /*
    *   Check if the given object is currently excluded from the tracker view.
    *
    *   @param Target    The object to check.
    *
    *   @return True if the object is excluded, false otherwise.
    */
    bool IsObjectExcluded(ObjectInfo* Target);

signals:
    void filterChanged();

};
