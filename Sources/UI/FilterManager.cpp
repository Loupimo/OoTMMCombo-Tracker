#include <QWidgetAction>
#include <QMenu>
#include <QStyle>

#include "UI/ObjectRenderer.h"
#include "UI/FilterManager.h"


/*void FilterItemWidget::paintEvent(QPaintEvent*)
{
    QStyleOptionMenuItem opt;
    opt.initFrom(this);
    opt.state |= QStyle::State_Enabled;

    if (underMouse())
        opt.state |= QStyle::State_Selected;

    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_PanelMenu, &opt, &p, this);

    QWidget::paintEvent(nullptr);
}*/



FilterManager::FilterManager(GameTab* TabRef) : QToolButton(TabRef)
{
	this->TabOwner = TabRef;
    this->setIcon(QIcon("./Resources/Common/Filter.png"));
    this->setPopupMode(QToolButton::InstantPopup);
    QMenu* menu = new QMenu(this);

	if (TabRef != nullptr)
	{
		if (TabRef->GameID == OOT_GAME)
		{	// OoT filter

            this->FilterTypes = &OoTTypes;
		}
		else
		{	// MM filter
			
            this->FilterTypes = &MMTypes;
		}
	}

    QList<ObjectType> tmp = this->FilterTypes->values();

    QList<ObjectTypeEntry> entries;

    for (ObjectType type : this->FilterTypes->values())
    {
        entries.append({ type, ObjTypeName[type] });
    }

    std::sort(entries.begin(), entries.end(),
        [](const ObjectTypeEntry& a, const ObjectTypeEntry& b) {
            return QString::localeAwareCompare(a.name, b.name) < 0;
        }
    );

    QIcon* icons = ObjectIcons::GetObjectIcons();

    for (const auto& entry : entries)
    {
        QWidgetAction* action = new QWidgetAction(menu);
        auto widget = new FilterItemWidget(icons[entry.type], entry.name, entry.type);
        action->setDefaultWidget(widget);
        menu->addAction(action);

        QObject::connect(widget, &FilterItemWidget::clicked, this, [=]() mutable {
            this->ToggleActiveType(widget->AssociatedType);
        });

        // Activate the type by default
        this->ToggleActiveType(widget->AssociatedType);
    }

    this->setMenu(menu);

    QObject::connect(menu, &QMenu::aboutToHide, this, &FilterManager::filterChanged);
}


void FilterManager::ToggleActiveType(ObjectType Target)
{
    if (this->ActiveFilter.contains(Target))
    {   // We need to remove it

        this->ActiveFilter.remove(Target);
    }
    else
    {   // We need to add it

        this->ActiveFilter.insert(Target);
    }
}