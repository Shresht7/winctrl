#ifndef FEATURES_H
#define FEATURES_H

class Feature
{
public:
    static bool isWinCtrlEnabled;
    static bool Move;
    static bool Resize;
    static bool Transparency;
    static bool VirtualDesktopScroll;

    static void toggleWinCtrlEnabled();
    static void toggleMove();
    static void toggleResize();
    static void toggleTransparency();
    static void toggleVirtualDesktopScroll();
};

#endif // FEATURES_H
