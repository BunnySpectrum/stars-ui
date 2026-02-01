#pragma once

#include "lcars.h"

class TacticalPanel : public LCARSPanel {
public:
    const char* GetTitle() const override;
    void DrawUpper() override;
    void DrawLower() override;
};
