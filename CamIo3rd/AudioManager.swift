//
//  AudioMnanger.swift
//  CamIo3rd
//
//  Created by Huiying Shen on 7/30/19.
//  Copyright © 2019 Huiying Shen. All rights reserved.
//

import AVFoundation
import UIKit



class AudioManager: NSObject, AVAudioRecorderDelegate,AVAudioPlayerDelegate,AVSpeechSynthesizerDelegate {
    
    var audioSession: AVAudioSession!
    var audioRecorder: AVAudioRecorder?
    var audioPlayer: AVAudioPlayer!
    let synth = AVSpeechSynthesizer()
    
    let engine = AVAudioEngine()
    var playerNode = AVAudioPlayerNode()
    
    var bufLow:AVAudioPCMBuffer!
    var bufHigh:AVAudioPCMBuffer!
    
    var hasPermission = false
    
    override init() {
        super.init()
        synth.delegate = self
//        getMicPermission()
        playerNode.volume = 1.0
        engine.attach(playerNode)
        loadBuffers()
        playBuf(player:playerNode, buf: bufLow)  // needed to prepare engine
        engine.prepare()
        try! engine.start()
    }
    
    func getVolume() -> Float{
        let vol = AVAudioSession.sharedInstance().outputVolume
        return vol
    }
    
    func loadBuffers(){
        bufLow = prepareBuffer(frequency: 440,timeSec: 0.1)
        bufHigh = prepareBuffer(frequency: 440*1.25,timeSec: 0.1)
    }

    var obj_bufs = [String:AVAudioPCMBuffer]()

    func try_load_mp3(_ name: String, path: String) -> Bool{
        obj_bufs[name] = file2Buf(path, suff:".mp3")
        let b = obj_bufs[name] != nil
        if !b {
            print("try_load_mp3(), not loaded, name, rt = \(name), \(path)")
        }
        return b
    }
    
    func show_obj_bufs(){
        print("show_obj_bufs()")
        for buf in obj_bufs{
            print("    key = \(buf.key)")
        }
    }
    
    var cur_obj = ""
    func play_obj(_ obj:String){
        guard let buf = obj_bufs[obj] else {return}
        cur_obj = obj
        playBuf(player:playerNode, buf: buf)
        playerNode.play()
    }
    var nPlay = 0
    func complete(){
        print("done playing")
        nPlay += 1
    }
    var vol:Float = 1.0
    func play_complete(){
        done_playing = true
//        print("play_complete()!!!")
    }
    func playBuf(player: AVAudioPlayerNode, buf: AVAudioPCMBuffer,completionHandler: AVAudioNodeCompletionHandler? = nil){
//        print("in playBuf()")
        player.volume = vol
        engine.reset()
        player.stop()
        engine.connect(player,  to: engine.mainMixerNode, format: buf.format)
        player.scheduleBuffer(buf, at: nil, options: AVAudioPlayerNodeBufferOptions.interruptsAtLoop, completionHandler: completionHandler)
        print("AVAudioSession.sharedInstance().outputVolume = \(AVAudioSession.sharedInstance().outputVolume)")
        print("player.volume = \(player.volume)")
    }
    func volInc(){
        vol += 0.25
        if vol > 1{ vol = 1}
    }
    func volDec(){
        vol -= 0.25
        if vol < 0.01 { vol = 0.01}
    }
    

    func file2Buf(_ fn: String, suff: String) -> AVAudioPCMBuffer? {
        guard let path = Bundle.main.path(forResource: fn, ofType: suff) else { return nil}
        let url = NSURL.fileURL(withPath: path)
        let file = try? AVAudioFile(forReading: url)
        let buffer = AVAudioPCMBuffer(pcmFormat: file!.processingFormat, frameCapacity: AVAudioFrameCount(file!.length))!
        try! file!.read(into: buffer)
        return buffer
    }
    
    func currentTimeInMilliSeconds() -> Int64
    {
        return Int64(Date().timeIntervalSince1970 * 1000)
    }
    
    var _str = ""
    var _timestamp = Date().currentTimeMillis()


