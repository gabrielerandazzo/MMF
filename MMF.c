#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DISPLAYS 8
typedef struct {
    CGDirectDisplayID id;
    CGFloat minX;
    CGFloat minY;
} DisplayInfo;

int compareDisplays(const void *a, const void *b) {
    DisplayInfo *da = (DisplayInfo *)a;
    DisplayInfo *db = (DisplayInfo *)b;
    
    if (da->minY < db->minY - 100) return -1;  
    if (da->minY > db->minY + 100) return 1;
    
    if (da->minX < db->minX) return -1;
    if (da->minX > db->minX) return 1;
    return 0;
}

AXUIElementRef findTopWindowOnDisplay(CGDirectDisplayID displayID) {
    CGRect displayBounds = CGDisplayBounds(displayID);
    CGPoint displayCenter = CGPointMake(
        displayBounds.origin.x + displayBounds.size.width / 2,
        displayBounds.origin.y + displayBounds.size.height / 2
    );
    
    CFArrayRef windowList = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
        kCGNullWindowID
    );
    
    if (!windowList) {
        return NULL;
    }
    
    pid_t targetPID = -1;
    CGFloat highestLevel = -INFINITY;
    
    CFIndex count = CFArrayGetCount(windowList);
    for (CFIndex i = 0; i < count; i++) {
        CFDictionaryRef window = CFArrayGetValueAtIndex(windowList, i);
        
        CFStringRef ownerName = CFDictionaryGetValue(window, kCGWindowOwnerName);
        if (!ownerName) continue;
        
        char name[256];
        CFStringGetCString(ownerName, name, sizeof(name), kCFStringEncodingUTF8);
        
        if (strcmp(name, "Window Server") == 0 || 
            strcmp(name, "Dock") == 0 ||
            strcmp(name, "SystemUIServer") == 0 ||
            strcmp(name, "ControlCenter") == 0) {
            continue;
        }
        
        CFDictionaryRef boundsDict = CFDictionaryGetValue(window, kCGWindowBounds);
        CGRect windowRect;
        if (!CGRectMakeWithDictionaryRepresentation(boundsDict, &windowRect)) {
            continue;
        }
        
        if (windowRect.size.width < 50 || windowRect.size.height < 50) {
            continue;
        }
        
        CGFloat overlapArea = 0;
        CGRect intersection = CGRectIntersection(windowRect, displayBounds);
        if (!CGRectIsNull(intersection)) {
            overlapArea = intersection.size.width * intersection.size.height;
            CGFloat windowArea = windowRect.size.width * windowRect.size.height;
            if (overlapArea / windowArea < 0.5) {
                continue;
            }
        } else {
            continue;
        }
        
        CFNumberRef layerNum = CFDictionaryGetValue(window, kCGWindowLayer);
        int layer = 0;
        if (layerNum) {
            CFNumberGetValue(layerNum, kCFNumberIntType, &layer);
        }
        
        if (layer > 100) {
            continue;
        }
        
        if (layer == 0 && targetPID == -1) {
            CFNumberRef pidNum = CFDictionaryGetValue(window, kCGWindowOwnerPID);
            if (pidNum) {
                pid_t pid;
                CFNumberGetValue(pidNum, kCFNumberIntType, &pid);
                highestLevel = layer;
                targetPID = pid;
                break; 
            }
        }
    }
    
    CFRelease(windowList);
    
    if (targetPID == -1) {
        return NULL;
    }
    
    AXUIElementRef app = AXUIElementCreateApplication(targetPID);
    return app;
}

bool focusApplication(AXUIElementRef app) {
    if (!app) return false;
    AXError error = AXUIElementPerformAction(app, kAXRaiseAction); 
    if (error != kAXErrorSuccess) {
        CFBooleanRef trueValue = kCFBooleanTrue;
        AXUIElementSetAttributeValue(app, kAXFrontmostAttribute, trueValue);
    }
       
    return false;
}

void moveFocusToNextScreen() {
    CGDisplayCount displayCount;
    CGDirectDisplayID displays[MAX_DISPLAYS];
    CGGetActiveDisplayList(MAX_DISPLAYS, displays, &displayCount);
    
    if (displayCount < 2) {
        return;
    }
    
    
    DisplayInfo displayInfos[MAX_DISPLAYS];

    for (int i = 0; i < displayCount; i++) {
        CGRect bounds = CGDisplayBounds(displays[i]);
        displayInfos[i].id = displays[i];
        displayInfos[i].minX = bounds.origin.x;
        displayInfos[i].minY = bounds.origin.y;
    }
    
    qsort(displayInfos, displayCount, sizeof(DisplayInfo), compareDisplays);
    
    CGEventRef event = CGEventCreate(NULL);
    CGPoint mouseLocation = CGEventGetLocation(event);
    CFRelease(event);
    
    int currentDisplayIndex = -1;
    for (int i = 0; i < displayCount; i++) {
        CGRect displayBounds = CGDisplayBounds(displayInfos[i].id);
        if (CGRectContainsPoint(displayBounds, mouseLocation)) {
            currentDisplayIndex = i;
            break;
        }
    }
    
    if (currentDisplayIndex == -1) {
        currentDisplayIndex = 0;
    }
    
    int nextDisplayIndex = (currentDisplayIndex + 1) % displayCount;
    
    CGDirectDisplayID nextDisplay = displayInfos[nextDisplayIndex].id;
    CGRect nextDisplayBounds = CGDisplayBounds(nextDisplay);
    
    CGPoint newMouseLocation = CGPointMake(
        nextDisplayBounds.origin.x + nextDisplayBounds.size.width / 2,
        nextDisplayBounds.origin.y + nextDisplayBounds.size.height / 2
    );
    
    CGEventRef mouseMove = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, newMouseLocation, kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, mouseMove);
    CFRelease(mouseMove);
    
    
    AXUIElementRef targetApp = findTopWindowOnDisplay(nextDisplay);

    if (targetApp) {
        usleep(50000); 
        focusApplication(targetApp);
    }
}

int main() {
    moveFocusToNextScreen();
    return 0;
}
