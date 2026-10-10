// Kameraa macOS Camera Extension (CMIOExtension) — consumer side skeleton.
//
// This is a System Extension that publishes "Kameraa Virtual Camera" to macOS, so
// apps (FaceTime/Zoom/Teams/Meet/browsers) list it like a webcam. It reads frames
// from the POSIX shared-memory region the Kameraa app writes to (name must match
// kPosixChannelName in src/output/VirtualCamShared.h: "/KameraaVCam").
//
// BUILD/DEPLOY (not part of the CMake build — use Xcode):
//  - Add a "Camera Extension" target; embed it in the Kameraa app bundle.
//  - Entitlements: com.apple.developer.system-extension.install + App Groups.
//  - Code-sign + notarize (Apple Developer account required for distribution).
//  - The app activates it via OSSystemExtensionManager; the user approves it in
//    System Settings > Privacy & Security.
// See docs/05-PLATFORM-INTEGRATION.md.

import CoreMediaIO
import Foundation
import IOKit

private let kChannelName = "/KameraaVCam"
private let kMagic: UInt32 = 0x414D_524B  // 'KMRA'
private let kHeaderSize = 32               // 8 x UInt32
private let kMaxWidth = 1920
private let kMaxHeight = 1080

// Maps the app's POSIX shared-memory frame channel and exposes the latest frame.
final class SharedChannelReader {
    private var base: UnsafeMutableRawPointer?
    private let capacity = kHeaderSize + kMaxWidth * kMaxHeight * 3

    func open() -> Bool {
        if base != nil { return true }
        let fd = shm_open(kChannelName, O_RDONLY, 0)
        if fd < 0 { return false }
        defer { close(fd) }
        guard let p = mmap(nil, capacity, PROT_READ, MAP_SHARED, fd, 0),
              p != MAP_FAILED else { return false }
        base = p
        return true
    }

    // Returns (width, height, BGR bytes) of the current frame, or nil if none.
    func latest() -> (Int, Int, Data)? {
        if base == nil && !open() { return nil }
        guard let b = base else { return nil }
        let hdr = b.bindMemory(to: UInt32.self, capacity: 8)
        if hdr[0] != kMagic || hdr[6] == 0 { return nil }  // magic / sequence
        let w = Int(hdr[2]), h = Int(hdr[3])
        if w <= 0 || h <= 0 || w > kMaxWidth || h > kMaxHeight { return nil }
        let data = Data(bytes: b.advanced(by: kHeaderSize), count: w * h * 3)
        return (w, h, data)
    }
}

// NOTE: The CMIOExtensionProvider/Device/Stream implementation (publishing the
// device, negotiating the format, and calling stream.send(_:) with CMSampleBuffers
// built from SharedChannelReader.latest()) follows Apple's Camera Extension
// template. It is created and wired in Xcode from the template; the frame source
// above is the Kameraa-specific part. See:
// https://developer.apple.com/documentation/coremediaio/creating-a-camera-extension-with-core-media-i-o