    var n_played = 0
    let delimiter = ",------,"
    var s2say_array = [" ",]
    func speak(_ s2say: String){
//        print("playerNode.isPlaying, done_playing, n_played = \(playerNode.isPlaying), \(done_playing), \(n_played)")
//        if (synth.isSpeaking || playerNode.isPlaying) && s2say == _str { return }
        if s2say == _str && (synth.isSpeaking || !done_playing) { return } // do not interrupt yourself
//        print("not busy,... _str, s2say = \(_str),  \(s2say)")
        var min_msec_to_repeat = 10000
        if s2say.contains("stylus straight upright") { min_msec_to_repeat = 2000 }
        let dt = Date().currentTimeMillis() - _timestamp
          // reset when last played long time ago
        if s2say != _str {
            n_played = 0}             // reset when s2say is new
        else if dt > min_msec_to_repeat {
            n_played = 0
            _str = ""
        }
        else {n_played += 1}
        if n_played >= 2 { return }     // just played name & description
        print("min_msec_to_repeat, dt = \(min_msec_to_repeat), \(dt)")
        print("speaking:...n_played = \(n_played),  s2say = \(s2say)")
        _str = s2say
        _timestamp = Date().currentTimeMillis()
        synth.stopSpeaking(at: .immediate)  // just in case???
        playerNode.stop()                   // just in case???
        s2say_array = s2say.components(separatedBy: delimiter)
        if s2say_array.count == 0 {s2say_array = [s2say,]}
        if n_played == 0 {speak_name()}
        else if n_played == 1  {speak_des()}
    }
    
    func speak_name(){
        for t in obj_bufs{
            if t.key == s2say_array[0] {
                // found audio buffer
                print("playing \(t.key)")
                playBuf(player:playerNode, buf: t.value,completionHandler:play_complete)
                playerNode.play()
                done_playing = false
                return
            }
        }
        // no audio buffer found: use tts
        print("synthSpeak, s2say_array[0] = \(s2say_array[0])")
        synthSpeak(s2say_array[0])
    }
    
    func speak_des(){
        for t in obj_bufs{
            if t.key == s2say_array[0] + " des" {
                // found audio buffer
                print("playing \(t.key)")
                playBuf(player:playerNode, buf: t.value,completionHandler:play_complete)
                playerNode.play()
                done_playing = false
                return
            }
        }
        // no audio buffer found: use tts
        if s2say_array.count<2 {return}

//        synthSpeak(s2say_array[1])
    }
    

    func synthSpeak(_ s2say:String){
        let utt = AVSpeechUtterance(string: s2say)
        utt.voice = AVSpeechSynthesisVoice(identifier: "com.apple.ttsbundle.Samantha-premium")
        utt.volume = vol
        print("utt.volume = \(utt.volume)")
        synth.speak(utt)
    }
    
    var buf = RingBufStateString(20)
    func processState(iState:Int32, stylusString: String){
        switch iState{
            case 0: buf.add("0")
            case 1: buf.add("1")
//            case 5: buf.add("5")
            case 4,5: buf.add(stylusString)
            default: buf.add("?")
        }
        if iState==5{
//            print("iState==5, stylusString = \(stylusString)")
        }
        buf.getDict(nTotal: 7)
        let (s1,_) = buf.getMode()  // newer state string
        buf.getDict()
        let (s2,_) = buf.getMode()  // older (could be the same as newer) state string
        if (s1 != s2 && s1.count > 2) {
//            print("iState==5, s1 = \(s1)")
            speak(s1)
        } else if (s2.count > 2){
            speak(s2)
        } else if s2 == "5"{
            //print("stylus visible, but not on an obj/zone.  do nothing")
        }
        else if s2 == "1"{  // stylus is invisible, stop talking
//            print("stylus is invisible, stop talking")
            if audioPlayer != nil {audioPlayer.stop()}
            if synth.isSpeaking || playerNode.isPlaying{
                synth.stopSpeaking(at: .immediate)
                playerNode.stop()
//                synthStopped = true
            }
//            else   print("synthStopped = \(synthStopped)")
        }else if s2 == "0" {
            if noAudio()  {playCrickets(vol: 0.0001)}
        }
    }
    
    func noAudio() -> Bool {
        if audioPlayer != nil && audioPlayer.isPlaying{return false}
        if synth.isSpeaking {return false}
        return true
    }
    


    
    func playUrl(_ url: URL, vol: Float, nLoop: Int = -1){
        if  preparePlayer(url,vol:vol,nLoop:nLoop) { audioPlayer.play()}
    }
    
    func preparePlayer(_ url: URL, vol: Float, nLoop: Int = -1) -> Bool {
        do {audioPlayer = try AVAudioPlayer(contentsOf: url)}
        catch { return false }
        audioPlayer.delegate = self
        audioPlayer.setVolume(vol, fadeDuration: 0.5)
        audioPlayer.enableRate = true
        audioPlayer.numberOfLoops = nLoop
        return true
    }
    
