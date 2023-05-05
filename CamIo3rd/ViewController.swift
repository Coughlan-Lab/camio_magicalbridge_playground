//
//  ViewController.swift
//  CamIoAgain
//
//  Created by Huiying Shen on 2/14/19.
//  Copyright © 2019 Huiying Shen. All rights reserved.
//

import UIKit
import AVFoundation
import Vision
import Speech


class PickerViewController: SimpleCamViewController{
    let camIoWrapper = CamIoWrapper()
    let stylusChoices = ["2 in cube","3 in cube"]
    var iStylus = -1
    func numberOfComponents(in pickerView: UIPickerView) -> Int {
        return 1
    }
    var pickerView = UIPickerView()
    
    override func viewDidLoad() {
        super.viewDidLoad()
            setPickview()
    }
    func setPickview(){
        pickerView = UIPickerView()
        pickerView.isHidden = true
        pickerView.backgroundColor = .lightGray
        pickerView.dataSource = self
        pickerView.delegate = self
        pickerView.frame = CGRect(x:100,y:50,width:160,height:50);
        view.addSubview(pickerView)
    }
}

extension PickerViewController: UIPickerViewDelegate, UIPickerViewDataSource {
    func pickerView(_ pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
        return stylusChoices.count // number of dropdown items
    }
    func pickerView(_ pickerView: UIPickerView, titleForRow row: Int, forComponent component: Int) -> String? {
        return stylusChoices[row] // dropdown item
    }
    func pickerView(_ pickerView: UIPickerView, didSelectRow row: Int, inComponent component: Int) {
        iStylus = row
        camIoWrapper.setStylusCube(Int32(iStylus))
        pickerView.isHidden = true
    }
    func pickerView(_ pickerView: UIPickerView, attributedTitleForRow row: Int, forComponent component: Int) -> NSAttributedString? {
//        print("pickerView(): row = \(row)")
        iStylus = row
        camIoWrapper.setStylusCube(Int32(iStylus))
        return NSAttributedString(string: stylusChoices[row], attributes: [NSAttributedString.Key.foregroundColor: UIColor.blue])
    }
}

class ButtonLabelViewController: PickerViewController{
    var buttons = [UIButton]()
    var labels = [UILabel]()
    
    
    func addButton(x:Int, y: Int, w: Int, h: Int, title: String, color: UIColor, selector: Selector) -> UIButton{
        let btn =  UIButton()
        btn.frame = CGRect (x:x, y:y, width:w, height:h)
        btn.setTitle(title, for: UIControl.State.normal)
        btn.setTitleColor(color, for: .normal)
        btn.backgroundColor = .lightGray
        btn.addTarget(self, action: selector, for: UIControl.Event.touchUpInside)
        self.view.addSubview(btn)
        buttons.append(btn)
        return btn
    }
    
    func addLabel(x:Int, y: Int, w: Int, h: Int, text: String, color: UIColor) -> UILabel{
        let label =  UILabel()
        label.frame = CGRect (x:x, y:y, width:w, height:h)
        label.font = UIFont.preferredFont(forTextStyle: .body)
        label.textColor =  color
        label.backgroundColor = .lightGray
        label.text = text
        self.view.addSubview(label)
        labels.append(label)
        return label
    }
}

/*
 *****************************************************************************************************
 *
 class ViewController
 */
class ViewController: ButtonLabelViewController{

    override var supportedInterfaceOrientations: UIInterfaceOrientationMask { get {return .portrait} }
    
    private let context = CIContext()
    let audioManager = AudioManager()
    let imageView = UIImageView()
    let yBtn = 20
        
    func setupImageView(){
        view.addSubview(imageView)
        imageView.contentMode = .scaleAspectFit
        imageView.frame = CGRect(x:0 , y:0, width: view.bounds.width, height: view.bounds.height)
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        setupImageView()
        imageView.contentMode = .scaleAspectFit
        imageView.frame = CGRect(x:0 , y:0, width: view.bounds.width, height: view.bounds.height)
        let scale:Float = 1.016  //1.016 for the bronze
        loadPlaygroundData(scale)
        
        camIoWrapper.clearingYouAreHereBoundary()
        
//        for name in names4landmark.components(separatedBy: "\n"){
//            let s = camIoWrapper.getHighestP3f(name)
//            print("HighestP3f: \(name), \(s)")
//        }
        
        DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
            self.readLandmarkData()
        }

//        addSaveBtn()
//        addWsBtn()
        
        webSocketTask =  setWebSocket(ip_txt,port_txt)
        let notificationCenter = NotificationCenter.default
        notificationCenter.addObserver(self, selector: #selector(didBecomeActive), name: UIApplication.didBecomeActiveNotification, object: nil)
    }
    
