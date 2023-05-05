//
//  utils.swift
//  CamIOPlayground
//
//  Created by Huiying Shen on 1/10/22.
//  Copyright © 2022 Huiying Shen. All rights reserved.
//

import AVFoundation

func getDocumentsDirectory() -> URL {
    let paths = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
    return paths[0]
}

extension Date {
   func getFormattedDate(format: String) -> String {
        let dateformat = DateFormatter()
        dateformat.dateFormat = format
        return dateformat.string(from: self)
    }
}

extension UIViewController{
    public static var documentsDirectoryURL: URL {
        return FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
    }
    public static func fileURLInDocumentDirectory(_ fileName: String) -> URL {
        return self.documentsDirectoryURL.appendingPathComponent(fileName)
    }
    
    public static func storeImageToDocumentDirectory(image: UIImage, fileName: String) -> URL? {
        guard let data = image.jpegData(compressionQuality: 0.8) else {
               return nil
        }
        
        let fileURL = self.fileURLInDocumentDirectory(fileName)
        do {
            try data.write(to: fileURL)
            print("saved: "+fileName)
            return fileURL
        } catch {
            return nil
        }
    }
    
    func listDir(){
        let fileManager = FileManager.default
        
        let documentsURL = fileManager.urls(for: .documentDirectory, in: .userDomainMask)[0]
        do {
            let contents = try fileManager.contentsOfDirectory(at: documentsURL, includingPropertiesForKeys: nil)
            for filename in contents {
                print(filename)
            }
        } catch {
            print("Error while enumerating files \(documentsURL.path): \(error.localizedDescription)")
        }
    }
    
    func deleteFile(_ fn2Del:String){
        let fileManager = FileManager.default
        let paths = NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)
        let documentDirectory = paths[0]
        let filePath = documentDirectory.appendingFormat("/" + fn2Del)
        do {try fileManager.removeItem(atPath: filePath)}
        catch let error as NSError {
            print("Error : \(error)")
        }
    }
    
    func writeTo(fn:String,  dat:String){
        if let dir = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first {
            let fileURL = dir.appendingPathComponent(fn)
            //writing
            do {
                try dat.write(to: fileURL, atomically: false, encoding: .utf8)
            }
            catch {/* error handling here */}
        }
    }
    
    func readFr(_ fn:String) -> String{
        var text = ""
        if let dir = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first {
            let fileURL = dir.appendingPathComponent(fn)
            do {
                text = try String(contentsOf: fileURL, encoding: .utf8)
            }
            catch {
                return ""
            }
        }
        return text
    }
    
    func postTo(urlStr: String, route:String, json: [String:String]){
        let session = URLSession.shared
        let url = URL(string: urlStr+route)!
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        request.setValue("Powered by Swift!", forHTTPHeaderField: "X-Powered-By")
        
        let jsonData = try! JSONSerialization.data(withJSONObject: json, options: [])
        let task = session.uploadTask(with: request, from: jsonData) { data, response, error in
            if let data = data, let dataString = String(data: data, encoding: .utf8) {
                print(dataString)
            }
        }
        task.resume()
    }
    
    func getSubstring(str: String, after: Character, upto: Character) -> String{
          guard after != upto else {return str}
        return String(str[str.firstIndex(of: after)! ..< str.firstIndex(of: upto)!].dropFirst())
    }
}

extension UIImage {
    
