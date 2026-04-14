#include "Common.h"
#include <QString>
#include <QLabel>
#include <QLineEdit>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "UI/ObjectRenderer.h"

QString ICommonFunc::GetRefreshedName(const char* BaseName, uint32_t FoundCount, uint32_t TotalCount)
{
    QString res = BaseName;
    res += " (" + QString::number(FoundCount) + " / " + QString::number(TotalCount) + ")";
    
    return res;
};



#pragma region CustomTreeWidget

CustomTreeWidget::CustomTreeWidget(QString TreeName, int MaxWidth, QWidget* Parent) : QWidget(Parent)
{
    this->setMaximumWidth(MaxWidth);

    // Label
    this->Label = new QLabel(TreeName);
    
    // Search bar
    this->SearchBar = new QLineEdit();
    this->SearchBar->setPlaceholderText("Find...");

    // Expand button
    this->ToggleButton = new QPushButton("Expand All", this);
    this->ToggleButton->setCheckable(true);

    // Tree widget
    this->List = new QTreeWidget();
    this->List->setHeaderHidden(true);

    // Main layout
    this->MainLayout = new QVBoxLayout(this);
    this->MainLayout->addWidget(this->Label);
    this->MainLayout->addWidget(this->SearchBar);
    this->MainLayout->addWidget(this->ToggleButton);
    this->MainLayout->addWidget(this->List);
    this->MainLayout->setStretch(0, 0);     // No stretch for label
    this->MainLayout->setStretch(1, 0);     // No stretch for search bar
    this->MainLayout->setStretch(2, 1);     // Tree list take all the remaining space

    // Widget behavior
    QObject::connect(this->ToggleButton, &QPushButton::clicked, this, [&]()
        {
            this->IsExpanded = !this->IsExpanded;
            this->ToggleButton->setText(this->IsExpanded ? "Collapse All" : "Expand All");
            this->OnToggleExpandCollapse(this->List, this->IsExpanded);
        });

    QObject::connect(this->SearchBar, &QLineEdit::textChanged, [&](const QString& text) {
        this->FilterTree(text);
    });

}


void CustomTreeWidget::OnToggleExpandCollapse(QTreeWidget* TreeWidget, bool Expand)
{
    if (!Expand)
    {
        TreeWidget->collapseAll();
    }
    else
    {
        TreeWidget->expandAll();
    }
}


void CustomTreeWidget::FilterTree(const QString& SearchText)
{
    for (int i = 0; i < this->List->topLevelItemCount(); ++i)
    {   // Browse all tree item from the top

        CommonBaseItemTree* parentItem = (CommonBaseItemTree*)this->List->topLevelItem(i);
        bool parentVisible = false;

        for (int j = 0; j < parentItem->childCount(); ++j)
        {   // Browse all current item children

            CommonBaseItemTree* childItem = (CommonBaseItemTree*)parentItem->child(j);

            if (childItem->childCount() > 0)
            {   // The current child has also some child

                CommonBaseItemTree* childItem2 = (CommonBaseItemTree*)childItem->child(0);

                if (childItem2->GetTotalObjectAvailable() > 0)
                {   // The item is not excluded

                    bool match = childItem2->text(0).contains(SearchText, Qt::CaseInsensitive);

                    // Hide or unhide this part of the tree
                    childItem->setHidden(!match);
                    childItem2->setHidden(!match);
                    parentVisible |= match;
                }
                else
                {
                    childItem2->setHidden(true);
                }
            }
            else
            {   // The current child has no child

                if (childItem->GetTotalObjectAvailable() > 0)
                {
                    bool match = childItem->text(0).contains(SearchText, Qt::CaseInsensitive);

                    // Hide or unhide this part of the tree
                    childItem->setHidden(!match);
                    parentVisible |= match;
                }
                else
                {
                    childItem->setHidden(true);
                }
            }
        }

        bool matchParent = parentItem->text(0).contains(SearchText, Qt::CaseInsensitive);
        parentItem->setHidden(!parentVisible && !matchParent);
    }
}

#pragma endregion // CustomTreeWidget