    var landMarkRead = false
    let file4landmarkData = "landmark.txt"
    func readLandmarkData(){
        let dat = readFr(file4landmarkData)
        if dat.count > 0{
            camIoWrapper.setFeatureBaseImagePoints(dat)
        }
        
    }
    
    @objc func didBecomeActive(){
        self.pickerView.isHidden = false
//        self.poseBtn.isHidden = false
        camIoWrapper.setStylusCube(Int32(iStylus))
        print("camIoWrapper.setStylusCube(): iStylus = \(iStylus)")
        DispatchQueue.main.asyncAfter(deadline: .now() + 5) {
            self.pickerView.isHidden = true
//            self.poseBtn.isHidden = true
        }
    }
//
//    let names4landmark =    """
//                            Kinder Bells
//                            Rocking Horse
//                            You are Here at the Magic Map
//                            Playhouse
//                            Umbrella Pole
//                            Exercise Bike
//                            """

    func loadPlaygroundData(_ scaling:Float = 1.0){
        let (_,zones) = readPlaygroundObjFileOld()
        let (objs,_) = readPlaygroundObjFile()
        let idNameMapping = getIdNameMappingBoth().components(separatedBy: "\n")
        let objs_new = mappingLoops(objs,idNameMapping)
        let zones_new = mappingLoops(zones,idNameMapping)
        
        var nPnt:Int32 = 0
        for s in objs_new {
            let n = camIoWrapper.newRegion(s)
            print("n = \(n)")
            nPnt += n
        }
        for s in zones_new {
            let n = camIoWrapper.newZone(s)
            print("n = \(n)")
            nPnt += n
        }
        print("nPnt = \(nPnt)")
        
        load_mp3_files()
        
        camIoWrapper.scaleModel(scaling)
    }
    
    func mappingLoops(_ objs: [String], _ mapping: [String]) -> [String] {
        var objs_new = [String]()
        for id_name in mapping{
            let id_nm = id_name.components(separatedBy: "    ")
            for obj in objs {
                if obj.contains(id_nm[0]) {
                    if id_nm.count == 4 {
                        let out = obj.replacingOccurrences(of: id_nm[0], with: id_nm[1]+"\t"+id_nm[3])
                        objs_new.append(out)
                    } else {
                        let out = obj.replacingOccurrences(of: id_nm[0], with: id_nm[1])
                        objs_new.append(out)
                        break
                    }
                }
            }
        }
        return objs_new
    }

    func load_mp3_files(){
        let names = camIoWrapper.getRegionNames().components(separatedBy: "\n")
        print("mp3 not found: begin")
        var lst = [String]()
        var lst_des = [String]()
        for name in names{
            if name.count < 1 { continue } // skip empty line
            let rt = name.replacingOccurrences(of:" ",with:"_")
            let path = Bundle.main.path(forResource: "obj_names/" + rt, ofType: ".mp3")
            if path != nil{
                lst.append(name)
//                if name.contains("Climbing Loops"){
//                    print(name)
//                }
                audioManager.try_load_mp3(name, path:"obj_names/" + rt)
            }else{
                print(name)
            }
            let path_des = Bundle.main.path(forResource: "obj_des/" + rt + "_des", ofType: ".mp3")
            if path_des != nil {  // the obj has description
                lst_des.append(name + " des")
                audioManager.try_load_mp3(name + " des", path:"obj_des/" + rt + "_des")
            }
        }
        print("mp3 not found: end")
        for l in lst_des{
            print(l)
        }
        audioManager.show_obj_bufs()
        
        // special messages in mp3 files
        for msg in ["Welcome to the Magic Map","Stylus Straight Upright","This is an audio test"]{
            let rt = msg.replacingOccurrences(of:" ",with:"_")
            audioManager.try_load_mp3(msg, path: "special_mp3/" + rt)
        }
    }

    func readPlaygroundObjFile() -> ([String],[String]){
        var objs = [String](),zones=[String]()
//        let uNG = Bundle.main.url(forResource: "20210609 OPTICAL MAP", withExtension: "obj")
//        let fn_bronze = "20220612BRONZE"
//        let fn_sparse = "sparse"
        let uNG = Bundle.main.url(forResource: "sparse", withExtension: "obj")
        print(uNG?.path ?? "nil")
        do {
            let contents = try String(contentsOf: uNG!)
            let all_items = contents.components(separatedBy: "o ")
            for item in all_items[1...]{
                let lines = item.components(separatedBy: "\n")
                var obj = lines[0] + "\n"
                for line in lines[1...]{
                    if line.contains("vn ") {break}
                    obj += line + "\n "
                }
//                if !obj.contains("PLAYHOUSE_SPARSE"){continue}
                if obj.contains("GROUND_") {continue}
                if obj.contains("ZONE_") { zones.append(obj) }
                else{objs.append(obj)}
            }
        }
        catch {
                    // contents could not be loaded
        }
        return (objs,zones)
    }
    