    public func imageRotatedByDegrees(degrees: CGFloat, flip: Bool) -> UIImage {
        let degreesToRadians: (CGFloat) -> CGFloat = {
            return $0 / 180.0 * CGFloat.pi
        }
        
        // calculate the size of the rotated view's containing box for our drawing space
        let rotatedViewBox = UIView(frame: CGRect(origin: .zero, size: size))
        rotatedViewBox.transform = CGAffineTransform(rotationAngle: degreesToRadians(degrees))
        let rotatedSize = rotatedViewBox.frame.size
        
        // Create the bitmap context
        UIGraphicsBeginImageContext(rotatedSize)
        let bitmap = UIGraphicsGetCurrentContext()
        
        // Move the origin to the middle of the image so we will rotate and scale around the center.
        bitmap?.translateBy(x: rotatedSize.width / 2.0, y: rotatedSize.height / 2.0)
        
        //   // Rotate the image context
        bitmap?.rotate(by: degreesToRadians(degrees))
        
        // Now, draw the rotated/scaled image into the context
        var yFlip = CGFloat(1.0)
        if flip { yFlip = CGFloat(-1.0) }
        
        bitmap?.scaleBy(x: yFlip, y: -1.0)
        let rect = CGRect(x: -size.width / 2, y: -size.height / 2, width: size.width, height: size.height)
        bitmap?.draw(cgImage!, in: rect)
        let newImage = UIGraphicsGetImageFromCurrentImageContext()
        UIGraphicsEndImageContext()
        return newImage!
    }
}
extension UIImage {
    func invertedImage(cgResult: Bool = true) -> UIImage? {
        let coreImage = self.ciImage
        guard let filter = CIFilter(name: "CIColorInvert") else { return nil }
        filter.setValue(coreImage, forKey: kCIInputImageKey)
        guard let result = filter.value(forKey: kCIOutputImageKey) as? UIKit.CIImage else { return nil }
        if cgResult { // I've found that UIImage's that are based on CIImages don't work with a lot of calls properly
            return UIImage(cgImage: CIContext(options: nil).createCGImage(result, from: result.extent)!)
        }
        return UIImage(ciImage: result)
    }
}
extension Character {
    var asciiValue: Int {
        get {
            let s = String(self).unicodeScalars
            return Int(s[s.startIndex].value)
        }
    }
}

extension String {
    func characterAtIndex(index: Int) -> Character? {
        var cur = 0
        for char in self {
            if cur == index {
                return char
            }
            cur+=1
        }
        return nil
    }
}

extension String {
    subscript(_ range: CountableRange<Int>) -> String {
        let idx1 = index(startIndex, offsetBy: max(0, range.lowerBound))
        let idx2 = index(startIndex, offsetBy: min(self.count, range.upperBound))
        return String(self[idx1..<idx2])
    }
}

extension String {
    func convertToValidFileName() -> String {
        let invalidFileNameCharactersRegex = "[^a-zA-Z0-9_]+"
        let fullRange = startIndex..<endIndex
        let validName = replacingOccurrences(of: invalidFileNameCharactersRegex,
                                           with: "-",
                                        options: .regularExpression,
                                          range: fullRange)
        return validName
    }
}

//  "name.name?/!!23$$@1asd".convertToValudFileName()           // "name-name-23-1asd"
//  "!Hello.312,^%-0//\r\r".convertToValidFileName()            // "-Hello-312-0-"
//  "/foo/bar/pop?soda=yes|please".convertToValidFileName()     // "-foo-bar-pop-soda-yes-please"

extension String {
    func index(from: Int) -> Index {
        return self.index(startIndex, offsetBy: from)
    }

    func substring(from: Int) -> String {
        let fromIndex = index(from: from)
        return String(self[fromIndex...])
    }

    func substring(to: Int) -> String {
        let toIndex = index(from: to)
        return String(self[..<toIndex])
    }

    func substring(with r: Range<Int>) -> String {
        let startIndex = index(from: r.lowerBound)
        let endIndex = index(from: r.upperBound)
        return String(self[startIndex..<endIndex])
    }
}
//let str = "Hello, playground"
//print(str.substring(from: 7))         // playground
//print(str.substring(to: 5))           // Hello
//print(str.substring(with: 7..<11))    // play


extension ViewController{
func getZoneIdNameMapping(_ zones: [String]) -> [String] {
    var out = [String]()
    let zoneData = getIdNameMappingZone20220919()
    let zoneLines = zoneData.components(separatedBy: "\n")
    for line in zoneLines{
        let id_nm = line.components(separatedBy: "    ")
        for zn in zones{
            if zn.contains(id_nm[0]){
                print(id_nm[0])
                if id_nm.count == 4{
                    let tmp = zn.replacingOccurrences(of: id_nm[0], with: id_nm[1]+"\t"+id_nm[3])
                    out.append(tmp)
                }
                else {
                    let tmp = zn.replacingOccurrences(of: id_nm[0], with: id_nm[1])
                    out.append(tmp)
                }
                break
            }
        }
    }
    return out
}

}



