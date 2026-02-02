#pragma once


#include <CoreFoundation/CoreFoundation.h>
#include <string>

// Function to find a resource path within the app bundle
inline std::string getResourcePath(const std::string& fileName, const std::string& fileType) {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFStringRef fileNameCF = CFStringCreateWithCString(NULL, fileName.c_str(), kCFStringEncodingUTF8);
    CFStringRef fileTypeCF = CFStringCreateWithCString(NULL, fileType.c_str(), kCFStringEncodingUTF8);
    CFURLRef fileURL = CFBundleCopyResourceURL(mainBundle, fileNameCF, fileTypeCF, NULL);

    if (!fileURL) {
        // Handle error: file not found in bundle
        CFRelease(fileNameCF);
        CFRelease(fileTypeCF);
        return "";
    }

    CFStringRef filePathCF = CFURLCopyFileSystemPath(fileURL, kCFURLPOSIXPathStyle);
    CFIndex length = CFStringGetLength(filePathCF);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
    std::string filePath(maxSize, '\0');
    CFStringGetCString(filePathCF, &filePath[0], maxSize, kCFStringEncodingUTF8);

    CFRelease(fileURL);
    CFRelease(filePathCF);
    CFRelease(fileNameCF);
    CFRelease(fileTypeCF);

    // Trim null terminator
    filePath.pop_back();

    return filePath;
}