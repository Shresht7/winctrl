#include "features.h"

bool Feature::isWinCtrlEnabled = true;
bool Feature::Move = true;
bool Feature::Resize = true;
bool Feature::Transparency = true;
bool Feature::VirtualDesktopScroll = true;

void Feature::toggleWinCtrlEnabled() { isWinCtrlEnabled = !isWinCtrlEnabled; }
void Feature::toggleMove() { Move = !Move; }
void Feature::toggleResize() { Resize = !Resize; }
void Feature::toggleTransparency() { Transparency = !Transparency; }
void Feature::toggleVirtualDesktopScroll() { VirtualDesktopScroll = !VirtualDesktopScroll; }