func string2AudioFile(_ s2write:String, fileName:String, voiceId:String = "com.apple.ttsbundle.Samantha-premium"){
    let synthesizer = AVSpeechSynthesizer()
    let utterance = AVSpeechUtterance(string: s2write)
    utterance.voice = AVSpeechSynthesisVoice(identifier:voiceId)
    var output: AVAudioFile?

    synthesizer.write(utterance) { (buffer: AVAudioBuffer) in
       guard let pcmBuffer = buffer as? AVAudioPCMBuffer else {
          fatalError("unknown buffer type: \(buffer)")
       }
       if pcmBuffer.frameLength == 0 {
         // done
       } else {
         // append buffer to file
         if output == nil {
//             let path = Bundle.main.path(forResource: "hello2tts", ofType: ".caf")!
//             let url = NSURL.fileURL(withPath: path)
             output = try!  AVAudioFile(
             forWriting: URL(fileURLWithPath: fileName),
//                forWriting:url,
//             settings: pcmBuffer.format.settings,
             settings: [AVFormatIDKey: kAudioFormatMPEG4AAC],
             commonFormat: .pcmFormatInt16,
             interleaved: false)
         }
         try! output?.write(from: pcmBuffer)
       }
    }
}
extension ViewController{
    func getIdNameMappingBoth() -> String {
        return getIdNameMapping20220919() + "\n" + getIdNameMappingZone20220919()
    }
    