    func readPlaygroundObjFileOld() -> ([String],[String]){
        let uNG = Bundle.main.url(forResource: "20210609 OPTICAL MAP", withExtension: "obj")
        var objs = [String](),zones=[String]()
        print(uNG?.path ?? "nil")
        do {
            let contents = try String(contentsOf: uNG!)
//                print(contents)
            let all_items = contents.components(separatedBy: "s off")
            print("all_items.count = \(all_items.count)")
            
            for item in all_items{
                let tmp = item.components(separatedBy: "o ")  // o is start of object name.
                if tmp.count > 1{
//                    print(tmp[1])
                    let i = tmp[1].firstIndex(of: "\n")
                    let name = String(tmp[1][...i!])
//                    print("\(name)")
                    if name.contains("GROUND_") {continue}
                    if name.contains("ZONE_") {
                        zones.append(tmp[1])
                        print(name)
                    }
                    else{ objs.append(tmp[1]) }
                }
                else{
                    print("tmp.count <=1 ")
                }
            }
        } catch {
                // contents could not be loaded
        }
        print("objs.count,zones.count = \(objs.count),\(zones.count)")
        return (objs,zones)
    }

    var btn = UIButton()

    func addWsBtn(){
        let x = 0, w = 110, h = 30, dy = 30
        var y = yBtn + dy
        _ = addButton(x:x, y:y, w:w, h:h, title: "WS Server", color: .blue, selector: #selector(setWebSocketServer))

    }
    var saveFrm = false
    var loopFrm = false
    var data_filename = ""

    var imgs = [UIImage]()
    var cntFrm = 0
    func save_frames(){
        if saveFrm && !loopFrm{
            imgs = [UIImage]()
        } else if !saveFrm {
            loopFrm = true
            cntFrm = 0
        } else {
            saveFrm = false
            loopFrm = false
        }
    }

    func delAllButtonsLabels(){
        buttons.forEach { (item) in
             item.removeFromSuperview()
        }
        
        labels.forEach { (item) in
             item.removeFromSuperview()
        }
    }

    var lastSent = Int64(Date().timeIntervalSince1970*1000.0)
    var videoPaused = false
    
    var frm = UIImage()
    var resultImage = UIImage()
    var camMat3val = ""
    func processFrame(_ image:UIImage) {
        resultImage = camIoWrapper.procImage(image)
        imageView.image = resultImage
        let txt = camIoWrapper.getState()
        audioManager.processState(iState:camIoWrapper.getStateIdx(), stylusString: txt)
        if txt.contains("Please find next landmark:") && Int64(Date().timeIntervalSince1970*1000.0) - lastSent > 2000 {
            webSocketTask?.send(text: txt)
            lastSent = Int64(Date().timeIntervalSince1970*1000.0)
            print("sent: \(txt)")
        }
    }
    
    override func captureOutput(_ output: AVCaptureOutput, didOutput sampleBuffer: CMSampleBuffer, from connection: AVCaptureConnection) {
//        if self.videoPaused == true {
//            audioManager.stopPlaying()
//            return
//        }
        usleep(10000) //will sleep for 10 milli seconds
        DispatchQueue.main.sync {
            if let camData = CMGetAttachment(sampleBuffer, key:kCMSampleBufferAttachmentKey_CameraIntrinsicMatrix, attachmentModeOut:nil) as? Data {
                let matrix: matrix_float3x3 = camData.withUnsafeBytes { $0.pointee }
                self.camMat3val = "\(matrix[0][0]) \(matrix[2][1]) \(matrix[2][0])"  // image is rotated, so cx,cy is swapped
                camIoWrapper.setCamMat3val(self.camMat3val)
            }
            
            guard let pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer) else {return}
            let ciImage = CIImage(cvPixelBuffer: pixelBuffer)
            guard let cgImage = self.context.createCGImage(ciImage, from: ciImage.extent) else { return }
            let image = UIImage(cgImage: cgImage).imageRotatedByDegrees(degrees:90.0,flip:false)
            self.frm = image
//            if self.saveFrm && !self.loopFrm{
//                if self.imgs.count < 9999{
//                    self.imgs.append(image)
//                }
//            }
//            else if self.loopFrm{
//                self.cntFrm = (self.cntFrm + 1)%self.imgs.count
//                image = self.imgs[self.cntFrm]
//            }
            self.processFrame(image)
        }

        let dt = Int64(Date().timeIntervalSince1970) - last_active_time  // seconds
        if dt > 60{
            self.webSocketTask?.send(text: "ping, dt = \(dt)")
            last_active_time = Int64(Date().timeIntervalSince1970)
            if !self.wsConnected{
                self.webSocketTask =  self.setWebSocket(self.ip_txt,self.port_txt)
            }
        }
    }
    
