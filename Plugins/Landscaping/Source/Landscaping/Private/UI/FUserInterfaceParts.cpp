// Copyright (c) 2021-2023 Josef Prenner
// support@ludicdrive.com
// ludicdrive.com
// All Rights Reserved

#include "FUserInterfaceParts.h"

TSharedRef<SVerticalBox> FUserInterfaceParts::SHeader1(FString InHeader, FString InHeaderLabel)
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        [
            SNew(STextBlock)
            .Text_Lambda([=]()
            {
                return FText::FromString(InHeaderLabel);
            })
        ]

        + SVerticalBox::Slot()
        .Padding(0.0f, 5.0f)
        [
            SNew(SHeader)
            .Content()
            [
                SNew(STextBlock)
                .Text_Lambda([=]()
            {
                return FText::FromString(InHeader);
            })
            ]
        ]
    ;
}