    func getIdNameMapping20220919() -> String {
        return """
ACCESS_1_SPARSE_043    Ava's Bridge
ACCESS_1_SPARSE_044    Northeast Gated Entrance
ACCESS_1_SPARSE_045    Northwest Gated Entrance            slightly wrong location in James's pdf
BENCH_1_SPARSE_035    Bench in Tot Zone
BENCH_2_SPARSE_083    Bench in Spinning Zone            Each of the benches inside the zones are places to sit and enjoy the sounds of the playground.
BENCH_3_SPARSE_042    Bench Number 1 in Swinging and Swaying Zone
BENCH_4_SPARSE_006    Bench Number 2 in Swinging and Swaying Zone
BENCH_5_SPARSE_058    Bench Number 3 in Swinging and Swaying Zone
BENCH_6_SPARSE_084    Bench in Music Zone
BICYCLE_SPARSE_040    Exercise Bike
CLIMBING_LOOPS_SPARSE_002    Climbing Loops on Slide Mound        Climbing Loops offer a fun way to get to the top of the slide mound with secure hand holds for support. 13 large metal loops are mounted at various places up in incline of the slide mound just due west of the slide mound summit. For people with VI is is recommended to use a cane to sense the location of the climbing loops when getting to the top.
DISK_SWINGS_SPARSE_014    Disk Swings        These disc swings are shaped like a large saucer, with rubber edges for one or more to swing togher. Made of hard plastic, and a height for a wheelchair user to get on too. There are three swings here that are attached seperately with plenty of distance between them for safety, but do watch your heads!
DISK_THING_SPARSE_081    Disk Spinner        The disk spinner may seem like a basic spinner, but of all the spinners at Magical Bridge it can offer the most thrilling and challenging spinning experience. The disk spinner almost looks like an enormous thumbtack that isn’t put in the ground perfectly straight. The disk is around 5 feet in diameter. On one side it is 1 foot off the ground and on the highest side it is 3 feet off the ground. So not only will you rotate when you are on the disk spinner, but you will go up and down. Users can sit or lay on the disk, or for a more challenging adventure, stand on the disk and rotate it by walking. The disk spinner is 12 inches high on one side, which is a bit high to transfer from a wheelchair so it would be good to have some assistance to do it. Even those who can’t move much or at all could be lifted and placed on the disk spinner for an enjoyable spin ride.
DRINKING_FOUNTAIN_SPARSE_064    Drinking Fountain        This Drinking Fountain has a higher and lower fountain, as well as a bottle filler
ELIPTICAL_THING_SPARSE_046    Walking Machine     Side of the Swing Zone    The Free Runner is designed for those over 13 and able-bodied guests as it is not wheelchair accessible. This machine exercises the lower body with an even, low-impact resistance machine like running.
LASER_HARP_SPARSE_001    Laser Harp    Music zone    The Magical Harp, created by artist Jen Lewin, is a permanent, outdoor motion sensitive instrument . Though the motion sensors are high overhead and out of reach, they detect movements through the 24 laser hards directly under the arch. Much like plucking the string of a harp, passing through the beams triggers custom circuitry and sensors to produce musical notes tuned to the panatonic scale and soothing to many listeners.For more information on this remarkable and innovative installation, please visit CODA Works, where the Magical Harp won a 2016 CODA Award.
MINI_ROLLER_THING_SPARSE_068    Roller Table    Swing Zone    This roller table is designed for those in a wheelchair to transfer onto, and lay flat to make use of the four loop metal bars that are up and down the length of the table. Designed to provide those with limited mobility some sensory benefits for their back, and strengh building for their arms, everyone can enjoy it. Be mindful of hitting your head on those large loops when laying onto the table!
MINISLIDEMOUND_CLIMBING_LOOPS_SPARSE_018    Climbing Loops in Tot Zone        Three large climbing loops are placed strategically on the small tot-friendly slide hill inside this zone to help younger guests with climbing to the top.
MINISLIDEMOUND_GIRAFF_SPARSE_030    Climbing Giraffe    Tot zone    The Climbing Giraffe is a small structure perfect for toddlers. It has tactile elements, a short climbing net and bench designed to be climbed on and under. The structure looks like a Giraffe with its neck curved to the ground.
MINISLIDEMOUND_HARP_SPARSE_015    Kinder Bells    Tot zone        No description available
MINISLIDEMOUND_HORSE_SPARSE_085    Rocking Horse    Tot zone    A very short rocking horse for toddlers, but be careful and use your cane as the rocking horse is about 2 feet high.
MINISLIDEMOUND_SLIDE_SPARSE_020    Toddler Slide    Tot zone        Not sure what this is or what it's called
MINISLIDEMOUND_VERTICAL_PIPES_SPARSE_052    Climbing Poles    Tot zone        Not sure what this is or what it's called
PICNIC_TABLE_1_SPARSE_012    Round Table            Not sure what it's called -- it has a cChimeshess board on it
PICNIC_TABLE_2_SPARSE_080    Round Table            Not sure what it's called -- it has a chess board on it
PICNIC_TABLE_3_SPARSE_067    Round Table            Not sure what it's called -- it has a chess board on it
PILASTER_1_SPARSE_039    Picnic and Performance Area Plinth
PILASTER_2_SPARSE_022    Tot Zone Plinth
PILASTER_3_SPARSE_003    Spinning Zone Plinth
PILASTER_4_SPARSE_072    Swinging and Swaying Zone Plinth
PILASTER_5_SPARSE_053    Slide Mound Plinth
PILASTER_6_SPARSE_028    Music Zone Plinth
PILASTER_7_SPARSE_016    Playhouse and Tree Deck Plinth
PLANTER_SPARSE_078    Oval Bench
PLAYHOUSE_SPARSE_059    Playhouse        The majestic wood playhouse is at the center of this space and features two levels of fun and artisitic beauty for all to enjoy. Designed and built by Barbara Butler and team, it is multi-colored and offers two small round tables and bench seating upstairs to resemble a cafe. There are also windows with metal safety bars which enable visitors to hear activities below and have a view of the whole space. On the ground level of the playhouse is a small semi-circluar stage at the front. A 6 inch bevelled barrier at the edge prevents wheelchairs from rolling off but use your cane to make sure it's not a tripping hazard. Facing the stage to the left, is a fun pretend "magical bakery" where cakes and a cash register out of wood invites imagination. To the right of that is a wood working bench with tools, also made out of wood, which invite more pretend play from all. Both levels of the playhouse have entries that are wheelchair friendly and clearings tall enough for adults of all sizes to enjoy!
PLAYHOUSE_RAMP_SPARSE_041    Playhouse Ramp        All playhouse ramps are wheelchair friendly. The lower level has 2 ramps on either side of the playhouse.
PLAYHOUSE_RAMP_1_SPARSE_025    Playhouse Ramp
PLAYHOUSE_RAMP_3_SPARSE_007    Playhouse Ramp
PLAYHOUSE_SEATS_SPARSE_073    Playhouse Stage Audience Benches
PLAYHOUSE_STAGE_SPARSE_055    Playhouse Stage        The playhouse stage is a cozy semi circle which has a 6 inch beveled edge around the front, to prevent wheelchairs from rolling off. Be careful not to trip on this when exploring.
POD_1_SPARSE_060    Cozy Cocoon in Spinning Zone        The Cozy Cocoon is a made of plastic and a place for kids and adults, especially those with autism and sensory challenges, to hang out in when active play feels overwhelming and frenetic. The Cozy Cocoon in the Spin Zone is one of three located throughout the playground. A Cozy Cocoon is shaped like a large sphere with openings. There is one large opening in the front with room for one person to sit down in the cocoon, and there are smaller openings in the side and back of each cocoon to use as a window to peer out at the playground. The cocoon can be gently rotated so that the user or a helper can rotate it away from the action and noise. A cocoon's main purpose is to be a calming, cozy place to regroup while observing from the round openings in the cocoon and to re-emerge and re-engage in active play when ready.
POD_2_SPARSE_045    Cozy Cocoon in Music Zone        The Cozy Cocoon is a place for kids and adults, especially those with autism and sensory challenges, to hang out in when active play feels overwhelming and frenetic. The Cozy Cocoon in the Spin Zone is one of three located throughout the playground. A Cozy Cocoon is shaped like a large sphere with openings. There is one large opening in the front with room for one person to sit down in the cocoon, and there are smaller openings in the side and back of each cocoon to use as a window to peer out at the playground. The cocoon can be gently rotated so that the user or a helper can rotate it away from the action and noise. A cocoon's main purpose is to be a calming, cozy place to regroup while observing from the round openings in the cocoon and to re-emerge and re-engage in active play when ready.
POD_3_SPARSE_024    Cozy Cocoon in Swinging and Swaying Zone        The Cozy Cocoon is a place for kids and adults, especially those with autism and sensory challenges, to hang out in when active play feels overwhelming and frenetic. The Cozy Cocoon in the Spin Zone is one of three located throughout the playground. A Cozy Cocoon is shaped like a large sphere with openings. There is one large opening in the front with room for one person to sit down in the cocoon, and there are smaller openings in the side and back of each cocoon to use as a window to peer out at the playground. The cocoon can be gently rotated so that the user or a helper can rotate it away from the action and noise. A cocoon's main purpose is to be a calming, cozy place to regroup while observing from the round openings in the cocoon and to re-emerge and re-engage in active play when ready.
RAMP_ELEVATED_WALKWAY_SPARSE_008    Elevated Walkway Behind Playhouse
RAMP_LONG_SPARSE_075    Long Ramp
RAMP_SIDEWALK_SPARSE_054    Sidewalk to Main Bridge        This entry is a gentle slope that offers easy acess to the top of the playhouse and into the playground in general. The path is made of wood with either side having secure rope netting and a stability bar for wheelchair users.
RAMP_TREEHOUSE_DECK_SPARSE_033    Tree Deck
RING_THING_SPARSE_066    Nest Spinner        The Nest Spinner is a relaxing spin ride where one or two people curl up safely into the nest and spin slowly and rhythmically making it a good introduction to spinning for those who would find it challenging. The structure of the Nest Spinner looks like two large rings that are both crossed in space and connected together (be careful not to bump your head on the rings). The base of the two rings has a cupped area which is big enough for two people to sit and a constant push on the sides of the rings to cause the nest to rotate.
SHAKING_BENCH_SPARSE_026    Sway Fun        Smooth and carefully designed with ramped access to the Sway Fun allows those unable to leave their mobility device the ability to enjoy gentle swaying with no need to transfer. Users can rock back and forth on the deck of the Sway Fun to get it moving, or a person stand on the outside of the Sway Fun and push it to get the equipment in motion. If you decide to not lock the wheels of your mobility device when riding, be sure to hold onto the handholds on the table on the deck of the Sway Fun. The sway Fun is in the shape of a boat.
SITTING_WALL_1_SPARSE_005    Sitting Wall        Each of Magical Bridge play zones are created with concrete seating walls which are 4 feel high and not only help define and enclose the spaces, but also provide plenty of seating.
SITTING_WALL_2_SPARSE_031    Sitting Wall
SITTING_WALL_3_SPARSE_017    Sitting Wall
SITTING_WALL_4_SPARSE_061    Sitting Wall
SITTING_WALL_5_SPARSE_047    Sitting Wall
SLIDE_SLIDEMOUND_ROLLER_SLIDE_SPARSE_048    Roller Slide        The roller slide is a series of horizontal metal cylinders that rotate underneath the rider to provide a bumpy and stimulating experience. The slide goes down at a moderate incline, flattens out, and then goes down another incline. The bottom platform, also known as the Dignity Landing, gives people that need the assistance of a helper a safe place to wait at the end of the slide without blocking others.
SLIDEMOUND_BRIDGE_SPARSE_036    Elevated Walkway to Slide Mound        This entry is a gentle sloped path made of wood wide enough for 2 wheelchairs to pass if need be. There is rope mesh on either side for safety, as well as a safety bar for extra assistance a wheelchair user may need. The path slopes up towards a fat area on the top, where there is a choice to get to the top of the slide mound. The choice on the left side is a moving sway bridge which is wheelchair accessible and to the right is a traditional hardscaped path. Both options are part of the wood ramp.
SLIDEMOUND_CURVED_SLIDE_SPARSE_010    Curved Slide        This classic curved slide is made of plastic and has higher curved sides for added security and offers a slower sliding experience.    Unsure of name
SLIDEMOUND_GROUP_SLIDE_SPARSE_079    Group Slide        The Group Slide enables two or three people to slide down simultaneously, perfect for those needing additional assistance or just the added fun of sliding with your friend next to you! The slide is metal so be careful on hot days. This slide is the fastest and most slippery slide at the playground.
SLIDEMOUND_INCLINE_SPARSE_021    Shallow Turf Hill
SLIDEMOUND_NET_SPARSE_050    Climbing Net on Slide Mound
SLIDEMOUND_SLIDE_RAILS_SPARSE_032    Parallel Bar Slide        These two metal bars are installed on the slide mound and offer the option to use them like a slide experience.
SLIDEMOUND_STAIRS_SPARSE_065    Stairs on Slide Mound        A staircase of 15 stairs is provided to the south of the summit of the slide mound. Caution, the end of the railing at the base of the stairs protrudes into the walking path. The railing on the stairs has two heights. A narrow fence opening is at the top of the slide that prevents mobility devices from accidentally rolling down the stairs.
SLIDEMOUND_UMBRELA_SPARSE_038    Umbrella Pole        Umbrella Pole on top of slide mound
SPINNING_CONE_SPARSE_076    Net Spinner        The Net Spinner is a combined climbing experience and a spinning experience. The structure of the Net Spinner is a large cone shaped net. The net is anchored at the bottom to a large horizontal disk about 8 feet in diameter and about 1 foot from the ground. Coming up from the center of the disk is pole that is 12 feet. A network of ropes is attached from the top of the pole to the outside edge of the disks to form a net that can be climbed on. A person standing next to the net spinner can push on the ropes in a circular direction causing the entire net spinner to rotate. Many people can climb on the net or sit safely on the disk inside at once. Not only is the net spinner a climbing challenge but you will get some good stimulus to your balance (vestibular) system if someone rotates the net spinning while you are at the top. Keep in mind the whole thing can spin as you are at a height of 8 or more feet off the ground. How do you know if you are ready to climb to the top of the net spinner? People should be comfortable with climbing short distances up and down the net before going to the top.
SPINNING_TABLE_SPARSE_057    Ground Carousel        The Ground Carousel is flush with the ground so everyone can ride. For those who are blind or with other challenging visual impairments, approach the ground carousel with caution. The ground carousel has metal poles that can come around very fast and sometimes people spin with their legs and arms extended out from the carousel. So best if you are visually impaired to ask folks to stop the carousel before approaching it. The carousel has two benches for up to 3 people each to sit on while riding the carousel and also has two magical parking spaces for wheelchairs. To transfer a wheelchair onto the ground carousel simply lift the safety rail, roll into the space provided and park the wheelchair. Set the safety break on the wheelchair, then bring the safety rail down. If you have the arm strength to do it, you can “turn” the disk in the middle of the spinner which will cause the whole carousel to rotate.
STEPPING_SOUNDS_SPARSE_037    Stepping Sounds        The stepping sounds used to provide sounds when a visitor made their way towards one of the main playground entries but, sadly, the city has disabled the sounds.
SWING_BRIDGE_SPARSE_011    Bouncy Bridge        The bouncy bridge is one of two ways to get to the top of the slide hill, where there are 4 unique slides. The bouncy bridge is on the left of the ramp and fully wheelchair accessible.
SWINGS_SPARSE_082    Bucket Swings        The bucket swings are supportive, allowing for use by all abilities, and promote collaborative swinging. The bucket swings have a rigid plastic harness with a metal latching mechanism to keep the rider secure when swinging. The harness hinges are located near the rider shoulders and the whole harness rotates overhead. Be careful to not hit your head on the harness when getting in and out of the bucket swings.
TREE_1_SPARSE_062    Tree
TREE_2_SPARSE_027    Tree
TREE_3_SPARSE_019    Tree
TREE_4_SPARSE_004    Tree
TREE_5_SPARSE_069    Tree
YOU_ARE_HERE_SPARSE_056    You are Here at the Magic Map
CREEK_SPARSE_071    Adobe Creek Wall
"""
    }
    
