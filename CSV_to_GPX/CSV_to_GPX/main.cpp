//
//  main.cpp
//  NewGPSToGPX
//
//  Created by Bergþór on 27.12.2014.
//  Copyright (c) 2014 Bergþór Þrastarson. All rights reserved.
//

#include <iostream>
#include <vector>

#include <fstream>

using namespace std;

struct GPSPoint {
    string time;
    double latitude;
    double longitude;
    double altitude;
    double temperature;
    
    bool equals (GPSPoint that) {
        return this->latitude  == that.latitude &&
        this->longitude == that.longitude;
        
    }
};

GPSPoint parseLine(string line) {
    GPSPoint p;
    size_t nextComma = 0, pointer = 0;
    string delimiter = ";";
    
    nextComma = line.find(delimiter, pointer);
    p.time = line.substr(pointer, nextComma-pointer);
    pointer = nextComma + 1;
    
    nextComma = line.find(delimiter, pointer);
    p.latitude = atof(line.substr(pointer, nextComma-pointer).c_str());
    pointer = nextComma + 1;
    
    nextComma = line.find(delimiter, pointer);
    p.longitude = atof(line.substr(pointer, nextComma-pointer).c_str());
    pointer = nextComma + 1;
    
    nextComma = line.find(delimiter, pointer);
    p.altitude = atof(line.substr(pointer, nextComma-pointer).c_str());
    pointer = nextComma + 1;
    
    nextComma = line.find(delimiter, pointer);
    cout << line[pointer] << endl;
    if (line[pointer] == 'N') {
        
        p.temperature = numeric_limits<double>::min();
    } else {
        p.temperature = atof(line.substr(pointer, nextComma-pointer).c_str());
    }
    
    pointer = nextComma + 1;
    
    return p;
}

void logHeader(ofstream &f) {
    f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><gpx version=\"1.1\" creator=\"Garmin Connect\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd http://www.garmin.com/xmlschemas/TrackPointExtension/v1 http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd\" xmlns=\"http://www.topografix.com/GPX/1/1\" xmlns:gpxtpx=\"http://www.garmin.com/xmlschemas/TrackPointExtension/v1\" xmlns:gpxx=\"http://www.garmin.com/xmlschemas/GpxExtensions/v3\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"><trk> <name>RH508</name><trkseg>";
}

void logToFile(GPSPoint p, ofstream &f) {
    if (p.temperature == numeric_limits<double>::min()) {
        f << "<trkpt lon=\""<< p.longitude << "\" lat=\"" << p.latitude << "\"><ele>" << p.altitude << "</ele><time>" << p.time << "</time></trkpt>";
    } else {
        f << "<trkpt lon=\""<< p.longitude << "\" lat=\"" << p.latitude << "\"><ele>" << p.altitude << "</ele><time>" << p.time << "</time><extensions><gpxtpx:TrackPointExtension><gpxtpx:atemp>" << p.temperature << "</gpxtpx:atemp></gpxtpx:TrackPointExtension></extensions></trkpt>";
    }
    
}

void logFooter(ofstream &f) {
    f << "</trkseg></trk></gpx>";
}

int main(int argc, const char * argv[]) {
    freopen("/Volumes/UNTITLED/LOG0053.CSV", "r", stdin);
    string line;
    GPSPoint now, old;
    ofstream gpxFile;
    
    gpxFile.setf(ios::fixed);
    gpxFile.precision(14);
    gpxFile.open("HELLOTHERE4.gpx");
    
    logHeader(gpxFile);
    while (cin.good()) {
        getline(cin, line);
        now = parseLine(line);
        if ((!now.equals(old) && line != "") && (now.time != old.time)) {
            logToFile(now, gpxFile);
        }
        old = now;
    }
    logFooter(gpxFile);
    
    return 0;
}