    func playResourceFile(_ fn: String, vol: Float,nLoop: Int = 5){
        let url = URL(fileURLWithPath: Bundle.main.path(forResource: fn, ofType:nil)!)
        playUrl(url,vol:vol,nLoop:nLoop)
    }
    
    func stopPlaying(){
        playSilence()  // silence audioPlayer instantly
        if audioPlayer != nil {audioPlayer.stop()}
    }

    var isBeeping = false
    
    func playBeep0(){
        playResourceFile("censor-beep-01.mp3",vol:0.25,nLoop: 1)
    }
    func playSingleClick(){
        playResourceFile("single click.mp3",vol:0.25,nLoop: 1)
    }
    
    func playDoubleClick(){
        playResourceFile("double click.mp3",vol:0.25,nLoop: 1)
    }
    
    func playCrickets(vol: Float = 0.01){
        playResourceFile("crickets3.mp3",vol:vol,nLoop: 0)
    }
    func playSilence(){
        playResourceFile("silence.wav",vol:0.1,nLoop:0)
    }

    var newRegionXyz = ""
    var done_playing = true
    func audioPlayerDidFinishPlaying(_ player: AVAudioPlayer, successfully flag: Bool) {
        done_playing = true
        print("set done_playing = true")
        if isBeeping {isBeeping = false}
    }
    
    func speechSynthesizer(_ synthesizer: AVSpeechSynthesizer, didFinish utterance: AVSpeechUtterance){
        done_playing = true
    }
    func speechSynthesizer(_ synthesizer: AVSpeechSynthesizer, didCancel utterance: AVSpeechUtterance){
        done_playing = true
    }
}

func prepareBuffer(frequency: Double, timeSec: Double, sampleRate: Double = 22_100.0, volume: Double = 0.5) -> AVAudioPCMBuffer {
    
    let bufferCapacity: AVAudioFrameCount = AVAudioFrameCount(Int(timeSec*sampleRate))
    
    let audioFormat = AVAudioFormat(standardFormatWithSampleRate: sampleRate, channels: 1)
    let buffer = AVAudioPCMBuffer(pcmFormat: audioFormat!, frameCapacity: bufferCapacity)!
    
    let data = buffer.floatChannelData?[0]
    let numberFrames = buffer.frameCapacity
    
    for t in 0..<Int(numberFrames) {
        data?[t] = Float32(volume*sin(2.0 * .pi * frequency / sampleRate*Double(t)))
    }
    
    buffer.frameLength = numberFrames
    
    return buffer
}


extension Date {
    func currentTimeMillis() -> Int64 {
        return Int64(self.timeIntervalSince1970 * 1000)
    }
}

extension String{
    func toFloat(default val:Float) -> Float{
        if Float(self) == nil {return val}
        return Float(self)!
    }
}



struct RingBufStateString{
    var idx:Int = 0
    var array = [String]()
    var dict = [String:Int]()
    init(_ sz:Int = 30){
        while idx<sz{
            array.append(" ")
            idx += 1
        }
        idx = 0
    }
    mutating func add(_ s:String){
        array[idx] = s
        idx -= 1  //need to go backward, so getDict(nTotal: Int) will get newest first
        idx = (idx + array.count)%array.count
    }
    mutating func getDict(nTotal: Int) {
        let n = min(nTotal,array.count)
        dict = [String:Int]()
        for k in idx...idx + n - 1{
            let i = array[k%array.count]
            if dict[i] == nil{ dict[i] = 1}
            else {dict[i]! += 1}
        }
    }
    mutating func getDict() {
        getDict(nTotal: array.count)
    }
    
    func getMode() -> (String,Int){
        var out = ("",-1)
        for i in dict{
            if out.1 < i.value{
                out = (i.key,i.value)
            }
        }
        return out
    }
    static func test0(){
        let data1 = """
                    0
                    0
                    15
                    15
                    0
                    15
                    15
                    blah
                    blah
                    blah
                    blah
                    blah
                    blah
                    """
        let array = data1.description.components(separatedBy:"\n")
        var buf = RingBufStateString(10)
        for s in array{
            buf.add(s)
        }
        buf.getDict(nTotal: array.count-2)
        print(buf.dict)
        print("mode = \(buf.getMode())")
        print("done")
    }
    
    
//    func getMode() -> (Int,State){
//
//    }
}