    func getIdNameMappingZone20220919() -> String {
            return
"""
ZONE_61_SLIDEMOUND_.035    Slide Mound Zone        The slide mound is a large hill with artificial grass and rubber surfacing for ease of walking up. There are 4 slides which start at the top and each slide enables the transfer of a wheelchair user as well.
ZONE_62_TOTZONE_.036    Tot Zone        The tot zone is the first area on the left when entering through the Peery Plaza gate. It offers the smallest of visitors "tot" experiences of play.
ZONE_66_SPINNING_.040    Spinning Zone        The spinning zone is the first are on the right that provides 4 experiences for spinning movement. Immediately to the right is a cozy cocoon ball for some quieter time where one can tranfer out of a wheelchair or just crawl in and enjoy the partially enclosed plastic noise cancellation. Moving around the space, about 20 feet from to the cozy coccon closest to the perimeter fence, is the ground carrousel. This is a ground spinner with two large metal bar handles that slide upwards to allow wheelchair users to roll on. Once inside the spinner, they pull the handles down and mind their head while doing so. There is a round steering wheel in the center that controls the spinning and there is also bench seating for others to spin too.  20 feet from this, is a piece of equipment that looks like a 10 foot in diameter dish installed a bit off center to enable easier spinning. One can sit, lay or stand on this dish. Further down in the zone is a 13 foot high net spinner with a wheelchair-transferrable round flatform. At least 20 can hop or slide onto this item to experience spinning which needs to be activivated by people pushing it around. Finally, is a cozy open seat which is called the "nest" and it enable one person to get in and hang out or have someone spin them around. This has two metal bars that cross over erach other about 8 feet from the seated area so mind your heads when getting in!
ZONE_65_PLAYHOUSE_TREE_DECK_.039    Playhouse and Tree Deck Zone
ZONE_60_PICNIC_PERFORMANCE_.034    Picnic and Performance Zone        The picnic area has 3 round tables with checker patterns on the top for those who wish to play (with their own checker pieces). There are three benches for seating adjacent to the table and one opening where a wheelchair user can slide in. Each table can seat about 4-5 comfortably with one wheelchair user.
ZONE_63_SWINGING_SWAYING_.037    Swinging and Swaying Zone        At the entry of this zone to the left, is a gentle and short ramp that goes to a piece of wheelchair friendly play equipment called "sway fun." It enables 2 wheelchair users to roll on, while others can sit on either side of a built in bench and rock the whole unit back and forth. There is not much movement. About 30 feet straight from there are 6 accessible bucket seat swings with a hard plastic harness for safety. Adults are encouraged! To the left of this bay of swings, is another seperate swinging experience with 3 bays of large disc shapped swings. Easy to transfer onto frow a wheelchair seat, these discs can hold one of up to several swingers. In all zones with movement, please mind your steps!
ZONE_64_MUSIC_.038    Music Zone
"""
    }
}
