//
//  SimpleCamViewController.swift
//  CamIo3rd
//
//  Created by Huiying Shen on 5/6/19.
//  Copyright © 2019 Huiying Shen. All rights reserved.
//

import AVFoundation

class SimpleCamViewController:UIViewController,AVCaptureVideoDataOutputSampleBufferDelegate{
    var isVga = false
    override func viewDidLoad() {
        super.viewDidLoad()
        
        setupCamera()
    }
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        session.startRunning()
    }
    
    lazy var session: AVCaptureSession = {
        let s = AVCaptureSession()
        if isVga{
            s.sessionPreset = .vga640x480

        }else {
//            s.sessionPreset = .hd1280x720
            s.sessionPreset = .hd1920x1080
//            s.sessionPreset = .hd4K3840x2160  // no video
        }
        return s
    }()
    var captureDevice: AVCaptureDevice?
//    func setFocalLenth(_ length: Float, device: AVCaptureDevice){
//        do {  //length 0 ~ 1.0, where 1.0 -> infinity?
//            try device.lockForConfiguration()
//            device.focusMode = AVCaptureDevice.FocusMode.locked
//            device.setFocusModeLocked(lensPosition: length, completionHandler: { (time) -> Void in
//                print("camera focal length set to: \(length)")
//           })
//            device.unlockForConfiguration()
//        } catch {
//            // Handle errors here
//            print("There was an error focusing the device's camera")
//        }
//    }
    
    func setupCamera() {
        let availableCameraDevices = AVCaptureDevice.DiscoverySession(deviceTypes: [.builtInWideAngleCamera], mediaType: AVMediaType.video, position: .back)
//        var activeDevice: AVCaptureDevice?
        for device in availableCameraDevices.devices as [AVCaptureDevice]{
            if device.position == .back {
                captureDevice = device
                
                break
            }
        }

        //activeDevice?.set(frameRate: frameRate)
        do {
            let camInput = try AVCaptureDeviceInput(device: captureDevice!)
//            print("activeDevice!.focusMode = \(activeDevice!.focusMode)")
//            camInput.device.autoFocusRangeRestriction = .far  // this will crash the app -- Huiying 08/23/2021
            if session.canAddInput(camInput) {
                session.addInput(camInput)
                
            }
        } catch {print("no camera") }
        let videoOutput = AVCaptureVideoDataOutput()
        videoOutput.alwaysDiscardsLateVideoFrames = true
        videoOutput.setSampleBufferDelegate(self, queue: DispatchQueue(label: "buffer queue", qos: .userInteractive, attributes: .concurrent, autoreleaseFrequency: .inherit, target: nil))
        
        if session.canAddOutput(videoOutput) {
            session.addOutput(videoOutput)
            // enable getting camera intrinsic matrix (and distortion coefficient?) delivery
            for c in videoOutput.connections{
                if c.isCameraIntrinsicMatrixDeliverySupported{
                    c.isCameraIntrinsicMatrixDeliveryEnabled = true
                }
            }
        }
    }
    
    func captureOutput(_ output: AVCaptureOutput, didOutput sampleBuffer: CMSampleBuffer, from connection: AVCaptureConnection) {
        DispatchQueue.main.async {
            //            let ct0 = CACurrentMediaTime()
            guard let _ = CMSampleBufferGetImageBuffer(sampleBuffer) else {return}
            
        }
    }
}