    func setWebSocket(_ ip:String, _ port:String) -> WebSocketTaskConnection{
        let webSocketTask = WebSocketTaskConnection(url: URL(string: "ws://" + ip + ":" + port)!)
        webSocketTask.delegate = self
        webSocketTask.connect()
        webSocketTask.send(text: "Hello Socket, are you there?")
        webSocketTask.listen()
        return webSocketTask;
    }

//    var ip_txt = "172.20.220.166", port_txt = "8100"
//    var ip_txt = "34.237.62.252", port_txt = "8001"
    var ip_txt = "34.237.62.252", port_txt = "8081"
    var last_active_time = Int64(Date().timeIntervalSince1970)
    var wsConnected = false

    @objc func setWebSocketServer() {
        let model = UIAlertController(title: "WS Server", message: "", preferredStyle: .alert)
        model.addTextField { (textField) in
            textField.placeholder = "host name/ip address"
            textField.textColor = .blue
            textField.text = self.ip_txt
        }
        model.addTextField { (textField) in
            textField.placeholder = "port"
            textField.textColor = .blue
            textField.text = self.port_txt
        }
        let save = UIAlertAction(title: "Save", style: .default) { (alertAction) in
            let host = model.textFields![0] as UITextField
            let port = model.textFields![1] as UITextField
            self.ip_txt = host.text!
            self.port_txt = port.text!
            self.webSocketTask =  self.setWebSocket(self.ip_txt,self.port_txt)
            
        }

        model.addAction(save)
        model.addAction(UIAlertAction(title: "Cancel", style: .default) { (alertAction) in
        })
        
        self.present(model, animated:true, completion: nil)
    }
    
    var webSocketTask:WebSocketTaskConnection?
}

extension ViewController: WebSocketConnectionDelegate {
    func onConnected(connection: WebSocketConnection) {
        print("connected: ", connection)
        last_active_time = Int64(Date().timeIntervalSince1970)
        wsConnected = true
    }
    
    func onDisconnected(connection: WebSocketConnection, error: Error?) {
        print("disconnected")
        wsConnected = false
    }
    
    func onError(connection: WebSocketConnection, error: Error) {
        print("error,...")
        wsConnected = false
    }
    
    func onMessage(connection: WebSocketConnection, text: String) {
        last_active_time = Int64(Date().timeIntervalSince1970)
        print("received text: ", text)
        if text.contains("invert color"){
            camIoWrapper.invert_color()
        }
        if text.contains("image, please"){
            guard let txt = self.frm.base64 else {return}
            webSocketTask?.send(text: txt)
        }
        if text.contains("result image"){
            guard let txt = self.resultImage.base64 else {return}
            webSocketTask?.send(text: txt)
        }
        if text.contains("cam mat & image"){
            guard let txt = self.frm.base64 else {return}
            webSocketTask?.send(text: self.camMat3val + "___cam+img___" + txt)
        }
        if text.contains("New Landmark Data:"){
            camIoWrapper.resetFeatureBase()
            let dat = text.components(separatedBy: "Landmark Data:")[1]
            print("landmark:\n", dat)
            camIoWrapper.setFeatureBaseImagePoints(dat)
            writeTo(fn:file4landmarkData,  dat:dat)
        }
        if text.contains("vol+"){
            self.audioManager.volInc()
        }
        if text.contains("vol-"){
            self.audioManager.volDec()
        }
        if text.contains("skeri: hello"){
            DispatchQueue.main.asyncAfter(deadline: .now()) {
                for _ in 1..<10{
                    self.audioManager.processState(iState:4, stylusString: "James from Smith-Kettle-well says hello")
                }
            }
//            DispatchQueue.main.asyncAfter(deadline: .now() + 5) {
//                for _ in 1..<10{
//                    self.audioManager.processState(iState:4, stylusString: "James from Smith-Kettle-well says hello, again,------, ")
//                }
//            }
//            DispatchQueue.main.asyncAfter(deadline: .now() + 10) {
//                for _ in 1..<10{
//                    self.audioManager.processState(iState:4, stylusString: "James from Smith-Kettle-well says hello, one more time,------, ")
//                }
//            }
        }
        if text.contains("skeri: This is an audio test"){
            DispatchQueue.main.asyncAfter(deadline: .now()) {
                for _ in 1..<100{
                    self.audioManager.processState(iState:4, stylusString: "This is an audio test,------, ")
                }
            }
        }
    }
    
    func onMessage(connection: WebSocketConnection, data: Data) {
//        print("received data: ", data)
    }
}

extension UIImage {
    var base64: String? {
        self.jpegData(compressionQuality: 0.8)?.base64EncodedString()
    }
}

extension String {
    var imageFromBase64: UIImage? {
        guard let imageData = Data(base64Encoded: self, options: .ignoreUnknownCharacters) else {
            return nil
        }
        return UIImage(data: imageData)
    }
}
