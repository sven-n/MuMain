#import <AppKit/AppKit.h>

int main(int argc, const char* argv[])
{
    @autoreleasepool
    {
        if (argc != 4)
        {
            return 1;
        }

        NSString* input = [NSString stringWithUTF8String:argv[1]];
        NSString* output = [NSString stringWithUTF8String:argv[2]];
        NSInteger size = [[NSString stringWithUTF8String:argv[3]] integerValue];
        NSImage* image = [[NSImage alloc] initWithContentsOfFile:input];
        if (image == nil)
        {
            return 2;
        }

        NSBitmapImageRep* bitmap = [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:NULL
                          pixelsWide:size
                          pixelsHigh:size
                       bitsPerSample:8
                     samplesPerPixel:4
                            hasAlpha:YES
                            isPlanar:NO
                      colorSpaceName:NSDeviceRGBColorSpace
                         bytesPerRow:0
                        bitsPerPixel:0];
        bitmap.size = NSMakeSize(size, size);

        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:bitmap]];
        [[NSColor clearColor] setFill];
        NSRectFillUsingOperation(NSMakeRect(0, 0, size, size), NSCompositingOperationCopy);
        [image drawInRect:NSMakeRect(0, 0, size, size)
                 fromRect:NSZeroRect
                operation:NSCompositingOperationSourceOver
                 fraction:1.0];
        [NSGraphicsContext restoreGraphicsState];

        NSData* png = [bitmap representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
        return [png writeToFile:output atomically:YES] ? 0 : 3;
    }
}
